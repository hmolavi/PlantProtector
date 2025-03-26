/**
 * @file hamming.h
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)

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
 *
 * @version 1.0
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Calculate the even-parity for a given parity bit position.
 *
 * This function calculates the parity for a specified parity bit position
 * in the data array. The parity bit position is determined by the index p.
 *
 * @param n The total number of bits in the encoded data.
 * @param data Pointer to the array containing the data bits.
 * @param p The index of the parity check (0 for P1, 1 for P2, etc.).
 * @return The calculated parity (0 or 1).
 */
int parity_check(int n, int *data, int p);

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
void hamming_encode(int *data, int data_bits, int *encoded_data);

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
int calculate_syndrome(int n, int *encoded_data);

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
void hamming_decode(int *encoded_data, int n, int *decoded_data);

void print_array(int *arr, int n, const char *label);