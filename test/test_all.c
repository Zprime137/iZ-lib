// test_all.c - Test Harness for Sieve Algorithms and Related Functionalities

#include <iZ.h>

// Test functions prototypes
int testing_sieve_integrity(void);
int testing_sieve_vx(void);
int testing_vx_io(void);
int testing_next_prime_gen(void);
int testing_prime_gen_algorithms(void);

int main(void)
{
    // Set log level to debug to show detailed test output
    log_set_log_level(LOG_DEBUG);
    log_info("=== Running All Tests ===");

    printf("=== Running All Tests ===\n");

    int is_success = 0;
    // Run all tests:
    is_success = testing_sieve_integrity();
    is_success = testing_sieve_vx();
    is_success = testing_vx_io();
    is_success = testing_next_prime_gen();
    is_success = testing_prime_gen_algorithms();

    printf("\n");
    if (is_success)
        printf("All tests passed successfully ^_^\n");
    else
        printf("Some tests failed. Please check the logs for details :/\n");

    printf("=== Tests Completed ===\n");

    return 0;
}
