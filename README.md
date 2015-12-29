# lrc-ec-rs-v
 LRC(Local Reconstruction Codes) Erasure Code based on Reed-Solomon with Vandermonde matrix.
 
 This is a specialized Erause Code implementation for storage service.
 What matrix to choose does not matter.
 Because usually most CPU cycles are spent on matrix multiplication to decode lost data,
 but not on finding reversed matrix.
 
 In this implementation Vandermonde matrix is used.
 
 TODO LRC parameters

