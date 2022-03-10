LUAINC = -I ../lua

all : file.so

file.so : file.c
	gcc -g -Wall --shared -o $@ $^ $(LUAINC) -D FILE_TEST

clean :
	rm -f file.so
