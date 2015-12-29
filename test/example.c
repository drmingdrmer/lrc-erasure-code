#include "lrc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {

    int        chunk_size = 16;
    lrc_t     *lrc        = &(lrc_t) {0};
    lrc_buf_t *buf        = &(lrc_buf_t) {0};

    if (lrc_init_n(lrc, 2, (uint8_t[]) {2, 2}, 2) != 0) {
        exit(-1);
    }

    if (lrc_buf_init(buf, lrc, chunk_size) != 0) {
        exit(-1);
    }

    strcpy(buf->data[0], "hello");
    strcpy(buf->data[1], "world");
    strcpy(buf->data[2], "lrc");
    strcpy(buf->data[3], "ec");

    if (lrc_encode(lrc, buf) != 0) {
        exit(-1);
    }

    int8_t erased[2 + 2 + 3] = {1, 0, 0, 0, 0, 0};

    strcpy(buf->data[0], "*");

    printf("data[0] damaged: %s %s %s %s\n",
           buf->data[0], buf->data[1], buf->data[2], buf->data[3]);

    if (lrc_decode(lrc, buf, erased) != 0) {
        exit(-1);
    }

    printf("data[0] reconstructed: %s %s %s %s\n",
           buf->data[0], buf->data[1], buf->data[2], buf->data[3]);

    lrc_destroy(lrc);
    lrc_buf_destroy(buf);

    return 0;
}

// vim:sw=2:fdl=0
