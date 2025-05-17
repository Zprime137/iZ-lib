/**
 * @file iZ.h
 * @brief Header file for exposing the API of the iZ-Library, a prime sieve and generation library based on the iZ-Framework.
 * @version 0.0.1
 * @repository: https://github.com/Zprime137/iZ-library
 * @license: GPL-3.0
 *
 * @description:
 * This file serves as the main API header for the iZ-library, it includes data structures,
 * declaration of iZ-based utilities and subroutines, the declaration of prime sieve methods,
 * and random prime generation methods.
 *
 * @api:
 * * ** Data structures:
 * - @b BITMAP: A structure for efficient bit representation and manipulation. More details in bitmap.h.
 * - @b PRIMES_OBJ: A structure for holding prime numbers and their metadata. More details in primes_obj.h.
 * - @b VX_OBJ: A structure for holding the prime gaps in a VX6 segment and their metadata. More details in vx_obj.h.
 *
 * * ** iZ-based utilities and subroutines:
 * - @b iZ: Computes the value of 6x + i up to 2^64.
 * - @b iZ_gmp: Computes 6x + i for arbitrary precision values using GMP.
 * - @b compute_limited_vx: Computes vx for a given range of bits.
 * - @b compute_max_vx_gmp: Computes the maximum vx for a given bit size.
 * - @b construct_vx2: Constructs vx2 bitmaps for marking composites of 5 and 7.
 * - @b construct_iZm_segment: Populates x5 and x7 bitmaps with the base iZm segment of size vx, pre-sieved for primes that divide vx.
 * - @b normalized_xp: Normalizes x_p based on the p_id and p.
 * - @b solve_for_x: Solves for x given p_id, p, vx, and y.
 * - @b solve_for_x_gmp: Solves for x given p_id, p, vx, and y using GMP.
 * - @b modular_inverse: Computes the modular inverse of a modulo m.
 * - @b modular_inverse_gmp: Computes the modular inverse of a modulo m using GMP.
 * - @b solve_for_y: Solves for y given p_id, p, vx, and x.
 * - @b sieve_vx_root_primes: Sieve root primes in a given VX6 segment.
 *
 * * ** Sieve methods:
 * - @b classic_sieve_eratosthenes: Classic Sieve of Eratosthenes algorithm.
 * - @b sieve_eratosthenes: Optimized Sieve of Eratosthenes algorithm.
 * - @b segmented_sieve: Segmented Sieve of Eratosthenes algorithm.
 * - @b sieve_euler: Sieve of Euler algorithm.
 * - @b sieve_atkin: Sieve of Atkin algorithm.
 * - @b sieve_iZ: Classic Sieve-iZ algorithm.
 * - @b sieve_iZm: Segmented Sieve-iZm algorithm.
 * - @b sieve_vx: Advanced Sieve-iZm algorithm that processes a VX segment of a specific y in the iZ-Matrix and encodes prime gaps.
 *
 * * ** Random prime generation methods:
 * - @b search_iZprime: Vertical search routine for a random prime that combines the iZ-Matrix space-filtering techniques and Miller-Rabin primality testing. It could be used independently, or via random_iZprime for parallel processing.
 * - @b random_iZprime: Generates a random prime of a specified bit size using the search_iZprime function.
 * - @b iZ_next_prime: Find the next/previous prime number after a given base number.
 * - @b iZ_random_next_prime: Generates a random prime using the iZ_next_prime function.
 * - @b gmp_random_next_prime: Generates a random prime using GMP's mpz_nextprime function invoked on a random base.
 * - @b test_iZ_next_prime: Compares the iZ_next_prime function with the GMP's mpz_nextprime function, using the same base.
 */

#ifndef IZ_H
#define IZ_H

#include <utils.h> ///< For utility functions

// Including data structures modules
#include <bitmap.h>     ///< Bitmap data structure for efficient bit manipulation
#include <primes_obj.h> ///< Primes object for holding prime numbers and their metadata
#include <vx_obj.h>     ///< VX object for holding prime gaps in a VX6 segment and their metadata

// Global Directories
#define DIR_output "output" ///< Directory for output files

// Global Constants
#define VX6 (5 * 7 * 11 * 13 * 17 * 19) // 1,616,615
#define TEST_ROUNDS 25                  ///< Default rounds for Miller-Rabin primality testing

/**
 * @brief Computes 6x + i for a given x and i.
 *
 * @param x (uint64_t) The value of `x` in `6x + i`. Must be greater than 0.
 * @param i (int) The value of `i` in `6x + i`. Must be -1 or 1.
 *
 * @return The computed value `6x + i` as a 64-bit unsigned integer.
 */
uint64_t iZ(uint64_t x, int i);

/**
 * @brief Computes 6x + i for arbitrary precision values using GMP.
 *
 * @param z (mpz_t) The result of the calculation `6x + i`.
 * @param x (mpz_t) The input value of `x` in `6x + i`. Must be greater than 0.
 * @param i (int) The value of `i` in `6x + i`. Must be -1 or 1.
 */
void iZ_gmp(mpz_t z, mpz_t x, int i);

/**
 * @brief
 *
 */
void analyze_iZm_prime_space(void);

/**
 * @brief Computes vx for a given range of bits.
 *
 * @param x_n the range of bits to be vectorized
 * @param vx_limit the number of primes to be multiplied
 *
 * @return size_t vx in range 35 to iZ primorial of limit primes
 */
size_t compute_limited_vx(size_t x_n, int limit);

// Compute closest vx to the given bit-size
/**
 * @brief Computes the maximum vx for a given bit size.
 *
 * @param vx (mpz_t) The computed vx value.
 * @param bit_size (int) The target bit size for vx.
 */
void compute_max_vx_gmp(mpz_t vx, int bit_size);

/**
 * @brief Constructs vx2 bitmaps for marking composites of 5 and 7.
 *
 * @param x5 A BITMAP as a segment of iZm5.
 * @param x7 A BITMAP as a segment of iZm7.
 */
void construct_vx2(BITMAP *x5, BITMAP *x7);

/**
 * @brief Generate the first iZm segment of size vx
 *
 * @param vx the size of the segment
 * @param x5 bitmap for iZ-
 * @param x7 bitmap for iZ+
 */
void construct_iZm_segment(size_t vx, BITMAP *x5, BITMAP *x7);

/**
 * @brief Solve for x given p_id, p, vx, and y.
 *
 * @param p_id  Integer indicating the matrix type (-1 for iZm5, 1 for iZm7).
 * @param p          Unsigned 64-bit integer parameter.
 * @param vx         Size_t parameter representing the vx value.
 * @param y          Unsigned 64-bit integer parameter.
 *
 * @return The computed x value as a 64-bit unsigned integer.
 */
uint64_t solve_for_x(int p_id, uint64_t p, size_t vx, uint64_t y);

/**
 * @brief Solve for x given p_id, p, vx, and y using GMP.
 *
 * @param p_id  Integer indicating the matrix type (-1 for iZm5, 1 for iZm7).
 * @param p          Unsigned 64-bit integer parameter.
 * @param vx         Size_t parameter representing the vx value.
 * @param y          mpz_t parameter representing the y value.
 *
 * @return The computed x value as a 64-bit unsigned integer.
 */
uint64_t solve_for_x_gmp(int p_id, uint64_t p, size_t vx, mpz_t y);

/**
 * @brief Compute the modular inverse of a modulo m.
 *
 * @param a The value to find the modular inverse of.
 * @param m The modulus.
 */
int modular_inverse(int a, int m);

/**
 * @brief Compute the modular inverse of a modulo m using GMP.
 *
 * @param mod_inv (mpz_t) The result of the modular inverse calculation.
 * @param a (mpz_t) The value to find the modular inverse of.
 * @param m (mpz_t) The modulus.
 */
void modular_inverse_gmp(mpz_t mod_inv, mpz_t a, mpz_t m);

/**
 * @brief Solve for y given p_id, p, vx, and x.
 *
 * @param p_id  Integer indicating the matrix type (-1 for iZm5, 1 for iZm7).
 * @param p          Unsigned 64-bit integer parameter.
 * @param vx         Size_t parameter representing the vx value.
 * @param x          Unsigned 64-bit integer parameter.
 *
 * @return The computed y value as a 64-bit unsigned integer.
 */
uint64_t solve_for_y(int p_id, uint64_t p, size_t vx, uint64_t x);

// * Sieve Algorithms: Declarations
// =========================================================

/**
 * @brief Traditional Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @param n The upper limit to find primes.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *classic_sieve_eratosthenes(uint64_t n);

/**
 * @brief Optimized implementation of the Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @param n The upper limit to find primes.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_eratosthenes(uint64_t n);

/**
 * @brief Segmented Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @param n The upper limit to find primes.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails during the process.
 */
PRIMES_OBJ *segmented_sieve(uint64_t n);

/**
 * @brief Sieve of Euler: Generates a list of prime numbers up to a given limit using the Euler Sieve algorithm.
 *
 * @param n The upper limit up to which prime numbers are to be found.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_euler(uint64_t n);

/**
 * @brief Sieve of Atkin: Generates a list of prime numbers up to a given limit using the Sieve of Atkin algorithm.
 *
 * @param n The upper limit up to which prime numbers are to be found.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_atkin(uint64_t n);

/**
 * @brief Classic Sieve-iZ algorithm to generate prime numbers up to a given limit.
 *
 * @param n The upper limit for generating prime numbers.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_iZ(uint64_t n);

/**
 * @brief Segmented Sieve-iZm algorithm to generate prime numbers up to a given limit.
 *
 * @param n The upper limit for generating prime numbers.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails during the process.
 */
PRIMES_OBJ *sieve_iZm(uint64_t n);

/**
 * @brief An advanced implementation of the Sieve-iZm algorithm that processes a VX6 segment of a specific y in the iZ-Matrix.
 *
 * Parameters:
 * @param y_str         Pointer to a numeric string representing the y value in iZm.
 * @param p_test_rounds Number of rounds to be used by the Miller-Rabin primality test.
 * @param filename      Optional file path to which the detected prime gaps will be written.
 * @return
 *      - VX_OBJ* A pointer to the VX_OBJ structure containing the list of prime gaps in the VX6 segment.
 *      - NULL if memory allocation fails or if any error occurs during the process.
 */
// VX_OBJ *sieve_vx6(char *y_str, char *filename);

VX_OBJ **sieve_vx6_range(char *start_y, int range_y);

/**
 * @brief This function performs the sieve process on a given vx and y, defined
 * in the VX_OBJ structure, and stores the primes gaps in the vx_obj->p_gaps array.
 *
 * @param vx_obj The VX_OBJ to be processed.
 * @param root_primes The root primes used for sieving.
 * @param base_x5 The base bitmap for x5.
 * @param base_x7 The base bitmap for x7.
 */
void sieve_vx(VX_OBJ *vx_obj, VX_ASSETS *vx_assets);

/**
 * @brief This function marks composites of root primes in the x5 and x7 bitmaps.
 *
 * @param vx The segment size.
 * @param y The segment index in iZm.
 * @param root_primes The root primes used for sieving.
 * @param x5 The bitmap for iZ- numbers.
 * @param x7 The bitmap for iZ+ numbers.
 */
void sieve_vx_root_primes(int vx, mpz_t y, PRIMES_OBJ *root_primes, BITMAP *x5, BITMAP *x7);

// * Random prime generation algorithms: Declarations
// =========================================================

/**
 * @brief Seed the GMP random state.
 *
 * @description: This function seeds the GMP random state using /dev/urandom.
 *
 * @param state The GMP random state.
 */
void gmp_seed_randstate(gmp_randstate_t state);

/**
 * @brief Vertical search routine for a random prime.
 *
 * @param p The prime number found in the search.
 * @param p_id The iZ identifier (1 or -1).
 * @param vx The horizontal vector of the iZ matrix.
 * @return 1 if a prime is found, 0 otherwise.
 */
int search_iZprime(mpz_t p, int p_id, mpz_t vx);

/**
 * @brief Generates a random prime candidate using a combination of iZ-Matrix space-filtering techniques and Miller-Rabin primality testing.
 *
 * @param p                    The generated random prime candidate.
 * @param p_id                 The target prime identity (iZ- or iZ+).
 * @param bit_size             The target bit size of the prime.
 * @param cores_num            The number of cores to use for parallel processing.
 * @return int                 1 if a prime is found, 0 otherwise.
 *
 * @note The caller is responsible for initializing the mpz_t variable `p` before calling this function
 * and clearing it after use.
 * @note The function uses fork() to create child processes for parallel processing.
 *       The child processes will generate random primes and send them back to the parent process through a pipe.
 *       The parent process will read the first result from the pipe and terminate all child processes.
 *       The function will recursively call itself if no prime is found within the specified attempts.
 */
int random_iZprime(mpz_t p, int p_id, int bit_size, int cores_num);

/**
 * @brief Find the next/previous prime number after a given base number.
 *
 * @param p The mpz_t variable to store the found prime number.
 * @param base The base number to start the search from.
 * @param forward If true, search for the next prime; if false, search for the previous prime.
 * @return 1 if a prime is found, 0 otherwise.
 */
int iZ_next_prime(mpz_t p, mpz_t base, int forward);

/**
 * @brief Generates a random prime number using the iZ_next_prime function.
 *
 * @param p The mpz_t variable to store the generated prime number.
 * @param bit_size The bit size of the prime number to be generated.
 */
int iZ_random_next_prime(mpz_t p, int bit_size);

/**
 * @brief Generates a random prime using GMP's mpz_nextprime function invoked on a random base.
 *
 * @param p         The generated random prime.
 * @param bit_size  The bit size of the prime to be generated.
 *
 * @note The caller is responsible for initializing the mpz_t variable `p` before calling this function
 * and clearing it after use.
 */
void gmp_random_next_prime(mpz_t p, int bit_size);

/**
 * @brief Compares the iZ_next_prime function with the GMP's mpz_nextprime function.
 *
 * @param bit_size The bit size of the prime to be generated.
 * @return int 1 if the iZ_next_prime function is faster, 0 otherwise.
 *
 * @note This function is used for testing the correctness of the iZ_next_prime function.
 */
int test_iZ_next_prime(int bit_size);

#endif // IZ_H
