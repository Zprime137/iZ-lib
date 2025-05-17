/**
 * @file vx_obj.c
 * @brief VX_OBJ and VX_ASSETS structure initialization and management functions.
 *
 * @description:
 * This file contains functions to initialize, free, and manage the VX_OBJ and VX_ASSETS structures.
 * The VX_OBJ structure is used to represent a segment of the iZ set, while the VX_ASSETS structure
 * contains reusable assets for sieving, such as root primes and pre-sieved base bitmaps.
 */

#include <vx_obj.h>
#include <iZ.h>

VX_ASSETS *vx_assets_init(size_t vx)
{
    VX_ASSETS *vx_assets = malloc(sizeof(VX_ASSETS));
    if (vx_assets == NULL)
    {
        log_error("Memory allocation failed for vx_assets.");
        return NULL;
    }

    vx_assets->vx = vx;
    // get root primes for sieving
    vx_assets->root_primes = sieve_iZ(vx);
    // construct pre-sieved base_x5, base_x7 bitmaps
    vx_assets->base_x5 = bitmap_create(vx + 10);
    vx_assets->base_x7 = bitmap_create(vx + 10);
    construct_iZm_segment(vx, vx_assets->base_x5, vx_assets->base_x7);

    return vx_assets;
}

void vx_assets_free(VX_ASSETS *vx_assets)
{
    if (vx_assets == NULL)
        return;

    bitmap_free(vx_assets->base_x5);
    bitmap_free(vx_assets->base_x7);
    free(vx_assets);
    vx_assets = NULL;
}

/**
 * @brief Initialize the members of the VX_OBJ structure with the given parameters.
 *
 * @description:
 * This function allocates memory for a VX_OBJ structure and initializes its members.
 * The p_count is initialized to 0, and the p_gaps array is allocated with
 * an initial size of (vx/2) of GAP_SIZE = uint16_t.
 *
 * Parameters:
 * @param y A character pointer representing a numeric string.
 *
 * @return VX_OBJ* A pointer to the initialized VX_OBJ structure.
 *        NULL if memory allocation fails or y is not a numeric string.
 */
VX_OBJ *vx_init(int vx, char *y)
{
    // check if y is numeric string
    if (!is_numeric_str(y))
    {
        printf("Invalid y string in vx_init\n");
        return NULL;
    }

    VX_OBJ *vx_obj = malloc(sizeof(VX_OBJ));
    if (vx_obj == NULL)
    {
        log_error("Memory allocation failed in vx_init\n");
        return NULL;
    }

    vx_obj->vx = vx;
    vx_obj->y = y;
    vx_obj->p_count = 0;
    vx_obj->p_gaps = malloc(vx / 2 * GAP_SIZE); // initial estimate

    return vx_obj;
}

/**
 * @brief Free the VX_OBJ structure.
 *
 * @description:
 * This function frees all memory associated with the VX_OBJ structure.
 *
 * @note: Do not free the y string, as it's ownership belongs to the caller.
 *
 * Parameters:
 * @param vx_obj Pointer to the VX_OBJ structure to be freed.
 */
void vx_free(VX_OBJ *vx_obj)
{
    if (vx_obj == NULL)
        return;

    // clear p_gaps array
    if (vx_obj->p_gaps)
    {
        free(vx_obj->p_gaps);
        vx_obj->p_gaps = NULL;
    }

    free(vx_obj);
    vx_obj = NULL;
}

/**
 * @brief Append a gap to the p_gaps array in the VX_OBJ structure.
 *
 * @param vx_obj Pointer to the VX_OBJ structure to which the gap will be appended.
 * @param gap The gap value to append.
 */
void vx_append_p_gap(VX_OBJ *vx_obj, int gap)
{
    // Append the new gap and increment the count
    vx_obj->p_gaps[vx_obj->p_count++] = gap;
}

/**
 * @brief Resize the p_gaps array in the VX_OBJ structure.
 *
 * @description:
 * This function resizes the p_gaps array to fit the actual count of gaps (p_count).
 * It reallocates memory for the p_gaps array based on the current p_count value.
 *
 * Parameters:
 * @param vx_obj Pointer to the VX_OBJ structure whose p_gaps array needs resizing.
 */
void vx_resize_p_gaps(VX_OBJ *vx_obj)
{
    if (vx_obj == NULL)
        return;

    vx_obj->p_gaps = realloc(vx_obj->p_gaps, vx_obj->p_count * sizeof(int));
}

/**
 * @brief Hash the p_gaps array in the VX_OBJ structure.
 *
 * @description:
 * This function computes the SHA256 hash of the p_gaps array in the VX_OBJ structure.
 * It uses the hash_int_array function to perform the hashing.
 *
 * Parameters:
 * @param vx_obj Pointer to the VX_OBJ structure whose p_gaps array needs hashing.
 */
void vx_compute_hash(VX_OBJ *vx_obj)
{
    if (vx_obj == NULL)
        return;

    hash_int_array((int *)vx_obj->p_gaps, vx_obj->p_count * GAP_SIZE, vx_obj->sha256);
}

/**
 * @brief Verify the SHA256 hash of the p_gaps array in the VX_OBJ structure.
 *
 * @description:
 * This function verifies the SHA256 hash of the p_gaps array in the VX_OBJ structure.
 * It compares the computed hash with the stored hash in the sha256 member of the VX_OBJ.
 *
 * Parameters:
 * @param vx_obj Pointer to the VX_OBJ structure whose hash needs verification.
 *
 * @return 1 if the hashes match, 0 otherwise.
 */
int vx_verify_hash(VX_OBJ *vx_obj)
{
    if (vx_obj == NULL)
        return 0;

    int is_valid = 1;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    hash_int_array((int *)vx_obj->p_gaps, vx_obj->p_count * GAP_SIZE, hash);

    // Compare the computed hash with the stored hash
    // memcmp returns 0 if the hashes match
    if (memcmp(vx_obj->sha256, hash, SHA256_DIGEST_LENGTH) != 0)
    {
        printf("Hash mismatch: data integrity check failed\n");
        is_valid = 0;
    }

    return is_valid;
}

/**
 * @brief vx_write_file - Write a VX_OBJ structure to a binary file.
 *
 * @description:
 * This function serializes the VX_OBJ structure into a binary file. It writes the following data:
 *   - The length of the y string (including the terminating null character) followed by the y string.
 *   - The p_count value indicating the number of elements in the p_gaps array.
 *   - The p_gaps array itself.
 *   - A SHA256 hash computed over the p_gaps array for data integrity, which is then written to the file.
 *
 * Parameters:
 * @param vx_obj: Pointer to a VX_OBJ structure containing data to be written.
 * @param filename: The full path of the file to write to. If the filename does not include the
 *            ".vx6" extension, it is automatically appended.
 *
 * @return:
 *   - 1 on successful write,
 *   - 0 if any error occurs (e.g., invalid parameters, failure to open the file, or file write errors).
 */
int vx_write_file(VX_OBJ *vx_obj, char *filename)
{
    if (vx_obj == NULL || filename == NULL)
        return 0;

    // check if filename includes the extension .vx6, if not append it
    if (strstr(filename, VX_EXT) == NULL)
        strcat(filename, VX_EXT);

    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        log_error("Could not open file %s for writing\n", filename);
        return 0;
    }

    // Write the length of the y string including null terminator
    size_t y_len = strlen(vx_obj->y) + 1;
    fwrite(&y_len, sizeof(size_t), 1, file);

    // Write the y string
    fwrite(vx_obj->y, sizeof(char), y_len, file);

    // Write p_count
    fwrite(&vx_obj->p_count, sizeof(size_t), 1, file);

    // Write p_gaps array
    fwrite(vx_obj->p_gaps, GAP_SIZE, vx_obj->p_count, file);

    // Calculate and write SHA256 hash of p_gaps
    vx_compute_hash(vx_obj);

    // Write hash
    fwrite(vx_obj->sha256, SHA256_DIGEST_LENGTH, 1, file);

    fclose(file);
    return 1;
}

/**
 * vx_read_file - Read a VX_OBJ structure from a binary file.
 *
 * @description:
 * This function reads the VX_OBJ structure from a binary file by performing the following steps:
 *   - Reads the length of the y string and allocates memory for it, then reads the y string.
 *   - Reads the p_count value to determine the number of elements in the p_gaps array.
 *   - Reads the p_gaps array (assuming the p_gaps field in vx_obj has been properly allocated).
 *   - Reads the previously stored SHA256 hash and computes a new hash on the read p_gaps array.
 *   - Compares the computed hash with the read hash to validate data integrity.
 *
 *  Parameters:
 * @param vx_obj: Pointer to a VX_OBJ structure where the read data will be stored.
 * @param filename: The name (or path) of the file to read from. If the filename does not include the
 *            ".vx6" extension, it is automatically appended.
 *
 * @return:
 *   - 1 if the file is successfully read and the hash validation passes,
 *   - 0 if any error occurs (e.g., invalid parameters, file read errors, memory allocation failure,
 *         or hash mismatch).
 */
int vx_read_file(VX_OBJ *vx_obj, char *filename)
{
    if (vx_obj == NULL || filename == NULL)
        return 0;

    // check if filename includes the extension .vx, if not append it
    if (strstr(filename, VX_EXT) == NULL)
        strcat((char *)filename, VX_EXT);

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("Could not open file %s for reading\n", filename);
        return 0;
    }

    // Read the length of the y string and allocate memory
    int is_valid = 1;
    size_t y_len;
    fread(&y_len, sizeof(size_t), 1, file);
    vx_obj->y = malloc(y_len);

    if (vx_obj->y == NULL)
    {
        log_error("Memory allocation failed for y in vx6_read_file\n");
        is_valid = 0;
    }
    fread(vx_obj->y, sizeof(char), y_len, file);

    // Read p_count
    if (fread(&vx_obj->p_count, sizeof(size_t), 1, file) != 1)
        is_valid = 0;

    // Read p_gaps array
    if (fread(vx_obj->p_gaps, GAP_SIZE, vx_obj->p_count, file) != (size_t)vx_obj->p_count)
        is_valid = 0;

    // Read hash
    if (fread(vx_obj->sha256, SHA256_DIGEST_LENGTH, 1, file) != 1)
        is_valid = 0;

    // Validate integrity of the p_gaps array
    is_valid = vx_verify_hash(vx_obj);

    fclose(file);
    return is_valid;
}

/**
 * @brief Print the p_gaps array in the VX_OBJ structure.
 *
 * @description:
 * This function prints the first 'count' elements of the p_gaps array in the VX_OBJ structure.
 *
 * Parameters:
 * @param vx_obj Pointer to the VX_OBJ structure containing the p_gaps array.
 * @param count  The number of elements to print from the p_gaps array.
 */
void vx_print_p_gaps(VX_OBJ *vx_obj, int count)
{
    printf("| %-16s: [%d", "p_gaps", vx_obj->p_gaps[0]);
    for (int i = 1; i < count && i < vx_obj->p_count; i++)
        printf(", %d", vx_obj->p_gaps[i]);
    printf(", ...]\n");
}

// Analyze and print primes parity in given vx vectors in iZm5 and iZm7.
/**
 * @brief Print the header for vx statistics.
 *
 * This function prints the header for the vx statistics table.
 * It includes the range of natural numbers, the number of primes in iZ- and iZ+,
 * and the total number of primes, twin primes, cousin primes, and sexy primes.
 */
void print_vx_header(void)
{
    print_line(92);
    printf("| %-12s", "Range");      // a segment of natural numbers
    printf("| %-12s", "#(Primes)");  // total #primes
    printf("| %-12s", "#(Twins)");   // total #twin primes
    printf("| %-12s", "#(Cousins)"); // total #cousin primes
    printf("| %-12s", "#(Sexy)");    // total #sexy primes
    print_line(92);
}

/**
 * @brief Analyze and print primes parity in given vx vectors in iZm5 and iZm7.
 *
 * Parameters:
 * @param vx The vx size, covering 6 * vx natural numbers.
 * @param x5 A BITMAP pointer representing the primes in iZm5.
 * @param x7 A BITMAP pointer representing the primes in iZm7.
 */
void print_vx_stats(VX_OBJ *vx_obj)
{
    // Initialize counters
    int twin_count = 0;
    int cousins_count = 0;
    int sexy_count = 0;

    for (int i = 0; i < vx_obj->p_count; i++)
    {
        if (vx_obj->p_gaps[i] == 2)
            twin_count++;
        else if (vx_obj->p_gaps[i] == 4)
            cousins_count++;
        else if (vx_obj->p_gaps[i] == 6)
            sexy_count++;
    }

    // Print results as a formatted row
    printf("| %-12d", 6 * vx_obj->vx); // range of natural numbers
    printf("| %-12d", vx_obj->p_count);
    printf("| %-12d", twin_count);
    printf("| %-12d", cousins_count);
    printf("| %-12d\n", sexy_count);
}
