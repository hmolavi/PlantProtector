
/**
 * @brief Calculate the parity for a given parity bit position.
 *
 * This function calculates the parity for a specific parity bit position in the Hamming code.
 *
 * @param n The total number of bits in the encoded data.
 * @param data The array of bits representing the encoded data.
 * @param p The index of the parity check (0 for P1, 1 for P2, etc.).
 * @return The calculated parity (0 or 1).
 */
int parity_check(int n, int *data, int p);

/**
 * @brief Encode data using Hamming code.
 *
 * This function encodes the given data using Hamming code by calculating and inserting the necessary parity bits.
 *
 * @param data The array of bits representing the original data.
 * @param data_bits The number of bits in the original data.
 * @param encoded_data The array to store the encoded data with parity bits.
 */
void hamming_encode(int *data, int data_bits, int *encoded_data);

/**
 * @brief Calculate the syndrome for error detection.
 *
 * This function calculates the syndrome for the given encoded data to detect any errors.
 *
 * @param n The total number of bits in the encoded data.
 * @param encoded_data The array of bits representing the encoded data.
 * @return The calculated syndrome value.
 */
int calculate_syndrome(int n, int *encoded_data);

/**
 * @brief Decode encoded data using Hamming code.
 *
 * This function decodes the given encoded data by correcting any single-bit errors and extracting the original data bits.
 *
 * @param encoded_data The array of bits representing the encoded data.
 * @param n The total number of bits in the encoded data.
 * @param decoded_data The array to store the decoded original data bits.
 */
void hamming_decode(int *encoded_data, int n, int *decoded_data);
