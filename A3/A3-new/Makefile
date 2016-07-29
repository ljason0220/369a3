CC = gcc
CFLAGS = -g -Wall 

all: ext2_ls ext2_cp ext2_mkdir ext2_ln ext2_rm

ext2_ls: ext2_ls.o helper_functions.o
	$CC ${CFLAGS} -o $@ $^

ext2_cp: ext2_cp.o helper_functions.o
	$CC ${CFLAGS} -o $@ $^

ext2_mkdir: ext2_mkdir.o helper_functions.o
	$CC ${CFLAGS} -o $@ $^

ext2_ln: ext2_ln.o helper_functions.o
	$CC ${CFLAGS} -o $@ $^

ext2_rm: ext2_rm.o helper_functions.o
	$CC ${CFLAGS} -o $@ $^

%.o: %.c ext2.h helper.h
	$CC ${CFLAGS} -c $<

clean :
	rm -f -v *o ext2_ls ext2_cp ext2_mkdir ext2_ln ext2_rm


