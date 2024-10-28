#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include "duck.h"
#include <string.h>

SearchResults* parse_duck(htmlDocPtr doc) {
	if (!doc) return NULL;

	xmlChar *xpath = (xmlChar*) "//div[@id='links']/div/div";
	xmlXPathContextPtr context = xmlXPathNewContext(doc);
	xmlXPathObjectPtr results = xmlXPathEvalExpression(xpath, context);
	if (!results) return NULL;

	xmlNodeSetPtr nodeset = results->nodesetval;
	if (xmlXPathNodeSetIsEmpty(nodeset)) {
		fprintf(stderr, "No results\n");
		xmlXPathFreeObject(results);
		return NULL;
	}

	SearchResults* output = alloc_sresults(nodeset->nodeNr);

	for (int i = 0; i < nodeset->nodeNr; i++) {
		xmlNodePtr link = nodeset->nodeTab[i];
		SearchResult* sres = &output->values[i];

		context->node = link;
		xmlChar* titlePath = (xmlChar*)".//h2//a/text()";
		xmlXPathObjectPtr titleNode = xmlXPathEvalExpression(titlePath, context);
		if (titleNode && titleNode->nodesetval->nodeNr) {
			sres->title = strdup((char*)titleNode->nodesetval->nodeTab[0]->content);
		}

		xmlChar* hrefPath = (xmlChar*)".//a[@class='result__url']/text()";
		xmlXPathObjectPtr hrefNode = xmlXPathEvalExpression(hrefPath, context);
		if (hrefNode && hrefNode->nodesetval->nodeNr) {
			sres->url = str_trim((char*)hrefNode->nodesetval->nodeTab[0]->content);
		}

		xmlChar* snipPath = (xmlChar*)".//a[@class='result__snippet']//text()";
		xmlXPathObjectPtr snipNode = xmlXPathEvalExpression(snipPath, context);
		if (snipNode && snipNode->nodesetval->nodeNr) {
			int snipParts = snipNode->nodesetval->nodeNr;
			char** parts = calloc(sizeof(char*), snipParts);
			for (int i = 0; i < snipParts; i++) {
				parts[i] = (char*)snipNode->nodesetval->nodeTab[i]->content;
			}
			sres->snippet = str_trim(str_concat(parts[0], snipParts, parts, 0x1f));
			free(parts);
		}
	}

	xmlXPathFreeContext(context);
	return output;
}
