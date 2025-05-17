/**
 * @file bitmap.c
 * @brief Functions for manipulating bitmaps.
 *
 * This file contains functions to create, free, set, clear, and manipulate bitmaps.
 * It also includes functions for reading and writing bitmaps to files.
 *
 */

#include <iZ.h>

/**
 * @brief Creates a BITMAP of a specified size.
 *
 * @param size The number of bits in the array.
 * @return BITMAP* A pointer to the newly created BITMAP, or NULL on failure.
 */
BITMAP *bitmap_create(size_t size)
{
    if (size == 0)
    {
        log_error("Cannot create a BITMAP of size 0");
        return NULL;
    }

    BITMAP *bitmap = (BITMAP *)malloc(sizeof(BITMAP));
    if (bitmap == NULL)
    {
        log_error("bitmap_create: Memory allocation failed for BITMAP struct");
        return NULL;
    }

    bitmap->size = size;
    size_t byte_size = (size + 7) / 8;
    bitmap->data = (unsigned char *)calloc(byte_size, sizeof(unsigned char));
    if (bitmap->data == NULL)
    {
        free(bitmap);
        log_error("bitmap_create: Memory allocation failed for BITMAP");
        return NULL;
    }

    return bitmap;
}

/**
 * @brief Frees the memory associated with a BITMAP.
 *
 * @param bitmap The BITMAP to free.
 */
void bitmap_free(BITMAP *bitmap)
{
    if (bitmap != NULL)
    {
        if (bitmap->data != NULL)
        {
            free(bitmap->data);
            bitmap->data = NULL;
        }

        free(bitmap);
        bitmap = NULL;
    }
}

/**
 * @brief Sets all bits in the array to 1.
 *
 * @param bitmap The BITMAP to modify.
 */
void bitmap_set_all(BITMAP *bitmap)
{
    memset(bitmap->data, 0xFF, (bitmap->size + 7) / 8);
}

/**
 * @brief Clears all bits in the bitmap (sets them to 0).
 *
 * @param bitmap A pointer to the BITMAP structure.
 */
void bitmap_clear_all(BITMAP *bitmap)
{
    memset(bitmap->data, 0x00, (bitmap->size + 7) / 8);
}

/**
 * @brief Sets a specific bit to 1.
 *
 * @param bitmap The BITMAP to modify.
 * @param idx The index of the bit to set.
 */
void bitmap_set_bit(BITMAP *bitmap, size_t idx)
{
    bitmap->data[idx / 8] |= (1 << (idx % 8));
}

/**
 * @brief Gets the value of a specific bit.
 *
 * @param bitmap The BITMAP to read from.
 * @param idx The index of the bit to read.
 * @return int 1 if the bit is set, 0 if unset, -1 on error.
 */
int bitmap_get_bit(BITMAP *bitmap, size_t idx)
{
    return (bitmap->data[idx / 8] & (1 << (idx % 8))) != 0;
}

/**
 * @brief Flips the value of a specific bit.
 *
 * @param bitmap The BITMAP to modify.
 * @param idx The index of the bit to flip.
 */
void bitmap_flip_bit(BITMAP *bitmap, size_t idx)
{
    bitmap->data[idx / 8] ^= (1 << (idx % 8));
}

/**
 * @brief Clears (sets to 0) a specific bit.
 *
 * @param bitmap The BITMAP to modify.
 * @param idx The index of the bit to clear.
 */
void bitmap_clear_bit(BITMAP *bitmap, size_t idx)
{
    // Clear the bit by ANDing with the negation of the bit mask
    bitmap->data[idx / 8] &= ~(1 << (idx % 8));
}

/**
 * @brief Clears bits that are multiples of a prime number `p`, starting from `start_idx` to `limit`.
 *
 * @param bitmap The BITMAP to modify.
 * @param p The prime number whose multiples will be cleared.
 * @param start_idx The starting index.
 * @param limit The upper limit.
 */
void bitmap_clear_mod_p(BITMAP *bitmap, uint64_t p, size_t start_idx, size_t limit)
{
    // set limit to the minimum of bitmap->size and limit
    limit = MIN(limit, bitmap->size);

    if (start_idx <= limit)
    {
        for (size_t idx = start_idx; idx <= limit; idx += p)
            bitmap->data[idx / 8] &= ~(1 << (idx % 8));
    }
}

/**
 * @brief Creates an exact copy of the given bitmap.
 *
 * @param bitmap The BITMAP to copy.
 * @return *BITMAP A pointer to the newly created copy, or NULL on failure.
 */
BITMAP *bitmap_clone(BITMAP *bitmap)
{
    BITMAP *clone = bitmap_create(bitmap->size);

    size_t byte_size = (bitmap->size + 7) / 8;
    memcpy(clone->data, bitmap->data, byte_size);
    return clone;
}

/**
 * @brief Copies a segment of the source bitmap to the destination bitmap.
 *
 * @param dest The destination bitmap.
 * @param dest_idx The starting index in the destination data.
 * @param src The source bitmap.
 * @param src_idx The starting index in the source data.
 * @param length The length of the segment to copy.
 */
void bitmap_copy(BITMAP *dest, size_t dest_idx, BITMAP *src, size_t src_idx, size_t length)
{
    // Copy the bits from src to dest
    for (size_t i = 0; i < length; i++)
    {
        if (bitmap_get_bit(src, src_idx + i))
            dest->data[(dest_idx + i) / 8] |= 1 << ((dest_idx + i) % 8);
        else
            dest->data[(dest_idx + i) / 8] &= ~(1 << ((dest_idx + i) % 8));
    }
}

/**
 * @brief Duplicates a segment of the bitmap->data multiple times.
 *
 * @param bitmap The BITMAP to modify.
 * @param start_idx The starting index of the segment.
 * @param segment_size The size of the segment.
 * @param y The number of times to duplicate the segment.
 */
void bitmap_duplicate_segment(BITMAP *bitmap, size_t start_idx, size_t segment_size, size_t y)
{
    size_t cursor = start_idx + segment_size;
    for (size_t i = 1; i < y; i++)
    {
        bitmap_copy(bitmap, cursor, bitmap, start_idx, segment_size);
        cursor += segment_size;
    }
}

/**
 * @brief Sets the bits of the bitmap based on a string of 0s and 1s.
 *
 * @param bitmap The BITMAP to modify.
 * @param str The string of 0s and 1s representing the bit values.
 */
void bitmap_from_string(BITMAP *bitmap, const char *str)
{
    if (bitmap == NULL || str == NULL)
    {
        log_error("bitmap_from_string called with NULL argument");
        return;
    }

    size_t len = strlen(str);
    if (len > bitmap->size)
    {
        log_error("String length %zu exceeds bitmap size %zu in bitmap_from_string", len, bitmap->size);
        len = bitmap->size;
    }

    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == '1')
            bitmap->data[i / 8] |= 1 << (i % 8);
        else if (str[i] == '0')
            bitmap->data[i / 8] &= ~(1 << (i % 8));
        else
            log_warn("Invalid character '%c' at position %zu in bitmap_from_string", str[i], i);
    }
}

/**
 * @brief Converts the bitmap->data into a string of 0s and 1s.
 *
 * @param bitmap The BITMAP to convert.
 * @return char* A dynamically allocated string of 0s and 1s, or NULL on failure.
 */
char *bitmap_to_string(BITMAP *bitmap)
{
    if (bitmap == NULL)
    {
        log_error("bitmap_to_string called with NULL bitmap");
        return NULL;
    }

    size_t size = bitmap->size;
    char *str = (char *)malloc((size + 1) * sizeof(char));
    if (!str)
    {
        log_error("Memory allocation failed for BITMAP to string conversion");
        return NULL;
    }

    for (size_t i = 0; i < size; ++i)
    {
        int bit = bitmap_get_bit(bitmap, i);
        if (bit == -1)
        {
            log_error("Failed to get bit at index %zu in bitmap_to_string", i);
            free(str);
            return NULL;
        }
        str[i] = bit ? '1' : '0';
    }
    str[size] = '\0';

    return str;
}

/**
 * @brief Generates a SHA-256 hash for the bitmap->data.
 *
 * @param bitmap The BITMAP for which the SHA-256 hash is generated.
 */
void bitmap_compute_hash(BITMAP *bitmap)
{
    // Calculate size in bytes
    size_t byte_size = (bitmap->size + 7) / 8;

    // Generate SHA-256 hash and store it in the struct
    SHA256((unsigned char *)bitmap->data, byte_size, bitmap->sha256);
}

/**
 * @brief Validates the SHA-256 hash stored in bitmap->sha256.
 *
 * @param bitmap The BITMAP whose hash is validated.
 * @return int 1 if the hash matches, 0 otherwise.
 */
int bitmap_validate_hash(BITMAP *bitmap)
{
    unsigned char correct_hash[SHA256_DIGEST_LENGTH]; // Buffer to hold the computed hash

    size_t byte_size = (bitmap->size + 7) / 8; // Calculate size in bytes

    // Generate SHA-256 hash and store it in correct_hash
    SHA256((unsigned char *)bitmap->data, byte_size, correct_hash);

    // Compare actual_hash with the stored hash in bitmap->sha256
    if (memcmp(correct_hash, bitmap->sha256, SHA256_DIGEST_LENGTH) == 0)
    {
        log_info("SHA-256 checksum validation passed.");
        return 1; // SHA-256 match
    }
    else
    {
        log_error("SHA-256 checksum validation failed.");
        return 0; // SHA-256 mismatch
    }
}

/**
 * @brief Writes a bitmap to a file in the specified directory.
 *
 * @param dir The directory where the file will be saved.
 * @param file_name The name of the file.
 * @return int 1 on success, 0 on failure.
 */
int bitmap_write_file(const char *file_name, BITMAP *bitmap)
{
    if (file_name == NULL || bitmap == NULL)
    {
        log_error("bitmap_write_to_file called with NULL argument");
        return 0;
    }

    // Create file path
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s.%s", file_name, BITMAP_EXT);

    // Open file for writing
    FILE *file = fopen(file_path, "wb");
    if (!file)
    {
        log_error("Failed to open file %s for writing", file_path);
        return 0;
    }

    // Write bitmap->size first
    if (fwrite(&bitmap->size, sizeof(size_t), 1, file) != 1)
    {
        log_error("Failed to write bitmap size to file %s", file_path);
        fclose(file);
        return 0;
    }

    // Calculate size in bytes
    size_t byte_size = (bitmap->size + 7) / 8;

    // Write bitmap->data
    if (fwrite(bitmap->data, sizeof(unsigned char), byte_size, file) != byte_size)
    {
        log_error("Failed to write bitmap data to file %s", file_path);
        fclose(file);
        return 0;
    }

    // Generate SHA-256 hash and store it in the struct
    bitmap_compute_hash(bitmap);

    // Write the SHA-256 hash stored in the struct
    if (fwrite(bitmap->sha256, sizeof(unsigned char), SHA256_DIGEST_LENGTH, file) != SHA256_DIGEST_LENGTH)
    {
        log_error("Failed to write SHA-256 hash to file %s", file_path);
        fclose(file);
        return 0;
    }

    // Close the file
    fclose(file);
    return 1;
}

/**
 * @brief Reads a bit array from a file in the specified directory.
 *
 * @param dir The directory where the file is located.
 * @param file_name The name of the file.
 * @return bitmap* A pointer to the newly read bitmap, or NULL on failure.
 */
BITMAP *bitmap_read_file(const char *file_name)
{
    if (file_name == NULL)
    {
        log_error("bitmap_read_from_file called with NULL argument");
        return NULL;
    }

    // Create file path
    char file_path[1024];
    snprintf(file_path, sizeof(file_path), "%s.%s", file_name, BITMAP_EXT);

    // Open file for reading
    FILE *file = fopen(file_path, "rb");
    if (!file)
    {
        log_error("Failed to open file %s for reading", file_path);
        return NULL;
    }

    // Read the size of the BITMAP first
    size_t size;
    if (fread(&size, sizeof(size_t), 1, file) != 1)
    {
        log_error("Failed to read bitmap size from file %s", file_path);
        fclose(file);
        return NULL;
    }

    // Create the BITMAP
    BITMAP *bitmap = bitmap_create(size);
    if (bitmap == NULL)
    {
        log_error("Failed to create bitmap for reading");
        fclose(file);
        return NULL;
    }

    // Calculate size in bytes
    size_t byte_size = (size + 7) / 8;

    // Read bitmap->data from the file
    if (fread(bitmap->data, sizeof(unsigned char), byte_size, file) != byte_size)
    {
        log_error("Failed to read bitmap data from file %s", file_path);
        bitmap_free(bitmap);
        fclose(file);
        return NULL;
    }

    // Read the stored SHA-256 hash
    unsigned char stored_sha256[SHA256_DIGEST_LENGTH];
    if (fread(stored_sha256, sizeof(unsigned char), SHA256_DIGEST_LENGTH, file) != SHA256_DIGEST_LENGTH)
    {
        log_error("Failed to read SHA-256 hash from file %s", file_path);
        bitmap_free(bitmap);
        fclose(file);
        return NULL;
    }

    fclose(file);

    // Generate SHA-256 hash again to validate the data
    bitmap_compute_hash(bitmap);

    // Validate the hash stored in the file
    if (memcmp(stored_sha256, bitmap->sha256, SHA256_DIGEST_LENGTH) != 0)
    {
        log_error("Corrupted Data: SHA-256 checksum validation failed for file %s", file_path);
        bitmap_free(bitmap);
        return NULL;
    }

    return bitmap;
}
