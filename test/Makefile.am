# GF-Complete 'test' AM file

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -O2 $(SIMD_FLAGS) -fPIC

noinst_PROGRAMS = test_lrc example

test_lrc_SOURCES = test_lrc.c
test_lrc_LDADD = ../src/liblrc.la

example_SOURCES = example.c
example_LDADD = ../src/liblrc.la
