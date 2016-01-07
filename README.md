# LRC-EC

LRC(Local Reconstruction Codes) Erasure Code based on Reed-Solomon with Vandermonde matrix.

Erasure Code Algorithm makes it possible to achieve as high reliability(11 9s)
as 3-copy replication provides, with highly reduced storage overhead(130% against 300%).

But one of the problems with Erasure Code is the high IO consumption during
data reconstruction.
Normally to reconstruct `1` chunk it is required to read `n` chunks.
LRC is a trade-off between storage cost and IO cost.

With several additional `Coding` chunks calculated from subsets of `Data`
chunks, average IO consumption during reconstruction would be reduced to
`1 / number_of_local_sets`(normally 10% ~ 20%), as storage space would
increase by about 10%(depends on LRC policy).

**You can also just use this lib as a standard Erasure Code lib too.
Because LRC-EC is a generalized implementation of Erasure Code.**

# Synopsis

```c
#include "lrc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {

    int        size = 16;
    lrc_t     *lrc  = &(lrc_t) {0};
    lrc_buf_t *buf  = &(lrc_buf_t) {0};

    if (lrc_init_n(lrc, 2, (uint8_t[]) {2, 2}, 2) != 0) {
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
```

# Install

```shell
./configure
make install
```

# Try it!

```shell
cd test
gcc example.c -o example -llrc
./example
```

# API

## lrc_init_n

`int lrc_init_n(lrc_t *lrc, int n_local, uint8_t *local_arr, int m)`

Initializes LRC descriptor `lrc`.

Parameters:

* `lrc`
Pointer to a struct `lrc_t`. a `lrc_t` describes the parameters LRC to
generate codes.

* `n_local`
The number of local EC to create.

* `local_arr`
An array of number of data chunks in each local EC.

* `m`
TODO 


This is a specialized Erasure Code implementation for storage service.
What matrix to choose does not matter.
Because usually most CPU cycles are spent on matrix multiplication to decode lost data,
but not on finding reversed matrix.

In this implementation Vandermonde matrix is used.

TODO LRC parameters

# Analysis

In calculation, each TB of storage requires
`k * 0.13G` IO throuhput(both for network and disk drive) each day
to reconstruct lost data.
Wheren `k` is the number of members in a Erasure Code group.


