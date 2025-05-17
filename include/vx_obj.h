
/**
 * @file vx_obj.h
 * @brief Header file for VX_OBJ structure and its associated functions.
 *
 * @description:
 * This file contains the definition of the VX_OBJ structure, which is used to hold
 * the prime gaps in a VX6 segment and their metadata. The VX_OBJ structure includes:
 * - @vx: The horizontal vector size.
 * - @y: A pointer to the numeric string y.
 * - @p_count: The number of primes found.
 * - @p_gaps: A pointer to the dynamically allocated array of prime gaps.
 * - @sha256: The SHA-256 hash of the p_gaps data for validation.
 * - @bit_ops: The number of bitwise mark operations performed during the sieve process.
 * - @p_test_ops: The number of primality test operations performed during the sieve process.
 *
 * @api:
 * - @vx_init: Initializes a new VX_OBJ structure with the given y string.
 * - @vx_free: Frees the memory allocated for the VX_OBJ structure.
 * - @vx_resize_p_gaps: Resizes the p_gaps array to fit the actual count of prime gaps.
 * - @vx_write_file: Writes the contents of the VX_OBJ structure to a file.
 * - @vx_read_file: Reads the contents of a file into a VX_OBJ structure.
 * - @test_vx_file_io: Tests writing and reading of a VX6 file.
 * - @print_p_gaps: Prints the prime gaps in the VX_OBJ structure.
 * - @print_vx_header: Prints the header for VX statistics.
 * - @print_vx_stats: Prints VX statistics.
 */

#ifndef VX_OBJ_H
#define VX_OBJ_H

#include <utils.h>
#include <bitmap.h>
#include <primes_obj.h>

#define VX_EXT ".vx"              // File extension for VX files
#define GAP_TYPE uint16_t         // Type of an integer
#define GAP_SIZE sizeof(uint16_t) // Size of an integer in bytes

/**
 * @brief Sieve assets for vx prime sieve.
 *
 * This structure contains the size of the segment (vx), a pointer to the root primes
 * used for sieving, and two base bitmaps (base_x5 and base_x7) pre-sieved for
 * primes that divide vx.
 *
 * @param vx (int) The size of the segment.
 * @param root_primes (PRIMES_OBJ *) Pointer to the root primes used for sieving.
 * @param base_x5 (BITMAP *) Pointer to the base bitmap for iZm5/vx.
 * @param base_x7 (BITMAP *) Pointer to the base bitmap for iZm7/vx.
 */
typedef struct
{
    int vx;                  ///< Size of the segment
    PRIMES_OBJ *root_primes; ///< Root primes used for sieving
    BITMAP *base_x5;         ///< Base bitmap for iZm5/vx
    BITMAP *base_x7;         ///< Base bitmap for iZm7/vx
} VX_ASSETS;

/**
 * @brief Initializes vx assets for the sieve.
 *
 * @param vx (size_t) The size of the segment.
 *
 * @return A pointer to the initialized VX_ASSETS structure.
 */
VX_ASSETS *vx_assets_init(size_t vx);

/**
 * @brief Frees the memory allocated for vx assets.
 *
 * @param vx_assets (VX_ASSETS *) The vx assets to be freed.
 */
void vx_assets_free(VX_ASSETS *vx_assets);

/**
 * @struct VX_OBJ
 * @brief Structure representing a collection of prime gaps and their metadata.
 *
 * This structure contains the vector size vx, a pointer to a numeric string y,
 * the count of primes p_count, and a pointer to the array of prime gaps p_gaps.
 * The p_gaps array is dynamically allocated to store the prime gaps found during
 * the sieving process. And for performance analysis, it includes the number of
 * bitwise mark operations (bit_ops) and the number of primality test operations
 * (p_test_ops) performed during the sieve process.
 * The structure also includes a SHA-256 hash for validating the integrity of
 * the p_gaps data.
 *
 * @note The SHA-256 hash is used to verify the integrity of the p_gaps data during I/O operations.
 *
 * @param vx The vector size vx.
 * @param y A pointer to the numeric string y.
 * @param p_count The number of primes found.
 * @param p_gaps A pointer to the dynamically allocated array of prime gaps.
 *      The array is used to store the gaps between consecutive primes within the VX6 segment.
 *      It uses 16-bit unsigned integers (uint16_t) to store the gaps.
 * @param bit_ops The number of bitwise mark operations performed during the sieve process.
 * @param p_test_ops The number of primality test operations performed during the sieve process.
 * @param sha256 The SHA-256 hash of the p_gaps data for validation.
 */
typedef struct
{
    int vx;                                     ///< The horizontal vector size.
    char *y;                                    ///< Pointer to the numeric y string.
    int p_count;                                ///< Number of elements in the p_gaps array.
    GAP_TYPE *p_gaps;                           ///< Pointer to the p_gaps array.
    int bit_ops;                                ///< Number of bitwise mark operations performed.
    int p_test_ops;                             ///< Number of primality test operations performed.
    unsigned char sha256[SHA256_DIGEST_LENGTH]; ///< SHA-256 hash of the p_gaps data for validation.
} VX_OBJ;

/**
 * @brief Initializes a VX_OBJ structure.
 *
 * @description:
 * Allocates memory for a new VX_OBJ structure and initializes its members.
 * The p_count is initialized to 0, and the p_gaps array is allocated with
 * an initial size of (vx/2) of GAP_SIZE = uint16_t.
 *
 * @param y A C-string used for initialization.
 *
 * @return Pointer to the newly created VX_OBJ, or NULL if allocation fails.
 */
VX_OBJ *vx_init(int vx, char *y);

/**
 * @brief Frees a VX_OBJ structure.
 *
 * Frees all memory associated with the VX_OBJ.
 *
 * @param vx_obj Pointer to the VX_OBJ to free.
 */
void vx_free(VX_OBJ *vx_obj);

/**
 * @brief Append a gap to the p_gaps array in the VX_OBJ structure.
 *
 * @param vx_obj Pointer to the VX_OBJ structure to which the gap will be appended.
 * @param gap The gap value to append.
 */
void vx_append_p_gap(VX_OBJ *vx_obj, int gap);

/**
 * @brief Resizes the p_gaps array in a VX_OBJ structure.
 *
 * Adjusts the size of the p_gaps array to fit the actual count of prime gaps.
 *
 * @param vx_obj Pointer to the VX_OBJ whose p_gaps array will be resized.
 */
void vx_resize_p_gaps(VX_OBJ *vx_obj);

/**
 * @brief Hash the p_gaps array in the VX_OBJ structure.
 *
 * Parameters:
 * @param vx_obj Pointer to the VX_OBJ structure whose p_gaps array needs hashing.
 */
void vx_compute_hash(VX_OBJ *vx_obj);

/**
 * @brief Verify the SHA256 hash of the p_gaps array in the VX_OBJ structure.
 *
 * Parameters:
 * @param vx_obj Pointer to the VX_OBJ structure whose hash needs verification.
 *
 * @return 1 if the hashes match, 0 otherwise.
 */
int vx_verify_hash(VX_OBJ *vx_obj);

/**
 * @brief Writes VX data to a file.
 *
 * Writes the contents of the VX_OBJ structure to the specified file.
 *
 * @param vx_obj Pointer to the VX_OBJ containing the data.
 * @param filename C-string representing the file name.
 * @return:
 *   - 1 on successful write,
 *   - 0 if any error occurs
 */
int vx_write_file(VX_OBJ *vx_obj, char *filename);

/**
 * @brief Reads VX data from a file.
 *
 * Populates a VX_OBJ structure with data read from the specified file.
 *
 * @param vx_obj Pointer to the VX_OBJ to populate.
 * @param filename C-string representing the file name.
 * @return:
 *   - 1 on successful read,
 *   - 0 if any error occurs
 */
int vx_read_file(VX_OBJ *vx_obj, char *filename);

/**
 * ! Analysis functions
 * =============================================================
 */

/**
 * @brief Prints the prime gaps in the VX_OBJ structure.
 *
 * @param vx_obj Pointer to the VX_OBJ structure containing the prime gaps.
 * @param count The number of prime gaps to print.
 */
void vx_print_p_gaps(VX_OBJ *vx_obj, int count);

/**
 * @brief Prints the header for VX statistics.
 */
void print_vx_header(void);

/**
 * @brief Prints VX statistics.
 *
 * @param vx_obj Pointer to the VX_OBJ structure containing the data.
 */
void print_vx_stats(VX_OBJ *vx_obj);

#endif // VX_OBJ_H
