/**
 * @file hamming.c
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 *
 * @brief Implementation of Hamming Code for error detection and correction.
 *
 * This file contains functions to encode data using Hamming code, decode
 * encoded data, and detect and correct single-bit errors. It also includes
 * a comprehensive test function to validate the implementation.
 *
 * @note
 *  This code was specifically designed for use and integration of
 *  Hamming(7,4) and calculates the required number of parity bits.
 *
 * @version 1.1
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#include "hamming.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/*-----------------------------------------------------------*/

static void hamming_encode(int *data, int data_bits, int *encoded_data)
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

/*-----------------------------------------------------------*/

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

/*-----------------------------------------------------------*/

static void hamming_decode(int *encoded_data, int n, int *decoded_data)
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
    int decoded_index = 0;
    for (int i = 0; i < n; i++) {
        // Check if the current position is a data bit and not a parity position
        if ((i & (i + 1)) != 0) {
            decoded_data[decoded_index++] = encoded_data[i];
        }
    }
}

/*-----------------------------------------------------------*/

void hamming_encode_74(const int *input_bits, int total_bits, int *out_bits)
{
    // total_bits should be a multiple of 4
    for (int i = 0; i < total_bits; i += 4) {
        int block[4] = {
            input_bits[i],
            input_bits[i + 1],
            input_bits[i + 2],
            input_bits[i + 3]};
        int encoded[7];
        hamming_encode(block, 4, encoded);
        for (int j = 0; j < 7; j++) {
            out_bits[(i / 4) * 7 + j] = encoded[j];
        }
    }
}

/*-----------------------------------------------------------*/

void hamming_decode_74(const int *in_bits, int total_bits, int *decoded_bits)
{
    for (int i = 0; i < total_bits / 4; i++) {
        int encoded[7];
        for (int j = 0; j < 7; j++) {
            encoded[j] = in_bits[i * 7 + j];
        }
        int block[4];
        hamming_decode(encoded, 7, block);

        // Copy back 4 decoded bits
        decoded_bits[i * 4 + 0] = block[0];
        decoded_bits[i * 4 + 1] = block[1];
        decoded_bits[i * 4 + 2] = block[2];
        decoded_bits[i * 4 + 3] = block[3];
    }
}

/*-----------------------------------------------------------*/

/* Tests used to verify hamming code implementation. Ignore lol */

// static void print_array(int *arr, int n, const char *label)
// {
//     printf("%s: ", label);
//     for (int i = 0; i < n; i++) {
//         printf("%d ", arr[i]);
//     }
//     printf("\n");
// }

// void test_all_combinations()
// {
//     int total_tests = 0;
//     int passed_tests = 0;
//     const int data_bits = 4;

//     printf("=== Starting Comprehensive Hamming Code Test ===\n");

//     // Test all 16 possible 4-bit data patterns
//     for (int pattern = 0; pattern < 16; pattern++) {
//         // Convert pattern number to binary array (MSB first)
//         int data[4];
//         for (int i = 0; i < 4; i++) {
//             data[i] = (pattern >> (3 - i)) & 1;
//         }

//         // Calculate required parameters
//         int n = 7;  // 4 data bits + 3 parity bits
//         int encoded_data[n];
//         hamming_encode(data, data_bits, encoded_data);

//         // Test every possible single-bit error position
//         for (int error_pos = 0; error_pos < n; error_pos++) {
//             total_tests++;

//             // Create modified copy with single-bit error
//             int modified[n];
//             memcpy(modified, encoded_data, sizeof(int) * n);
//             modified[error_pos] ^= 1;

//             // Decode the modified data
//             int decoded[data_bits];
//             hamming_decode(modified, n, decoded);

//             // Verify results
//             int match = 1;
//             for (int i = 0; i < data_bits; i++) {
//                 if (decoded[i] != data[i]) {
//                     match = 0;
//                     break;
//                 }
//             }

//             if (match) {
//                 passed_tests++;
//             }
//             else {
//                 printf("\nTest failed!\n");
//                 printf("Original data: ");
//                 for (int i = 0; i < data_bits; i++) printf("%d ", data[i]);
//                 printf("\nError injected at position: %d", error_pos);
//                 printf("\nDecoded result: ");
//                 for (int i = 0; i < data_bits; i++) printf("%d ", decoded[i]);
//                 printf("\n");
//             }
//         }
//     }

//     printf("\n=== Test Results ===\n");
//     printf("Total test cases: %d\n", total_tests);
//     printf("Successful corrections: %d\n", passed_tests);
//     printf("Failure rate: %.2f%%\n",
//            (total_tests - passed_tests) * 100.0 / total_tests);
// }

// int main()
// {
//     printf("=== Demonstration Test using encode_chunk and decode_chunk with 100 iterations ===\n");
//     {
//         for (int iter = 0; iter < 100; iter++) {
//             Chunk_t chunk_orig, chunk_decoded;
//             /* Initialize the dummy Chunk_t structure */
//             chunk_orig.header = 0xAA;
//             strncpy((char *)chunk_orig.data, "123456789 123456789 12345678", sizeof(chunk_orig.data));
//             /* Set crc to 0; encode_chunk will compute the proper CRC */
//             chunk_orig.crc = 0;

//             /* Buffer for encoded chunk data */
//             uint8_t encoded_chunk[CHUNK_ENCODED_SIZE];

//             /* Encode the original chunk */
//             if (encode_chunk(chunk_orig, encoded_chunk) != EXIT_SUCCESS) {
//                 printf("Encoding failed at iteration %d.\n", iter);
//                 exit(EXIT_FAILURE);
//             }

//             /* Introduce a single-bit error at a random bit position */
//             int total_encoded_bits = CHUNK_ENCODED_SIZE * 8;
//             int error_bit = rand() % total_encoded_bits;
//             int byte_index = error_bit / 8;
//             int bit_index = 7 - (error_bit % 8);
//             encoded_chunk[byte_index] ^= (1 << bit_index);

//             /* Decode the chunk back */
//             if (decode_chunk(encoded_chunk, &chunk_decoded) != EXIT_SUCCESS) {
//                 printf("\nDecoding FAILED (CRC mismatch) at iteration %d:\n", iter);
//                 printf("Original Chunk: Header = 0x%02X, Data = %s, CRC = 0x%04X\n",
//                        chunk_orig.header, chunk_orig.data, chunk_orig.crc);
//                 printf("Decoded  Chunk: Header = 0x%02X, Data = %s, CRC = 0x%04X\n",
//                        chunk_decoded.header, chunk_decoded.data, chunk_decoded.crc);
//             }
//         }
//         printf("\nTest iterations complete.\n");
//     }
//     return 0;
// }