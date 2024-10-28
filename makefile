build/coogle: src/main.c src/util.c src/sresults.c src/duck.c src/request.c
	gcc -ggdb -I/usr/include/libxml2 \
		src/util.c \
		src/sresults.c \
		src/duck.c \
		src/request.c \
		src/main.c -o build/coogle \
		-lcurl -lxml2

