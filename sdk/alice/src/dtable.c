#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <physfs.h>

#include "alice/dtable.h"

typedef struct alice_dtable_scanner_t {
	const char* start;
	const char* current;
	u32 line;
} alice_dtable_scanner_t;

typedef enum alice_dtable_token_type_t {
	ALICE_DTT_LEFT_CURL,
	ALICE_DTT_RIGHT_CURL,
	ALICE_DTT_LEFT_BRACE,
	ALICE_DTT_RIGHT_BRACE,
	ALICE_DTT_SEMICOLON,
	ALICE_DTT_QUOTE,
	ALICE_DTT_NUMBER,
	ALICE_DTT_IDENTIFIER,
	ALICE_DTT_ERROR,
	ALICE_DTT_EOF,
	ALICE_DTT_STRING,
	ALICE_DTT_TRUE,
	ALICE_DTT_FALSE
} alice_dtable_token_type_t;

typedef struct alice_dtable_token_t {
	alice_dtable_token_type_t type;
	const char* start;
	u32 length;
	u32 line;
} alice_dtable_token_t;

alice_dtable_scanner_t scanner;

static void alice_init_dtable_scanner(const char* source) {
	scanner.start = source;
	scanner.current = source;
	scanner.line = 1;
}

static bool alice_dtable_scanner_is_at_end() {
	return *scanner.current == '\0';
}

static alice_dtable_token_t alice_dtable_scanner_new_token(alice_dtable_token_type_t type) {
	return (alice_dtable_token_t) {
		.type = type,
			.start = scanner.start,
			.length = (u32)(scanner.current - scanner.start),
			.line = scanner.line
	};
}

static alice_dtable_token_t alice_dtable_scanner_error_token(const char* message) {
	return (alice_dtable_token_t) {
		.type = ALICE_DTT_ERROR,
			.start = message,
			.length = (u32)strlen(message),
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

static alice_dtable_token_t alice_dtable_scanner_string() {
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

static alice_dtable_token_t alice_dtable_scanner_number() {
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

static alice_dtable_token_type_t alice_dtable_scanner_check_keyword(u32 start, u32 length,
	const char* rest, alice_dtable_token_type_t type) {
	if (scanner.current - scanner.start == start + length &&
		memcmp(scanner.start + start, rest, length) == 0) {
		return type;
	}

	return ALICE_DTT_IDENTIFIER;
}

static alice_dtable_token_type_t alice_dtable_scanner_identifier_type() {
	switch (scanner.start[0]) {
	case 't':
		return alice_dtable_scanner_check_keyword(1, 3, "rue", ALICE_DTT_TRUE);
	case 'f':
		return alice_dtable_scanner_check_keyword(1, 4, "alse", ALICE_DTT_FALSE);
	}

	return ALICE_DTT_IDENTIFIER;
}

static alice_dtable_token_t alice_dtable_scanner_identifier() {
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

static alice_dtable_token_t alice_dtable_scanner_next() {
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
		case '[': return alice_dtable_scanner_new_token(ALICE_DTT_LEFT_BRACE);
		case ']': return alice_dtable_scanner_new_token(ALICE_DTT_RIGHT_BRACE);
		case ';': return alice_dtable_scanner_new_token(ALICE_DTT_SEMICOLON);
		case '"': return alice_dtable_scanner_string();
	}

	return alice_dtable_scanner_error_token("Unexpected character");
}

static void alice_dtable_error(alice_dtable_token_t* t, const char* message) {
	assert(t);

	alice_log_error("Error parsing dtable [line %d]: %s", t->line, message);
}

static alice_dtable_value_t alice_parse_dtable_value(alice_dtable_token_t token) {
	switch(token.type) {
		case ALICE_DTT_NUMBER: {
			alice_dtable_value_t value = (alice_dtable_value_t){
				.type = ALICE_DTABLE_NUMBER,
				.as = {.number = strtod(token.start, NULL) }
			};

			token = alice_dtable_scanner_next();

			if (token.type != ALICE_DTT_SEMICOLON) {
				alice_dtable_error(&token, "Expected `;' after value.");
			}
			return value;
		}
		case ALICE_DTT_STRING: {
			alice_dtable_value_t value = (alice_dtable_value_t){
				.type = ALICE_DTABLE_STRING,
				.as = {.string = malloc(token.length - 1) }
			};

			memcpy(value.as.string, token.start + 1, token.length - 1);
			value.as.string[token.length - 2] = '\0';

			token = alice_dtable_scanner_next();

			if (token.type != ALICE_DTT_SEMICOLON) {
				alice_dtable_error(&token, "Expected `;' after value.");
			}
			return value;
		}
		case ALICE_DTT_TRUE: {
			alice_dtable_value_t value = (alice_dtable_value_t){
				.type = ALICE_DTABLE_BOOL,
				.as = {.boolean = true}
			};

			token = alice_dtable_scanner_next();

			if (token.type != ALICE_DTT_SEMICOLON) {
				alice_dtable_error(&token, "Expected `;' after value");
			}
			return value;
		}
		case ALICE_DTT_FALSE: {
			alice_dtable_value_t value = (alice_dtable_value_t){
				.type = ALICE_DTABLE_BOOL,
				.as = {.boolean = false}
			};

			token = alice_dtable_scanner_next();

			if (token.type != ALICE_DTT_SEMICOLON) {
				alice_dtable_error(&token, "Expected `;' after value");
			}
			return value;
		}
		default:
			alice_dtable_error(&token, "Unexpected token.");
			return (alice_dtable_value_t){ 0 };
	}

}

static alice_dtable_token_t alice_parse_dtable(alice_dtable_token_t token, alice_dtable_t* table) {
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
	table->name_hash = alice_hash_string(table->name);

	token = alice_dtable_scanner_next();

	if (token.type == ALICE_DTT_LEFT_CURL) {
		for (;;) {
			token = alice_dtable_scanner_next();
			if (token.type == ALICE_DTT_RIGHT_CURL) {
				break;
			}
			else if (token.type == ALICE_DTT_EOF) {
				alice_dtable_error(&token, "Expected `}' after table.");
			}

			alice_dtable_t child;
			token = alice_parse_dtable(token, &child);
			alice_dtable_add_child(table, child);
		}
	} else if (token.type == ALICE_DTT_LEFT_BRACE) {
		table->value = (alice_dtable_value_t) {
			.type = ALICE_DTABLE_ARRAY,
			.as = {
				.array = alice_new_dtable_value_array()
			}
		};

		alice_dtable_value_array_t* array = table->value.as.array;

		for (;;) {
			token = alice_dtable_scanner_next();
			if (token.type == ALICE_DTT_RIGHT_BRACE) {
				break;
			}
			else if (token.type == ALICE_DTT_EOF) {
				alice_dtable_error(&token, "Expected `]' after array.");
			}

			alice_dtable_value_t value = alice_parse_dtable_value(token);
			alice_dtable_value_array_add(array, value);
		}
	} else {
		table->value = alice_parse_dtable_value(token);
	}


	return token;
}

alice_dtable_value_array_t* alice_new_dtable_value_array() {
	alice_dtable_value_array_t* new = malloc(sizeof(alice_dtable_value_array_t));

	new->values = alice_null;
	new->count = 0;
	new->capacity = 0;

	return new;
}

void alice_free_dtable_value_array(alice_dtable_value_array_t* array) {
	assert(array);

	for (u32 i = 0; i < array->count; i++) {
		alice_free_dtable_value(&array->values[i]);
	}

	if (array->capacity > 0) {
		free(array->values);
	}

	free(array);
}

void alice_dtable_value_array_add(alice_dtable_value_array_t* array, alice_dtable_value_t value) {
	assert(array);

	if (value.type == ALICE_DTABLE_EMPTY ||
			value.type == ALICE_DTABLE_ARRAY) {
		alice_log_warning("DTable array cannot contain arrays or value-less tables");
		return;
	}

	if (array->count >= array->capacity) {
		array->capacity = alice_grow_capacity(array->capacity);
		array->values = realloc(array->values, array->capacity * sizeof(alice_dtable_value_t));
	}

	array->values[array->count++] = value;
}

void alice_free_dtable_value(alice_dtable_value_t* value) {
	assert(value);

	if (value->type == ALICE_DTABLE_STRING) {
		free(value->as.string);
	} else if (value->type == ALICE_DTABLE_ARRAY) {
		alice_free_dtable_value_array(value->as.array);
	}
}

alice_dtable_t alice_new_number_dtable(const char* name, double value) {
	alice_dtable_t table = alice_new_empty_dtable(name);

	table.value = (alice_dtable_value_t) {
		.type = ALICE_DTABLE_NUMBER,
		.as = {.number = value}
	};

	return table;
}

alice_dtable_t alice_new_bool_dtable(const char* name, bool value) {
	alice_dtable_t table = alice_new_empty_dtable(name);

	table.value = (alice_dtable_value_t) {
		.type = ALICE_DTABLE_BOOL,
		.as = {.boolean = value}
	};

	return table;
}

alice_dtable_t alice_new_string_dtable(const char* name, const char* value) {
	alice_dtable_t table = alice_new_empty_dtable(name);

	table.value = (alice_dtable_value_t) {
		.type = ALICE_DTABLE_STRING,
		.as = {.string = alice_copy_string(value) }
	};

	return table;
}

alice_dtable_t alice_new_array_dtable(const char* name, alice_dtable_value_array_t* array) {
	assert(array);

	alice_dtable_t table = alice_new_empty_dtable(name);

	table.value = (alice_dtable_value_t) {
		.type = ALICE_DTABLE_ARRAY,
		.as = {.array = array}
	};

	return table;
}

alice_dtable_t alice_new_empty_dtable(const char* name) {
	alice_dtable_t table = (alice_dtable_t){
		.name = alice_copy_string(name),
		.name_hash = alice_hash_string(name),

		.value = (alice_dtable_value_t){
			.type = ALICE_DTABLE_EMPTY,
			.as = {.number = 0.0 }
		},

		.children = alice_null,
		.child_count = 0,
		.child_capacity = 0
	};

	return table;
}

alice_dtable_t* alice_dtable_find_child(alice_dtable_t* table, const char* name) {
	assert(table);

	u32 name_hash = alice_hash_string(name);

	for (u32 i = 0; i < table->child_count; i++) {
		alice_dtable_t* child = &table->children[i];

		if (name_hash == child->name_hash) {
			return child;
		}
	}

	return alice_null;
}

alice_dtable_t* alice_read_dtable(alice_resource_t* string) {
	assert(string);

	alice_init_dtable_scanner(string->payload);

	alice_dtable_t* table = malloc(sizeof(alice_dtable_t));
	alice_parse_dtable(alice_dtable_scanner_next(), table);

	return table;
}

static void alice_write_dtable_value(PHYSFS_File* file, alice_dtable_value_t value, u32 indent) {
	switch (value.type) {
	case ALICE_DTABLE_NUMBER: {
		char number_str[256];
		sprintf(number_str, "%g", value.as.number);
		PHYSFS_writeBytes(file, number_str, strlen(number_str));
		break;
	}
	case ALICE_DTABLE_STRING:
		PHYSFS_writeBytes(file, "\"", 1);
		PHYSFS_writeBytes(file, value.as.string, strlen(value.as.string));
		PHYSFS_writeBytes(file, "\"", 1);
		break;
	case ALICE_DTABLE_BOOL: {
		const char* string = "false";
		if (value.as.boolean) {
			string = "true";
		}
		PHYSFS_writeBytes(file, string, strlen(string));
		break;
	}
	case ALICE_DTABLE_ARRAY: {
		alice_dtable_value_array_t* array = value.as.array;
		PHYSFS_writeBytes(file, "[\n", 2);
		for (u32 i = 0; i < array->count; i++) {
			for (u32 i = 0; i < indent + 1; i++) {
				PHYSFS_writeBytes(file, "\t", 1);
			}

			alice_write_dtable_value(file, array->values[i], indent);

			PHYSFS_writeBytes(file, "\n", 1);
		}

		for (u32 i = 0; i < indent; i++) {
			PHYSFS_writeBytes(file, "\t", 1);
		}

		PHYSFS_writeBytes(file, "]\n", 2);
	}
	default:
		break;
	}

	if (value.type != ALICE_DTABLE_ARRAY &&
			value.type != ALICE_DTABLE_EMPTY) {
		PHYSFS_writeBytes(file, ";", 1);
	}
}

static void impl_alice_write_dtable(alice_dtable_t* table, PHYSFS_File* file, u32 indent) {
	assert(table);
	assert(file);

	for (u32 i = 0; i < indent; i++) {
		PHYSFS_writeBytes(file, "\t", 1);
	}

	PHYSFS_writeBytes(file, table->name, strlen(table->name));

	PHYSFS_writeBytes(file, " ", 1);

	alice_write_dtable_value(file, table->value, indent);

	if (table->child_count > 0) {
		PHYSFS_writeBytes(file, "{", 1);
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

void alice_write_dtable(alice_dtable_t* table, const char* file_path) {
	assert(table);

	PHYSFS_File* file = PHYSFS_openWrite(file_path);
	if (!file) {
		alice_log_error("Failed to open %s", file_path);
		return;
	}

	impl_alice_write_dtable(table, file, 0);

	PHYSFS_close(file);
}

void alice_dtable_add_child(alice_dtable_t* table, alice_dtable_t child) {
	assert(table);

	if (table->child_count >= table->child_capacity) {
		table->child_capacity = alice_grow_capacity(table->child_capacity);
		table->children = realloc(table->children,
			table->child_capacity * sizeof(alice_dtable_t));
	}

	table->children[table->child_count++] = child;
}

void alice_deinit_dtable(alice_dtable_t* table) {
	assert(table);

	free(table->name);

	alice_free_dtable_value(&table->value);

	for (u32 i = 0; i < table->child_count; i++) {
		alice_deinit_dtable(&table->children[i]);
	}

	if (table->child_count > 0) {
		free(table->children);
	}
}

void alice_free_dtable(alice_dtable_t* table) {
	assert(table);

	alice_deinit_dtable(table);

	free(table);
}
