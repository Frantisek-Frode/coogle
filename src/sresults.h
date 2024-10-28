#pragma once
#include <stddef.h>


typedef struct {
	char* title;
	char* url;
	char* snippet;
} SearchResult;

typedef struct {
	size_t len;
	SearchResult* values;
} SearchResults;

SearchResults* alloc_sresults(size_t count);
void free_sresults(SearchResults* results);

