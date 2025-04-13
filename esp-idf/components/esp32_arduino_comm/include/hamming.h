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
 * @version 1.1
 * @date 2025-03-08
 *
 * @copyright Copyright (c) 2025
 */

#ifndef __HAMMING_H__
#define __HAMMING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

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
 * @brief Encode data using Hamming(7,4) error correction.
 *
 * This function encodes the provided data using the Hamming(7,4) scheme,
 * by calculating and inserting the appropriate parity bits. The parity
 * bits are determined using the parity_check() function to ensure even parity.
 *
 * The function handles the conversion from data bits to an encoded form
 * that includes both data and parity bits, preparing the array for error
 * detection and correction.
 *
 * @param data Pointer to the array containing the data bits to be encoded.
 * @param data_bits The number of data bits in the input array.
 * @param encoded_data Pointer to the array where the complete encoded data 
 *                     (including parity bits) will be stored.
 *
 * @note Ensure that the encoded_data array has been pre-allocated with
 * sufficient size to accommodate both the data and parity bits.
 */
void hamming_encode_74(const int *input_bits, int total_bits, int *out_bits);

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
 * @brief Decode Hamming(7,4) encoded data with error detection and correction.
 *
 * This function decodes data encoded with the Hamming(7,4) error correction scheme.
 * It performs the following operations:
 *   1. Calculates the syndrome (using calculate_syndrome()) to detect single-bit errors.
 *   2. Corrects the erroneous bit if a non-zero syndrome is detected.
 *   3. Determines the positions of parity bits based on the Hamming(7,4) format.
 *   4. Extracts the original data bits from the encoded array, discarding the parity bits.
 *
 * @note The encoded_data is expected to be generated using the Hamming(7,4) encoding method.
 *       This function depends on both the calculate_syndrome() and parity_check() functions.
 *
 * @param encoded_data Pointer to the array containing the encoded data bits (data and parity).
 * @param n The total number of bits in the encoded_data array.
 * @param decoded_data Pointer to the array where the original decoded data bits will be stored.
 */
void hamming_decode_74(const int *in_bits, int total_bits, int *decoded_bits);

#ifdef __cplusplus
}
#endif

#endif  // __HAMMING_H__