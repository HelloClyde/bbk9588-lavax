#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct j2me9588_file
{
	int descriptor;
	int error;
	int eof;
	int standard;
};

static FILE j2me9588_stdout_file = { 1, 0, 0, 1 };
static FILE j2me9588_stderr_file = { 2, 0, 0, 1 };
FILE* stdout = &j2me9588_stdout_file;
FILE* stderr = &j2me9588_stderr_file;

static char g_j2me9588_stdio_trace[2048];
static size_t g_j2me9588_stdio_trace_length;

const char* j2me9588_stdio_trace(void)
{
	return g_j2me9588_stdio_trace;
}

static void append_stdio_trace(const char* text)
{
	size_t length = strlen(text);
	size_t available = sizeof(g_j2me9588_stdio_trace) -
		g_j2me9588_stdio_trace_length - 1u;
	if (length > available)
		length = available;
	if (length != 0u)
	{
		memcpy(g_j2me9588_stdio_trace + g_j2me9588_stdio_trace_length,
			text, length);
		g_j2me9588_stdio_trace_length += length;
		g_j2me9588_stdio_trace[g_j2me9588_stdio_trace_length] = '\0';
	}
}

void pcsl_print(const char* text)
{
	if (text != NULL)
		append_stdio_trace(text);
}

#include "bda_sdk.h"

int errno;
void* __dso_handle;

static int g_j2me9588_exit_status;

void j2me9588_set_exit_environment(unsigned long* environment)
{
	(void)environment;
}

void j2me9588_clear_exit_environment(void)
{
}

int j2me9588_exit_status(void)
{
	return g_j2me9588_exit_status;
}

int __cxa_atexit(void (*function)(void*), void* argument, void* dso)
{
	(void)function;
	(void)argument;
	(void)dso;
	return 0;
}

void exit(int status)
{
	g_j2me9588_exit_status = status;
	for (;;)
		bda_sys_delay(1u);
}

void abort(void)
{
	exit(EXIT_FAILURE);
}

char* setlocale(int category, const char* locale)
{
	static char current[] = "C";
	(void)category;
	(void)locale;
	return current;
}

void* memcpy(void* destination, const void* source, size_t size)
{
	unsigned char* out = (unsigned char*)destination;
	const unsigned char* in = (const unsigned char*)source;
	while (size-- != 0)
		*out++ = *in++;
	return destination;
}

void* memmove(void* destination, const void* source, size_t size)
{
	unsigned char* out = (unsigned char*)destination;
	const unsigned char* in = (const unsigned char*)source;
	if (out < in)
		while (size-- != 0)
			*out++ = *in++;
	else if (out > in)
	{
		out += size;
		in += size;
		while (size-- != 0)
			*--out = *--in;
	}
	return destination;
}

void* memset(void* destination, int value, size_t size)
{
	unsigned char* out = (unsigned char*)destination;
	while (size-- != 0)
		*out++ = (unsigned char)value;
	return destination;
}

int memcmp(const void* a, const void* b, size_t size)
{
	const unsigned char* left = (const unsigned char*)a;
	const unsigned char* right = (const unsigned char*)b;
	while (size-- != 0)
	{
		if (*left != *right)
			return (int)*left - (int)*right;
		++left;
		++right;
	}
	return 0;
}

void* memchr(const void* data, int value, size_t size)
{
	const unsigned char* at = (const unsigned char*)data;
	while (size-- != 0)
	{
		if (*at == (unsigned char)value)
			return (void*)at;
		++at;
	}
	return NULL;
}

size_t strlen(const char* text)
{
	const char* end = text;
	while (*end != '\0')
		++end;
	return (size_t)(end - text);
}

int strcmp(const char* a, const char* b)
{
	while (*a != '\0' && *a == *b)
	{
		++a;
		++b;
	}
	return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char* a, const char* b, size_t size)
{
	while (size != 0 && *a != '\0' && *a == *b)
	{
		++a;
		++b;
		--size;
	}
	return size == 0 ? 0 :
		(int)(unsigned char)*a - (int)(unsigned char)*b;
}

char* strcpy(char* destination, const char* source)
{
	char* result = destination;
	while ((*destination++ = *source++) != '\0')
		;
	return result;
}

char* strncpy(char* destination, const char* source, size_t size)
{
	char* result = destination;
	while (size != 0 && *source != '\0')
	{
		*destination++ = *source++;
		--size;
	}
	while (size-- != 0)
		*destination++ = '\0';
	return result;
}

char* strcat(char* destination, const char* source)
{
	strcpy(destination + strlen(destination), source);
	return destination;
}

char* strncat(char* destination, const char* source, size_t size)
{
	char* out = destination + strlen(destination);
	while (size-- != 0 && *source != '\0')
		*out++ = *source++;
	*out = '\0';
	return destination;
}

size_t strlcpy(char* destination, const char* source, size_t size)
{
	size_t length = strlen(source);
	if (size != 0u)
	{
		size_t copy = length < size - 1u ? length : size - 1u;
		memcpy(destination, source, copy);
		destination[copy] = '\0';
	}
	return length;
}

size_t strlcat(char* destination, const char* source, size_t size)
{
	size_t used = strlen(destination);
	size_t source_length = strlen(source);
	if (used < size)
	{
		size_t room = size - used - 1u;
		size_t copy = source_length < room ? source_length : room;
		memcpy(destination + used, source, copy);
		destination[used + copy] = '\0';
	}
	return used + source_length;
}

char* strchr(const char* text, int value)
{
	for (;; ++text)
	{
		if (*text == (char)value)
			return (char*)text;
		if (*text == '\0')
			return NULL;
	}
}

char* strrchr(const char* text, int value)
{
	const char* result = NULL;
	do
	{
		if (*text == (char)value)
			result = text;
	} while (*text++ != '\0');
	return (char*)result;
}

char* strstr(const char* text, const char* needle)
{
	size_t length = strlen(needle);
	if (length == 0)
		return (char*)text;
	while (*text != '\0')
	{
		if (strncmp(text, needle, length) == 0)
			return (char*)text;
		++text;
	}
	return NULL;
}

typedef struct j2me9588_allocation_header
{
	size_t size;
	u32 guard;
} j2me9588_allocation_header_t;

#define J2ME9588_ALLOC_GUARD 0x4a324d45u

#ifndef J2ME9588_ALLOC_LIMIT_BYTES
#define J2ME9588_ALLOC_LIMIT_BYTES 0u
#endif

static size_t g_j2me9588_alloc_current;
static size_t g_j2me9588_alloc_peak;

unsigned int j2me9588_alloc_current(void)
{
	return (unsigned int)g_j2me9588_alloc_current;
}

unsigned int j2me9588_alloc_peak(void)
{
	return (unsigned int)g_j2me9588_alloc_peak;
}

unsigned int j2me9588_alloc_limit(void)
{
	return (unsigned int)J2ME9588_ALLOC_LIMIT_BYTES;
}

void* malloc(size_t size)
{
	j2me9588_allocation_header_t* header;
	if (size > 0xffffffffu - sizeof(*header))
		return NULL;
	if (J2ME9588_ALLOC_LIMIT_BYTES != 0u &&
		size + sizeof(*header) >
			J2ME9588_ALLOC_LIMIT_BYTES - g_j2me9588_alloc_current)
		return NULL;
	header = (j2me9588_allocation_header_t*)bda_alloc(
		(u32)(sizeof(*header) + size));
	if (header == NULL)
		return NULL;
	header->size = size;
	header->guard = J2ME9588_ALLOC_GUARD;
	g_j2me9588_alloc_current += size + sizeof(*header);
	if (g_j2me9588_alloc_current > g_j2me9588_alloc_peak)
		g_j2me9588_alloc_peak = g_j2me9588_alloc_current;
	return header + 1;
}

void free(void* pointer)
{
	if (pointer != NULL)
	{
		j2me9588_allocation_header_t* header =
			((j2me9588_allocation_header_t*)pointer) - 1;
		if (header->guard == J2ME9588_ALLOC_GUARD)
		{
			g_j2me9588_alloc_current -= header->size + sizeof(*header);
			header->guard = 0u;
			bda_free(header);
		}
	}
}

void* calloc(size_t count, size_t size)
{
	void* result;
	size_t total;
	if (size != 0 && count > ((size_t)-1) / size)
		return NULL;
	total = count * size;
	result = malloc(total);
	if (result != NULL)
		memset(result, 0, total);
	return result;
}

void* realloc(void* pointer, size_t size)
{
	j2me9588_allocation_header_t* header;
	void* replacement;
	size_t copy_size;
	if (pointer == NULL)
		return malloc(size);
	if (size == 0)
	{
		free(pointer);
		return NULL;
	}
	header = ((j2me9588_allocation_header_t*)pointer) - 1;
	if (header->guard != J2ME9588_ALLOC_GUARD)
		return NULL;
	replacement = malloc(size);
	if (replacement == NULL)
		return NULL;
	copy_size = header->size < size ? header->size : size;
	memcpy(replacement, pointer, copy_size);
	free(pointer);
	return replacement;
}

char* strdup(const char* text)
{
	size_t size;
	char* copy;
	if (text == NULL)
		return NULL;
	size = strlen(text) + 1u;
	copy = (char*)malloc(size);
	if (copy != NULL)
		memcpy(copy, text, size);
	return copy;
}

char* strerror(int error)
{
	static char message[] = "filesystem error";
	(void)error;
	return message;
}

static int numeric_digit(int value)
{
	if (value >= '0' && value <= '9')
		return value - '0';
	if (value >= 'a' && value <= 'z')
		return value - 'a' + 10;
	if (value >= 'A' && value <= 'Z')
		return value - 'A' + 10;
	return -1;
}

unsigned long strtoul(const char* text, char** end, int base)
{
	const char* at = text;
	unsigned long value = 0;
	int digit;
	while (isspace((unsigned char)*at))
		++at;
	if (*at == '+')
		++at;
	if ((base == 0 || base == 16) && at[0] == '0' &&
		(at[1] == 'x' || at[1] == 'X'))
	{
		base = 16;
		at += 2;
	}
	else if (base == 0)
		base = *at == '0' ? 8 : 10;
	while ((digit = numeric_digit((unsigned char)*at)) >= 0 && digit < base)
	{
		value = value * (unsigned int)base + (unsigned int)digit;
		++at;
	}
	if (end != NULL)
		*end = (char*)at;
	return value;
}

long strtol(const char* text, char** end, int base)
{
	const char* at = text;
	int negative = 0;
	unsigned long value;
	while (isspace((unsigned char)*at))
		++at;
	if (*at == '-' || *at == '+')
	{
		negative = *at == '-';
		++at;
	}
	value = strtoul(at, end, base);
	return negative ? -(long)value : (long)value;
}

int atoi(const char* text)
{
	return (int)strtol(text, NULL, 10);
}

int abs(int value) { return value < 0 ? -value : value; }

static unsigned int g_random = 1u;

void srand(unsigned int seed)
{
	g_random = seed;
}

int rand(void)
{
	g_random = g_random * 1103515245u + 12345u;
	return (int)((g_random >> 16) & 0x7fffu);
}

int isdigit(int value)
{
	return value >= '0' && value <= '9';
}

int islower(int value)
{
	return value >= 'a' && value <= 'z';
}

int isupper(int value)
{
	return value >= 'A' && value <= 'Z';
}

int isprint(int value)
{
	return value >= 0x20 && value <= 0x7e;
}

int isalpha(int value)
{
	return islower(value) || isupper(value);
}

int isalnum(int value)
{
	return isalpha(value) || isdigit(value);
}

int isspace(int value)
{
	return value == ' ' || value == '\t' || value == '\n' ||
		value == '\r' || value == '\f' || value == '\v';
}

int tolower(int value)
{
	return isupper(value) ? value + ('a' - 'A') : value;
}

int toupper(int value)
{
	return islower(value) ? value - ('a' - 'A') : value;
}

time_t time(time_t* result)
{
	time_t value = (time_t)(bda_gui_millisecond_count() / 1000u);
	if (result != NULL)
		*result = value;
	return value;
}

clock_t clock(void)
{
	return (clock_t)bda_gui_millisecond_count();
}

unsigned int j2me_phoneme_bbk9588_clock_ms(void)
{
	return bda_gui_millisecond_count();
}

void j2me_phoneme_bbk9588_sleep_ms(unsigned int milliseconds)
{
	if (milliseconds != 0u)
		bda_sys_delay(milliseconds);
}

double fmod(double value, double divisor)
{
	long long quotient;
	if (divisor == 0.0)
		return __builtin_nan("");
	quotient = (long long)(value / divisor);
	return value - (double)quotient * divisor;
}

float fmodf(float value, float divisor)
{
	long quotient;
	if (divisor == 0.0f)
		return __builtin_nanf("");
	quotient = (long)(value / divisor);
	return value - (float)quotient * divisor;
}

static void format_put(char* buffer, size_t size, size_t* at, char value)
{
	if (size != 0 && *at + 1 < size)
		buffer[*at] = value;
	++*at;
}

static void format_padding(
	char* buffer, size_t size, size_t* at, int count, char value)
{
	while (count-- > 0)
		format_put(buffer, size, at, value);
}

static int unsigned_text(
	char* out, unsigned long long value, unsigned int base, int upper)
{
	char reverse[32];
	int count = 0;
	int i;
	do
	{
		unsigned int digit = (unsigned int)(value % base);
		reverse[count++] = (char)(digit < 10 ? '0' + digit :
			(upper ? 'A' : 'a') + digit - 10);
		value /= base;
	} while (value != 0);
	for (i = 0; i < count; ++i)
		out[i] = reverse[count - i - 1];
	return count;
}

int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments)
{
	size_t at = 0;
	if (buffer == NULL && size != 0)
		return -1;
	while (*format != '\0')
	{
		char pad = ' ';
		int left = 0;
		int width = 0;
		int precision = -1;
		int length = 0;
		char value_buffer[40];
		const char* value_text = value_buffer;
		int value_length = 0;
		int output_length;
		int negative = 0;
		char specifier;
		unsigned long long unsigned_value;

		if (*format != '%')
		{
			format_put(buffer, size, &at, *format++);
			continue;
		}
		++format;
		if (*format == '%')
		{
			format_put(buffer, size, &at, *format++);
			continue;
		}
		if (*format == '-')
		{
			left = 1;
			++format;
		}
		if (*format == '0')
		{
			pad = '0';
			++format;
		}
		while (*format >= '0' && *format <= '9')
			width = width * 10 + (*format++ - '0');
		if (*format == '.')
		{
			++format;
			if (*format == '*')
			{
				precision = va_arg(arguments, int);
				++format;
			}
			else
			{
				precision = 0;
				while (*format >= '0' && *format <= '9')
					precision = precision * 10 + (*format++ - '0');
			}
		}
		if (*format == 'l')
		{
			length = 1;
			++format;
			if (*format == 'l')
			{
				length = 2;
				++format;
			}
		}
		else if (*format == 'z')
		{
			length = 3;
			++format;
		}
		specifier = *format != '\0' ? *format++ : '\0';
		if (specifier == 's')
		{
			value_text = va_arg(arguments, const char*);
			if (value_text == NULL)
				value_text = "(null)";
			value_length = (int)strlen(value_text);
			if (precision >= 0 && value_length > precision)
				value_length = precision;
		}
		else if (specifier == 'c')
		{
			value_buffer[0] = (char)va_arg(arguments, int);
			value_length = 1;
		}
		else if (specifier == 'p')
		{
			uintptr_t pointer = (uintptr_t)va_arg(arguments, void*);
			value_buffer[0] = '0';
			value_buffer[1] = 'x';
			value_length = 2 + unsigned_text(value_buffer + 2,
				(unsigned long long)pointer, 16, 0);
		}
		else if (specifier == 'd' || specifier == 'i')
		{
			long long signed_value = length == 2 ?
				va_arg(arguments, long long) :
				(length == 1 ? va_arg(arguments, long) :
				(long long)va_arg(arguments, int));
			if (signed_value < 0)
			{
				negative = 1;
				unsigned_value = (unsigned long long)(-(signed_value + 1)) + 1;
			}
			else
				unsigned_value = (unsigned long long)signed_value;
			value_length = unsigned_text(value_buffer, unsigned_value, 10, 0);
		}
		else if (specifier == 'u' || specifier == 'x' || specifier == 'X')
		{
			unsigned_value = length == 2 ? va_arg(arguments, unsigned long long) :
				(length == 1 ? va_arg(arguments, unsigned long) :
				(length == 3 ? (unsigned long long)va_arg(arguments, size_t) :
				(unsigned long long)va_arg(arguments, unsigned int)));
			value_length = unsigned_text(value_buffer, unsigned_value,
				specifier == 'u' ? 10u : 16u, specifier == 'X');
		}
		else
		{
			value_buffer[0] = '%';
			value_buffer[1] = specifier;
			value_length = specifier == '\0' ? 1 : 2;
		}

		if (!left && pad == ' ')
			format_padding(buffer, size, &at,
				width - value_length - negative, pad);
		if (negative)
			format_put(buffer, size, &at, '-');
		if (!left && pad == '0')
			format_padding(buffer, size, &at,
				width - value_length - negative, pad);
		output_length = value_length;
		while (value_length-- > 0)
			format_put(buffer, size, &at, *value_text++);
		if (left)
			format_padding(buffer, size, &at,
				width - output_length - negative, ' ');
	}
	if (size != 0)
		buffer[at < size ? at : size - 1] = '\0';
	return (int)at;
}

int snprintf(char* buffer, size_t size, const char* format, ...)
{
	int result;
	va_list arguments;
	va_start(arguments, format);
	result = vsnprintf(buffer, size, format, arguments);
	va_end(arguments);
	return result;
}

int vsprintf(char* buffer, const char* format, va_list arguments)
{
	return vsnprintf(buffer, (size_t)-1, format, arguments);
}

int sprintf(char* buffer, const char* format, ...)
{
	int result;
	va_list arguments;
	va_start(arguments, format);
	result = vsprintf(buffer, format, arguments);
	va_end(arguments);
	return result;
}

static int scan_set_contains(const char* begin, const char* end, char value)
{
	const char* at;
	for (at = begin; at < end; ++at)
	{
		if (at + 2 < end && at[1] == '-')
		{
			if (value >= at[0] && value <= at[2])
				return 1;
			at += 2;
		}
		else if (*at == value)
			return 1;
	}
	return 0;
}

int sscanf(const char* text, const char* format, ...)
{
	int assigned = 0;
	va_list arguments;
	va_start(arguments, format);
	while (*format != '\0')
	{
		if (isspace((unsigned char)*format))
		{
			while (isspace((unsigned char)*format))
				++format;
			while (isspace((unsigned char)*text))
				++text;
			continue;
		}
		if (*format != '%')
		{
			if (*text != *format)
				break;
			++text;
			++format;
			continue;
		}
		++format;
		if (*format == '%')
		{
			if (*text != '%')
				break;
			++text;
			++format;
			continue;
		}
		if (*format == 'd')
		{
			int sign = 1;
			int value = 0;
			int digits = 0;
			int* destination = va_arg(arguments, int*);
			if (*text == '-')
			{
				sign = -1;
				++text;
			}
			while (isdigit((unsigned char)*text))
			{
				value = value * 10 + (*text++ - '0');
				++digits;
			}
			if (digits == 0)
				break;
			*destination = value * sign;
			++assigned;
			++format;
			continue;
		}
		if (*format == '[')
		{
			const char* set_begin = ++format;
			const char* set_end;
			char* destination = va_arg(arguments, char*);
			int count = 0;
			while (*format != '\0' && *format != ']')
				++format;
			set_end = format;
			if (*format == ']')
				++format;
			while (*text != '\0' &&
				scan_set_contains(set_begin, set_end, *text))
			{
				*destination++ = *text++;
				++count;
			}
			if (count == 0)
				break;
			*destination = '\0';
			++assigned;
			continue;
		}
		break;
	}
	va_end(arguments);
	return assigned;
}

static int stream_vformat(FILE* stream, const char* format, va_list arguments)
{
	char buffer[512];
	int result;
	(void)stream;
	result = vsnprintf(buffer, sizeof(buffer), format, arguments);
	append_stdio_trace(buffer);
	return result;
}

int printf(const char* format, ...)
{
	int result;
	va_list arguments;
	va_start(arguments, format);
	result = stream_vformat(stdout, format, arguments);
	va_end(arguments);
	return result;
}

int fprintf(FILE* stream, const char* format, ...)
{
	int result;
	va_list arguments;
	va_start(arguments, format);
	result = stream_vformat(stream, format, arguments);
	va_end(arguments);
	return result;
}

int fputs(const char* text, FILE* stream)
{
	size_t length;
	if (text == NULL)
		return EOF;
	length = strlen(text);
	if (stream != NULL && !stream->standard)
		return fwrite(text, 1u, length, stream) == length ? 0 : EOF;
	append_stdio_trace(text);
	return (int)length;
}

int puts(const char* text)
{
	return fputs(text, stdout);
}

int fflush(FILE* stream)
{
	(void)stream;
	return 0;
}
