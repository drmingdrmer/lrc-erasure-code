/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Zhang Yanpo (张炎泼) <drdr.xp@gmail.com>
 */

#include "lrc.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int gf_256_power_table[256] = {
  0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1d, 0x3a, 0x74, 0xe8, 0xcd, 0x87, 0x13, 0x26,
  0x4c, 0x98, 0x2d, 0x5a, 0xb4, 0x75, 0xea, 0xc9, 0x8f, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0,
  0x9d, 0x27, 0x4e, 0x9c, 0x25, 0x4a, 0x94, 0x35, 0x6a, 0xd4, 0xb5, 0x77, 0xee, 0xc1, 0x9f, 0x23,
  0x46, 0x8c, 0x05, 0x0a, 0x14, 0x28, 0x50, 0xa0, 0x5d, 0xba, 0x69, 0xd2, 0xb9, 0x6f, 0xde, 0xa1,
  0x5f, 0xbe, 0x61, 0xc2, 0x99, 0x2f, 0x5e, 0xbc, 0x65, 0xca, 0x89, 0x0f, 0x1e, 0x3c, 0x78, 0xf0,
  0xfd, 0xe7, 0xd3, 0xbb, 0x6b, 0xd6, 0xb1, 0x7f, 0xfe, 0xe1, 0xdf, 0xa3, 0x5b, 0xb6, 0x71, 0xe2,
  0xd9, 0xaf, 0x43, 0x86, 0x11, 0x22, 0x44, 0x88, 0x0d, 0x1a, 0x34, 0x68, 0xd0, 0xbd, 0x67, 0xce,
  0x81, 0x1f, 0x3e, 0x7c, 0xf8, 0xed, 0xc7, 0x93, 0x3b, 0x76, 0xec, 0xc5, 0x97, 0x33, 0x66, 0xcc,
  0x85, 0x17, 0x2e, 0x5c, 0xb8, 0x6d, 0xda, 0xa9, 0x4f, 0x9e, 0x21, 0x42, 0x84, 0x15, 0x2a, 0x54,
  0xa8, 0x4d, 0x9a, 0x29, 0x52, 0xa4, 0x55, 0xaa, 0x49, 0x92, 0x39, 0x72, 0xe4, 0xd5, 0xb7, 0x73,
  0xe6, 0xd1, 0xbf, 0x63, 0xc6, 0x91, 0x3f, 0x7e, 0xfc, 0xe5, 0xd7, 0xb3, 0x7b, 0xf6, 0xf1, 0xff,
  0xe3, 0xdb, 0xab, 0x4b, 0x96, 0x31, 0x62, 0xc4, 0x95, 0x37, 0x6e, 0xdc, 0xa5, 0x57, 0xae, 0x41,
  0x82, 0x19, 0x32, 0x64, 0xc8, 0x8d, 0x07, 0x0e, 0x1c, 0x38, 0x70, 0xe0, 0xdd, 0xa7, 0x53, 0xa6,
  0x51, 0xa2, 0x59, 0xb2, 0x79, 0xf2, 0xf9, 0xef, 0xc3, 0x9b, 0x2b, 0x56, 0xac, 0x45, 0x8a, 0x09,
  0x12, 0x24, 0x48, 0x90, 0x3d, 0x7a, 0xf4, 0xf5, 0xf7, 0xf3, 0xfb, 0xeb, 0xcb, 0x8b, 0x0b, 0x16,
  0x2c, 0x58, 0xb0, 0x7d, 0xfa, 0xe9, 0xcf, 0x83, 0x1b, 0x36, 0x6c, 0xd8, 0xad, 0x47, 0x8e, 0x01,
};

int gf_256_log_table[256] = {
  0x00, 0x00, 0x01, 0x19, 0x02, 0x32, 0x1a, 0xc6, 0x03, 0xdf, 0x33, 0xee, 0x1b, 0x68, 0xc7, 0x4b,
  0x04, 0x64, 0xe0, 0x0e, 0x34, 0x8d, 0xef, 0x81, 0x1c, 0xc1, 0x69, 0xf8, 0xc8, 0x08, 0x4c, 0x71,
  0x05, 0x8a, 0x65, 0x2f, 0xe1, 0x24, 0x0f, 0x21, 0x35, 0x93, 0x8e, 0xda, 0xf0, 0x12, 0x82, 0x45,
  0x1d, 0xb5, 0xc2, 0x7d, 0x6a, 0x27, 0xf9, 0xb9, 0xc9, 0x9a, 0x09, 0x78, 0x4d, 0xe4, 0x72, 0xa6,
  0x06, 0xbf, 0x8b, 0x62, 0x66, 0xdd, 0x30, 0xfd, 0xe2, 0x98, 0x25, 0xb3, 0x10, 0x91, 0x22, 0x88,
  0x36, 0xd0, 0x94, 0xce, 0x8f, 0x96, 0xdb, 0xbd, 0xf1, 0xd2, 0x13, 0x5c, 0x83, 0x38, 0x46, 0x40,
  0x1e, 0x42, 0xb6, 0xa3, 0xc3, 0x48, 0x7e, 0x6e, 0x6b, 0x3a, 0x28, 0x54, 0xfa, 0x85, 0xba, 0x3d,
  0xca, 0x5e, 0x9b, 0x9f, 0x0a, 0x15, 0x79, 0x2b, 0x4e, 0xd4, 0xe5, 0xac, 0x73, 0xf3, 0xa7, 0x57,
  0x07, 0x70, 0xc0, 0xf7, 0x8c, 0x80, 0x63, 0x0d, 0x67, 0x4a, 0xde, 0xed, 0x31, 0xc5, 0xfe, 0x18,
  0xe3, 0xa5, 0x99, 0x77, 0x26, 0xb8, 0xb4, 0x7c, 0x11, 0x44, 0x92, 0xd9, 0x23, 0x20, 0x89, 0x2e,
  0x37, 0x3f, 0xd1, 0x5b, 0x95, 0xbc, 0xcf, 0xcd, 0x90, 0x87, 0x97, 0xb2, 0xdc, 0xfc, 0xbe, 0x61,
  0xf2, 0x56, 0xd3, 0xab, 0x14, 0x2a, 0x5d, 0x9e, 0x84, 0x3c, 0x39, 0x53, 0x47, 0x6d, 0x41, 0xa2,
  0x1f, 0x2d, 0x43, 0xd8, 0xb7, 0x7b, 0xa4, 0x76, 0xc4, 0x17, 0x49, 0xec, 0x7f, 0x0c, 0x6f, 0xf6,
  0x6c, 0xa1, 0x3b, 0x52, 0x29, 0x9d, 0x55, 0xaa, 0xfb, 0x60, 0x86, 0xb1, 0xbb, 0xcc, 0x3e, 0x5a,
  0xcb, 0x59, 0x5f, 0xb0, 0x9c, 0xa9, 0xa0, 0x51, 0x0b, 0xf5, 0x16, 0xeb, 0x7a, 0x75, 0x2c, 0xd7,
  0x4f, 0xae, 0xd5, 0xe9, 0xe6, 0xe7, 0xad, 0xe8, 0x74, 0xd6, 0xf4, 0xea, 0xa8, 0x50, 0x58, 0xaf,
};


int lrc_init_n(lrc_t *lrc, int n_local, uint8_t *local_k_arr, int m, int64_t chunk_size) {

  int ret = -1;

  if (lrc->inited_ == 1) {
    return LRC_INIT_TWICE;
  }

  ret = lrc_param_init_n(&lrc->lrc_param, n_local, local_k_arr, m);
  if (ret != 0) {
    goto exit;
  }

  ret = lrc_buf_init(&lrc->lrc_buf, &lrc->lrc_param, chunk_size);
  if (ret != 0) {
    goto exit;
  }

  lrc->k = lrc->lrc_param.k;
  lrc->m = lrc->lrc_param.m;
  lrc->n = lrc->lrc_param.n;

  lrc->inited_ = 1;

exit:

  if (ret != 0) {
    lrc_param_destroy(&lrc->lrc_param);
    lrc_buf_destroy(&lrc->lrc_buf);
  }

  return ret;
}


void lrc_destroy(lrc_t *lrc) {

  if (lrc->inited_ == 0) {
    return;
  }

  lrc_param_destroy(&lrc->lrc_param);
  lrc_buf_destroy(&lrc->lrc_buf);
}


int lrc_encode(lrc_t *lrc) {
  return lrc_decode(lrc, lrc->lrc_param.code_erased);
}


int lrc_decode(lrc_t *lrc, int *erased) {

  int ret = 0;
  lrc_decoder_t *dec = &(lrc_decoder_t) {0};

  ret = lrc_decoder_init(dec, &lrc->lrc_param, &lrc->lrc_buf, erased);
  if (ret != 0) {
    goto exit;
  }

  ret = lrc_decoder_decode(dec);

exit:

  lrc_decoder_destroy(dec);

  return ret;
}


int lrc_param_init_n(lrc_param_t *lrc, int n_local, uint8_t *local_k_arr, int m) {

  int ret = 0;

  if (lrc->inited_ == 1) {
    return LRC_INIT_TWICE;
  }

  if (m < n_local) {
    return LRC_INVALID_M;
  }

  bzero(lrc, sizeof(*lrc));

  lrc->n_local = n_local;

  lrc->locals = malloc(sizeof(*lrc->locals) * lrc->n_local);
  if (lrc->locals == NULL) {
    ret = LRC_OUT_OF_MEMORY;
    goto exit;
  }

  lrc->k = 0;
  lrc->m = m;

  for (int i = 0; i < n_local; i++) {

    lrc->locals[i].start = lrc->k;
    lrc->locals[i].len = local_k_arr[i];

    lrc->k += local_k_arr[i];
  }

  lrc->n = lrc->k + lrc->m;

  /* matrix */
  lrc->matrix = lrc_param_make_matrix(lrc);
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

  for (int i = 0; i < lrc->m; i++) {
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


void lrc_param_destroy(lrc_param_t *lrc) {

  if (lrc->inited_ == 0) {
    return;
  }

  free(lrc->code_erased);
  free(lrc->locals);
  free(lrc->matrix);

  bzero(lrc, sizeof(*lrc));
}


int lrc_param_encode(lrc_param_t *lrc, lrc_buf_t *lb) {
  return lrc_param_decode(lrc, lb, lrc->code_erased);
}


int lrc_param_decode(lrc_param_t *lrc, lrc_buf_t *lb, int *erased) {

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


int lrc_param_get_source(lrc_param_t *lrc, int *erased, int *source) {

  /* we need at least as many equations as erased chunks  */

  int n_erased = lrc_count_erased(lrc->n, erased);
  int ret = 0;

  for (int i = 0; i < lrc->n_local; i++) {

    int n = lrc_param_get_n_locally_erased(lrc, i, erased);
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


int *lrc_param_make_matrix(lrc_param_t *lrc) {
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
   *  k = 6, n_local = 3, m = 5
   *
   *  | 1 1 0 0 0 0 |     | d1 |     | c1.1 | |
   *  | 0 0 1 1 0 0 |     | d2 |     | c1.2 | | ^= c1
   *  | 0 0 0 0 1 1 |  X  | d3 |  =  | c1.3 | |
   *  | 1 2 4 8 * * |     | d4 |     | c2   |
   *  | 1 3 9 * * * |     | d5 |     | c3   |
   */

  int k = lrc->k;
  int m = lrc->m;
  int *lrc_matrix = NULL;

  lrc_matrix = malloc(sizeof(int) * k * lrc->m);
  if (lrc_matrix == NULL) {
    goto exit;
  }

  lrc_init_vandermonde_matrix(&lrc_matrix[(lrc->n_local - 1) * k], m - lrc->n_local + 1, k);

  /* init the first lrc->n_local rows */
  bzero(lrc_matrix, sizeof(int) * k * lrc->n_local);

  for (int i = 0; i < lrc->n_local; i++) {

    lrc_local_t *l = &lrc->locals[i];

    for (int j = 0; j < l->len; j++) {
      lrc_matrix[i * k + l->start + j] = 1;
    }

  }

  dd("lrc distribution matrix");
  lrc_debug_matrix(lrc_matrix, lrc->m, k);

exit:
  return lrc_matrix;
}


int lrc_param_get_n_locally_erased(lrc_param_t *lrc, int idx_local, int *erased) {

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


int lrc_count_erased(int n, int *erased) {

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

  for (int i = 0; i < row; i++) {

    dlog("| ");

    for (int j = 0; j < col; j++) {

      int e = matrix[i * col + j];
      if (e == 0) {
        dlog(" . ");
      } else {
        dlog("%02x ", e);
      }
    }
    dlog(" |");
    dlog("\n");
  }
}


void lrc_debug_sources_(int n, int *source) {

  dlog("( ");
  for (int i = 0; i < n; i++) {

    int8_t e = source[i];

    if (e == 0) {
      dlog(" . ");
    } else {
      dlog("%02x ", e);
    }
  }

  dlog(" )\n");
}


/* lrc_buf_t */

int lrc_buf_init(lrc_buf_t *lb, lrc_param_t *lrc, int64_t chunk_size) {

  int ret = 0;

  if (lb->inited_ == 1) {
    return LRC_INIT_TWICE;
  }

  bzero(lb, sizeof(*lb));

  lb->n_data = lrc->k;
  lb->n_code = lrc->m;
  lb->n = lb->n_data + lb->n_code;

  lb->chunk_size = chunk_size;
  lb->aligned_chunk_size = lrc_align_16(chunk_size);

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

int lrc_decoder_init(lrc_decoder_t *dec, lrc_param_t *lrc, lrc_buf_t *lb, int *erased) {

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

  ret = lrc_param_get_source(lrc, erased, dec->source);
  if (ret != 0) {
    goto exit;
  }

  /* only copy erased data. erased code will be remapped */
  for (int i = 0; i < lrc->k; i++) {
    dec->erased[i] = erased[i];
  }

  dec->decode_matrix = malloc(sizeof(int) * lrc->m * k);
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
  lrc_debug_sources(dec->lrc->n, dec->source);

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

  dd("decoding..");
  return lrc_matrix_decode(lb->n_data, lb->n_code, dec->decode_matrix, dec->erased, lb->data,
                           lb->code, lb->chunk_size);
}

int lrc_matrix_decode(int k, int m, int *matrix, int *erased,
                      char **data_ptrs, char **coding_ptrs, int size) {

  int i;
  int *decoding_matrix, *valid_indexes;

  /* Find the number of data drives failed */

  valid_indexes = NULL;
  decoding_matrix = NULL;

  valid_indexes = malloc(sizeof(int) * k);
  if (valid_indexes == NULL) {
    return -1;
  }

  decoding_matrix = malloc(sizeof(int) * k * k);
  if (decoding_matrix == NULL) {
    free(valid_indexes);
    return -1;
  }

  lrc_debug_sources(k + m, erased);

  if (lrc_make_decoding_matrix(k, m, matrix, erased, decoding_matrix, valid_indexes) < 0) {
    free(valid_indexes);
    free(decoding_matrix);
    return -1;
  }

  dd("inversed matrix for decoding:");
  lrc_debug_matrix(decoding_matrix, k, k);

  /* Decode the data drives.
     If row_k_ones is true and coding device 0 is intact, then only decode edd-1 drives.
     This is done by stopping at lastdrive.
     We test whether edd > 0 so that we can exit the loop early if we're done.
   */

  for (i = 0; i < k; i++) {
    if (erased[i]) {
      jerasure_matrix_dotprod(k, 8, decoding_matrix + (i * k), valid_indexes, i, data_ptrs, coding_ptrs,
                              size);
      dd("reconstructed: %d", i);
    }
  }

  dd("Done reconstruct datas");

  /* Finally, re-encode any erased coding devices */

  for (i = 0; i < m; i++) {
    if (erased[k + i]) {
      jerasure_matrix_dotprod(k, 8, matrix + (i * k), NULL, i + k, data_ptrs, coding_ptrs, size);
    }
  }

  if (valid_indexes != NULL) { free(valid_indexes); }
  if (decoding_matrix != NULL) { free(decoding_matrix); }

  return 0;
}

int lrc_make_decoding_matrix(int k, int m, int *matrix, int *erased,
                                  int *decoding_matrix, int *valid_indexes) {
  int i, j, *tmpmat;

  j = 0;
  for (i = 0; j < k; i++) {
    if (erased[i] == 0) {
      valid_indexes[j] = i;
      j++;
    }
  }

  tmpmat = malloc(sizeof(int) * k * k);

  if (tmpmat == NULL) {
    return -1;
  }

  for (i = 0; i < k; i++) {

    if (valid_indexes[i] < k) {

      for (j = 0; j < k; j++) {
        tmpmat[i * k + j] = 0;
      }

      tmpmat[i * k + valid_indexes[i]] = 1;

    } else {

      for (j = 0; j < k; j++) {
        tmpmat[i * k + j] = matrix[(valid_indexes[i] - k) * k + j];
      }
    }
  }

  i = lrc_invert_matrix(tmpmat, decoding_matrix, k);
  free(tmpmat);
  return i;
}



void lrc_init_vandermonde_matrix(int *matrix, int rows, int cols) {

  int  i, j, k;

  for (i = 0; i < rows; i++) {

    k = 1;

    for (j = 0; j < cols; j++) {
      matrix[i * cols + j] = k;
      k = lrc_gf_mul(k, i + 1);
    }
  }

  dd("init vandermonde:");
  lrc_debug_matrix(matrix, rows, cols);
}

int lrc_invert_matrix(int *mat, int *inv, int rows) {

  int cols, i, j, k, x;
  int row_start;
  int row_start_2;
  int tmp;
  int inverse;

  cols = rows;

  bzero(inv, sizeof(int) * rows * cols);

  for (i = 0; i < rows; i++) {
    inv[i * cols + i] = 1;
  }

  /* First -- convert into upper triangular  */
  for (i = 0; i < cols; i++) {

    row_start = cols * i;

    /* Swap rows if we ave a zero i,i element.  If we can't swap, then the
     * matrix was not invertible  */

    if (mat[row_start + i] == 0) {

      /* find the first non-zero mat[j, i] */

      for (j = i + 1; j < rows && mat[cols * j + i] == 0; j++) {
        /* void */
      }

      if (j == rows) {
        return -1;
      }

      row_start_2 = j * cols;

      for (k = 0; k < cols; k++) {
        lrc_swap(mat[row_start + k], mat[row_start_2 + k]);
        lrc_swap(inv[row_start + k], inv[row_start_2 + k]);
      }
    }


    /* Multiply the row by 1/element i,i  */
    tmp = mat[row_start + i];

    if (tmp != 1) {

      inverse = lrc_gf_div(1, tmp);

      for (j = 0; j < cols; j++) {
        mat[row_start + j] = lrc_gf_mul(mat[row_start + j], inverse);
        inv[row_start + j] = lrc_gf_mul(inv[row_start + j], inverse);
      }
    }

    /* Now for each j>i, add A_ji*Ai to Aj  */

    for (j = i + 1; j < rows; j++) {

      k = j * cols + i;

      if (mat[k] == 0) {
        continue;
      }

      row_start_2 = j * cols;

      tmp = mat[k];

      for (x = i; x < cols; x++) {
        mat[row_start_2 + x] ^= lrc_gf_mul(tmp, mat[row_start + x]);
        inv[row_start_2 + x] ^= lrc_gf_mul(tmp, inv[row_start + x]);
      }

    }
  }

  /* Now the matrix is upper triangular.  Start at the top and multiply down  */

  for (i = rows - 1; i >= 0; i--) {

    row_start = i * cols;

    for (j = 0; j < i; j++) {

      row_start_2 = j * cols;

      if (mat[row_start_2 + i] != 0) {

        tmp = mat[row_start_2 + i];
        mat[row_start_2 + i] = 0;

        for (k = 0; k < cols; k++) {
          inv[row_start_2 + k] ^= lrc_gf_mul(tmp, inv[row_start + k]);
        }
      }
    }
  }

  return 0;
}


int lrc_gf_mul(int x, int y) {

  if (x == 0 || y == 0) {
    return 0;
  }

  int lg = gf_256_log_table[x] + gf_256_log_table[y];
  return lrc_gf_power(lg);
}


int lrc_gf_div(int x, int y) {

  if (x == 0) {
    return 0;
  }

  if (y == 0) {
    return -1;
  }

  int lg = gf_256_log_table[x] - gf_256_log_table[y];
  return lrc_gf_power(lg);
}


int lrc_gf_power(int x) {
  return gf_256_power_table[x % 255];
}



// vim:sw=2:fdl=0
