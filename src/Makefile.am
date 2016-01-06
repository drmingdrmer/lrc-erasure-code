AM_CPPFLAGS = -I$(top_srcdir)/include
AM_CFLAGS = -O2 $(SIMD_FLAGS) -fPIC

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = liblrc.la
liblrc_la_SOURCES = \
                  gf.c  \
                  gf_method.c  \
                  gf_wgen.c  \
                  gf_w4.c  \
                  gf_w8.c  \
                  gf_w16.c  \
                  gf_w32.c  \
                  gf_w64.c  \
                  gf_w128.c  \
                  gf_rand.c  \
                  gf_general.c  \
                  cauchy.c \
                  galois.c \
                  liberation.c \
                  reed_sol.c \
                  jerasure.c \
                  lrc.c

if HAVE_NEON
liblrc_la_SOURCES += neon/gf_w4_neon.c  \
                    neon/gf_w8_neon.c  \
                    neon/gf_w16_neon.c \
                    neon/gf_w32_neon.c \
                    neon/gf_w64_neon.c
endif

# vim:expandtab
