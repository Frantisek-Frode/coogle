#include <stdio.h>
#include "util.h"
#include "duck.h"
#include "sresults.h"
#include "request.h"

int main(int argc, char** argv)
{
	if (argc == 1) {
		printf("No query\n");
		return 0;
	}

	// char* google = "https://www.google.com/search?q=";
	char* ddg = "https://html.duckduckgo.com/html?q=";
	char* url = str_concat(ddg, argc, argv, '+');
	htmlDocPtr response = Get(url);
	free(url);

	if (response) {
		SearchResults* results = parse_duck(response);
		for (size_t i = 0; i < results->len; i++) {
			SearchResult* res = &results->values[i];

			printf(" \e[0;92m%s\e[0m\n", res->title);
			printf("\e[0;94m%s\e[0m\n", res->url);
			printf("\e[2m%s\e[0m\n", res->snippet);
			printf("\n");
		}
		free_sresults(results);
	}

	xmlFreeDoc(response);

	printf("\n");
	return 0;
}

