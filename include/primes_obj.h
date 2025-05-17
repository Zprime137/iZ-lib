
/**
 * @file primes_obj.h
 * @brief Header file for the PRIMES_OBJ structure and related functions. The implementation
 * are in the src/modules/primes_obj.c file.
 *
 * @description:
 * This file contains the definition of the PRIMES_OBJ structure, which is used to
 * represent a collection of prime numbers. It includes functions for initializing,
 * appending primes, resizing the array, computing and validating SHA-256 hashes,
 * and reading/writing the structure to/from files.
 *
 * @api:
 * @primes_obj_init: Initializes a new PRIMES_OBJ structure with an initial estimate of capacity.
 * @primes_obj_append: Appends a prime number to the primes array.
 * @primes_obj_resize_to_p_count: Resizes the primes array to match the current number of primes stored.
 * @primes_obj_free: Frees the memory allocated for the PRIMES_OBJ structure.
 * @primes_obj_compute_hash: Computes the SHA-256 hash of the primes array.
 * @primes_obj_verify_hash: Validates the SHA-256 hash of the primes array.
 * @primes_obj_write_file: Writes the PRIMES_OBJ structure to a file.
 * @primes_obj_read_file: Reads a PRIMES_OBJ structure from a file.
 */

#ifndef PRIMES_OBJ_H
#define PRIMES_OBJ_H

#include <utils.h>

/**
 * @struct PRIMES_OBJ
 * @brief Structure representing a collection of prime numbers.
 *
 * This structure contains an array of prime numbers, its current capacity,
 * the number of primes currently stored, and a SHA-256 hash for validation.
 * The array is dynamically allocated, allowing for efficient storage and
 * manipulation of prime numbers.
 *
 * @note The SHA-256 hash is used to verify the integrity of the prime array.
 * The array is resized dynamically as new primes are appended.
 *
 * @param p_count The number of primes currently stored in the array.
 * @param p_array A pointer to the dynamically allocated array of prime numbers.
 * @param sha256 The SHA-256 hash of the primes array for validation.
 */
typedef struct
{
    int p_count;                                ///< Number of primes currently stored.
    uint64_t *p_array;                          ///< Pointer to the dynamically allocated primes uint64_t array.
    unsigned char sha256[SHA256_DIGEST_LENGTH]; ///< SHA-256 hash of p_array for validation.
} PRIMES_OBJ;

/**
 * @brief Initializes a new PRIMES_OBJ structure with an initial estimate of capacity.
 *
 * @param initial_estimate The initial estimate for the capacity of the primes array.
 * @return A pointer to the newly created PRIMES_OBJ structure.
 */
PRIMES_OBJ *primes_obj_init(int initial_estimate);

/**
 * @brief Appends a prime number p to the primes_obj->primes array,
 * and increments the models_count of primes primes_obj->p_count.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @param p The prime number to be appended.
 */
void primes_obj_append(PRIMES_OBJ *primes_obj, uint64_t p);

/**
 * @brief Resizes the primes array to match the current number of primes stored.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @return 0 on success, non-zero on failure.
 */
int primes_obj_resize_to_p_count(PRIMES_OBJ *primes_obj);

/**
 * @brief Frees the memory allocated for the PRIMES_OBJ structure.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure to be freed.
 */
void primes_obj_free(PRIMES_OBJ *primes_obj);

/**
 * @brief Computes the SHA-256 hash of the primes array.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @return 0 on success, non-zero on failure.
 */
int primes_obj_compute_hash(PRIMES_OBJ *primes_obj);

/**
 * @brief Validates the SHA-256 hash of the primes array.
 *
 * @param primes_obj A pointer to the PRIMES_OBJ structure.
 * @return 0 if the hash is valid, non-zero if invalid.
 */
int primes_obj_verify_hash(PRIMES_OBJ *primes_obj);

/**
 * @brief Writes the PRIMES_OBJ structure to a file.
 *
 * @param file_path The path to the file where the structure will be written.
 * @param primes_obj A pointer to the PRIMES_OBJ structure to be written.
 * @return 0 on success, non-zero on failure.
 */
int primes_obj_write_file(const char *file_path, PRIMES_OBJ *primes_obj);

/**
 * @brief Reads a PRIMES_OBJ structure from a file.
 *
 * @param file_path The path to the file from which the structure will be read.
 * @return A pointer to the newly read PRIMES_OBJ structure, or NULL on failure.
 */
PRIMES_OBJ *primes_obj_read_file(const char *file_path);

#endif // PRIMES_OBJ_H
