// main.c
/**
 * @file main.c
 * @brief Entry point for the iZ-Library, including usage examples.
 *
 */

#include <iZ.h>

int main(void)
{
    log_set_log_level(LOG_DEBUG);

    // * Sieve example
    PRIMES_OBJ *primes = sieve_iZ(1000);

    // Print the first 10 primes
    for (int i = 0; i < 10; i++)
    {
        printf("%d ", primes->p_array[i]);
    }
    printf("\n");

    // Free the allocated memory after use
    primes_obj_free(primes);

    // * RANDOM prime generation example
    int bit_size = 1024; // 1 KB
    mpz_t prime;
    mpz_init(prime);

    random_prime_gen(prime, bit_size); // Generate a random prime number
    // Print the generated prime number
    printf("Random prime: ");
    mpz_out_str(stdout, 10, prime);
    printf("\n");
    // Free the allocated memory
    mpz_clear(prime);

    return 0;
}
