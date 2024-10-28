#include "util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *ctx)
{
	size_t realsize = sz * nmemb;
	memory *mem = (memory*) ctx;
	char *ptr = realloc(mem->buf, mem->size + realsize);
	if(!ptr) {
		/* out of memory */
		fprintf(stderr, "not enough memory (realloc returned NULL)\n");
		return 0;
	}
	mem->buf = ptr;
	memcpy(&(mem->buf[mem->size]), contents, realsize);
	mem->size += realsize;
	return realsize;
}

char* str_concat(char* base, int argc, char** argv, char sep) {
	if (argc == 1) return base;

	size_t size = strlen(base);
	for (int i = 1; i < argc; i++) {
		size += strlen(argv[i]) + 1;
	}

	char* res = malloc(size);

	size_t len = strlen(base);
	memcpy(res, base, len);
	char* tail = res + len;

	len = strlen(argv[1]);
	memcpy(tail, argv[1], len);
	tail += len;

	for (int i = 2; i < argc; i++) {
		len = strlen(argv[i]);
		tail[0] = sep;
		memcpy(tail + 1, argv[i], len);
		tail += len + 1;
	}

	tail[0] = '\0';
	return res;
}

char* str_trim(char* str) {
	int start = 0;
	int end = strlen(str) - 1;

	while (start <= end && isspace(str[start])) start++;
	while (start <= end && isspace(str[end])) end--;

	char* res = malloc(end - start + 2);
	memcpy(res, str + start, end - start + 1);
	res[end - start + 1] = '\0';

	return res;
}

