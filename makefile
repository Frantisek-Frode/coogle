build/coogle: src/coogle.c
	gcc -ggdb -I/usr/include/libxml2 coogle.c -o build/coogle -lcurl -lxml2

