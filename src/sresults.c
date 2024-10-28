#include "sresults.h"
#include <stdlib.h>

char* nil_str = "";

/// Allocates a memeory for search results and
/// populates the memory by empty results.
///
/// Call `free_sresults` to free the memory and all contained strings.
SearchResults* alloc_sresults(size_t count) {
	SearchResults* mem = malloc(
		sizeof(size_t)
		+ sizeof(SearchResult*)
		+ count * sizeof(SearchResult)
	);

	mem->len = count;
	mem->values = ((void*)mem) + sizeof(size_t) + sizeof(SearchResult*);

	for (size_t i = 0; i < count; i++) {
		mem->values[i].title = nil_str;
		mem->values[i].url = nil_str;
		mem->values[i].snippet = nil_str;
	}
	return mem;
}

void free_sresults(SearchResults* results) {
	for (size_t i = 0; i < results->len; i++) {
		if (results->values[i].title != nil_str)
			free(results->values[i].title);

		if (results->values[i].url != nil_str)
			free(results->values[i].url);

		if (results->values[i].snippet != nil_str)
			free(results->values[i].snippet);
	}

	free(results);
}

