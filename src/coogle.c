#include <stdio.h>
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libxml/HTMLparser.h>
#include <libxml/xpath.h>

/* resizable buffer */
typedef struct {
	char *buf;
	size_t size;
} memory;

size_t grow_buffer(void *contents, size_t sz, size_t nmemb, void *ctx)
{
	size_t realsize = sz * nmemb;
	memory *mem = (memory*) ctx;
	char *ptr = realloc(mem->buf, mem->size + realsize);
	if(!ptr) {
		/* out of memory */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	mem->buf = ptr;
	memcpy(&(mem->buf[mem->size]), contents, realsize);
	mem->size += realsize;
	return realsize;
}

CURL *make_handle(char *url)
{
	CURL *handle = curl_easy_init();

	/* Important: use HTTP2 over HTTPS */
	curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(handle, CURLOPT_URL, url);

	/* buffer body */
	memory *mem = malloc(sizeof(memory));
	mem->size = 0;
	mem->buf = malloc(1);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, grow_buffer);
	curl_easy_setopt(handle, CURLOPT_WRITEDATA, mem);
	curl_easy_setopt(handle, CURLOPT_PRIVATE, mem);

	/* For completeness */
	curl_easy_setopt(handle, CURLOPT_ACCEPT_ENCODING, "");
	curl_easy_setopt(handle, CURLOPT_TIMEOUT, 5L);
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	/* only allow redirects to HTTP and HTTPS URLs */
	curl_easy_setopt(handle, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS | CURLPROTO_HTTP);
	curl_easy_setopt(handle, CURLOPT_AUTOREFERER, 1L);
	curl_easy_setopt(handle, CURLOPT_MAXREDIRS, 10L);
	/* each transfer needs to be done within 20 seconds! */
	curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, 20000L);
	/* connect fast or fail */
	curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT_MS, 5000L);
	/* skip files larger than a gigabyte */
	curl_easy_setopt(handle, CURLOPT_MAXFILESIZE_LARGE,
			(curl_off_t)1024*1024*1024);
	// curl_easy_setopt(handle, CURLOPT_COOKIEFILE, "");
	// curl_easy_setopt(handle, CURLOPT_FILETIME, 1L);
	// curl_easy_setopt(handle, CURLOPT_USERAGENT, "Firefox/131.0");
	// curl_easy_setopt(handle, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
	// curl_easy_setopt(handle, CURLOPT_UNRESTRICTED_AUTH, 1L);
	// curl_easy_setopt(handle, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
	curl_easy_setopt(handle, CURLOPT_EXPECT_100_TIMEOUT_MS, 0L);
	return handle;
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

int parse_ddg(CURL* handle, memory* mem, char* url) {
	int opts = HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET;
	htmlDocPtr doc = htmlReadMemory(mem->buf, mem->size, url, NULL, opts);
	if (!doc) return -1;

	xmlChar *xpath = (xmlChar*) "//div[@id='links']/div/div";
	xmlXPathContextPtr context = xmlXPathNewContext(doc);
	xmlXPathObjectPtr result = xmlXPathEvalExpression(xpath, context);
	if (!result) return -1;

	xmlNodeSetPtr nodeset = result->nodesetval;
	if (xmlXPathNodeSetIsEmpty(nodeset)) {
		fprintf(stderr, "No results\n");
		xmlXPathFreeObject(result);
		return -1;
	}


	char* nil = "#";
	for (int i = 0; i < nodeset->nodeNr; i++) {
		xmlNodePtr link = nodeset->nodeTab[i];

		char* title = nil;
		char* snippet = nil;
		char* href = nil;

		context->node = link;
		xmlChar* titlePath = (xmlChar*)".//h2//a/text()";
		xmlXPathObjectPtr titleNode = xmlXPathEvalExpression(titlePath, context);
		if (titleNode && titleNode->nodesetval->nodeNr) {
			title = strdup((char*)titleNode->nodesetval->nodeTab[0]->content);
		}

		xmlChar* hrefPath = (xmlChar*)".//a[@class='result__url']/text()";
		xmlXPathObjectPtr hrefNode = xmlXPathEvalExpression(hrefPath, context);
		if (hrefNode && hrefNode->nodesetval->nodeNr) {
			href = str_trim((char*)hrefNode->nodesetval->nodeTab[0]->content);
		}

		xmlChar* snipPath = (xmlChar*)".//a[@class='result__snippet']//text()";
		xmlXPathObjectPtr snipNode = xmlXPathEvalExpression(snipPath, context);
		if (snipNode && snipNode->nodesetval->nodeNr) {
			int snipParts = snipNode->nodesetval->nodeNr;
			char** parts = calloc(sizeof(char*), snipParts);
			for (int i = 0; i < snipParts; i++) {
				parts[i] = (char*)snipNode->nodesetval->nodeTab[i]->content;
			}
			snippet = str_trim(str_concat(parts[0], snipParts, parts, 0x1f));
			free(parts);
		}

		printf(" \e[0;92m%s\e[0m\n", title);
		printf("\e[0;94m%s\e[0m\n", href);
		printf("\e[2m%s\e[0m\n", snippet);
		printf("\n");

		// TODO: fix memleak
		// free(title);
		// free(href);
		// free(snippet);
	}

	xmlXPathFreeContext(context);
	return 0;
}

int is_html(char *ctype)
{
	return ctype != NULL && strlen(ctype) > 10 && strstr(ctype, "text/html");
}

void Get(char* reqUrl)
{
	CURL *handle = make_handle(reqUrl);
	CURLcode res = curl_easy_perform(handle);
	CURLMsg *m = NULL;
	memory *mem;
	char* url;

	curl_easy_getinfo(handle, CURLINFO_PRIVATE, &mem);
	curl_easy_getinfo(handle, CURLINFO_EFFECTIVE_URL, &url);
	if(res == CURLE_OK) {
		long res_status;
		curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &res_status);
		if(res_status == 200) {
			char *ctype;
			curl_easy_getinfo(handle, CURLINFO_CONTENT_TYPE, &ctype);
			// printf("HTTP 200 (%s): %s\n", ctype, url);

			if (is_html(ctype)) {
				parse_ddg(handle, mem, url);
			}
		}
		else {
			printf("HTTP %d: %s\n", (int) res_status, url);
		}
	}
	else {
		printf("Connection failure: %s\n", url);
	}

	curl_easy_cleanup(handle);
	free(mem->buf);
	free(mem);

	curl_global_cleanup();
}


int main(int argc, char** argv)
{
	if (argc == 1) {
		printf("No query\n");
		return 0;
	}

	// char* google = "https://www.google.com/search?q=";
	char* ddg = "https://html.duckduckgo.com/html?q=";
	char* url = str_concat(ddg, argc, argv, '+');

	Get(url);

	free(url);
	printf("\n");
	return 0;
}
