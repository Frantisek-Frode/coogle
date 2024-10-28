#pragma once
#include <stddef.h>

/* resizable buffer */
typedef struct {
	char *buf;
	size_t size;
} memory;

size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *ctx);

char* str_concat(char* base, int argc, char** argv, char sep);
char* str_trim(char* str);

