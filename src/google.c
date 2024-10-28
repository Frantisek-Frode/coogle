// this file is unused

#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>
#include <string.h>
#include "util.h"
#include <curl/curl.h>

void pritn_href(xmlNodePtr anchor) {
	for (xmlAttr* prop = anchor->properties; prop; prop = prop->next) {
		if (0 == strncmp((char*)prop->name, "href", 4)) {
			printf("%s\n", prop->children->content);
			return;
		}
	}

	fprintf(stderr, "no href\n");
}

int parse_goo(CURL* handle, memory* mem, char* url) {
	int opts = HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET;
	htmlDocPtr doc = htmlReadMemory(mem->buf, mem->size, url, NULL, opts);
	if (!doc) return -1;

	xmlChar *xpath = (xmlChar*) "//div[@id='rso']";
	xmlXPathContextPtr context = xmlXPathNewContext(doc);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);
	xmlXPathFreeContext(context);
	if (!result) return -1;

	xmlNodeSetPtr nodeset = result->nodesetval;
	if (xmlXPathNodeSetIsEmpty(nodeset)) {
		fprintf(stderr, "No result <div>\n");
		xmlXPathFreeObject(result);
		return -1;
	}

	if (nodeset->nodeNr > 1) {
		fprintf(stderr, "Multiple result <div>s. Using first one.\n");
	}

	xmlNodePtr resList = nodeset->nodeTab[0];
	xmlChar* apath = (xmlChar*) "//a[@href]";
	context = xmlXPathNewContext(doc);

	for (xmlNodePtr resNode = resList->children; resNode; resNode = resNode->next) {
		printf("\n");
		if (!resNode->children || !resNode->children->children || !resNode->children->children->children) {
			fprintf(stderr, "Malformed result\n");
			continue;
		}

		xmlNodePtr relevant = resNode->children->children->children;
		context->node = relevant;

		xmlXPathObjectPtr anchors = xmlXPathEvalExpression(apath, context);

		if (anchors->nodesetval->nodeNr == 0) {
			fprintf(stderr, "no <a> tags\n");
		}

		for (int i = 0; i < anchors->nodesetval->nodeNr; i++) {
			pritn_href(anchors->nodesetval->nodeTab[i]);
		}

	}

	xmlXPathFreeContext(context);

	return 0;
}

