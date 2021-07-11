#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <physfs.h>

#include "alice/dtable.h"

typedef struct alice_DTableScanner {
	const char* start;
	const char* current;
	u32 line;
} alice_DTableScanner;

typedef enum alice_DTableTokenType {
	ALICE_DTT_LEFT_CURL,
	ALICE_DTT_RIGHT_CURL,
	ALICE_DTT_SEMICOLON,
	ALICE_DTT_QUOTE,
	ALICE_DTT_NUMBER,
	ALICE_DTT_IDENTIFIER,
	ALICE_DTT_ERROR,
	ALICE_DTT_EOF,
	ALICE_DTT_STRING,
	ALICE_DTT_TRUE,
	ALICE_DTT_FALSE
} alice_DTableTokenType;

typedef struct alice_DTableToken {
	alice_DTableTokenType type;
	const char* start;
	u32 length;
	u32 line;
} alice_DTableToken;

alice_DTableScanner scanner;

static void alice_init_dtable_scanner(const char* source) {
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}

static bool alice_dtable_scanner_is_at_end() {
	return *scanner.current == '\0';
}

static alice_DTableToken alice_dtable_scanner_new_token(alice_DTableTokenType type) {
	return (alice_DTableToken) {
		.type = type,
			.start = scanner.start,
			.length = (u32)(scanner.current - scanner.start),
			.line = scanner.line
	};
}

static alice_DTableToken alice_dtable_scanner_error_token(const char* message) {
	return (alice_DTableToken) {
		.type = ALICE_DTT_ERROR,
			.start = message,
			.length = strlen(message),
			.line = scanner.line
	};
}

static char alice_dtable_scanner_advance() {
	scanner.current++;
	return scanner.current[-1];
}

static char alice_dtable_scanner_peek() {
	return *scanner.current;
}

static char alice_dtable_scanner_peek_next() {
	if (alice_dtable_scanner_is_at_end()) {
		return '\0';
	}

	return scanner.current[1];
}

static alice_DTableToken alice_dtable_scanner_string() {
	while (alice_dtable_scanner_peek() != '"' && !alice_dtable_scanner_is_at_end()) {
		if (alice_dtable_scanner_peek() == '\n') {
			scanner.line++;
		}
		alice_dtable_scanner_advance();
	}

	if (alice_dtable_scanner_is_at_end()) {
		return alice_dtable_scanner_error_token("Unterminated string.");
	}

	alice_dtable_scanner_advance();

	return alice_dtable_scanner_new_token(ALICE_DTT_STRING);
}

static bool alice_dtable_scanner_is_digit(char c) {
	return c >= '0' && c <= '9';
}

static alice_DTableToken alice_dtable_scanner_number() {
	while (alice_dtable_scanner_is_digit(alice_dtable_scanner_peek())) {
		alice_dtable_scanner_advance();
	}

	if (alice_dtable_scanner_peek() == '.' &&
		alice_dtable_scanner_is_digit(alice_dtable_scanner_peek_next())) {

		alice_dtable_scanner_advance();

		while (alice_dtable_scanner_is_digit(alice_dtable_scanner_peek())) {
			alice_dtable_scanner_advance();
		}
	}

	return alice_dtable_scanner_new_token(ALICE_DTT_NUMBER);

}

static bool alice_dtable_scanner_is_alpha(char c) {
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}

static alice_DTableTokenType alice_dtable_scanner_check_keyword(u32 start, u32 length,
	const char* rest, alice_DTableTokenType type) {
	if (scanner.current - scanner.start == start + length &&
		memcmp(scanner.start + start, rest, length) == 0) {
		return type;
	}

	return ALICE_DTT_IDENTIFIER;
}

static alice_DTableTokenType alice_dtable_scanner_identifier_type() {
	switch (scanner.start[0]) {
	case 't':
		return alice_dtable_scanner_check_keyword(1, 3, "rue", ALICE_DTT_TRUE);
	case 'f':
		return alice_dtable_scanner_check_keyword(1, 4, "alse", ALICE_DTT_FALSE);
	}

	return ALICE_DTT_IDENTIFIER;
}

static alice_DTableToken alice_dtable_scanner_identifier() {
	while (alice_dtable_scanner_is_alpha(alice_dtable_scanner_peek()) ||
		alice_dtable_scanner_is_digit(alice_dtable_scanner_peek())) {
		alice_dtable_scanner_advance();
	}
	return alice_dtable_scanner_new_token(alice_dtable_scanner_identifier_type());
}

static void alice_dtable_scanner_skip_whitespace() {
	for (;;) {
		char c = alice_dtable_scanner_peek();
		switch (c) {
		case ' ':
		case '\r':
		case '\t':
			alice_dtable_scanner_advance();
			break;
		case '\n':
			scanner.line++;
			alice_dtable_scanner_advance();
			break;
		case '/':
			if (alice_dtable_scanner_peek_next() == '/') {
				while (alice_dtable_scanner_peek() != '\n' &&
					!alice_dtable_scanner_is_at_end()) {
					alice_dtable_scanner_advance();
				}
			}
			else {
				return;
			}
			break;
		default:
			return;
		}
	}
}

static alice_DTableToken alice_dtable_scanner_next() {
	alice_dtable_scanner_skip_whitespace();

	scanner.start = scanner.current;

	if (alice_dtable_scanner_is_at_end()) {
		return alice_dtable_scanner_new_token(ALICE_DTT_EOF);
	}

	char c = alice_dtable_scanner_advance();

	if (alice_dtable_scanner_is_alpha(c)) {
		return alice_dtable_scanner_identifier();
	}

	if (alice_dtable_scanner_is_digit(c) || c == '-') {
		return alice_dtable_scanner_number();
	}

	switch (c) {
	case '{': return alice_dtable_scanner_new_token(ALICE_DTT_LEFT_CURL);
	case '}': return alice_dtable_scanner_new_token(ALICE_DTT_RIGHT_CURL);
	case ';': return alice_dtable_scanner_new_token(ALICE_DTT_SEMICOLON);
	case '"': return alice_dtable_scanner_string();
	}

	return alice_dtable_scanner_error_token("Unexpected character");
}

static void alice_dtable_error(alice_DTableToken* t, const char* message) {
	assert(t);

	alice_log_error("Error parsing dtable [line %d]: %s", t->line, message);
}

static alice_DTableToken alice_parse_dtable(alice_DTableToken token, alice_DTable* table) {
	assert(table);

	table->children = NULL;
	table->child_count = 0;
	table->child_capacity = 0;
	table->value.type = ALICE_DTABLE_EMPTY;

	if (token.type == ALICE_DTT_RIGHT_CURL) {
		return token;
	}

	if (token.type != ALICE_DTT_IDENTIFIER) {
		alice_dtable_error(&token, "Expected an identifier.");
		return alice_dtable_scanner_error_token("");
	}

	table->name = malloc(token.length + 1);
	memcpy(table->name, token.start, token.length);
	table->name[token.length] = '\0';

	token = alice_dtable_scanner_next();

	switch (token.type) {
	case ALICE_DTT_LEFT_CURL: {
		for (;;) {
			token = alice_dtable_scanner_next();
			if (token.type == ALICE_DTT_RIGHT_CURL) {
				break;
			}
			else if (token.type == ALICE_DTT_EOF) {
				alice_dtable_error(&token, "Expected `}' after table.");
			}

			alice_DTable child;
			token = alice_parse_dtable(token, &child);
			alice_dtable_add_child(table, child);
		}
		break;
	}
	case ALICE_DTT_NUMBER: {
		table->value = (alice_DTableValue){
			.type = ALICE_DTABLE_NUMBER,
			.as = {.number = strtod(token.start, NULL) }
		};

		token = alice_dtable_scanner_next();

		if (token.type != ALICE_DTT_SEMICOLON) {
			alice_dtable_error(&token, "Expected `;' after value.");
		}
		break;
	}
	case ALICE_DTT_STRING: {
		table->value = (alice_DTableValue){
			.type = ALICE_DTABLE_STRING,
			.as = {.string = malloc(token.length - 1) }
		};

		memcpy(table->value.as.string, token.start + 1, token.length - 1);
		table->value.as.string[token.length - 2] = '\0';

		token = alice_dtable_scanner_next();

		if (token.type != ALICE_DTT_SEMICOLON) {
			alice_dtable_error(&token, "Expected `;' after value.");
		}
		break;
	}
	case ALICE_DTT_TRUE: {
		table->value = (alice_DTableValue){
			.type = ALICE_DTABLE_BOOL,
			.as = {.boolean = true}
		};

		token = alice_dtable_scanner_next();

		if (token.type != ALICE_DTT_SEMICOLON) {
			alice_dtable_error(&token, "Expected `;' after value");
		}
		break;
	}
	case ALICE_DTT_FALSE: {
		table->value = (alice_DTableValue){
			.type = ALICE_DTABLE_BOOL,
			.as = {.boolean = false}
		};

		token = alice_dtable_scanner_next();

		if (token.type != ALICE_DTT_SEMICOLON) {
			alice_dtable_error(&token, "Expected `;' after value");
		}
		break;
	}
	default:
		alice_dtable_error(&token, "Unexpected token.");
		break;
	}

	return token;
}

alice_DTable alice_new_number_dtable(const char* name, double value) {
	alice_DTable table = (alice_DTable){
		.name = malloc(strlen(name) + 1),

		.value = (alice_DTableValue){
			.type = ALICE_DTABLE_NUMBER,
			.as = {.number = value }
		},

		.children = NULL,
		.child_count = 0,
		.child_capacity = 0
	};

	strcpy(table.name, name);

	return table;
}

alice_DTable alice_new_bool_dtable(const char* name, bool value) {
	alice_DTable table = (alice_DTable){
		.name = malloc(strlen(name) + 1),

		.value = (alice_DTableValue){
			.type = ALICE_DTABLE_BOOL,
			.as = {.boolean = value }
		},

		.children = NULL,
		.child_count = 0,
		.child_capacity = 0
	};

	strcpy(table.name, name);

	return table;
}

alice_DTable alice_new_string_dtable(const char* name, const char* value) {
	alice_DTable table = (alice_DTable){
		.name = malloc(strlen(name) + 1),

		.value = (alice_DTableValue){
			.type = ALICE_DTABLE_STRING,
			.as = {.string = malloc(strlen(value) + 1) }
		},

		.children = NULL,
		.child_count = 0,
		.child_capacity = 0
	};

	strcpy(table.name, name);
	strcpy(table.value.as.string, value);

	return table;
}

alice_DTable alice_new_empty_dtable(const char* name) {
	alice_DTable table = (alice_DTable){
		.name = malloc(strlen(name) + 1),

		.value = (alice_DTableValue){
			.type = ALICE_DTABLE_EMPTY,
			.as = {.number = 0.0 }
		},

		.children = NULL,
		.child_count = 0,
		.child_capacity = 0
	};

	strcpy(table.name, name);

	return table;
}

alice_DTable* alice_dtable_find_child(alice_DTable* table, const char* name) {
	assert(table);

	for (u32 i = 0; i < table->child_count; i++) {
		if (strcmp(name, table->children[i].name) == 0) {
			return &table->children[i];
		}
	}

	return NULL;
}

alice_DTable* alice_read_dtable(alice_Resource* string) {
	assert(string);

	alice_init_dtable_scanner(string->payload);

	alice_DTable* table = malloc(sizeof(alice_DTable));
	alice_parse_dtable(alice_dtable_scanner_next(), table);

	return table;
}

static void impl_alice_write_dtable(alice_DTable* table, PHYSFS_File* file, u32 indent) {
	assert(table);
	assert(file);

	for (u32 i = 0; i < indent; i++) {
		PHYSFS_writeBytes(file, "\t", 1);
	}

	PHYSFS_writeBytes(file, table->name, strlen(table->name));

	if (table->value.type != ALICE_DTABLE_EMPTY) {
		PHYSFS_writeBytes(file, " ", 1);
	}

	switch (table->value.type) {
	case ALICE_DTABLE_NUMBER: {
		char number_str[256];
		sprintf(number_str, "%g", table->value.as.number);
		PHYSFS_writeBytes(file, number_str, strlen(number_str));
		break;
	}
	case ALICE_DTABLE_STRING:
		PHYSFS_writeBytes(file, "\"", 1);
		PHYSFS_writeBytes(file, table->value.as.string, strlen(table->value.as.string));
		PHYSFS_writeBytes(file, "\"", 1);
		break;
	case ALICE_DTABLE_BOOL: {
		const char* string = "false";
		if (table->value.as.boolean) {
			string = "true";
		}
		PHYSFS_writeBytes(file, string, strlen(string));
		break;
	}
	default:
		break;
	}

	if (table->value.type != ALICE_DTABLE_EMPTY) {
		PHYSFS_writeBytes(file, ";", 1);
	}

	if (table->child_count > 0) {
		PHYSFS_writeBytes(file, " {", 2);
	}
	PHYSFS_writeBytes(file, "\n", 1);

	for (u32 i = 0; i < table->child_count; i++) {
		impl_alice_write_dtable(&table->children[i], file, indent + 1);
	}

	if (table->child_count > 0) {
		for (u32 i = 0; i < indent; i++) {
			PHYSFS_writeBytes(file, "\t", 1);
		}
		PHYSFS_writeBytes(file, "}\n", 2);
	}
}

void alice_write_dtable(alice_DTable* table, const char* file_path) {
	assert(table);

	PHYSFS_File* file = PHYSFS_openWrite(file_path);
	if (!file) {
		alice_log_error("Failed to open %s", file_path);
		return;
	}

	impl_alice_write_dtable(table, file, 0);

	PHYSFS_close(file);
}

void alice_dtable_add_child(alice_DTable* table, alice_DTable child) {
	assert(table);

	if (table->child_count >= table->child_capacity) {
		table->child_capacity = alice_grow_capacity(table->child_capacity);
		table->children = realloc(table->children,
			table->child_capacity * sizeof(alice_DTable));
	}

	table->children[table->child_count++] = child;
}

static void impl_alice_print_dtable(alice_DTable* table, u32 indent) {
	assert(table);

	for (u32 i = 0; i < indent; i++) {
		printf("\t");
	}

	printf("%s", table->name);

	switch (table->value.type) {
	case ALICE_DTABLE_NUMBER:
		printf(" %g", table->value.as.number);
		break;
	case ALICE_DTABLE_STRING:
		printf(" \"%s\"", table->value.as.string);
		break;
	case ALICE_DTABLE_BOOL:
		printf(table->value.as.boolean ? " true" : " false");
	default:
		break;
	}

	if (table->child_count > 0) {
		printf(" {");
	}
	printf("\n");

	for (u32 i = 0; i < table->child_count; i++) {
		impl_alice_print_dtable(&table->children[i], indent + 1);
	}


	if (table->child_count > 0) {
		for (u32 i = 0; i < indent; i++) {
			printf("\t");
		}
		printf("}\n");
	}

}

void alice_print_dtable(alice_DTable* table) {
	impl_alice_print_dtable(table, 0);
}

void alice_deinit_dtable(alice_DTable* table) {
	assert(table);

	free(table->name);

	if (table->value.type == ALICE_DTABLE_STRING) {
		free(table->value.as.string);
	}

	for (u32 i = 0; i < table->child_count; i++) {
		alice_deinit_dtable(&table->children[i]);
	}

	if (table->child_count > 0) {
		free(table->children);
	}
}

void alice_free_dtable(alice_DTable* table) {
	assert(table);

	alice_deinit_dtable(table);

	free(table);
}