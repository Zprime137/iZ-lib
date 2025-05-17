/**
 * @file sieve.c
 * @brief This file contains custom implementations of various sieve algorithms for
 * prime number generation.
 *
 * @description:
 * This file implements several sieve algorithms:
 * - @b classic_sieve_eratosthenes: Classic Sieve of Eratosthenes
 * - @b sieve_eratosthenes: Optimized Sieve of Eratosthenes
 * - @b segmented_sieve_eratosthenes: Segmented Sieve of Eratosthenes
 * - @b sieve_euler: Sieve of Euler
 * - @b sieve_atkin: Sieve of Atkin
 *
 * Each function takes an upper limit `n` and returns a pointer to a PRIMES_OBJ structure
 * containing the list of primes found.
 *
 * @usage:
 * PRIMES_OBJ *primes = classic_sieve_eratosthenes(1000); // or any other sieve function
 *
 * primes_obj_free(primes); // Free the allocated memory for the primes object after use
 */

#include <iZ.h>

//! Sieve Algorithms: Implementations
// =========================================================

/**
 * @brief Traditional Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @description:
 * This function implements the classic Sieve of Eratosthenes algorithm to find all prime numbers
 * up to a given limit n. It initializes a bitmap of size n + 1 to track prime numbers, then
 * for each root prime (p < n_sqrt), it marks the multiples of p up to n as non-prime. The algorithm
 * iterates through the bitmap, appending prime numbers to the primes array. The function
 * handles memory allocation and resizing of the primes array as needed. Finally, it returns the
 * PRIMES_OBJ structure containing the list of prime numbers found.
 *
 * @param n The upper limit to find primes.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *classic_sieve_eratosthenes(uint64_t n)
{
    // Check if n is less than 10, return NULL
    if (n < 10)
        return NULL;

    // Initialize the primes object with an estimated capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    if (primes == NULL)
    {
        // Error already logged in primes_obj_init
        // Possibly tried to allocate too much memory
        return NULL;
    }

    // Create a bitmap to mark prime numbers
    BITMAP *n_bits = bitmap_create(n + 1);
    if (n_bits == NULL)
    {
        primes_obj_free(primes);
        return NULL;
    }
    bitmap_set_all(n_bits);

    uint64_t n_sqrt = sqrt(n);

    // Sieve algorithm to mark non-prime numbers
    for (uint64_t p = 2; p <= n; p++)
    {
        if (bitmap_get_bit(n_bits, p))
        {
            primes_obj_append(primes, p);
            if (p <= n_sqrt)
                bitmap_clear_mod_p(n_bits, p, p * p, n + 1);
        }
    }

    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    primes_obj_resize_to_p_count(primes);

    return primes;
}

/**
 * @brief Optimized Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @description:
 * This function applies common speedups to the Sieve of Eratosthenes algorithm.
 * It works the same way as the classic sieve but skips checking or marking even
 * numbers and starts with 3, incrementing by 2, it also starts the sieve from p*p.
 *
 * @param n The upper limit to find primes.
 * @return
 *      - A pointer to a PRIMES_OBJ structure containing the list of prime numbers up to n,
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_eratosthenes(uint64_t n)
{
    // Check if n is less than 10, return NULL
    if (n < 10)
        return NULL;

    // Initialize the primes object with an estimated capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    if (primes == NULL)
    {
        // Error already logged in primes_obj_init
        // Possibly tried to allocate too much memory
        return NULL;
    }

    // Create a bitmap to mark prime numbers
    BITMAP *n_bits = bitmap_create(n + 1);
    if (n_bits == NULL)
    {
        primes_obj_free(primes);
        return NULL;
    }
    bitmap_set_all(n_bits);

    uint64_t n_sqrt = sqrt(n);

    // Add 2 as the first prime
    primes_obj_append(primes, 2);

    // Sieve algorithm to mark non-prime numbers, skipping even numbers
    for (uint64_t p = 3; p <= n; p += 2)
    {
        if (bitmap_get_bit(n_bits, p))
        {
            primes_obj_append(primes, p);
            if (p <= n_sqrt)
                bitmap_clear_mod_p(n_bits, 2 * p, p * p, n + 1);
        }
    }

    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    primes_obj_resize_to_p_count(primes);

    return primes;
}

/**
 * @brief Segmented Sieve of Eratosthenes algorithm to find all primes up to n.
 *
 * @description:
 * This function implements the Segmented Sieve of Eratosthenes algorithm to find all prime numbers
 * up to a given limit n. It divides the range into segments and uses a bitmap to mark prime numbers
 * in each segment. The function returns a pointer to a PRIMES_OBJ structure containing the list of
 * prime numbers found. The function also handles memory allocation and resizing of the primes array
 * as needed.
 *
 * @param n The upper limit to find primes.
 * @return
 *      - A pointer to the PRIMES_OBJ structure containing the list of primes up to n,
 *      - NULL if memory allocation fails or if n is less than 1000.
 */
PRIMES_OBJ *segmented_sieve(uint64_t n)
{
    // Check if n is less than 1000, use classic sieve
    if (n < 1000)
        return sieve_eratosthenes(n);

    // Initialize PRIMES_OBJ with an estimated capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);
    if (primes == NULL)
    {
        // Error already logged in primes_obj_init
        return NULL;
    }

    // Define the segment size; can be tuned based on memory constraints
    uint64_t segment_size = (uint64_t)sqrt(n);

    // Step 1: Sieve small primes up to sqrt(n) using the traditional sieve
    BITMAP *n_bits = bitmap_create(segment_size + 1); // +1 to include segment_size
    if (n_bits == NULL)
    {
        primes_obj_free(primes);
        return NULL;
    }
    bitmap_set_all(n_bits);

    primes->p_array[primes->p_count++] = 2; // Add 2 as the first prime

    // Sieve odd numbers starting from 3 up to segment_size
    for (uint64_t p = 3; p <= segment_size; p += 2)
    {
        if (bitmap_get_bit(n_bits, p))
        {
            primes_obj_append(primes, p);

            // Start marking multiples of p from p*p within the small_bits
            for (uint64_t multiple = p * p; multiple <= segment_size; multiple += 2 * p)
                bitmap_clear_bit(n_bits, multiple);
        }
    }

    // Step 2: Segmented sieve
    uint64_t low = segment_size + 1;
    uint64_t high = low + segment_size - 1;
    if (high > n)
        high = n;

    // Iterate over segments
    while (low <= n)
    {
        bitmap_set_all(n_bits);

        // Sieve the current segment using the small primes
        for (int i = 0; i < primes->p_count; i++)
        {
            uint64_t p = primes->p_array[i];
            if (p * p > high)
                break;

            // Find the minimum number in [low, high] that is a multiple of p
            uint64_t start = (low / p) * p;
            if (start < low)
                start += p;
            if (start < p * p)
                start = p * p;

            // Mark multiples of p within the segment
            for (uint64_t j = start; j <= high; j += p)
            {
                // Skip even multiples
                if (j % 2 == 0)
                    continue;

                size_t index = j - low;
                bitmap_clear_bit(n_bits, index);
            }
        }

        // Collect primes from the current segment
        for (uint64_t i = low; i <= high; i++)
        {
            // Skip even numbers
            if (i % 2 == 0)
                continue;

            if (bitmap_get_bit(n_bits, i - low))
                primes_obj_append(primes, i);
        }

        // Move to the next segment
        low = high + 1;
        high = low + segment_size - 1;
        if (high > n)
            high = n;
    }

    // Step 3: Finalize
    // Trim the primes array to the exact number of primes found
    primes_obj_resize_to_p_count(primes);

    return primes;
}

/**
 * @brief Sieve of Euler: Generates a list of prime numbers up to a given limit using the Euler Sieve algorithm.
 *
 * @description:
 * This function uses the Euler Sieve algorithm to find all prime numbers up to a specified limit `n`.
 * It marks each composite only once, allowing for a more efficient sieve process. It initializes a bitmap
 * to track prime numbers and iterates through the numbers, marking distinct multiples of each prime found.
 * The function also handles memory allocation and resizing of the primes array as needed.
 *
 * @param n The upper limit up to which prime numbers are to be found.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_euler(uint64_t n)
{
    // Check if n is less than 10, return NULL
    if (n < 10)
        return NULL;

    // initialization
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);
    if (primes == NULL)
    {
        return NULL;
    }
    BITMAP *n_bits = bitmap_create(n + 1);
    if (n_bits == NULL)
    {
        primes_obj_free(primes);
        return NULL;
    }
    bitmap_set_all(n_bits);

    // starting the prime list with 2 to skip reading even numbers
    primes_obj_append(primes, 2);

    // sieve logic
    for (uint64_t i = 3; i <= n; i += 2)
    {
        if (bitmap_get_bit(n_bits, i))
            primes_obj_append(primes, i);

        for (int j = 1; j < primes->p_count; ++j)
        {
            uint64_t p = primes->p_array[j];

            if (p * i > n)
                break;

            bitmap_clear_bit(n_bits, p * i);

            if (i % p == 0)
                break;
        }
    }

    // cleanup
    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    primes_obj_resize_to_p_count(primes);

    return primes;
}

/**
 * @brief Sieve of Atkin: Generates a list of prime numbers up to a given limit using the Sieve of Atkin algorithm.
 *
 * @description:
 * This function implements the Sieve of Atkin, an efficient algorithm to find all prime numbers up to a specified integer `n`.
 * It initializes a bitmap to mark potential primes and applies the Atkin conditions to identify primes.
 * The function also marks odd multiples of squares of primes as non-prime, then collects the remaining unmarked numbers as primes.
 *
 * @param n The upper limit (inclusive) up to which prime numbers are to be found.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_atkin(uint64_t n)
{
    // Check if n is less than 10, return NULL
    if (n < 10)
        return NULL;

    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);
    if (primes == NULL)
    {
        // Error already logged in primes_obj_init
        return NULL;
    }
    // Create a bitmap to mark potential primes
    BITMAP *n_bits = bitmap_create(n + 1);
    if (n_bits == NULL)
    {
        primes_obj_free(primes);
        return NULL;
    }

    uint64_t n_sqrt = sqrt(n) + 1;

    // Initialize 2 and 3 as primes
    primes_obj_append(primes, 2);
    primes_obj_append(primes, 3);

    // 1. Mark potential primes in the bitmap using the Atkin conditions
    for (uint64_t x = 1; x < n_sqrt; ++x)
    {
        for (uint64_t y = 1; y < n_sqrt; ++y)
        {
            uint64_t num = 4 * x * x + y * y;
            if (num <= n && (num % 12 == 1 || num % 12 == 5))
                bitmap_set_bit(n_bits, num); // Toggle the bit

            num = 3 * x * x + y * y;
            if (num <= n && num % 12 == 7)
                bitmap_set_bit(n_bits, num); // Toggle the bit

            num = 3 * x * x - y * y;
            if (x > y && num <= n && num % 12 == 11)
                bitmap_set_bit(n_bits, num); // Toggle the bit
        }
    }

    // 2. Remove composites by sieving out multiples of squares of primes
    for (uint64_t i = 5; i <= n_sqrt; i += 2)
    {
        if (bitmap_get_bit(n_bits, i))
        {
            // Mark odd multiples of i^2 as non-prime
            bitmap_clear_mod_p(n_bits, 2 * i, i * i, n + 1);
        }
    }

    // 3. Collect primes from the bitmap
    for (uint64_t i = 5; i <= n; i += 2)
    {
        if (bitmap_get_bit(n_bits, i))
            primes_obj_append(primes, i);
    }

    // cleanup
    bitmap_free(n_bits);

    // Resize primes array to fit the exact number of primes found
    primes_obj_resize_to_p_count(primes);

    return primes;
}
