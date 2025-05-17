// random_iZprime.c
/**
 * @file random_iZprime.c
 * @brief This file contains the implementation of various primes generation functions.
 *
 * @description:
 * This file implements the following prime generation functions:
 * @b random_iZprime: A function to generate a random prime number of a given bit-size and supports multi-core processing.
 * @b iZ_next_prime: A function to find the next/previous prime number after a given base.
 * @b iZ_random_next_prime: A function to find a random prime of a given bit-size using the iZ framework.
 * @b gmp_random_next_prime: A function to find a random prime of a given bit-size number using the GMP library.
 *
 * @usage:
 * mpz_t p1, p2, p3; // Declare mpz_t variables to hold the prime numbers
 * mpz_init(p1); // Initialize the variables
 * mpz_init(p2);
 * mpz_init(p3);
 *
 * random_iZprime(p1, 1024, 4); // Sets p1 to a random prime of 1024 bits using 4 cores
 * printf("Random prime using iZ_random_next_prime: ");
 * mpz_out_str(stdout, 10, p1); // Print the random prime in base 10
 * printf("\n");

 * gmp_random_next_prime(p2, 1024); // Sets p2 to a random prime of 1024 bits using gmp_nextprime on a random base
 * printf("Random prime using gmp_random_next_prime: ");
 * mpz_out_str(stdout, 10, p2); // Print the random prime in base 10
 * printf("\n");
 *
 * iZ_random_next_prime(p3, 1024); // Sets p3 to a random prime of 1024 bits using iZ_next_prime on a random base
 * printf("Random prime using iZ_random_next_prime: ");
 * mpz_out_str(stdout, 10, p3); // Print the random prime in base 10
 * printf("\n");
 *
 * mpz_clear(p1); // Free the allocated memory for p1
 * mpz_clear(p2);
 * mpz_clear(p3);
 */

#include <iZ.h> // For iZ api

#include <fcntl.h>    // For open
#include <signal.h>   // For kill
#include <sys/wait.h> // For waitpid
#include <unistd.h>   // For close
#include <stdlib.h>   // For exit
#include <string.h>   // For strlen
#include <time.h>     // For time

/**
 * @brief Seed the GMP random state.
 *
 * @description: This function seeds the GMP random state using /dev/urandom.
 *
 * @param state The GMP random state.
 */
void gmp_seed_randstate(gmp_randstate_t state)
{
    unsigned long seed;
    int random_fd = open("/dev/urandom", O_RDONLY); // Open /dev/urandom for reading
    if (random_fd == -1)
    {
        // Fallback if /dev/urandom cannot be opened
        seed = (unsigned long)time(NULL);
    }
    else
    {
        read(random_fd, &seed, sizeof(seed)); // Read random bytes from /dev/urandom
        close(random_fd);
    }

    gmp_randseed_ui(state, seed); // Seed the state with the random value
}

/**
 * @brief iZprime search routine for generating a random prime.
 *
 * @description: This function searches for a prime number using the given parameters.
 * It combines the iZ-Matrix filtering techniques with the Miller-Rabin primality test.
 * The search is performed by finding a suitable x value that does not correspond to
 * a composite of a prime that divides vx. Then, it iterates over y value in the
 * equation p = iZ(x + vx * y) until a prime is found.
 *
 * @param p The prime number found in the search.
 * @param p_id The identifier (1 or -1) for the iZ matrix.
 * @param vx The horizontal vector of the iZ matrix.
 * @return 1 if a prime is found, 0 otherwise.
 */
int search_iZprime(mpz_t p, int p_id, mpz_t vx)
{
    int found = 0; // flag to indicate if a prime was found

    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_seed_randstate(state); // seed random state

    mpz_t tmp, g;
    mpz_init(tmp);
    mpz_init(g);

    // set random x value in the range of vx
    mpz_urandomm(tmp, state, vx);
    // compute tmp = 6 * tmp + i
    iZ_gmp(tmp, tmp, p_id);

    // search for x value that doesn't correspond to a composite of a prime that divides vx
    while (1)
    {
        // increment tmp by 6 to increment x by 1
        mpz_add_ui(tmp, tmp, 6);

        // Compute g = gcd(vx, tmp)
        mpz_gcd(g, vx, tmp);

        // break if g = 1,
        // implying current x value can yield primes of the form iZ(x + vx * y, p_id)
        if (mpz_cmp_ui(g, 1) == 0)
            break;
    }

    // setting a practical limit for the number of attempts
    int attempts_limit = 1000000;
    int attempts = 0;

    while (!found && attempts < attempts_limit)
    {
        // increment tmp by vx to increment y by 1
        mpz_add(tmp, tmp, vx);

        // check if tmp is prime
        found = mpz_probab_prime_p(tmp, TEST_ROUNDS);

        // if tmp is prime, set p = tmp
        if (found)
            mpz_set(p, tmp);
    }

    if (!found)
        log_debug("No Prime was found :/");

    // cleanup
    mpz_clear(tmp);
    mpz_clear(g);
    gmp_randclear(state);

    return found;
}

/**
 * @brief Generates a random prime candidate using the search_iZprime routine.
 *
 * @description: This function generates a random prime of a given bit size using the search_iZprime routine.
 * It initializes the random base and sets up the search parameters. The function also allows
 * for parallel processing of the search using multiple child processes. The generated prime
 * is stored in the provided mpz_t p variable.
 *
 * @param p The mpz_t variable to store the generated prime number.
 * @param p_id The target iZ identity (-1 for iZ- or 1 for iZ+).
 * @param bit_size The target bit size of the prime.
 * @param cores_num The number of cores to use for parallel processing.
 * @return 1 if a prime is found, 0 otherwise.
 */
int random_iZprime(mpz_t p, int p_id, int bit_size, int cores_num)
{
    // Check if the bit size is within the valid range
    if (bit_size < 10)
    {
        log_debug("Using minimum bit size: 10");
        bit_size = 10; // minimum bit size
    }

    // Check if the number of cores is within the valid range
    if (cores_num > 16)
    {
        log_debug("Using maximum cores: 16");
        cores_num = 16; // maximum cores
    }

    // 1. Compute suitable vx for the given bit-size
    mpz_t vx;
    mpz_init(vx);
    compute_max_vx_gmp(vx, bit_size);

    int found = 0; // flag to indicate if a prime was found

    // 2. If 0 or 1 core, run the search in-process
    if (cores_num < 2)
    {
        found = search_iZprime(p, p_id, vx);
        mpz_clear(vx);
        return found;
    }

    // 3. Else, fork multiple processes to search for a prime
    // Create a pipe for inter-process communication.
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pids[cores_num];

    // Fork child processes.
    for (int i = 0; i < cores_num; i++)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            // Child process: close the read-end.
            mpz_t local_candidate;
            mpz_init(local_candidate);

            // Search for a candidate prime.
            found = search_iZprime(local_candidate, p_id, vx);

            // If a candidate is found, send it via the pipe.
            if (found)
            {
                char *candidate_str = mpz_get_str(NULL, 10, local_candidate);
                if (candidate_str != NULL)
                {
                    write(fd[1], candidate_str, strlen(candidate_str) + 1);
                    free(candidate_str);
                }
            }
            mpz_clear(local_candidate);

            close(fd[1]);
            exit(0);
        }
        else
        {
            // Parent process saves child's PID.
            pids[i] = pid;
        }
    }

    // 4. Parent reads first result from the pipe.
    close(fd[1]);
    int str_size = bit_size / 3;
    char buf[str_size];
    memset(buf, 0, str_size);

    ssize_t n = read(fd[0], buf, str_size); // Read from the pipe
    if (n == -1)
    {
        perror("read");
        exit(EXIT_FAILURE);
    }

    if (n > 0)
    {
        if (mpz_set_str(p, buf, 10) == 0)
            log_debug("Prime is set from buffer successfully");
        else
            log_debug("Failed to set prime from buffer");
    }
    close(fd[0]); // Close the read-end of the pipe.

    // 5. Terminate all child processes
    for (int i = 0; i < cores_num; i++)
    {
        kill(pids[i], SIGTERM);    // Terminate child process
        waitpid(pids[i], NULL, 0); // Wait for child process to terminate
    }

    // Cleanup
    mpz_clear(vx);

    return found;
}

/**
 * @brief Find the next prime number after a given base.
 *
 * @description: This function searches for the next prime number after a given base using the iZ framework.
 *
 * @param p The mpz_t variable to store the found prime number.
 * @param base The base number to start the search from.
 * @param forward If true, search for the next prime; if false, search for the previous prime.
 * @return 1 if a prime is found, 0 otherwise.
 */
int iZ_next_prime(mpz_t p, mpz_t base, int forward)
{
    // 1. Initialization
    int found = 0; // flag to indicate if a prime was found

    // select a suitable vx size enough to find a prime in a few segments
    // vx = 5 * 7 * 11 * 13 = 5005,
    // which covers a range of 6 * 5005 = 30030 natural numbers
    int vx = 5 * 7 * 11 * 13;

    // tmp variable to hold the base value until a prime is found
    mpz_t tmp;
    mpz_init_set(tmp, base); // set tmp = base

    // a. Edge cases:
    // if forward and base is iZ-, check next iZ+
    if (mpz_fdiv_ui(tmp, 6) == 5 && forward)
    {
        mpz_add_ui(tmp, tmp, 2); // increment tmp by 2
        if (mpz_probab_prime_p(tmp, TEST_ROUNDS))
        {
            mpz_set(p, tmp); // set p = tmp + 2
            mpz_clear(tmp);
            return 1;
        }
    }
    // if backward and base is iZ+, check previous iZ-
    else if (mpz_fdiv_ui(tmp, 6) == 1 && !forward)
    {
        mpz_sub_ui(tmp, tmp, 2); // decrement tmp by 2
        if (mpz_probab_prime_p(tmp, TEST_ROUNDS))
        {
            mpz_set(p, tmp); // set p = tmp - 2
            mpz_clear(tmp);
            return 1;
        }
    }

    // b. Create pre-sieved vx bitmaps
    BITMAP *x5 = bitmap_create(vx + 10);
    BITMAP *x7 = bitmap_create(vx + 10);
    // construct iZ matrix segment
    construct_iZm_segment(vx, x5, x7);

    // c. Initialize and set y and yvx
    mpz_t y, yvx;
    mpz_init(y);
    mpz_init(yvx);

    mpz_div_ui(y, base, 6 * vx); // compute y = base / 6 * vx
    mpz_mul_ui(yvx, y, vx);      // compute yvx = y * vx

    // d. Initialize and set x_p = tmp / 6
    mpz_t x_p;
    mpz_init(x_p);
    mpz_div_ui(x_p, tmp, 6);

    // 2. Iterate over the x5 and x7 bitmaps to find a prime
    // set start_x = x_p % vx +/- 1
    int step = forward ? 1 : -1;
    int start_x = mpz_fdiv_ui(x_p, vx) + step;
    int end_x = forward ? vx + 1 : 1;

    int i = 0;        // segment counter
    int max_i = 1000; // max number of segments to search

    while (!found && i < max_i)
    {
        if (forward)
        {
            if (i > 0)
                start_x = 1; // start from the beginning of the bitmap

            for (int x = start_x; x < end_x; x++)
            {
                // check if x5[x] is set
                if (bitmap_get_bit(x5, x))
                {
                    mpz_add_ui(x_p, yvx, x); // set x_p = yvx + x
                    iZ_gmp(tmp, x_p, -1);    // compute p = iZ(x_p, -1)
                    // check if tmp is prime
                    found = mpz_probab_prime_p(tmp, TEST_ROUNDS);

                    if (found)
                        break;
                }

                // check if x7[x] is set
                if (bitmap_get_bit(x7, x))
                {
                    mpz_add_ui(x_p, yvx, x); // set x_p = yvx + x
                    iZ_gmp(tmp, x_p, 1);     // compute tmp = iZ(x_p, 1)
                    // check if tmp is prime
                    found = mpz_probab_prime_p(tmp, TEST_ROUNDS);

                    if (found)
                        break;
                }
            }

            mpz_add_ui(yvx, yvx, vx); // increment yvx by vx for next segment
        }
        else // backward search
        {
            if (i > 0)
                start_x = vx; // start from the end of the bitmaps

            // check iZ+ first if backward
            for (int x = start_x; x > end_x; x--)
            {
                // check if x7[x] is set
                if (bitmap_get_bit(x7, x))
                {
                    mpz_add_ui(x_p, yvx, x); // set x_p = yvx + x
                    iZ_gmp(tmp, x_p, 1);     // compute tmp = iZ(x_p, 1)
                    // check if tmp is prime
                    found = mpz_probab_prime_p(tmp, TEST_ROUNDS);

                    if (found)
                        break;
                }

                // check iZ-
                if (bitmap_get_bit(x5, x))
                {
                    mpz_add_ui(x_p, yvx, x); // set x_p = yvx + x
                    iZ_gmp(tmp, x_p, -1);    // compute p = iZ(x_p, -1)
                    // check if tmp is prime
                    found = mpz_probab_prime_p(tmp, TEST_ROUNDS);

                    if (found)
                        break;
                }
            }

            mpz_sub_ui(yvx, yvx, vx); // decrement yvx by vx for next segment
        }

        i++; // increment segment counter
    }

    // 3. Set the found prime
    if (found)
        mpz_set(p, tmp); // set p = tmp
    else
        log_debug("No prime found :/");

    // cleanup
    bitmap_free(x5);
    bitmap_free(x7);
    mpz_clear(y);
    mpz_clear(yvx);
    mpz_clear(x_p);
    mpz_clear(tmp);

    return found;
}

/**
 * @brief Generates a random prime number using the iZ_next_prime function.
 *
 * @description: This function generates a random prime number of a given bit size using the iZ framework.
 * It initializes a random base and searches for the next prime number. The prime number is
 * stored in the provided mpz_t p variable.
 *
 * @param p The mpz_t variable to store the generated prime number.
 * @param bit_size The bit size of the prime number to be generated.
 */
int iZ_random_next_prime(mpz_t p, int bit_size)
{
    // Check if the bit size is within the valid range
    if (bit_size < 10)
    {
        log_debug("Using minimum bit size: 10\n");
        bit_size = 10; // minimum bit size
    }

    // Set the initial random number within the magnitude range
    mpz_t base;
    mpz_init(base);

    // initialize the random state
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_seed_randstate(state); // seed random state

    // Generate a random number in the given range
    mpz_urandomb(base, state, bit_size);

    // Find the next prime number after the random base
    int found = iZ_next_prime(p, base, 1);

    // cleanup
    mpz_clear(base);
    gmp_randclear(state);

    return found;
}

/**
 * @brief Generates a random prime number using GMP's mpz_nextprime function.
 *
 * @description: This function generates a random prime number of a given bit size using GMP's mpz_nextprime
 * function. It initializes a random base using GMP's random state and searches for the next prime
 * number. The prime number is stored in the provided mpz_t p variable.
 *
 * @param p The mpz_t variable to store the generated prime number.
 * @param bit_size The bit size of the prime number to be generated.
 */
void gmp_random_next_prime(mpz_t p, int bit_size)
{
    // Check if the bit size is within the valid range
    if (bit_size < 10)
    {
        log_debug("Using minimum bit size: 10");
        bit_size = 10; // minimum bit size
    }

    // Check if p is initialized
    if (!p->_mp_d)
        mpz_init(p);

    // Set the initial random number within the magnitude range
    mpz_t base;
    mpz_init(base);

    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_seed_randstate(state); // seed random state

    // Generate a random number in the given range
    mpz_urandomb(base, state, bit_size);

    // Find the next prime number after the random base
    mpz_nextprime(p, base);

    mpz_clear(base);
    gmp_randclear(state);
}
