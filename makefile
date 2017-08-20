lib/bs/libTruetypebindings.o:
	ocamlc -ccopt $(shell freetype-config --cflags) -ccopt -o -ccopt lib/bs/libTruetypebindings.o vendor/camlimages/src/ftintf.c
