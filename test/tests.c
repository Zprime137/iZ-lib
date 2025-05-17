#include <benchmark.h>

/**
 * @brief Tests the integrity of various sieve algorithms
 *
 * Verifies the correctness of sieve algorithms by comparing their output
 *
 * @param sieve_models Collection of sieve algorithm implementations to test
 * @return 1 if all algorithms pass the integrity test, 0 if any algorithm fails
 */
int testing_sieve_integrity(void)
{
    print_line(92);
    printf("Testing sieve algorithms integrity");
    print_line(92);

    // Define the list of sieve algorithms to test.
    SieveAlgorithm models_list[] = {
        SieveOfEratosthenes,
        SegmentedSieve,
        SieveOfEuler,
        SieveOfAtkin,
        Sieve_iZ,
        Sieve_iZm,
    };

    int models_count = sizeof(models_list) / sizeof(SieveAlgorithm);
    SieveModels sieve_models = {models_list, models_count};

    int is_valid = test_sieve_integrity(sieve_models, int_pow(10, 6));

    if (is_valid)
        printf("Success: All sieve algorithms passed the integrity test\n");
    else
        printf("Error: Some sieve algorithms failed the integrity test\n");

    return is_valid;
}

/**
 * @brief Tests primality of p_gaps in vx_obj
 *
 * Verifies the primality of the first 10 prime gaps in the vx_obj structure.
 * It initializes a base value, adds each p_gap to it, and checks if the result is prime.
 *
 * @param vx_obj Pointer to the VX_OBJ structure containing the p_gaps
 * @return 1 if first 10 p_gaps are prime, 0 otherwise
 */
int test_vx_p_gaps(VX_OBJ *vx_obj)
{
    // Test the p_gaps for primality
    mpz_t base;
    mpz_init(base);
    mpz_set_str(base, vx_obj->y, 10);
    mpz_mul_ui(base, base, VX6);
    iZ_gmp(base, base, 1);

    print_line(92);
    printf("Testing primality of first 10 prime gaps: base value + p_gaps[0:9]");
    print_line(92);
    gmp_printf("base value iZ(vx * y, 1): %Zd\n", base);

    // print first 10 p_gaps
    vx_print_p_gaps(vx_obj, 10);
    print_line(92);

    // confirm the p_gaps are correct
    int valid = 1;
    for (int i = 0; i < 10; i++)
    {
        // increment by next gap and test primality
        mpz_add_ui(base, base, vx_obj->p_gaps[i]);

        // perform miller-rabin primality test
        if (mpz_probab_prime_p(base, 25))
            gmp_printf("%Zd is prime\n", base);
        else
        {
            valid = 0;
            gmp_printf("%Zd is not prime\n", base);
        }
    }

    if (valid)
        printf("p_gaps seems correct\n\n");
    else
        printf("Some p_gaps are incorrect\n\n");

    mpz_clear(base);

    return valid;
}

/**
 * @brief Tests the vx6 sieve implementation with increasing values
 *
 * Runs the Sieve-VX6 algorithm with progressively larger numbers,
 * starting from 1000 and increasing by a factor of 10^9 each round.
 *
 * @param test_rounds Number of test rounds to perform
 */
int testing_sieve_vx(void)
{
    print_line(92);
    printf("Testing Sieve-VX algorithm");
    print_line(92);

    size_t vx = VX6; // default segment size
    char y[256] = "1000000000";

    VX_OBJ *vx_obj = vx_init(vx, y);           // Initialize VX_OBJ with a numeric starting y value
    VX_ASSETS *vx_assets = vx_assets_init(vx); // Initialize VX_ASSETS with the segment size
    sieve_vx(vx_obj, vx_assets);               // Perform the sieve process to obtain prime gaps

    int is_valid = test_vx_p_gaps(vx_obj);

    if (is_valid)
        printf("Success: p_gaps seem primes\n");
    else
        printf("Error: some p_gaps are not prime\n");

    vx_free(vx_obj);
    vx_assets_free(vx_assets);

    return is_valid;
}

/**
 * @brief Tests VX_OBJ I/O operations
 *
 * Verifies file I/O functionality for the VX_OBJ structure.
 *
 */
int testing_vx_io(void)
{
    print_line(92);
    printf("Testing VX_OBJ I/O operations");
    print_line(92);

    size_t vx = VX6; // default segment size
    char y[256] = "1000000000";

    VX_OBJ *vx_obj_write = vx_init(vx, y);
    VX_ASSETS *vx_assets = vx_assets_init(vx);
    sieve_vx(vx_obj_write, vx_assets);

    char filename[256];
    sprintf(filename, "%s/test_vx_io", DIR_output);

    // write VX6 object to a file
    if (vx_write_file(vx_obj_write, filename))
    {
        printf("Success: VX object written to file: %s\n", filename);
    }
    else
    {
        printf("Error: Could not write VX object to file: %s\n", filename);
        return 0;
    }

    printf("SHA256 write: ");
    print_sha256_hash(vx_obj_write->sha256);

    // read VX object from a file
    VX_OBJ *vx_obj_read = vx_init(VX6, y);
    int is_valid = vx_read_file(vx_obj_read, filename);

    // print hash
    printf("SHA256 read : ");
    print_sha256_hash(vx_obj_read->sha256);

    // the read function validates the integrity of the file
    if (is_valid)
    {
        printf("Success: VX object read from file: %s\n", filename);

        is_valid = test_vx_p_gaps(vx_obj_read);
        if (is_valid)
        {
            printf("Success: p_gaps seem primes\n");
            vx_free(vx_obj_read);
        }
        else
            printf("Error: some p_gaps are not prime\n");
    }
    else
    {
        printf("Error: Could not read VX object from file: %s\n", filename);
    }

    // cleanup
    vx_free(vx_obj_write);
    vx_assets_free(vx_assets);

    return is_valid;
}

int testing_next_prime_gen(void)
{
    print_line(92);
    printf("Comparing iZ_next_prime vs the gmp_nextprime method.");
    print_line(92);

    int bit_size = 1 * 1024;
    test_iZ_next_prime(bit_size);
}

/**
 * @brief Tests and benchmarks Random-iZprime algorithm
 *
 * Evaluates performance of prime generation algorithms with different
 * key sizes (1KB, 2KB, 4KB, 8KB) using multiple test rounds.
 */
int testing_prime_gen_algorithms(void)
{
    print_line(92);
    printf("Testing Random-iZprime algorithm for bit-size 1024 using single and multi-core parameters");
    print_line(92);

    int Kb = 1024;

    int is_prime = 0;
    mpz_t p;
    mpz_init(p);

    random_iZprime(p, -1, Kb, 1);
    gmp_printf("iZp (1 core): %Zd\n", p);
    is_prime = mpz_probab_prime_p(p, TEST_ROUNDS);

    random_iZprime(p, -1, Kb, 8);
    gmp_printf("iZp (8 cores): %Zd\n", p);
    is_prime = mpz_probab_prime_p(p, TEST_ROUNDS);

    // free memory
    mpz_clear(p);

    return is_prime;
}
