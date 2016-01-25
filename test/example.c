#include "lrc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* gcc example.c -llrc */

int main(int argc, char **argv) {

  int k, m, i;
  int        size = 8;
  lrc_t     *lrc  = &(lrc_t) {0};
  lrc_buf_t *buf  = &(lrc_buf_t) {0};

  /* if (lrc_init_n(lrc, 2, (uint8_t[]) {4, 4}, 5) != 0) { */
  if (lrc_init_n(lrc, 1, (uint8_t[]) {3}, 2) != 0) {
    exit(-1);
  }

  if (lrc_buf_init(buf, lrc, size) != 0) {
    exit(-1);
  }

  strcpy(buf->data[0], "hello");
  strcpy(buf->data[1], "world");
  strcpy(buf->data[2], "lrc");
  strcpy(buf->data[3], "ec");

  if (lrc_encode(lrc, buf) != 0) {
    exit(-1);
  }

  for (k = 0; k < lrc->k; k++) {
    printf("data[%d]: ", k);
    for (i = 0; i < size; i++) {
      printf("%02x ", (uint8_t)buf->data[k][i]);
    }
    printf("\n");
  }

  for (m = 0; m < lrc->m; m++) {
    printf("code[%d]: ", m);
    for (i = 0; i < size; i++) {
      printf("%02x ", (uint8_t)buf->code[m][i]);
    }
    printf("\n");
  }

  int8_t erased[2 + 2 + 3] = {1, 0, 0, 0, 0, 0};

  strcpy(buf->data[0], "*");

  printf("damaged: %s %s %s %s\n",
         buf->data[0], buf->data[1], buf->data[2], buf->data[3]);

  if (lrc_decode(lrc, buf, erased) != 0) {
    exit(-1);
  }

  printf("reconstructed: %s %s %s %s\n",
         buf->data[0], buf->data[1], buf->data[2], buf->data[3]);

  lrc_destroy(lrc);
  lrc_buf_destroy(buf);

  return 0;
}

// vim:sw=2:fdl=0
