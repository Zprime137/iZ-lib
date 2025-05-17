// iZ.c

#include <iZ.h>

/**
 * @brief Computes 6x + i for a given x and i.
 *
 * @description:
 * This function calculates the value of `6x + i`, where `x` is a positive integer
 * and `i` is either -1 or 1. The result is returned as a 64-bit unsigned integer.
 *
 * Parameters:
 * @param x (uint64_t) The value of `x` in `6x + i`. Must be greater than 0.
 * @param i (int) The value of `i` in `6x + i`. Must be -1 or 1.
 *
 * @return The computed value `6x + i` as a 64-bit unsigned integer.
 */
uint64_t iZ(uint64_t x, int i)
{
    // Ensure i is either -1 or 1
    assert((i == -1 || i == 1) && "i must be either -1 or 1");

    // Ensure x is greater than 0 and less than 2^64 to prevent memory overflow
    assert(x > 0 && "x must be greater than 0");
    assert((x * 6) < pow(2, 64) && "x must be less than 2^64/6");

    // Return the value of 6x + i
    return 6 * x + i;
}

/**
 * @brief Computes 6x + i for arbitrary precision values using GMP.
 *
 * @description:
 * This function computes `6x + i` using the GNU MP (GMP) library, allowing
 * for arbitrary-precision arithmetic. The result is stored in `z` which is an
 * `mpz_t` type, while `x` is the input `mpz_t` type representing the value of `x`.
 * The parameter `i` should be either -1 or 1, representing the value to add or
 * subtract from the product `6x`.
 *
 * Parameters:
 * @param z (mpz_t) The result of the calculation `6x + i`.
 * @param x (mpz_t) The input value of `x` in `6x + i`. Must be greater than 0.
 * @param i (int) The value of `i` in `6x + i`. Must be -1 or 1.
 */
void iZ_gmp(mpz_t z, mpz_t x, int i)
{
    // Ensure i is either -1 or 1
    assert((i == -1 || i == 1) && "i must be either -1 or 1");

    // Ensure x is greater than 0
    assert(mpz_sgn(x) > 0 && "x must be greater than 0");

    mpz_mul_ui(z, x, 6); // z = 6 * x

    if (i > 0)
        mpz_add_ui(z, z, 1); // z = z + 1
    else
        mpz_sub_ui(z, z, 1); // z = z - 1
}

/**
 * @brief Analyzes the search space for potential primes in iZm for vx sizes in range [vx1:vx8].
 *
 * This function performs a statistical analysis of the search space for primes, twins, cousins, and sexy primes.
 *
 * The function operates as follows:
 * 1. Initializes bitmaps x5 and x7.
 * 2. Sets all bits as candidates for primes.
 * 3. Iteratively increase vx size by next prime p and mark composites using the Xp-Wheel.
 * 4. Prints statistical data for each segment size.
 * 5. Cleans up allocated bitmaps.
 */
void analyze_iZm_prime_space(void)
{
    // Statistical analysis about the search space for primes
    print_line(92);
    printf("| %-12s", "VX");             // vx size
    printf("| %-8s|%-8s", "iZ-", "iZ+"); // how many primes in iZ- and iZ+
    printf("| %-12s", "#(Primes)");      // total #primes
    printf("| %-12s", "#(Twins)");       // total #twin primes
    printf("| %-12s", "#(Cousins)");     // total #cousin primes
    printf("| %-12s", "#(Sexy)");        // total #sexy primes
    print_line(92);

    // Initialize x5, x7 bitmaps
    BITMAP *x5, *x7;
    size_t max_vx = 5 * 7 * 11 * 13 * 17 * 19 * 23 * 29;

    // Allocate memory for x5 and x7 with vx + 100 bits
    x5 = bitmap_create(max_vx + 100);
    x7 = bitmap_create(max_vx + 100);

    // Set all bits initially as candidates for primes
    bitmap_set_all(x5);
    bitmap_set_all(x7);

    bitmap_clear_bit(x5, 0);
    bitmap_clear_bit(x7, 0);

    // Mark columns of 5
    bitmap_clear_bit(x5, 1);
    bitmap_clear_bit(x7, 4);

    uint64_t s_primes[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97};

    size_t current_size = 5;
    // Initialize counters
    int primes_count = 0;  // total potential primes
    int iZm5 = 0;          // potential primes in iZm5
    int iZm7 = 0;          // potential primes in iZm7
    int twin_count = 0;    // potential twin primes
    int cousins_count = 0; // potential cousin primes
    int sexy_count = 0;    // potential sexy primes

    int idx = 1; // to skip 5
    while (max_vx % s_primes[idx] == 0)
    {
        // Reset counters for each segment
        primes_count = 0;
        iZm5 = 0;
        iZm7 = 0;
        twin_count = 0;
        cousins_count = 0;
        sexy_count = 0;

        size_t p = s_primes[idx];
        idx++;

        // Convert prime p to its iZ index.
        // For p in iZ+, x = (p + 1)/6, etc.
        int x_p = (p + 1) / 6;

        // Duplicate the range [1:current_size] p.
        bitmap_duplicate_segment(x5, 1, current_size, p);
        bitmap_duplicate_segment(x7, 1, current_size, p);

        // Extend current_size by factor p.
        current_size *= p;

        // Mark p and its composites in the new region of x5, x7
        // using the Xp-Wheel.
        if ((p % 6) > 1)
        {
            // p is in iZ+ => mark its iZ^- composites, then iZ+.
            bitmap_clear_mod_p(x5, p, x_p, current_size + 1);
            bitmap_clear_mod_p(x7, p, p * x_p - x_p, current_size + 1);
        }
        else
        {
            // p is in iZ- => mark its iZ^+ composites, then iZ^-.
            bitmap_clear_mod_p(x5, p, p * x_p - x_p, current_size + 1);
            bitmap_clear_mod_p(x7, p, x_p, current_size + 1);
        }

        // for statistical analysis in this segment size
        // Iterate over [1:vx] in both bitmaps x5 (iZ-) and x7 (iZ+)
        for (uint64_t x = 1; x <= current_size; x++)
        {
            // Check if x is prime in iZ-
            if (bitmap_get_bit(x5, x))
            {
                primes_count++;
                iZm5++;
            }
            // Check if x is prime in iZ+
            if (bitmap_get_bit(x7, x))
            {
                primes_count++;
                iZm7++;
            }

            // Twin primes: check iZm5 & iZm7 at the same x
            if (bitmap_get_bit(x5, x) && bitmap_get_bit(x7, x))
                twin_count++;

            // Cousin primes: (p in x5 at x) and (p in x7 at x-1), etc.
            if (bitmap_get_bit(x5, x) && bitmap_get_bit(x7, x - 1))
                cousins_count++;

            // Sexy primes can occur in iZm5 at [x,x-1] or iZm7 at [x,x-1]
            if (bitmap_get_bit(x5, x) && bitmap_get_bit(x5, x - 1))
                sexy_count++;

            if (bitmap_get_bit(x7, x) && bitmap_get_bit(x7, x - 1))
                sexy_count++;
        }

        // Print results as a formatted row
        printf("| %-12zu", current_size); // range of natural numbers
        printf("| %-8d|%-8d", iZm5, iZm7);
        printf("| %-12d", primes_count);
        printf("| %-12d", twin_count);
        printf("| %-12d", cousins_count);
        printf("| %-12d\n", sexy_count);
    }

    // Clean up bitmaps
    bitmap_free(x5);
    bitmap_free(x7);
}

/**
 * @brief Calculate vx for a given range x_n
 *
 * @description:
 * This function computes the vx value for a given range x_n. The vx value is
 * calculated based on the product of small primes, ensuring that the product
 * does not exceed half of x_n. The function iterates through a predefined
 * array of small primes and multiplies them until the product exceeds the
 * specified limit. The function returns the computed vx value.
 *
 * Parameters:
 * @param x_n the number of bits to be vectorized
 * @param vx_limit the number of primes to be multiplied
 *
 * @return size_t vx in range 35 to iZ primorial of limit primes
 */
size_t compute_limited_vx(size_t x_n, int vx_limit)
{
    uint64_t s_primes[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47};

    size_t vx = 35; // minimum vx size
    int i = 2;      // to skip 5 and 7

    // while vx * current prime doesn't exceed x_n
    while (vx * s_primes[i] < (x_n / 2) && i < vx_limit)
    {
        vx *= s_primes[i];
        i++;
    }

    return vx;
}

// Compute closest vx to the given bit-size
void compute_max_vx_gmp(mpz_t vx, int bit_size)
{
    // get some primes to compute the primorial vx
    PRIMES_OBJ *primes = sieve_iZ(10000);
    assert(primes != NULL && "Failed to generate primes");

    int i = 2;                          // to skip 2, 3
    mpz_set_ui(vx, primes->p_array[i]); // set vx = 5

    // while vx * primes->p_array[i] < 2^bit_size
    while (mpz_sizeinbase(vx, 2) < (size_t)bit_size)
    {
        i++;
        mpz_mul_ui(vx, vx, primes->p_array[i]);
    }

    mpz_div_ui(vx, vx, primes->p_array[i]); // divide by the last prime
    // print last prime
    printf("Last prime: %zu\n", primes->p_array[i - 1]);
}

void construct_vx2(BITMAP *x5, BITMAP *x7)
{
    for (size_t i = 1; i <= 35; i++)
    {
        // In x5, skip indices that are 1 mod 5 or -1 mod 7.
        if (((i - 1) % 5) != 0 && ((i + 1) % 7) != 0)
            bitmap_set_bit(x5, i);

        // In x7, skip indices that are -1 mod 5 or 1 mod 7.
        if (((i + 1) % 5) != 0 && ((i - 1) % 7) != 0)
            bitmap_set_bit(x7, i);
    }
}

/**
 * @brief Constructs a pre-sieved iZm base segment of size vx.
 *
 * Marks all composites of small primes that divide vx in the bitmaps x5 (iZ-)
 * and x7 (iZ+), so that subsequent segments can reuse this base pattern.
 *
 * @description:
 * This function constructs a pre-sieved iZm base segment of size vx. It marks
 * all composites of small primes that divide vx in the bitmaps x5 (iZ-) and
 * x7 (iZ+). The function starts with a minimum size of 35 and iteratively
 * duplicates the range [1:current_size] for each prime p that divides vx.
 * It then marks the composites of p in the new region of x5 and x7 using
 * the Xp-Wheel. The function also handles the case where p is in iZ+ or iZ- by
 * marking the appropriate composites in the respective bitmaps.
 *
 * Parameters:
 * @param vx The total size of the segment.
 * @param x5 A BITMAP as a segment of iZm5.
 * @param x7 A BITMAP as a segment of iZm7.
 */
void construct_iZm_segment(size_t vx, BITMAP *x5, BITMAP *x7)
{
    uint64_t s_primes[] = {5, 7, 11, 13, 17, 19, 23, 29, 31, 37};

    size_t current_size = 35; // Minimum starting size (5*7).
    construct_vx2(x5, x7);

    // Step 2: For each prime p beyond 7 that divides vx:
    int idx = 2; // to skip 5, 7
    while (vx % s_primes[idx] == 0)
    {
        size_t p = s_primes[idx];
        idx++;

        // Convert prime p to its iZ index.
        // For p in iZ+, x = (p + 1)/6, etc.
        int x = (p + 1) / 6;

        // Step 2a: Duplicate the range [1:current_size] p.
        bitmap_duplicate_segment(x5, 1, current_size, p);
        bitmap_duplicate_segment(x7, 1, current_size, p);

        // Extend current_size by factor p.
        current_size *= p;

        // Step 2b: Mark p and its composites in the new region of x5, x7
        // using the Xp-Wheel.
        if ((p % 6) > 1)
        {
            // p is in iZ+ => mark its iZ^- composites, then iZ+.
            // bitmap_clear_mod_p(x5, p, x, current_size + 1);
            bitmap_clear_bit(x5, x);
            bitmap_clear_mod_p(x5, p, p * x + x, current_size + 1);
            bitmap_clear_mod_p(x7, p, p * x - x, current_size + 1);
        }
        else
        {
            // p is in iZ- => mark its iZ^+ composites, then iZ^-.
            bitmap_clear_bit(x7, x);
            bitmap_clear_mod_p(x5, p, p * x - x, current_size + 1);
            bitmap_clear_mod_p(x7, p, p * x + x, current_size + 1);
        }
    }
}

/**
 * @brief Solve for x given matrix_id, p, vx, and y.
 *
 * @description:
 * This function solves for the smallest x that satisfies:
 * (x + vx * y) \equiv x_p \mod p,
 * where x_p is normalized based on the matrix_id
 * and p to either x_p or p - x_p, then computes x and returns it.
 *
 * Parameters:
 * @param matrix_id  int indicating the matrix type (-1 for iZm5, 1 for iZm7).
 * @param p          Unsigned 64-bit integer parameter.
 * @param vx         size_t parameter representing the vx value.
 * @param y          Unsigned 64-bit integer parameter.
 *
 * @return The computed x value as a 64-bit unsigned integer.
 */
uint64_t solve_for_x(int matrix_id, uint64_t p, size_t vx, uint64_t y)
{
    // 1. Normalize x_p to x_p if p_id = matrix_id, else to p - x_p
    uint64_t x_p = (p + 1) / 6;
    int p_id = (p % 6 == 1) ? 1 : -1;
    x_p = matrix_id == p_id ? x_p : p - x_p;
    uint64_t yvx = vx * y;

    // 2. Compute the first composite mark of p in the given segment
    uint64_t x = p - (yvx - x_p) % p;
    return x;
}

/**
 * @brief Solve for x given matrix_id, p, vx, and y using GMP.
 * Same as above but using GMP. Suitable for arbitrary y values.
 *
 * Parameters:
 * @param matrix_id  int indicating the target matrix (-1 for iZm5, 1 for iZm7).
 * @param p          Unsigned 64-bit integer parameter.
 * @param vx         Size_t parameter representing the vx value.
 * @param y          mpz_t parameter representing the y value.
 *
 * @return The computed x value as a 64-bit unsigned integer.
 */
uint64_t solve_for_x_gmp(int matrix_id, uint64_t p, size_t vx, mpz_t y)
{
    mpz_t tmp;
    mpz_init(tmp);

    // 1. Normalize x_p to x_p if p_id = matrix_id, else to p - x_p
    uint64_t x_p = (p + 1) / 6;
    int p_id = (p % 6 == 1) ? 1 : -1;
    x_p = matrix_id == p_id ? x_p : p - x_p;

    // 2. Compute the first composite mark of p in the given segment
    // tmp = (vx * y - x_p) % p
    mpz_mul_ui(tmp, y, vx);
    mpz_sub_ui(tmp, tmp, x_p);
    mpz_mod_ui(tmp, tmp, p);

    // x = p - tmp
    uint64_t x = p - mpz_get_ui(tmp);

    // Clear memory
    mpz_clear(tmp);
    return x;
}

/**
 * @brief Solve for smallest y that satisfies (x + vx * y) \equiv x_p \mod p
 *
 * @description:
 * This function solves for the smallest y that satisfies the equation
 * (x + vx * y) \equiv x_p \mod p, where x_p is normalized based on the matrix_id
 * and p. The function checks if p and vx are co-primes, and if not,
 * it returns -1 indicating no solution can be found. If they are co-primes,
 * it calculates the multiplicative inverse of vx modulo p and computes
 * y using the formula y = (delta * vx_inv) % p, where delta is the
 * difference between x_p and x modulo p. The result is returned as a
 * 64-bit unsigned integer.
 *
 * Parameters:
 * @param matrix_id  Integer indicating the matrix type (-1 for iZm5, 1 for iZm7).
 * @param p          Unsigned 64-bit integer parameter.
 * @param vx         Size_t parameter representing the vx value.
 * @param x          Unsigned 64-bit integer parameter.
 *
 * @return The computed y value as a 64-bit unsigned integer.
 */
uint64_t solve_for_y(int matrix_id, uint64_t p, size_t vx, uint64_t x)
{
    // if p and vx are not co-primes, gcd(vx, p) != 1,
    // p has no modular inverse
    // if p is prime, we can reduce gcd(vx, p) to vx % p
    if (vx % p == 0)
    {
        printf("There's no solution for the given parameters\n");
        return -1; // No solution can be found
    }

    // Normalize x_p to x_p if p_id = matrix_id, else to p - x_p
    uint64_t x_p = (p + 1) / 6;
    int p_id = (p % 6 == 1) ? 1 : -1;
    x_p = matrix_id == p_id ? x_p : p - x_p;

    // y = 0 if x already satisfies the condition
    if (x % p == x_p)
        return 0;

    // Calculate delta
    int delta = (x_p - x) % p;
    delta += delta < 0 ? p : 0; // Ensure delta is positive

    // Find the multiplicative inverse of vx modulo p
    int vx_inv = modular_inverse(vx, p);

    // Compute y using the multiplicative inverse
    uint64_t y = (delta * vx_inv) % p;
    return y;
}

// Extended Euclidean Algorithm to find the multiplicative inverse of "a" modulo "m"
/**
 * @brief Compute the modular inverse of a modulo m.
 *
 * @description:
 * This function computes the modular inverse of a modulo m using the
 * Extended Euclidean Algorithm. The function takes two integers a and m
 * as input and returns the modular inverse of a modulo m. If a and m
 * are not co-prime, the function returns -1 indicating no modular
 * inverse exists. The function also handles the case where m is 1,
 * returning 0 as the modular inverse.
 *
 * Parameters:
 * @param a          The integer for which the modular inverse is to be computed.
 * @param m          The modulus.
 *
 * @return The modular inverse of a modulo m, or -1 if no inverse exists.
 */
int modular_inverse(int a, int m)
{
    int m0 = m, x0 = 0, x1 = 1;

    if (m == 1)
        return 0;

    while (a > 1)
    {
        // q is the quotient
        int q = a / m;
        int t = m;

        // m is remainder now, process the same as Euclid's algorithm
        m = a % m;
        a = t;
        t = x0;

        x0 = x1 - q * x0;
        x1 = t;
    }

    // Ensure x1 is positive
    if (x1 < 0)
        x1 += m0;

    return x1;
}

// Compute the modular inverse "mod_inv" using GMP
/**
 * @brief Compute the modular inverse of a modulo m using GMP.
 *
 * @description:
 * This function computes the modular inverse of a modulo m using the
 * Extended Euclidean Algorithm with GMP. The function takes two mpz_t
 * integers a and m as input and returns the modular inverse of a modulo
 * m in mod_inv. If a and m are not co-prime, the function sets mod_inv
 * to 0 indicating no modular inverse exists. The function also handles
 * the case where m is 1, setting mod_inv to 0.
 *
 * Parameters:
 * @param mod_inv    Output parameter (mpz_t) to store the modular inverse.
 * @param a          Input parameter (mpz_t) for which the modular inverse is to be computed.
 * @param m          Input parameter (mpz_t) representing the modulus.
 */
void modular_inverse_gmp(mpz_t mod_inv, mpz_t a, mpz_t m)
{
    mpz_t m0, t, q, x0, x1, temp_a, temp_m;

    mpz_inits(m0, t, q, x0, x1, temp_a, temp_m, NULL);

    mpz_set(temp_a, a);
    mpz_set(temp_m, m);
    mpz_set(m0, m);
    mpz_set_ui(x0, 0);
    mpz_set_ui(x1, 1);

    if (mpz_cmp_ui(m, 1) == 0)
    {
        mpz_set_ui(mod_inv, 0); // No inverse if modulus is 1
        return;
    }

    while (mpz_cmp_ui(temp_a, 1) > 0)
    {
        // q = a / m
        mpz_fdiv_q(q, temp_a, temp_m);

        // t = m
        mpz_set(t, temp_m);

        // m = a % m
        mpz_mod(temp_m, temp_a, temp_m);
        mpz_set(temp_a, t);

        // t = x0
        mpz_set(t, x0);

        // x0 = x1 - q * x0
        mpz_mul(t, q, x0);
        mpz_sub(x0, x1, t);

        // x1 = t
        mpz_set(x1, t);
    }

    // If x1 is negative, make it positive
    if (mpz_cmp_ui(x1, 0) < 0)
    {
        mpz_add(x1, x1, m0);
    }

    mpz_set(mod_inv, x1);

    mpz_clears(m0, t, q, x0, x1, temp_a, temp_m, NULL);
}
