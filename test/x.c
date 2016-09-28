#include "lrc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* gcc example.c -llrc */

int main(int argc, char **argv) {

  int k, m, i;
  int        size = 30;
  lrc_t     *lrc  = &(lrc_t) {0};
  lrc_buf_t *buf  = &(lrc_buf_t) {0};

  if (lrc_init_n(lrc, 2, (uint8_t[]) {5, 5}, 4) != 0) {
    exit(-1);
  }

  if (lrc_buf_init(buf, lrc, size) != 0) {
    exit(-1);
  }

  strcpy(buf->data[0], "hello");
  strcpy(buf->data[1], "world");
  strcpy(buf->data[2], "lrc");
  strcpy(buf->data[3], "ec");
  strcpy(buf->data[4], "iam");
  strcpy(buf->data[5], "xiao");
  strcpy(buf->data[6], "kyle");
  strcpy(buf->data[7], "is");
  strcpy(buf->data[8], "here");
  strcpy(buf->data[9], "ec-end");

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

  int8_t erased[5 + 5 + 4] = {
      0, 0, 1, 1, 0,
      1, 0, 1, 0, 0,
      0, 0, 0, 0
  };

  /*
   * strcpy(buf->data[2], "*");
   * strcpy(buf->data[3], "*");
   * strcpy(buf->data[5], "*");
   * strcpy(buf->data[7], "*");
   */

  printf("damaged: %s %s %s %s %s  %s %s %s %s %s\n",
         buf->data[0], buf->data[1], buf->data[2], buf->data[3], buf->data[4],
         buf->data[5], buf->data[6], buf->data[7], buf->data[8], buf->data[9]
  );

  if (lrc_decode(lrc, buf, erased) != 0) {
    exit(-1);
  }

  printf("reconstructed: %s %s %s %s %s  %s %s %s %s %s\n",
         buf->data[0], buf->data[1], buf->data[2], buf->data[3], buf->data[4],
         buf->data[5], buf->data[6], buf->data[7], buf->data[8], buf->data[9]
         );


  lrc_destroy(lrc);
  lrc_buf_destroy(buf);

  return 0;
}

// vim:sw=2:fdl=0
