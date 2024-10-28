#include <curl/curl.h>
#include <libxml/HTMLparser.h>

htmlDocPtr Get(char* reqUrl);
CURL *make_handle(char *url);

