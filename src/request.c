#include "request.h"
#include "util.h"
#include <string.h>

CURL *make_handle(char *url)
{
	CURL *handle = curl_easy_init();

	/* Important: use HTTP2 over HTTPS */
	// curl_easy_setopt(handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
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

	curl_easy_setopt(handle, CURLOPT_EXPECT_100_TIMEOUT_MS, 0L);
	return handle;
}

int is_html(char *ctype)
{
	return ctype != NULL && strlen(ctype) > 10 && strstr(ctype, "text/html");
}

htmlDocPtr Get(char* reqUrl)
{
	CURL *handle = make_handle(reqUrl);
	CURLcode res = curl_easy_perform(handle);
	CURLMsg *m = NULL;
	memory *mem;
	char* url;
	htmlDocPtr doc = NULL;

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
				int opts = HTML_PARSE_NOBLANKS | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NONET;
				doc = htmlReadMemory(mem->buf, mem->size, url, NULL, opts);
			}
		}
		else {
			fprintf(stderr, "HTTP %d: %s\n", (int) res_status, url);
		}
	}
	else {
		fprintf(stderr, "Connection failure: %s\n", url);
	}

	curl_easy_cleanup(handle);
	free(mem->buf);
	free(mem);

	curl_global_cleanup();

	return doc;
}

