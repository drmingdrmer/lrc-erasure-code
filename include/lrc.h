/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Zhang Yanpo (张炎泼) <drdr.xp@gmail.com>
 */

#ifndef LRC_EC_LRC_
#define LRC_EC_LRC_

#include <stdint.h>

/* #define LRC_DEBUG 1 */

#define LRC_OUT_OF_MEMORY (-1)
#define LRC_UNRECOVERABLE (-2)
#define LRC_INIT_TWICE    (-3)
#define LRC_INVALID_M     (-4)

#ifdef LRC_DEBUG
#   define dd( _fmt, ... )   fprintf( stderr, _fmt "\n", ##__VA_ARGS__)
#   define dlog( _fmt, ... ) fprintf( stderr, _fmt,      ##__VA_ARGS__)
#   define lrc_debug_buf_line(...) lrc_debug_buf_line_( __VA_ARGS__ )
#   define lrc_debug_matrix(...) lrc_debug_matrix_( __VA_ARGS__ )
#   define lrc_debug_sources(...) lrc_debug_sources_( __VA_ARGS__ )
#else
#   define dd( _fmt, ... )
#   define dlog( _fmt, ... )
#   define lrc_debug_buf_line(...)
#   define lrc_debug_matrix(...)
#   define lrc_debug_sources(...)
#endif /* LRC_DEBUG */

#define lrc_align_16(val) (((val - 1) / 16 + 1) * 16)

#define _lrc_concat(a, b) a ## b

#define _lrc_n_arr_k(...) (sizeof((uint8_t[]){__VA_ARGS__}) / sizeof(uint8_t)), \
  (uint8_t[]){__VA_ARGS__}

/* k_param is in form: 'k(2, 3, 4)'
 *
 * expansion of thie macro:
 *
 *      lrc_init(lrc, k(2, 3), 2)
 * ->   lrc_init_n(lrc, _lrc_concat(_lrc_n_arr_, k(2, 3), 2)
 * ->   lrc_init_n(lrc, _lrc_n_arr_ ## k(2, 3), 2)
 * ->   lrc_init_n(lrc, _lrc_n_arr_k(2, 3), 2)
 * ->   lrc_init_n(lrc, (sizeof((uint8_t[]){2, 3}) / sizeof(uint8_t)), (uint8_t[]){2, 3}, 2)
 * ->   lrc_init_n(lrc, 2, (uint8_t[]){2, 3}, 2)
 */
#define lrc_init(lrc, k_param, m) \
  lrc_init_n((lrc), _lrc_concat(_lrc_n_arr_, k_param), (m))


extern int *reed_sol_vandermonde_coding_matrix(int k, int m, int w);
extern int jerasure_matrix_decode(int k, int m, int w,
                                  int *matrix, int row_k_ones, int *erasures,
                                  char **data_ptrs, char **coding_ptrs, int size);

typedef struct {
  uint8_t start;
  uint8_t len;
} lrc_local_t;

typedef struct {

  int       n_data;
  int       n_code;
  int       n;                    /* n_data + n_code */

  char     *data[512];
  char    **code;

  int64_t   chunk_size;
  int64_t   aligned_chunk_size;
  char     *buf;

  int8_t    buf_owned;
  int8_t    inited_;

} lrc_buf_t;

typedef struct {

  int          k;             /* nr of data                                  */
  int          m;             /* nr of code of original reed-solomon ec      */
  int          n;             /* total number of chunks: k + m               */

  int          n_local;       /* nr of local EC                              */
  lrc_local_t *locals;        /* start index and nr of elts of each local EC */

  int         *matrix;        /* ecoding matrix m *k                         */
  int8_t      *code_erased;   /* for encode                                  */

  int8_t       inited_;

} lrc_t;

typedef struct {

  lrc_t     *lrc;
  lrc_buf_t  buf;

  int8_t     erased[512];     /* array of index of erased data/code                        */
  int8_t     source[512];     /* data/code indexes those are required to reconstruct       */
  int       *decode_matrix;   /* matrix with damaged data-row/unnecessary code-row removed */

  int8_t     inited_;

} lrc_decoder_t;

int  lrc_init_n(lrc_t *lrc, int n_local, uint8_t *local_k_arr, int m);
void lrc_destroy(lrc_t *lrc);
int  lrc_encode(lrc_t *lrc, lrc_buf_t *lb);
int  lrc_decode(lrc_t *lrc, lrc_buf_t *lb, int8_t *erased);
int  lrc_get_source(lrc_t *lrc, int8_t *erased, int8_t *source);

int *lrc_make_matrix(lrc_t *lrc);
int  lrc_get_n_locally_erased(lrc_t *lrc, int idx_local, int8_t *erased);
int  lrc_count_erased(int n, int8_t *erased);

void lrc_debug_buf_line_(lrc_buf_t *lb, int n);
void lrc_debug_matrix_(int *matrix, int row, int col);
void lrc_debug_sources_(int n, int8_t *source);

int  lrc_buf_init(lrc_buf_t *lb, lrc_t *lrc, int64_t chunk_size);
void lrc_buf_destroy(lrc_buf_t *lb);
int  lrc_buf_shadow(lrc_buf_t *lb, lrc_buf_t *src);

int  lrc_decoder_init(lrc_decoder_t *dec, lrc_t *lrc, lrc_buf_t *lb, int8_t *erased);
void lrc_decoder_destroy(lrc_decoder_t *dec);
int  lrc_decoder_decode(lrc_decoder_t *dec);

#endif /* LRC_EC_LRC_ */
// vim:sw=2:fdl=1
