/**
 * @file sieve_iZ.c
 * @brief Implementation of the different variants of the Sieve-iZ algorithm for
 * prime number generation.
 *
 * @description:
 * This file implements:
 * - @b sieve_iZ: Classic Sieve-iZ algorithm,
 * - @b sieve_iZm: Segmented Sieve-iZm algorithm,
 * - @b sieve_vx: Sieve-VX algorithm.
 * - @b sieve_vx6_range: Sieve-VX algorithm for a range of y values using VX6 segments.
 *
 * @b sieve_iZ and @b sieve_iZm take an upper limit `n` and returns a pointer to a PRIMES_OBJ
 * structure containing the list of primes found.
 * @usage:
 * PRIMES_OBJ *primes = sieve_iZ(1000); // or sieve_iZm
 *
 * primes_obj_free(primes); // Free the allocated memory for the primes object after use
 *
 * @b sieve_vx takes a VX_OBJ, to be populated with prime gaps, and a VX_ASSETS structure
 * containing reusable base bitmaps and root primes for sieving.
 *
 * @usage:
 * int vx6 = 5 * 7 * 11 * 13 * 17 * 19; // Define the segment size as a product of small primes (greater than 3)
 * char *y_str = "10"; // Define the y value as a string
 * VX_OBJ *vx_obj = vx_init(vx6, y_str); // Initialize VX_OBJ with a numeric starting y value
 * VX_ASSETS *vx_assets = vx_assets_init(vx6); // Initialize VX_ASSETS with the segment size
 * sieve_vx(vx_obj, vx_assets); // Perform the sieve process to obtain prime gaps
 *
 * // Print the prime gaps
 * printf("Prime gaps in the vx6 segment at y = %s:\n", y_str);
 * vx_print_p_gaps(vx_obj, 10); // Print the first 10 prime gaps detected in the segment
 * // Print the number of bitwise mark operations and primality test operations for performance analysis
 * printf("Bitwise mark operations: %llu\n", vx_obj->bitwise_marks);
 * printf("Primality test operations: %llu\n", vx_obj->primality_tests);
 *
 * vx_obj_free(vx_obj); // Free the VX_OBJ after use
 * vx_assets_free(vx_assets); // Free the VX_ASSETS after use
 *
 */

#include <iZ.h>

/**
 * @brief An implementation of the Classic Sieve-iZ algorithm to generate prime numbers up to a given limit.
 * It uses the Xp Wheel to mark composites in the iZ set, which consists of numbers of the form (6x +/- 1).
 *
 * @description:
 * The function initializes two bitmaps, base_x5 and base_x7, to represent the iZ- and iZ+ sets respectively.
 * It iterates through the x values in the range 0 < x < x_n, where x_n = n/6 as the maximum x value less than n.
 * For each x, it checks if base_x5[x] or base_x7[x] is set, indicating that the corresponding number is prime.
 * If it is prime, it appends the number to the primes object and marks its multiples in the bitmaps.
 * The function also handles the case where the last prime exceeds n by removing it.
 * Finally, it resizes the primes object to fit the number of primes found.
 *
 * @param n The upper limit for generating prime numbers.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails or if n is less than 10.
 */
PRIMES_OBJ *sieve_iZ(uint64_t n)
{
    // Check if n is less than 10, return NULL
    if (n < 10)
        return NULL;

    // Initialize primes object with enough initial estimation
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    // Memory allocation failed, check logs
    if (primes == NULL)
        return NULL;

    // Add 2, 3 to primes, the only non iZ primes
    primes_obj_append(primes, 2);
    primes_obj_append(primes, 3);

    // Calculate x_n, index of the upper bound n
    uint64_t x_n = n / 6 + 1;

    // Create bitmap X-Arrays base_x5, base_x7, each of size x_n + 1 bits,
    // thus, total (n/3) bits
    BITMAP *x5 = bitmap_create(x_n + 1);
    BITMAP *x7 = bitmap_create(x_n + 1);

    // Memory allocation failed, check logs
    if (x5 == NULL || x7 == NULL)
    {
        bitmap_free(x5);
        bitmap_free(x7);
        primes_obj_free(primes);
        return NULL;
    }

    // Set all bits initially as candidates for primes
    bitmap_set_all(x5);
    bitmap_set_all(x7);

    // Calculate n_sqrt: the upper bound for root primes
    uint64_t n_sqrt = sqrt(n) + 1;

    // Iterate through x values in range 0 < x < x_n
    // This main loop makes (n/3) `is prime` operations
    for (uint64_t x = 1; x < x_n; x++)
    {
        // if x5[x], implying it's iZ- prime
        if (bitmap_get_bit(x5, x))
        {
            uint64_t p = iZ(x, -1);       // compute p = iZ(x, -1)
            primes_obj_append(primes, p); // add p to primes

            // if p is root prime, mark its multiples in x5, x7
            if (p < n_sqrt)
            {
                bitmap_clear_mod_p(x5, p, p * x + x, x_n);
                bitmap_clear_mod_p(x7, p, p * x - x, x_n);
            }
        }

        // Do the same if x7[x], inverting the signs
        if (bitmap_get_bit(x7, x))
        {
            uint64_t p = iZ(x, 1);
            primes_obj_append(primes, p);

            if (p < n_sqrt)
            {
                bitmap_clear_mod_p(x5, p, p * x - x, x_n);
                bitmap_clear_mod_p(x7, p, p * x + x, x_n);
            }
        }
    }

    // Cleanup: free memory of x5, x7
    bitmap_free(x5);
    bitmap_free(x7);

    // Handle edge case: if last prime > n, remove it
    if (primes->p_array[primes->p_count - 1] > n)
        primes->p_count--;

    // Trim unused memory in primes object
    primes_obj_resize_to_p_count(primes);

    return primes;
}

/**
 * @brief A basic implementation of the Segmented Sieve-iZm algorithm to generate prime numbers up
 * to a given limit n.
 *
 * @description:
 * This function divides the target limit into segments of size vx, where vx is a product of small primes up to 19.
 * It constructs a pre-sieved segment from primes that divide vx, and marks composites of the remaining root primes
 * in each segment over a range of y values. Each segment is cloned from the pre-sieved base segment.
 *
 * The function uses the solve_for_x function to find the first composite index of a prime in a given segment,
 * then proceeds to mark the composites in the bitmaps x5 and x7 using the Xp Wheel.
 * The function also handles edge cases, such as removing the last prime if it exceeds the limit.
 * Finally, it resizes the primes object to fit the number of primes found.
 *
 * Aside from the output size, which can be offloaded, this function has a constant space complexity O(1),
 * requiring maximum 4 * 0.2 = 0.8 MB of memory.
 *
 * @param n The upper limit for generating prime numbers.
 * @return
 *      - PRIMES_OBJ* A pointer to the PRIMES_OBJ structure containing the list of primes up to n.
 *      - NULL if memory allocation fails.
 */
PRIMES_OBJ *sieve_iZm(uint64_t n)
{
    // Check if n is less than 1000, return sieve_iZ(n)
    if (n < 1000)
        return sieve_iZ(n);

    // 1. Initialization
    size_t x_n = n / 6 + 1;

    // Initialize primes array with enough capacity
    PRIMES_OBJ *primes = primes_obj_init(pi_n(n) * 1.5);

    // Memory allocation failed, check logs
    if (primes == NULL)
        return NULL;

    // add 2, 3 to the primes array
    primes_obj_append(primes, 2);
    primes_obj_append(primes, 3);

    // list of small primes to be pre-sieved
    uint64_t s_primes[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};

    // Calculate optimal segment size vx for x_n
    int vx_limit = 6; // max number of primes to be pre-sieved
    size_t vx = compute_limited_vx(x_n, vx_limit);

    int start_i = 2; // skip 2, 3 in the primes array

    // Add pre-sieved primes to primes array
    for (int i = 0; i < vx_limit; i++)
    {
        if (vx % s_primes[i] == 0)
        {
            primes_obj_append(primes, s_primes[i]);
            start_i++;
        }
        else
            break;
    }

    // Initialize base_x5, base_x7 bitmaps for resetting and x5, x7 for active sieve
    BITMAP *base_x5, *base_x7, *x5, *x7;

    // Initialize base segments with vx + 10 bits
    base_x5 = bitmap_create(vx + 10);
    base_x7 = bitmap_create(vx + 10);

    // 2. Preprocessing:
    // Generate pre-sieved segments of size vx in base_x5, base_x7
    construct_iZm_segment(vx, base_x5, base_x7);

    // Clone base_x5, base_x7 into x5, x7 for processing
    x5 = bitmap_clone(base_x5);
    x7 = bitmap_clone(base_x7);

    // 3. Process 1st segment to collect enough root primes
    for (uint64_t x = 2; x <= vx; x++)
    {
        if (bitmap_get_bit(x5, x)) // i.e. iZ- prime
        {
            uint64_t p = iZ(x, -1);
            primes_obj_append(primes, p);

            // Mark composites of p within this segment if any
            if ((p * p) / 6 < vx)
            {
                bitmap_clear_mod_p(x5, p, p * x + x, vx);
                bitmap_clear_mod_p(x7, p, p * x - x, vx);
            }
        }

        if (bitmap_get_bit(x7, x)) // i.e. iZ+ prime
        {
            uint64_t p = iZ(x, 1);
            primes_obj_append(primes, p);

            if ((p * p) / 6 < vx)
            {
                bitmap_clear_mod_p(x5, p, p * x - x, vx);
                bitmap_clear_mod_p(x7, p, p * x + x, vx);
            }
        }
    }

    // 4. Processing remaining segments:
    int max_y = x_n / vx; // number of segments
    uint64_t limit = vx;  // upper bound for marking composites
    uint64_t yvx = vx;    // base value

    // Process the remaining segments for y in 1:max_y (inclusive)
    for (int y = 1; y <= max_y; y++)
    {
        // Reset to base segment for each run
        x5 = bitmap_clone(base_x5);
        x7 = bitmap_clone(base_x7);

        // limit is vx or x_n % vx in the last segment
        if (y == max_y)
            limit = x_n % vx;

        // Mark composites of the rest of root primes in current segment
        for (int i = start_i; i < primes->p_count; i++)
        {
            uint64_t p = primes->p_array[i];

            // Exit if p doesn't have composites in this range
            if ((p * p) / 6 > (yvx + limit))
                break;

            // Mark composites of p in the current segment
            bitmap_clear_mod_p(x5, p, solve_for_x(-1, p, vx, y), limit);
            bitmap_clear_mod_p(x7, p, solve_for_x(1, p, vx, y), limit);
        }

        // Collect unmarked x values as primes
        for (uint64_t x = 2; x <= limit; x++)
        {
            if (bitmap_get_bit(x5, x)) // iZ- prime
                primes_obj_append(primes, iZ(x + yvx, -1));

            if (bitmap_get_bit(x7, x)) // iZ+ prime
                primes_obj_append(primes, iZ(x + yvx, 1));
        }

        yvx += vx; // increment yvx
    }

    // 5. Clean up bitmaps
    bitmap_free(base_x5);
    bitmap_free(base_x7);
    bitmap_free(x5);
    bitmap_free(x7);

    // Handle edge case: if last prime > n, remove it
    if (primes->p_array[primes->p_count - 1] > n)
        primes->p_count--;

    // Trim unused memory in primes object
    primes_obj_resize_to_p_count(primes);

    return primes;
}

/**
 * @brief This function initializes and processes a range of VX_OBJ.
 *
 * @param start_y The starting value for y.
 * @param range_y The number of segments to be sieved.
 * @return VX_OBJ** A pointer to an array of VX_OBJ.
 */
VX_OBJ **sieve_vx6_range(char *start_y, int range_y)
{
    // initialize a list of vx_obj
    VX_OBJ **vx_obj_list = malloc(range_y * sizeof(VX_OBJ *));

    if (vx_obj_list == NULL)
    {
        log_error("Memory allocation failed for vx_obj_list.");
        return NULL;
    }

    size_t vx = VX6; // default segment size
    VX_ASSETS *vx_assets = vx_assets_init(vx);

    mpz_t y;
    mpz_init(y);
    mpz_set_str(y, start_y, 10); // Set y from start_y

    for (int i = 0; i < range_y; i++)
    {
        vx_obj_list[i] = vx_init(VX6, mpz_get_str(NULL, 10, y));
        if (vx_obj_list[i] == NULL)
        {
            log_error("Failed to initialize VX_OBJ.");
            return NULL; // or handle error appropriately
        }

        sieve_vx(vx_obj_list[i], vx_assets);

        // increment y by 1 for each segment
        mpz_add_ui(y, y, 1);
    }

    // 4. Cleanup:
    // Free sieve assets
    vx_assets_free(vx_assets);
    mpz_clear(y);

    // 6. Return the VX_OBJ
    return vx_obj_list;
}

/**
 * @brief This function performs the sieve process on a given vx and y defined
 * in the VX_OBJ structure, and stores the primes gaps in the vx_obj->p_gaps array.
 *
 * @description: This function combines deterministic sieving and probabilistic
 * primality tests to identify prime candidates in a standard VX segment of a
 * specific y in the iZ-Matrix. It populates the vx_obj->p_gaps array with
 * prime gaps between consecutive primes detected in the segment.
 *
 * @param vx_obj The VX_OBJ to be processed.
 * @param vx_assets The VX_ASSETS containing the reusable base bitmaps and root primes
 * for the sieve process.
 */
void sieve_vx(VX_OBJ *vx_obj, VX_ASSETS *vx_assets)
{
    // 1. Initialization
    // Default number of rounds for Miller-Rabin primality test
    int p_test_rounds = 25;

    // Create x5 and x7 bitmaps cloned from base_x5 and base_x7
    BITMAP *x5 = bitmap_clone(vx_assets->base_x5);
    BITMAP *x7 = bitmap_clone(vx_assets->base_x7);

    int vx = vx_obj->vx; // segment size
    // Initialize mpz_t y and yvx
    mpz_t y, yvx;
    mpz_init(y);
    mpz_init(yvx);
    mpz_set_str(y, vx_obj->y, 10); // Set y from vx_obj->y
    mpz_mul_ui(yvx, y, vx);        // Compute yvx = y * vx

    // Initialize and compute root_limit = sqrt(iZ(vx * (y+1), 1))
    mpz_t root_limit;
    mpz_init(root_limit);
    mpz_add_ui(root_limit, yvx, vx);
    iZ_gmp(root_limit, root_limit, 1);
    mpz_sqrt(root_limit, root_limit);

    // Flag to determine if probabilistic primality test is needed:
    // if root_limit > vx, then we need to test
    int is_large_limit = mpz_cmp_ui(root_limit, vx) > 0 ? 1 : 0;

    // 2. Deterministic Sieve: Mark composites of primes < vx in x5, x7
    // Iterate through root primes, skipping 2, 3 and those that divide vx
    for (int i = 2; i < vx_assets->root_primes->p_count; i++)
    {
        int p = vx_assets->root_primes->p_array[i];

        // Skip if p divides vx
        if (vx % p == 0)
            continue;

        // Exit when p > root_limit
        if (!is_large_limit)
            if (mpz_cmp_ui(root_limit, p) < 0)
                break;

        // Mark composites of p in x5 and x7
        bitmap_clear_mod_p(x5, p, solve_for_x_gmp(-1, p, vx, y), vx);
        bitmap_clear_mod_p(x7, p, solve_for_x_gmp(1, p, vx, y), vx);

        vx_obj->bit_ops += (2 * vx) / p;
    }

    // 3. Collect prime gaps in the segment
    // Initialize GMP reusable variables p, x_p
    mpz_t p, x_p;
    mpz_init(p);
    mpz_init(x_p);

    // Initialize gap counter
    int gap = 0;

    // Iterate through x values in the range 1 <= x <= vx
    for (int x = 1; x <= vx; x++)
    {
        // Increment gap by 4 since: iZ(x, -1) - iZ(x-1, 1) = 4
        gap += 4;

        // Check if iZ(x + vx * y, -1) is prime, if not clear x in x5
        if (bitmap_get_bit(x5, x))
        {
            int is_prime = 1;

            if (is_large_limit)
            {
                // Compute x_p = x + vx * y
                mpz_add_ui(x_p, yvx, x);
                iZ_gmp(p, x_p, -1); // Compute p = iZ(x_p, -1)
                is_prime = mpz_probab_prime_p(p, p_test_rounds);
                vx_obj->p_test_ops++;
            }

            if (is_prime)
            {
                vx_append_p_gap(vx_obj, gap); // Append gap to vx_obj
                gap = 0;                      // Reset gap
            }
        }

        // Increment gap by 2 since: iZ(x, 1) - iZ(x, -1) = 2
        gap += 2;

        // Same for iZ+
        if (bitmap_get_bit(x7, x))
        {
            int is_prime = 1;

            if (is_large_limit)
            {
                mpz_add_ui(x_p, yvx, x);
                iZ_gmp(p, x_p, 1); // Compute p = iZ(x_p, 1)
                is_prime = mpz_probab_prime_p(p, p_test_rounds);
                vx_obj->p_test_ops++;
            }

            if (is_prime)
            {
                vx_append_p_gap(vx_obj, gap); // Append gap to vx_obj
                gap = 0;                      // Reset gap
            }
        }
    }

    // 4. Cleanup:
    // Free bitmap objects
    bitmap_free(x5);
    bitmap_free(x7);

    // Clear GMP variables
    mpz_clear(root_limit);
    mpz_clear(y);
    mpz_clear(yvx);
    mpz_clear(p);
    mpz_clear(x_p);

    // Resize p_gaps array to fit the actual count
    vx_resize_p_gaps(vx_obj);
}

/**
 * @brief This a utility function that marks composites of given root primes in the
 * x5 and x7 bitmaps representing iZ- and iZ+ segments at a given y.
 *
 * @description: This function iterates through the root primes and marks the composites
 * in the provided bitmaps using the solve_for_x function. skips primes that divide vx.
 *
 * @param vx The segment size.
 * @param y The segment index in iZm.
 * @param root_primes The root primes used for sieving.
 * @param x5 The bitmap for iZ- numbers.
 * @param x7 The bitmap for iZ+ numbers.
 */
void sieve_vx_root_primes(int vx, mpz_t y, PRIMES_OBJ *root_primes, BITMAP *x5, BITMAP *x7)
{
    // start from 2, skip 2, 3
    for (int i = 2; i < root_primes->p_count; i++)
    {
        int p = root_primes->p_array[i];

        // Skip if p divides vx
        if (vx % p == 0)
            continue;

        // Mark composites of p in x5 and x7
        bitmap_clear_mod_p(x5, p, solve_for_x_gmp(-1, p, vx, y), vx);
        bitmap_clear_mod_p(x7, p, solve_for_x_gmp(1, p, vx, y), vx);
    }
}
