/**
 * @file hamming_code.c
 * @brief Implementation of Hamming Code for error detection and correction.
 *
 * This file contains functions to encode data using Hamming code, decode
 * encoded data, and detect and correct single-bit errors. It also includes
 * a comprehensive test function to validate the implementation.
 *
 * Functions:
 * - int parity_check(int n, int *data, int p): Calculate the parity for a given parity bit position.
 * - void hamming_encode(int *data, int data_bits, int *encoded_data): Encode data using Hamming code.
 * - int calculate_syndrome(int n, int *encoded_data): Calculate the syndrome for error detection.
 * - void hamming_decode(int *encoded_data, int n, int *decoded_data): Decode encoded data using Hamming code.
 * - void print_array(int *arr, int n, const char *label): Print an array with a label.
 * - void test_all_combinations(): Test all possible single-bit errors for 4-bit data patterns.
 *
 * Example usage:
 * - The main function demonstrates encoding, introducing an error, and decoding.
 * - The test_all_combinations function performs comprehensive testing of the Hamming code implementation.
 *
 * @note This implementation assumes the use of 4 data bits and calculates the required number of parity bits.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Calculate the parity for a given parity bit position.
 *
 * This function calculates the parity for a specified parity bit position
 * in the data array. The parity bit position is determined by the index p.
 *
 * @param n The total number of bits in the encoded data.
 * @param data Pointer to the array containing the data bits.
 * @param p The index of the parity check (0 for P1, 1 for P2, etc.).
 * @return The calculated parity (0 or 1).
 */
int parity_check(int n, int *data, int p)
{
    int mask = 1 << p;  // 1-based position of the parity bit (2^p)
    int sum = 0;
    // Iterate over all bits starting from the current parity bit's position
    for (int i = mask - 1; i < n; i++) {
        // Check if the (i+1)-th bit has the p-th bit set (1-based position)
        if (((i + 1) & mask) != 0) {
            sum ^= data[i];
        }
    }
    return sum;
}

/**
 * @brief Encode data using Hamming code.
 *
 * This function encodes the given data using Hamming code, which includes
 * calculating and setting the necessary parity bits. The encoded data is
 * stored in the provided encoded_data array.
 *
 * - Dependant on parity_check()
 *
 * @param data Pointer to the array containing the data bits to be encoded.
 * @param data_bits The number of data bits in the input array.
 * @param encoded_data Pointer to the array where the encoded data will be stored.
 */
void hamming_encode(int *data, int data_bits, int *encoded_data)
{
    // Calculate the required number of parity bits (r)
    int r = 0;
    while ((1 << r) < (data_bits + r + 1)) {
        r++;
    }
    int n = data_bits + r;

    // Initialize encoded data to 0
    for (int i = 0; i < n; i++) {
        encoded_data[i] = 0;
    }

    // Place data bits in non-parity positions
    int j = 0;
    for (int i = 0; i < n; i++) {
        // Check if current position is a parity bit position (i+1 is a power of 2)
        if ((i & (i + 1)) == 0) {
            continue;  // Skip parity positions
        }
        if (j < data_bits) {  // Ensure we don't exceed data array bounds
            encoded_data[i] = data[j];
            j++;
        }
    }

    // Calculate and set the parity bits
    for (int p = 0; p < r; p++) {
        int parity_pos = (1 << p) - 1;  // 0-based index
        if (parity_pos < n) {           // Ensure parity position is within bounds
            encoded_data[parity_pos] = parity_check(n, encoded_data, p);
        }
    }
}

/**
 * @brief Calculate the syndrome for the given encoded Hamming code data.
 *
 * This function calculates the syndrome for error detection in a Hamming code.
 * The syndrome is used to identify the position of a single-bit error in the
 * encoded data. The function depends on the `parity_check` function to compute
 * the parity for each parity bit position.
 *
 * - Dependant on parity_check()
 *
 * @param n The number of bits in the encoded_data array.
 * @param encoded_data Pointer to the array containing the encoded Hamming code data.
 * @return The calculated syndrome, which indicates the position of a single-bit error.
 */
int calculate_syndrome(int n, int *encoded_data)
{
    int syndrome = 0;
    int p = 0;
    // Determine the number of parity bits needed (r) such that 2^r >= n + r + 1
    // For simplicity, iterate while 2^p <= n
    while ((1 << p) <= n) {
        int parity_pos = (1 << p) - 1;  // 0-based position of parity bit
        if (parity_pos >= n) break;
        // Compute the parity check for this p
        int check = parity_check(n, encoded_data, p);
        // If the check is non-zero, set the corresponding syndrome bit
        syndrome |= (check << p);
        p++;
    }
    return syndrome;
}

/**
 * @brief Decodes encoded data using Hamming code.
 *
 * This function takes encoded data that uses Hamming code for error detection
 * and correction, calculates the syndrome to detect any errors, corrects the
 * error if detected, and then extracts the original data bits from the encoded
 * data.
 *
 * - Dependant on calculate_syndrome()
 * - Dependant on parity_check()
 *
 * @param encoded_data Pointer to the array of encoded data bits.
 * @param n The number of bits in the encoded data.
 * @param decoded_data Pointer to the array where the decoded data bits will be stored.
 *
 * The function performs the following steps:
 * 1. Calculates the syndrome to detect errors in the encoded data.
 * 2. If an error is detected (syndrome is non-zero), it corrects the error by flipping the incorrect bit.
 * 3. Determines the number of parity bits used in the encoded data.
 * 4. Extracts the original data bits from the encoded data, ignoring the parity bits.
 */
void hamming_decode(int *encoded_data, int n, int *decoded_data)
{
    // Calculate the syndrome to detect errors
    int syndrome = calculate_syndrome(n, encoded_data);

    // Correct the error if syndrome is non-zero
    if (syndrome != 0) {
        int error_pos = syndrome - 1;  // Convert to 0-based index
        if (error_pos < n) {
            encoded_data[error_pos] ^= 1;  // Flip the incorrect bit
        }
    }

    // Determine the number of parity bits (r) used in the encoded data
    int r = 0;
    while ((1 << r) <= n) {
        r++;
    }

    // Extract data bits
    int data_bits = n - r;
    int decoded_index = 0;
    for (int i = 0; i < n; i++) {
        // Check if the current position is a data bit and not a parity position
        if ((i & (i + 1)) != 0) {
            decoded_data[decoded_index++] = encoded_data[i];
        }
    }
}

void print_array(int *arr, int n, const char *label)
{
    printf("%s: ", label);
    for (int i = 0; i < n; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

void test_all_combinations()
{
    int total_tests = 0;
    int passed_tests = 0;
    const int data_bits = 4;

    printf("=== Starting Comprehensive Hamming Code Test ===\n");

    // Test all 16 possible 4-bit data patterns
    for (int pattern = 0; pattern < 16; pattern++) {
        // Convert pattern number to binary array (MSB first)
        int data[4];
        for (int i = 0; i < 4; i++) {
            data[i] = (pattern >> (3 - i)) & 1;
        }

        // Calculate required parameters
        int n = 7;  // 4 data bits + 3 parity bits
        int encoded_data[n];
        hamming_encode(data, data_bits, encoded_data);

        // Test every possible single-bit error position
        for (int error_pos = 0; error_pos < n; error_pos++) {
            total_tests++;

            // Create modified copy with single-bit error
            int modified[n];
            memcpy(modified, encoded_data, sizeof(int) * n);
            modified[error_pos] ^= 1;

            // Decode the modified data
            int decoded[data_bits];
            hamming_decode(modified, n, decoded);

            // Verify results
            int match = 1;
            for (int i = 0; i < data_bits; i++) {
                if (decoded[i] != data[i]) {
                    match = 0;
                    break;
                }
            }

            if (match) {
                passed_tests++;
            }
            else {
                printf("\nTest failed!\n");
                printf("Original data: ");
                for (int i = 0; i < data_bits; i++) printf("%d ", data[i]);
                printf("\nError injected at position: %d", error_pos);
                printf("\nDecoded result: ");
                for (int i = 0; i < data_bits; i++) printf("%d ", decoded[i]);
                printf("\n");
            }
        }
    }

    printf("\n=== Test Results ===\n");
    printf("Total test cases: %d\n", total_tests);
    printf("Successful corrections: %d\n", passed_tests);
    printf("Failure rate: %.2f%%\n",
           (total_tests - passed_tests) * 100.0 / total_tests);
}

int main()
{
    // Run single demonstration test
    int demo_data[] = {1, 1, 1, 0};
    int demo_bit_to_change = 1;
    int data_bits = sizeof(demo_data) / sizeof(demo_data[0]);
    int parity_bits = 0;
    while ((1 << parity_bits) < (data_bits + parity_bits + 1)) parity_bits++;
    int n = data_bits + parity_bits;

    int encoded[n], decoded[data_bits];

    printf("=== Demonstration Test ===\n");
    hamming_encode(demo_data, data_bits, encoded);
    print_array(encoded, n, "Encoded Data");

    encoded[demo_bit_to_change] ^= 1;
    print_array(encoded, n, "With Error  ");

    hamming_decode(encoded, n, decoded);
    print_array(decoded, data_bits, "Decoded Data");

    // Run comprehensive tests
    test_all_combinations();

    return 0;
}
