#include <utils.h>

void print_line(int length)
{
    printf("\n");
    for (int i = 0; i < length; i++)
        printf("-");
    printf("\n");
}

uint64_t int_pow(uint64_t base, int exp)
{
    uint64_t result = 1;
    for (int i = 0; i < exp; i++)
        result *= base;
    return result;
}

/**
 * @brief Check if a string is numeric.
 *
 * @param str Pointer to the string.
 * @return int 1 if the string is numeric, 0 otherwise.
 */
int is_numeric_str(const char *str)
{
    if (str == NULL || *str == '\0')
        return 0;

    while (*str)
    {
        if (*str < '0' || *str > '9')
            return 0;
        ++str;
    }
    return 1;
}

/**
 * @brief Compute the SHA-256 hash for an array of integers.
 *
 * @param array Pointer to the array of integers.
 * @param size Size of the array.
 * @param hash Pointer to the SHA-256 hash array.
 * @return int 1 on success, 0 on failure.
 */
int hash_int_array(int *array, size_t size, unsigned char *hash)
{
    if (array == NULL || size == 0)
    {
        log_error("Invalid array in hash_int_array.");
        return 0;
    }

    // Compute SHA-256 hash
    SHA256((unsigned char *)array, size, hash);
    return 1;
}

/**
 * @brief Validate the SHA-256 hash.
 *
 * @param hash1 Pointer to the first SHA-256 hash array.
 * @param hash2 Pointer to the second SHA-256 hash array.
 * @return int 1 on success, 0 on failure.
 */
int validate_sha256_hash(const unsigned char *hash1, const unsigned char *hash2)
{
    if (hash1 == NULL || hash2 == NULL)
    {
        log_error("Invalid hash in validate_sha256_hash.");
        return 0;
    }

    if (memcmp(hash1, hash2, SHA256_DIGEST_LENGTH) != 0)
    {
        log_error("SHA-256 hash mismatch.");
        return 0;
    }

    return 1;
}

/**
 * @brief Print the SHA-256 hash in hexadecimal format.
 *
 * @param hash Pointer to the SHA-256 hash array.
 */
void print_sha256_hash(const unsigned char *hash)
{
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        printf("%02x", hash[i]);

    printf("\n");
}

/**
 * @brief Create a directory if it does not exist.
 *
 * @param dir The directory path.
 * @return int 0 if the directory already exists or was successfully created, -1 otherwise.
 */
int create_dir(const char *dir)
{
    struct stat st = {0};
    if (stat(dir, &st) == -1)
    {
        if (mkdir(dir, 0700) != 0)
        {
            // log error if mkdir fails
            log_error("mkdir failed to create %s :\\", dir);
            return -1;
        }
    }

    return 0; // Directory already exists or was successfully created
}

// Compute n/ln(n) - Prime Counting Function
/**
 * @brief Compute the prime counting function.
 *
 * @param n The input value.
 * @return uint64_t The result of the prime counting function (n/ln(n)).
 */
uint64_t pi_n(int64_t n)
{
    return n / log(n);
}
