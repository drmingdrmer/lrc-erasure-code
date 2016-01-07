#include "lrc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define eq(a, b, fmt, ...) do {                                               \
    typeof(a) __a = (a);                                                      \
    typeof(b) __b = (b);                                                      \
    if (__a == __b) {                                                         \
      dd("OK: %lld == %lld, " fmt,                                            \
         (long long)__a, (long long)__b, ##__VA_ARGS__);                      \
    } else {                                                                  \
      printf("Failure: expected: %lld, but: %lld " fmt "\n",                  \
             (long long)__a, (long long)__b, ##__VA_ARGS__);                  \
      exit(1);                                                                \
    }                                                                         \
  } while (0)

int8_t *_to_erased(int *erasures) {

  static int8_t erased[512];

  bzero(erased, sizeof(erased));

  for (int i = 0; erasures[i] != -1; i++) {
    erased[erasures[i]] = 1;
  }

  return erased;
}

static void _corrupt(lrc_buf_t *lb, char *saved, int8_t *erased) {

  for (int i = 0; i < lb->n; i++) {
    saved[i] = lb->data[i][0];
  }

  for (int i = 0; i < lb->n; i++) {
    if (erased[i]) {
      lb->data[i][0] = 0;
    }
  }

  dd("corrupted:");
  lrc_debug_buf_line(lb, 0);
}

static void _restore(lrc_buf_t *lb, char *saved) {

  for (int i = 0; i < lb->n; i++) {
    lb->data[i][0] = saved[i];
  }
}

static void _check_reconstructed(lrc_buf_t *lb, char *saved, int8_t *erased) {

  for (int i = 0; i < lb->n; i++) {

    if (erased[i] == 1) {
      eq(saved[i], lb->data[i][0], "reconstructed: idx=%d", i);
    }
  }
}

static void _init_test_buf(lrc_t *lrc, lrc_buf_t *lb, int64_t chunk_size) {

  int ret = 0;

  ret = lrc_buf_init(lb, lrc, chunk_size);
  eq(0, ret, "lrc_buf_init");

  for (int i = 0; i < lrc->n; i++) {
    char *b = lb->data[i];
    memset(b, i + 1, chunk_size);
  }

  eq(0, lrc_encode(lrc, lb), "construct codes");
}

# define die_if_err(ret, mes) if (ret != 0) { printf("error "mes": %d\n", ret); exit(1); }

int test_basic_2_2_2() {

  int        ret      = 0;
  int        chunk_size = 16;
  lrc_t     *lrc      = &(lrc_t) {0};
  lrc_buf_t *buf_enc  = &(lrc_buf_t) {0};

  /*
   * ec (2, 2) + 3:
   *
   * d0, d1 -> c1.1
   * d2, d3 -> c1.2
   * d0, d1, d2, d3 -> c2
   *
   * c1.1 ^ c1.2 = c1
   *
   * we do not save c1.
   */

  ret = lrc_init_n(lrc, 2, (uint8_t[]) {2, 2}, 2);
  die_if_err(ret, "init lrc");

  /* -- construct code from data -- */

  ret = lrc_buf_init(buf_enc, lrc, chunk_size);
  die_if_err(ret, "init lrc_buf_t");

  strcpy(buf_enc->data[0], "hello");
  strcpy(buf_enc->data[1], "word");
  strcpy(buf_enc->data[2], "lrc");
  strcpy(buf_enc->data[3], "ec");

  ret = lrc_encode(lrc, buf_enc);
  die_if_err(ret, "encoding");

  /* -- erase data[0] and reconstruct -- */

  lrc_buf_t *buf_dec = &(lrc_buf_t) {0};

  ret = lrc_buf_init(buf_dec, lrc, chunk_size);
  die_if_err(ret, "init buf_dec");

  int8_t erased[2 + 2 + 3] = {1, 0, 0, 0, 0, 0};

  buf_dec->data[0][0] = '*';
  buf_dec->data[0][1] = '\0';

  /* find out which data/code are required by reconstruction */
  int8_t sources[2 + 2 + 3] = {0};

  ret = lrc_get_source(lrc, erased, sources);
  die_if_err(ret, "get reconstruction source");

  /* prepare data for reconstruction */
  for (int i = 0; i < lrc->n; i++) {
    if (sources[i] == 1) {
      memcpy(buf_dec->data[i], buf_enc->data[i], chunk_size);
    }
  }

  printf("data[0] damaged: %s\n", buf_dec->data[0]);

  ret = lrc_decode(lrc, buf_dec, erased);
  die_if_err(ret, "decode");

  printf("data[0] reconstructed: %s\n", buf_dec->data[0]);

  /* free memory allocated */
  lrc_destroy(lrc);
  lrc_buf_destroy(buf_enc);
  lrc_buf_destroy(buf_dec);

  return 0;

}

#undef die_if_err

int test_sources_2_2_2() {

  lrc_t *lrc = &(lrc_t) {0};
  int   ret = 0;

  ret = lrc_init(lrc, k(2, 2), 3);
  eq(0, ret, "lrc_init");
  eq(3, lrc->m, "m");
  eq(7, lrc->n, "n");
  eq(0, ret, "lrc_init");

  struct { int8_t erased[7]; int r; int8_t sources[7]; } cases[] = {
    {{0, 0, 0, 0,   0, 0, 0},                 0, {0, 0, 0, 0,   0, 0, 0}},
    {{1, 0, 0, 0,   0, 0, 0},                 0, {0, 1, 0, 0,   1, 0, 0}},
    {{0, 0, 0, 0,   1, 0, 0},                 0, {1, 1, 0, 0,   0, 0, 0}},
    {{0, 1, 0, 0,   0, 0, 0},                 0, {1, 0, 0, 0,   1, 0, 0}},
    {{1, 1, 0, 0,   0, 0, 0},                 0, {0, 0, 1, 1,   1, 0, 1}},
    {{1, 0, 1, 0,   0, 0, 0},                 0, {0, 1, 0, 1,   1, 1, 0}},
    {{1, 0, 1, 1,   0, 0, 0},                 0, {0, 1, 0, 0,   1, 1, 1}},
    {{1, 1, 1, 1,   0, 0, 0}, LRC_UNRECOVERABLE, {0, 0, 0, 0,   0, 0, 0}},
  };

  for (int i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {

    typeof(cases[i]) c = cases[i];

    int8_t src[7] = {0};

    ret = lrc_get_source(lrc, c.erased, src);
    eq(c.r, ret, "ret");

    if (ret == 0) {
      for (int j = 0; j < 7; j++) {
        eq(c.sources[j], src[j], "check source[%d]", j);
      }
    }
  }

  lrc_destroy(lrc);
  return 0;
}

int test_12_4() {

  lrc_t lrc = {0};
  lrc_buf_t *lb = &(lrc_buf_t) {0};
  int8_t erased[512] = {0};
  char saved[512];
  int ret = 0;

  ret = lrc_init(&lrc, k(12), 4);
  eq(0, ret, "lrc_init");

  _init_test_buf(&lrc, lb, 1);

  for (int a = 0; a < lrc.n; a++) {

    for (int b = a + 1; b < lrc.n; b++) {

      for (int c = b + 1; c < lrc.n; c++) {

        for (int d = c + 1; d < lrc.n; d++) {

          bzero(erased, sizeof(erased));

          erased[a] = 1;
          erased[b] = 1;
          erased[c] = 1;
          erased[d] = 1;

          _corrupt(lb, saved, erased);

          ret = lrc_decode(&lrc, lb, erased);
          eq(0, ret, "reconstruct, %d, %d, %d, %d", a, b, c, d);

          lrc_debug_buf_line(lb, 0);
          _check_reconstructed(lb, saved, erased);

        }
      }
    }
  }

  lrc_buf_destroy(lb);
  lrc_destroy(&lrc);
  return 0;
}

int test_lrc_6_6_3() {

  lrc_t     *lrc = &(lrc_t) {0};
  lrc_buf_t *lb  = &(lrc_buf_t) {0};
  int        ret = 0;
  char       saved[512];

  ret = lrc_init(lrc, k(6, 6), 4);
  eq(4, lrc->m, "m");
  eq(16, lrc->n, "n");
  eq(0, ret, "lrc_init");

  _init_test_buf(lrc, lb, 1);
  lrc_debug_buf_line(lb, 0);

  struct { int erasures[255]; int r; } cases[] = {
    {{0, -1}, 0},
    {{1, -1}, 0},
    {{5, -1}, 0},
    {{6, -1}, 0},
    {{12, -1}, 0},
    {{13, -1}, 0},
    {{14, -1}, 0},
    {{15, -1}, 0},
    {{0, 6, -1}, 0},
    {{0, 1, 2, -1}, 0},
    {{0, 1, 2, 6, -1}, 0},
    {{0, 1, 6, 7, -1}, 0},

    {{0, 1, 2, 3, -1}, LRC_UNRECOVERABLE},
    {{0, 1, 14, 15, -1}, LRC_UNRECOVERABLE},
  };

  for (int i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {

    typeof(cases[i]) c = cases[i];

    int8_t *erased = _to_erased(c.erasures);
    int8_t  source[512] = {0};

    ret = lrc_get_source(lrc, erased, source);
    eq(c.r, ret, "get reconstruction source");
    if (ret != 0) {
      continue;
    }

    _corrupt(lb, saved, erased);

    /* remove unrequired chunks */
    for (int j = 0; j < lrc->n; j++) {
      if (source[j] == 0) {
        lb->data[j][0] = 0;
      }
    }

    dd("data required to reconstruct:");
    lrc_debug_buf_line(lb, 0);

    ret = lrc_decode(lrc, lb, erased);

    dd("reconstructed:");
    lrc_debug_buf_line(lb, 0);

    eq(0, ret, "decode");

    if (ret == 0) {
      _check_reconstructed(lb, saved, erased);
    }

    _restore(lb, saved);
  }

  lrc_buf_destroy(lb);
  lrc_destroy(lrc);
  return 0;
}

int test_lrc_6_6_3_count_all() {

  lrc_t lrc = {0};
  lrc_buf_t *lb = &(lrc_buf_t) {0};
  int8_t erased[512] = {0};
  char saved[512];
  int ret = 0;
  int n_reconsruct = 0;
  int n_ok = 0;

  ret = lrc_init(&lrc, k(6, 6), 4);
  eq(12, lrc.k, "k");
  eq(4, lrc.m, "m");
  eq(16, lrc.n, "n");
  eq(0, ret, "lrc_init");

  _init_test_buf(&lrc, lb, 1);

  lrc_debug_buf_line(lb, 0);

  for (int a = 0; a < lrc.n; a++) {

    for (int b = a + 1; b < lrc.n; b++) {

      for (int c = b + 1; c < lrc.n; c++) {

        for (int d = c + 1; d < lrc.n; d++) {

          bzero(erased, sizeof(erased));

          erased[a] = 1;
          erased[b] = 1;
          erased[c] = 1;
          erased[d] = 1;

          _corrupt(lb, saved, erased);

          n_reconsruct ++;
          ret = lrc_decode(&lrc, lb, erased);

          lrc_debug_buf_line(lb, 0);

          if (ret == 0) {
            n_ok ++;
            _check_reconstructed(lb, saved, erased);
          }

          _restore(lb, saved);

        }
      }
    }
  }

  /* there are only 87% pattern of data loss that can be reconstructed with lrc-ec 6, 6, 3 */
  eq(1820, n_reconsruct, "nr reconstruct");
  eq(1568, n_ok, "nr ok");

  lrc_buf_destroy(lb);
  lrc_destroy(&lrc);
  return 0;
}

int bench_12_4() {

  lrc_t lrc = {0};
  lrc_buf_t *lb = &(lrc_buf_t) {0};

  int8_t erased[512] = {0};
  int64_t n = 1000;
  int64_t chunk_size = 1024 * 1024;
  int ret = 0;

  ret = lrc_init(&lrc, k(12), 4);
  eq(0, ret, "lrc_init");

  _init_test_buf(&lrc, lb, chunk_size);

  erased[0] = 1;
  erased[1] = 1;
  erased[2] = 1;
  erased[3] = 1;

  struct timeval t0;
  struct timeval t1;
  int64_t spent_usec;
  char saved[512];

  _corrupt(lb, saved, erased);

  gettimeofday(&t0, NULL);

  for (int64_t i = 0; i < n; i++) {
    ret = lrc_decode(&lrc, lb, erased);
    eq(0, ret, "reconstruct, 1234");
  }

  gettimeofday(&t1, NULL);

  lrc_debug_buf_line(lb, 0);
  _check_reconstructed(lb, saved, erased);

  spent_usec = (t1.tv_sec - t0.tv_sec);
  spent_usec = spent_usec * 1000 * 1000 + (t1.tv_usec - t0.tv_usec);

  time_t ns_per_call = spent_usec * 1000 / n;
  printf("runs %llu times, costs %lld mseconds,"
         " calls/s: %llu, ns/call(10e-9): %lu"
         " %lld MB/s\n"
         ,
         (long long)n,
         (long long)spent_usec / 1000,
         (long long)n * 1000 * 1000 / spent_usec,
         ns_per_call,
         (long long)n * chunk_size * 1000 * 1000 / 1024 / 1024 / spent_usec
        );

  lrc_buf_destroy(lb);
  lrc_destroy(&lrc);
  return 0;
}

int main(int argc, char **argv) {

  test_basic_2_2_2();
  test_sources_2_2_2();
  test_12_4();
  test_lrc_6_6_3();
  test_lrc_6_6_3_count_all();
  /* bench_12_4(); */

  return 0;
}

// vim:sw=2:fdl=0
