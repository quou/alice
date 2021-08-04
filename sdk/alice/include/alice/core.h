#pragma once

#include <stdint.h>
#include <stdbool.h>

/* ==== PLATFORM DETECTION ==== */
#if !defined(ALICE_PLATFORM_LINUX) || !defined(ALICE_PLATFORM_WINDOWS)
	#if defined(_WIN32)
		#define ALICE_PLATFORM_WINDOWS
	#elif defined(__linux__)
		#define ALICE_PLATFORM_LINUX
	#endif
#endif

/* ==== SYMBOL HELPERS ==== */
/* For scripting on Windows */
#if defined(ALICE_EXPORT_SYMBOLS)
	#undef ALICE_EXPORT_SYMBOLS

	#if defined(ALICE_API)
		#undef ALICE_API
	#endif

	#ifdef ALICE_PLATFORM_WINDOWS
		#define ALICE_API __declspec(dllexport)
	#else
		#define ALICE_API
	#endif

#elif defined(ALICE_IMPORT_SYMBOLS)
	#undef ALICE_IMPORT_SYMBOLS

	#if defined(ALICE_API)
		#undef ALICE_API
	#endif

	#ifdef _WIN32
		#define ALICE_API __declspec(dllimport)
	#else
		#define ALICE_API
	#endif
#else
#define ALICE_API
#endif

#if !defined(ALICE_CALL)
	#ifdef _MSC_VER
		#define ALICE_CALL __cdecl
	#else
		#define ALICE_CALL
	#endif
#endif

#define alice_null 0x0

#define alice_grow_capacity(c_) \
	((c_) < 8 ? 8 : (c_) * 2)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

ALICE_API char* alice_copy_string(const char* string);
ALICE_API u32 alice_hash_string(const char* string);

ALICE_API void alice_log(const char* fmt, ...);
ALICE_API void alice_log_warning(const char* fmt, ...);
ALICE_API void alice_log_error(const char* fmt, ...);

#define alice_min(x_, y_) (((x_) < (y_)) ? (x_) : (y_))
#define alice_max(x_, y_) (((x_) > (y_)) ? (x_) : (y_))

typedef struct alice_type_info_t {
	u32 id;
	u32 size;
} alice_type_info_t;

#ifndef __cplusplus
#define alice_get_type_info(t_) ((alice_type_info_t){ \
			.id = alice_hash_string(#t_), \
			.size = sizeof(t_) \
		})
#else
#define alice_get_type_info(t_) { \
			alice_hash_string(#t_), \
			sizeof(t_) \
		}
#endif
