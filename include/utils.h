// utils.h
#ifndef UTILS_H
#define UTILS_H

// necessary includes across files

#include <stdio.h>    // For printf, FILE, fopen, fwrite, fread, etc.
#include <stdlib.h>   // For malloc, free, etc.
#include <stdint.h>   // For fixed-width integer types like uint64_t
#include <stddef.h>   // For size_t
#include <string.h>   // For string manipulation functions like snprintf
#include <assert.h>   // For assertions
#include <math.h>     // For math functions like sqrt
#include <sys/stat.h> // For creating directories (mkdir)

#include <gmp.h>         // GMP library for arbitrary precision arithmetic
#include <openssl/sha.h> // For SHA-256 hashing

#include <logger.h> // Logger module for error logging

// minimum and maximum macros
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))

int create_dir(const char *dir);

uint64_t pi_n(int64_t n);
uint64_t int_pow(uint64_t base, int exp);
int is_numeric_str(const char *str);

// utility functions

// print_line - Print a line of dashes
// @param length: The length of the line to print.
void print_line(int length);

/**
 * @brief Compute the SHA-256 hash for an array of integers.
 *
 * @param array Pointer to the array of integers.
 * @param size Size of the array.
 * @param hash Pointer to the SHA-256 hash array.
 * @return int 0 on success, -1 on failure.
 */
int hash_int_array(int *array, size_t size, unsigned char *hash);

/**
 * @brief Validate the SHA-256 hash.
 *
 * @param hash1 Pointer to the first SHA-256 hash array.
 * @param hash2 Pointer to the second SHA-256 hash array.
 * @return int 1 on success, 0 on failure.
 */
int validate_sha256_hash(const unsigned char *hash1, const unsigned char *hash2);

/**
 * @brief Print the SHA-256 hash.
 *
 * @param hash Pointer to the SHA-256 hash array.
 */
void print_sha256_hash(const unsigned char *hash);

#endif // UTILS_H
