#include "lrc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* gcc example.c -llrc */

int chunk_size = 16;

void print_chunk(char **chunks, int n, char *type) {

  int i, j;
  for (i = 0; i < n; i++) {
    printf("%s[%d]: ", type, i);
    for (j = 0; j < chunk_size; j++) {
      printf("%02x ", (uint8_t)chunks[i][j]);
    }
    printf("\n");
  }
}

int main(int argc, char **argv) {

  lrc_t *lrc = &(lrc_t)lrc_null;

  if (lrc_init(lrc, k(2, 2), 3, chunk_size) != 0) {
    exit(-1);
  }

  char **datas = lrc->lrc_buf.data;
  char **codes = lrc->lrc_buf.code;

  strcpy(datas[0], "hello");
  strcpy(datas[1], "world");
  strcpy(datas[2], "lrc");
  strcpy(datas[3], "ec");

  if (lrc_encode(lrc) != 0) {
    exit(-1);
  }

  print_chunk(datas, lrc->k, "data");
  print_chunk(codes, lrc->m, "code");

  strcpy(datas[0], "*");
  printf("damaged: %s %s %s %s\n", datas[0], datas[1], datas[2], datas[3]);

  int erased[2 + 2 + 3] = {
    1, 0,
    0, 0,
    0, 0, 0
  };

  if (lrc_decode(lrc, erased) != 0) {
    exit(-1);
  }

  printf("reconstructed: %s %s %s %s\n", datas[0], datas[1], datas[2], datas[3]);

  lrc_destroy(lrc);
  return 0;
}

// vim:sw=2:fdl=0
