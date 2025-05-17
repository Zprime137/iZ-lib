#include <benchmark.h>
#include <sys/stat.h> // For mkdir
#include <time.h>     // For time
#include <string.h>   // For memcpy

SieveAlgorithm ClassicSieveOfEratosthenes = {classic_sieve_eratosthenes, "Classic Sieve of Eratosthenes"};
SieveAlgorithm SieveOfEratosthenes = {sieve_eratosthenes, "Sieve of Eratosthenes"};
SieveAlgorithm SegmentedSieve = {segmented_sieve, "Segmented Sieve"};
SieveAlgorithm SieveOfEuler = {sieve_euler, "Sieve of Euler"};
SieveAlgorithm SieveOfAtkin = {sieve_atkin, "Sieve of Atkin"};
SieveAlgorithm Sieve_iZ = {sieve_iZ, "Sieve-iZ"};
SieveAlgorithm Sieve_iZm = {sieve_iZm, "Sieve-iZm"};

/**
 * @brief Tests the integrity of different sieve models by comparing their hash values.
 *
 * This function iterates through a list of sieve models, computes the prime numbers up to `n` using each model,
 * and then compares the SHA-256 hash of the resulting prime numbers. If all hashes match, the integrity is confirmed.
 *
 * @param sieve_models A structure containing the list of sieve models to be tested.
 * @param n The upper limit for the prime number generation.
 * @return 1 if the integrity is confirmed, 0 if a hash mismatch is detected.
 */
int test_sieve_integrity(SieveModels sieve_models, uint64_t n)
{
    unsigned char results[sieve_models.models_count][32]; // Store hashes for each model

    for (int i = 0; i < sieve_models.models_count; i++)
    {
        SieveAlgorithm sieve_model = sieve_models.models_list[i];
        print_line(32);
        printf("Testing %s: n = %lld\n", sieve_model.name, n);

        // Call the sieve function
        PRIMES_OBJ *primes = sieve_model.function(n);
        if (primes == NULL)
        {
            printf("Failed to generate primes with %s\n", sieve_model.name);
            return 0;
        }
        // Compute the SHA-256 hash of the primes
        primes_obj_compute_hash(primes);

        printf("Primes Count : %d <= %lld\n", primes->p_count, n);
        printf("Last Prime: %lld\n", primes->p_array[primes->p_count - 1]);
        print_sha256_hash(primes->sha256);

        // Store the hash in the results array
        memcpy(results[i], primes->sha256, 32);

        primes_obj_free(primes);
        print_line(32);
    }

    // Compare all hashes to first hash
    int all_hashes_match = 1;
    for (int i = 1; i < sieve_models.models_count; i++)
    {
        if (memcmp(results[0], results[i], 32) != 0)
        {
            all_hashes_match = 0;
            printf("Hash mismatch detected for %s\n", sieve_models.models_list[i].name);
        }
    }

    if (all_hashes_match)
    {
        printf("All hashes match. Integrity confirmed ^_^\n");
        return 1;
    }
    else
    {
        printf("Hash mismatch detected. Integrity not confirmed :\\\n");
        return 0;
    }
}

/**
 * @brief Measures the execution time of a given sieve algorithm.
 *
 * This function takes a sieve algorithm and an upper limit `n`, runs the algorithm,
 * and measures the time taken to execute it. It prints the algorithm name, the value of `n`,
 * the count of prime numbers found, the last prime number in the list, and the time taken in seconds.
 * It returns the execution time in microseconds.
 *
 * @param algorithm The sieve algorithm to be measured.
 * @param n The upper limit for the sieve algorithm.
 * @return The execution time in microseconds.
 */
size_t measure_sieve_time(SieveAlgorithm model, uint64_t n)
{
    clock_t start, end;
    double cpu_time_used;

    start = clock();                        // Start time
    PRIMES_OBJ *primes = model.function(n); // Run time
    end = clock();                          // End time

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("| %-16lld", n);
    printf("| %-16d", primes->p_count);
    printf("| %-16lld", primes->p_array[primes->p_count - 1]);
    printf("| %-16f\n", cpu_time_used);

    primes_obj_free(primes);
    return (size_t)(cpu_time_used * 1000000); // time in microseconds;
}

// Function to save benchmarks results to a file named by timestamp
/**
 * @brief Saves the results of the sieve models to a file.
 *
 * This function creates a directory if it does not exist, generates a timestamped
 * filename, and writes the results of the sieve models to this file. The results
 * include metadata about the test range and the results of each sieve model.
 *
 * @param sieve_models The sieve models containing the algorithms and their names.
 * @param all_results A 2D array containing the results of the sieve models.
 * @param base The base number used in the test range.
 * @param min_exp The minimum exponent used in the test range.
 * @param max_exp The maximum exponent used in the test range.
 */
static void save_sieve_results_file(SieveModels sieve_models, int all_results[][32], int base, int min_exp, int max_exp)
{
    struct stat st = {0};
    if (stat(DIR_output, &st) == -1)
        mkdir(DIR_output, 0700);

    // Get the current timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char timestamp[64];
    strftime(timestamp, sizeof(timestamp) - 1, "%Y%m%d%H%M%S", t);

    // Create the output file path
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/sieve_results_%s.txt", DIR_output, timestamp);

    // Open the file for writing
    FILE *fp = fopen(file_path, "w");
    if (fp == NULL)
    {
        log_error("Failed to open file");
        return;
    }

    // Write the test range metadata
    fprintf(fp, "Test Range: %d^%d:%d^%d\n", base, min_exp, base, max_exp);

    // Write the results to the file
    for (int i = 0; i < sieve_models.models_count; i++)
    {
        SieveAlgorithm model = sieve_models.models_list[i];

        fprintf(fp, "%s: [", model.name);
        for (int j = 0; j <= max_exp - min_exp; j++)
        {
            fprintf(fp, "%d", all_results[i][j]);
            if (j < max_exp - min_exp)
                fprintf(fp, ", ");
        }
        fprintf(fp, "]\n");
    }

    // Close the file
    fclose(fp);
    printf("\nResults saved to %s\n", file_path);
}

/**
 * @brief Benchmarks the performance of different sieve algorithms over a range of exponents.
 *
 * This function measures the execution time of various sieve algorithms for a range of values
 * determined by the base raised to the power of exponents from min_exp to max_exp. The results
 * are printed and optionally saved to a file.
 *
 * @param sieve_models A structure containing the list of sieve algorithms to benchmark.
 * @param base The base value to be raised to the power of exponents.
 * @param min_exp The minimum exponent value.
 * @param max_exp The maximum exponent value.
 * @param save_results A flag indicating whether to save the results to a file named by timestamp in the output directory.
 */
void benchmark_sieve_models(SieveModels sieve_models, int base, int min_exp, int max_exp, int save_results)
{
    int all_results[sieve_models.models_count][32];

    for (int i = 0; i < sieve_models.models_count; i++)
    {
        SieveAlgorithm model = sieve_models.models_list[i];

        // results array: [time in microsecond]
        int results[32];
        int k = 0;

        printf("\nAlgorithm: %s", model.name);
        print_line(75);
        printf("| %-16s", "n");
        printf("| %-16s", "Primes Count");
        printf("| %-16s", "Last Prime");
        printf("| %-16s", "Time (s)");
        print_line(75);

        // warmup
        PRIMES_OBJ *primes = model.function(pow(base, min_exp));
        primes_obj_free(primes);

        for (int j = min_exp; j <= max_exp; j++)
            results[k++] = measure_sieve_time(model, pow(base, j)); // returns time in microseconds

        print_line(75);

        for (int j = 0; j < k; j++)
            all_results[i][j] = results[j];

        // print results array
        printf("Results summary of %s\n", model.name);
        printf("Test range: [%d^%d : %d^%d]\n", base, min_exp, base, max_exp);
        printf("Execution time in microseconds: [%d", results[0]);
        for (int j = 1; j < k; j++)
            printf(", %d", results[j]);
        printf("]\n");
        fflush(stdout);
    }

    // save results in a file named by timestamp
    if (save_results)
        save_sieve_results_file(sieve_models, all_results, base, min_exp, max_exp);
}

/**
 * benchmark_sieve_vx6 - Benchmark the sieve_vx6 function.
 *
 * This function benchmarks the sieve_vx function by measuring its execution time
 * and printing the results. It initializes a VX_OBJ structure with a given y value,
 * runs the sieve algorithm, and then prints the execution time and prime statistics.
 *
 * Parameters:
 * @param y Pointer to a string representing the y value for the VX_OBJ structure.
 * @param filename A pointer to a string representing the filename for output.
 */
void test_sieve_vx6(char *y, char *filename)
{
    print_line(92);
    printf("Testing Sieve-VX");
    print_line(92);
    fflush(stdout);

    size_t vx = VX6; // practical segment size
    VX_ASSETS *vx_assets = vx_assets_init(vx);
    VX_OBJ *vx_obj = vx_init(vx, y);

    clock_t start, end;
    double cpu_time_used;

    start = clock();             // Start time
    sieve_vx(vx_obj, vx_assets); // Run time
    end = clock();               // End time

    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("| %-16s: %d\n", "VX", vx_obj->vx);
    printf("| %-16s: %s\n", "Y", vx_obj->y);
    printf("| %-16s: %d\n", "Primes Count", vx_obj->p_count);
    printf("| %-16s: %f\n", "Execution time", cpu_time_used);
    printf("| %-16s: %d\n", "bit_ops", vx_obj->bit_ops);
    printf("| %-16s: %d\n", "p_test_ops", vx_obj->p_test_ops);
    vx_print_p_gaps(vx_obj, 10); // print p_gaps array

    // compute p = 6(y * vx)
    mpz_t p;
    mpz_init(p);
    mpz_set_str(p, vx_obj->y, 10);              // Set p from vx_obj->y
    mpz_mul_ui(p, p, vx_obj->vx);               // Compute p = y * vx
    mpz_mul_ui(p, p, 6);                        // Compute p = 6(y * vx)
    int p_byte_size = mpz_sizeinbase(p, 2) / 8; // Get byte size of p
    printf("| %-16s: %d Bytes\n", "Prime byte size", p_byte_size);
    printf("| %-16s: %d KB\n", "Total byte size", (vx_obj->p_count * p_byte_size) / 1024);

    // print primes statistics #primes, #twin, #cousin, #sexy
    print_vx_header();
    print_vx_stats(vx_obj);

    if (filename)
    {
        if (vx_write_file(vx_obj, filename))
            printf("\nResults saved to %s", filename);
        else
            printf("Error saving results to %s", filename);
    }

    print_line(92);
    fflush(stdout);

    // cleanup
    vx_free(vx_obj); // free vx_obj
    vx_assets_free(vx_assets);
    mpz_clear(p); // clear GMP variables
}
