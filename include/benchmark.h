/**
 * @file benchmark.h
 * @brief Header file for benchmarking sieve algorithms.
 *
 * @description: This file contains the declarations for benchmarking various sieve algorithms,
 * including functions for testing integrity, measuring execution time, and running benchmarks.
 * It also includes functions for benchmarking random prime generation methods.
 *
 * @note: Developers looking to extend or modify the library can use these tools for performance
 * evaluation and comparison of different sieve algorithms.
 *
 * @api: This section describes the functions available for benchmarking sieve algorithms.
 * - @test_sieve_integrity: Tests the integrity of the sieve algorithms by comparing their results.
 * - @measure_sieve_time: Measures the execution time to compute primes up to a given limit using a sieve model.
 * - @benchmark_sieve_models: Benchmarks the sieve algorithms for a given range of exponents.
 * - @benchmark_sieve_vx6: Benchmarks the sieve_vx function by measuring its execution time and printing results.
 * - @benchmark_prime_gen_methods: Benchmarks random prime generation algorithms for performance evaluation.
 *
 * @note: Developers looking to extend or modify the library can use these tools for performance
 * evaluation and comparison of different sieve algorithms.
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <iZ.h>

// Sieve function type, takes uint64_t limit and returns a PRIMES_OBJ pointer
typedef PRIMES_OBJ *(*sieve_fn)(uint64_t);

// Structure to associate the sieve function with its name
typedef struct
{
    sieve_fn function;
    const char *name;
} SieveAlgorithm;

// Structure to hold a list of sieve algorithms
typedef struct
{
    SieveAlgorithm *models_list;
    int models_count;
} SieveModels;

/**
 * @b Sieve_Algorithms
 * @brief List of sieve algorithms available for benchmarking.
 *
 * This section contains the declarations of various sieve algorithms that can be used for
 * generating prime numbers. Each algorithm is represented by a SieveAlgorithm structure,
 * which includes the function pointer and its name.
 */
extern SieveAlgorithm ClassicSieveOfEratosthenes;
extern SieveAlgorithm SieveOfEratosthenes;
extern SieveAlgorithm SegmentedSieve;
extern SieveAlgorithm SieveOfEuler;
extern SieveAlgorithm SieveOfAtkin;
extern SieveAlgorithm Sieve_iZ;
extern SieveAlgorithm Sieve_iZm;

/**
 * @b Benchmarking_Tools
 * @brief Tools for testing and benchmarking sieve algorithms.
 *
 * This section contains the declarations of functions used for testing the integrity
 * of sieve algorithms, measuring their execution time, and running benchmarks.
 * These functions are essential for evaluating the performance of different sieve algorithms
 * and ensuring their correctness.
 *
 * @note: Developers can utilize these functions to ensure the reliability and efficiency
 * of their sieve implementations.
 */

/**
 * @brief Test the integrity of the sieve algorithms.
 *
 * This function runs the sieve algorithms for a given upper limit `n` and compares the SHA-256
 * hash of the generated primes to ensure their correctness. If all hashes match, the integrity is confirmed.
 *
 * @param sieve_models A structure containing different sieve algorithm implementations.
 * @param n The upper limit for prime number generation.
 * @return int Returns 0 if the integrity test passes, otherwise returns an error code.
 */
int test_sieve_integrity(SieveModels sieve_models, uint64_t n);

/**
 * @brief Measure the execution time for a sieve algorithm.
 *
 * This function runs the sieve algorithm for a given upper limit `n` and measures the time taken
 * to compute the primes up to that limit. It prints the algorithm name, the value of `n`,
 * and the execution time in milliseconds.
 *
 * @param sieve_model The sieve algorithm to be used for measuring time.
 * @param n The upper limit for prime number generation.
 * @return size_t The execution time in milliseconds.
 */
size_t measure_sieve_time(SieveAlgorithm sieve_model, uint64_t n);

/**
 * @brief Benchmark the sieve algorithms for a given range of exponents.
 *
 * This function benchmarks the sieve algorithms by measuring their execution time
 * and printing the results. It initializes a 2D array to store the results and
 * iterates through the specified range of exponents, calling the measure_sieve_time
 * function for each algorithm.
 *
 * Parameters:
 * @param sieve_models A structure containing different sieve algorithm implementations.
 * @param base The base value to be raised to the power of exponents.
 * @param min_exp The minimum exponent value.
 * @param max_exp The maximum exponent value.
 * @param save_results A flag indicating whether to save the results to a file named by timestamp in the output directory.
 */
void benchmark_sieve_models(SieveModels sieve_models, int base, int min_exp, int max_exp, int save_results);

/**
 * test_sieve_vx6 - test the sieve_vx function.
 *
 * This function tests the sieve_vx function by measuring its execution time
 * and printing the results. It initializes a VX_OBJ structure with a given y value,
 * runs the sieve method, and then prints the execution time and prime statistics.
 *
 * Parameters:
 * @param y Pointer to a string representing the y value for the VX_OBJ structure.
 * @param filename A pointer to a string representing the filename for output.
 */
void test_sieve_vx6(char *y, char *filename);

/**
 * @brief Benchmark random prime generation algorithms.
 *
 * This function benchmarks the performance of random prime generation algorithms
 * for various bit sizes. It tests for 1 KB, 2 KB, 4 KB, and 8 KB sizes (where 1 KB is defined as 1024 bits).
 *
 * @param bit_size The size of the prime numbers to generate in bits.
 * @param primality_check_rounds The number of rounds for the primality check.
 * @param test_rounds The number of tests to perform for benchmarking.
 * @param save_results A flag indicating whether to save the results to a file.
 */
void benchmark_prime_gen_methods(int bit_size, int test_rounds, int save_results);

#endif
