# LRC-EC

LRC(Local Reconstruction Codes) Erasure Code based on Reed-Solomon with Vandermonde matrix.

Erasure Code Algorithm makes it possible to achieve as high reliability(11 9s)
as 3-copy replication provides, with highly reduced storage overhead(130% against 300%).

But one of the problems with Erasure Code is the high IO consumption during
data reconstruction.
Normally to reconstruct `1` chunk it requires to read `n` chunks.
LRC is a trade-off between storage cost and IO cost.

With several additional `Coding` chunks calculated from subsets of `Data`
chunks, average IO consumption caused by reconstruction would be reduced to
`1/l`, as storage space would increase by about 10%(depends on LRC policy).


# Synopsis
```
```

# Install

```shell
./configure
make install
```

# Try it!

```shell
cd test
gcc -llrc example.c -o example
./example
```

This is a specialized Erasure Code implementation for storage service.
What matrix to choose does not matter.
Because usually most CPU cycles are spent on matrix multiplication to decode lost data,
but not on finding reversed matrix.

In this implementation Vandermonde matrix is used.

TODO LRC parameters

