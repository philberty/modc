bin_PROGRAMS= cmod$(EXEEXT)
noinst_LIBRARIES = libparser.a

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_YFLAGS = -d

cmod_SOURCES = \
	mm-cmod.c \
	dd-util.c \
	dd-vec.c \
	ii-toplev.c \
	ii-mod-dot.c \
	bb-unix-i386.c

cmod_LDADD  = libparser.a

libcparser_a_YFLAGS = -d
libparser_a_CFLAGS = -DMOD -DPARSER
libparser_a_SOURCES = ss-parser.y ss-lexer.l
