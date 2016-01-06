#include "lrc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int lrc_init_n(lrc_t *lrc, int n_local, uint8_t *local_k_arr, int m) {

  int ret = 0;

  if (lrc->inited_ == 1) {
    return LRC_INIT_TWICE;
  }

  bzero(lrc, sizeof(*lrc));

  lrc->n_local = n_local;

  lrc->locals = malloc(sizeof(*lrc->locals) * lrc->n_local);
  if (lrc->locals == NULL) {
    ret = LRC_OUT_OF_MEMORY;
    goto exit;
  }

  lrc->k = 0;
  lrc->lrc_m = n_local - 1 + m;
  lrc->m = m;

  for (int i = 0; i < n_local; i++) {

    lrc->locals[i].start = lrc->k;
    lrc->locals[i].len = local_k_arr[i];

    lrc->k += local_k_arr[i];
  }

  lrc->n = lrc->k + lrc->lrc_m;

  /* matrix */
  lrc->matrix = lrc_make_matrix(lrc);
  if (lrc->matrix == NULL) {
    ret = LRC_OUT_OF_MEMORY;
    goto exit;
  }

  /* An error index that indicates all codes are damaged */
  lrc->code_erased = calloc(sizeof(lrc->code_erased[0]), lrc->n);
  if (lrc->code_erased == NULL) {
    ret = LRC_OUT_OF_MEMORY;
    goto exit;
  }

  for (int i = 0; i < lrc->lrc_m; i++) {
    lrc->code_erased[lrc->k + i] = 1;
  }

  lrc->inited_ = 1;

exit:

  if (ret != 0) {
    free(lrc->code_erased);
    free(lrc->locals);
    free(lrc->matrix);
  }

  return ret;
}

void lrc_destroy(lrc_t *lrc) {

  if (lrc->inited_ == 0) {
    return;
  }

  free(lrc->code_erased);
  free(lrc->locals);
  free(lrc->matrix);

  bzero(lrc, sizeof(*lrc));
}

int lrc_encode(lrc_t *lrc, lrc_buf_t *lb) {
  return lrc_decode(lrc, lb, lrc->code_erased);
}

int lrc_decode(lrc_t *lrc, lrc_buf_t *lb, int8_t *erased) {

  int ret = 0;
  lrc_decoder_t *dec = &(lrc_decoder_t) {0};

  ret = lrc_decoder_init(dec, lrc, lb, erased);
  if (ret != 0) {
    goto exit;
  }

  ret = lrc_decoder_decode(dec);

exit:

  lrc_decoder_destroy(dec);

  return ret;
}

int lrc_get_source(lrc_t *lrc, int8_t *erased, int8_t *source) {

  /* we need at least as many equations as erased chunks  */

  int n_erased = lrc_count_erased(lrc->n, erased);
  int ret = 0;

  for (int i = 0; i < lrc->n_local; i++) {

    int n = lrc_get_n_locally_erased(lrc, i, erased);
    if (n == 0) {
      continue;
    }

    n_erased--;

    /* local data for reconstruction */
    lrc_local_t *l = &lrc->locals[i];

    for (int j = l->start; j < l->start + l->len; j++) {
      source[j] = erased[j] == 0;
    }

    /* local code for reconstruction */
    int j = lrc->k + i;
    source[j] = erased[j] == 0;

  }

  if (n_erased > 0) {

    for (int i = 0; i < lrc->k; i++) {
      source[i] = (erased[i] == 0);
    }

    for (int i = lrc->k + lrc->n_local; i < lrc->n; i++) {

      source[i] = (erased[i] == 0);

      n_erased--;

      if (n_erased == 0) {
        break;
      }
    }
  }

  if (n_erased > 0) {
    ret = LRC_UNRECOVERABLE;
    goto exit;
  }

  lrc_debug_sources(lrc->n, source);

exit:

  return ret;
}

int *lrc_make_matrix(lrc_t *lrc) {
  /*
   * LRC Erasure Code:
   *  d0 d1   d2 d3   d4 d5
   * ------- ------- -------
   *   c1.1    c1.2    c1.3
   *
   * c1.1 c1.2 c1.3 are codes calculated from a sub set.
   * We have c1.1 ^ c1.2 ^ c.13 = c1
   * Because coefficient of the row 1 in a Vandermonde Matrix are always 1
   *
   *  k = 6, m = 3, lrc_m = 5
   *
   *  | 1 1 0 0 0 0 |     | d1 |     | c1.1 | |
   *  | 0 0 1 1 0 0 |     | d2 |     | c1.2 | | ^= c1
   *  | 0 0 0 0 1 1 |  X  | d3 |  =  | c1.3 | |
   *  | 1 2 4 8 * * |     | d4 |     | c2   |
   *  | 1 3 9 * * * |     | d5 |     | c3   |
   */

  int k = lrc->k;
  int m = lrc->m;
  int *matrix = NULL;
  int *lrc_matrix = NULL;

  matrix = reed_sol_vandermonde_coding_matrix(k, m, 8);
  if (matrix == NULL) {
    goto exit;
  }

  lrc_matrix = malloc(sizeof(int) * k * lrc->lrc_m);
  if (lrc_matrix == NULL) {
    goto exit;
  }

  bzero(lrc_matrix, sizeof(int) * k * lrc->lrc_m);

  for (int i = 0; i < lrc->n_local; i++) {

    lrc_local_t *l = &lrc->locals[i];

    for (int j = 0; j < l->len; j++) {
      lrc_matrix[i * k + l->start + j] = 1;
    }

  }

  for (int i = 1; i < m; i++) {
    for (int j = 0; j < k; j++) {
      lrc_matrix[(lrc->n_local - 1 + i)*k + j] = matrix[i * k + j];
    }
  }

exit:
  free(matrix);
  return lrc_matrix;
}

int lrc_get_n_locally_erased(lrc_t *lrc, int idx_local, int8_t *erased) {

  int start = lrc->locals[idx_local].start;
  int end = start + lrc->locals[idx_local].len;
  int n_damaged = 0;

  /* data in this region is damaged or its code is damaged */
  for (int i = start; i < end; i++) {
    if (erased[i] == 1) {
      n_damaged++;
    }
  }

  if (erased[lrc->k + idx_local]) {
    n_damaged++;
  }

  return n_damaged;
}

int lrc_count_erased(int n, int8_t *erased) {

  int en = 0;

  for (int i = 0; i < n; i++) {
    if (erased[i]) {
      en++;
    }
  }

  return en;
}

void lrc_debug_buf_line_(lrc_buf_t *lb, int n) {

  char *b;
  (void)b;

  dd("#%04d:", n);
  if (n < 0 || n >= lb->chunk_size) {
    dlog("--\n");
    return;
  }

  for (int i = 0; i < lb->n_data; i++) {
    b = lb->data[i];
    if (b[n] == 0) {
      dlog(" . ");
    } else {
      dlog("%02x ", (unsigned char)b[n]);
    }
  }

  dlog("| ");

  for (int i = 0; i < lb->n_code; i++) {
    b = lb->code[i];
    if (b[n] == 0) {
      dlog(" . ");
    } else {
      dlog("%02x ", (unsigned char)b[n]);
    }
  }

  dlog("\n");
}

void lrc_debug_matrix_(int *matrix, int row, int col) {

  dd("matrix:");

  for (int i = 0; i < row; i++) {

    for (int j = 0; j < col; j++) {

      int e = matrix[i * col + j];
      if (e == 0) {
        dlog(" . ");
      } else {
        dlog("%02x ", e);
      }
    }
    dlog("\n");
  }
}

void lrc_debug_sources_(int n, int8_t *source) {

  dd("source:");

  for (int i = 0; i < n; i++) {

    int8_t e = source[i];

    if (e == 0) {
      dlog(" . ");
    } else {
      dlog("%02x ", e);
    }
  }

  dlog("\n");
}

/* lrc_buf_t */

int lrc_buf_init(lrc_buf_t *lb, lrc_t *lrc, int64_t chunk_size) {

  int ret = 0;

  if (lb->inited_ == 1) {
    return LRC_INIT_TWICE;
  }

  bzero(lb, sizeof(*lb));

  lb->n_data = lrc->k;
  lb->n_code = lrc->lrc_m;
  lb->n = lb->n_data + lb->n_code;

  lb->chunk_size = chunk_size;
  lb->aligned_chunk_size = ALIGN_16(chunk_size);

  ret = posix_memalign((void **)&lb->buf, 16,
                       lb->aligned_chunk_size * lb->n);
  if (ret != 0) {
    goto exit;
  }

  for (int i = 0; i < lb->n; i++) {
    lb->data[i] = lb->buf + lb->aligned_chunk_size * i;
  }

  lb->code = &lb->data[lb->n_data];

  lb->buf_owned = 1;
  lb->inited_ = 1;

exit:

  if (ret != 0) {
    free(lb->buf);
  }

  return ret;
}

void lrc_buf_destroy(lrc_buf_t *lb) {

  if (lb == NULL || lb->inited_ == 0) {
    return;
  }

  if (lb->buf_owned == 1) {
    free(lb->buf);
  }

  bzero(lb, sizeof(*lb));
}

int lrc_buf_shadow(lrc_buf_t *lb, lrc_buf_t *src) {
  *lb = *src;
  lb->code = &lb->data[lb->n_data];
  lb->buf_owned = 0;
  return 0;
}

/* lrc decoder */

int lrc_decoder_init(lrc_decoder_t *dec, lrc_t *lrc, lrc_buf_t *lb, int8_t *erased) {

  /*
   * To a certain pattern of data loss, a specific matrix specific is required
   * to be created for decoding.
   * Matrix rows that do not cover lost data is removed.
   *
   * Because jerasure uses only first n_of_damaged row of the encoding matrix
   * to decode. It is not enough if lrc is used.
   *
   * For example with a encoding matrix 3*5:
   *   1 1 1 0 0
   *   0 0 0 1 1
   *   1 2 4 8 16
   * If data [0], [1], [2] are lost, row[0] contributes nothing to decode.
   */

  int k = lrc->k;
  int ret = 0;

  if (dec->inited_ == 1) {
    return LRC_INIT_TWICE;
  }

  bzero(dec, sizeof(*dec));

  dec->lrc = lrc;

  ret = lrc_buf_shadow(&dec->buf, lb);
  if (ret != 0) {
    goto exit;
  }

  ret = lrc_get_source(lrc, erased, dec->source);
  if (ret != 0) {
    goto exit;
  }

  /* only copy erased data. erased code will be remapped */
  for (int i = 0; i < lrc->k; i++) {
    dec->erased[i] = erased[i];
  }

  dec->decode_matrix = malloc(sizeof(int) * lrc->lrc_m * k);
  if (dec->decode_matrix == NULL) {
    ret = LRC_OUT_OF_MEMORY;
    goto exit;
  }

  int to = k;
  for (int i = lrc->k; i < lrc->n; i++) {
    if (dec->source[i] == 1 || erased[i] == 1) {
      dd("decoder map: %d -> %d", i, to);
      dec->buf.code[to - k] = lb->code[i - k];
      dec->erased[to] = erased[i];
      memcpy(&dec->decode_matrix[(to - k) * k], &lrc->matrix[(i - k) * k], sizeof(lrc->matrix[0]) * k);
      to++;
    }
  }

  dec->buf.n_code = to - k;
  dec->buf.n = dec->buf.n_data + dec->buf.n_code;
  dec->inited_ = 1;

  dd("\ndecoder inited:");
  lrc_debug_matrix(dec->decode_matrix, to - k, k);
  lrc_debug_sources(dec->lrc.n, dec->source);

exit:

  if (ret != 0) {
    free(dec->decode_matrix);
  }

  return ret;
}

void lrc_decoder_destroy(lrc_decoder_t *dec) {

  if (dec == NULL || dec->inited_ == 0) {
    return;
  }

  free(dec->decode_matrix);

  bzero(dec, sizeof(*dec));
}

int lrc_decoder_decode(lrc_decoder_t *dec) {

  lrc_buf_t *lb = &dec->buf;
  int erasures[512] = {0};
  int start = 0;

  for (int i = 0; i < dec->lrc->n; i++) {

    if (dec->erased[i] == 1) {

      erasures[start] = i;
      dd("erasures: %d", i);
      start++;
    }
  }
  erasures[start] = -1;

  return jerasure_matrix_decode(lb->n_data, lb->n_code, 8, dec->decode_matrix, 0,
                                erasures, lb->data, lb->code,
                                lb->chunk_size);
}

// vim:sw=2:fdl=0
