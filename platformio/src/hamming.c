/**
 * @file hamming.ino
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
 * @version 1.0
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
    int decoded_index = 0;
    for (int i = 0; i < n; i++) {
        // Check if the current position is a data bit and not a parity position
        if ((i & (i + 1)) != 0) {
            decoded_data[decoded_index++] = encoded_data[i];
        }
    }
}
