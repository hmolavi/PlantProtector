/**
 * @file param_table.inc
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * @brief Defined the parameters used in the PlantProtector project.
 * 
 * This file contains both the parameter macros which list all the configurable
 * parameters for the program. Each parameter is defined using the PARAM or ARRAY
 * macro, specifying its security level, data type, variable name, default value,
 * description, and PascalCase name.
 * 
 * The available data types for parameters are:
 * - char
 * - uint8_t
 * - uint16_t
 * - uint32_t
 * - int32_t
 * - float
 * - bool
 * 
 * For array parameters, the maximum length must be specified.
 * 
 * Refer to secure_level.c for more info on security levels 
 * 
 * Macros:
 * - SECURE_LEVEL(secure_level, description)
 *   Example:
 *      SECURE_LEVEL(0, "User level")
 *      SECURE_LEVEL(1, "Maintenance") 
 *      SECURE_LEVEL(2, "User level")
 * 
 * - PARAM(secure_level, type, name, default, description)
 *   Example:
 *      PARAM(2, char, ExampleChar, 'A', "example char")
 *      PARAM(2, uint8_t, ExampleUint8, 255, "example uint8_t")
 *      PARAM(2, uint16_t, ExampleUint16, 65535, "example uint16_t")
 *      PARAM(2, uint32_t, ExampleUint32, 4294967295, "example uint32_t")
 *      PARAM(2, int32_t, ExampleInt32, -2147483648, "example int32_t")
 *      PARAM(2, float, ExampleFloat, 3.14, "example float")
 * 
 * - ARRAY(secure_level, type, size, name, default, description)
 *   Example:
 *      ARRAY(2, char, 32, ExampleCharArray, "example char array", "example char array")
 *      ARRAY(2, uint8_t, 5, ExampleUint8Array, ARRAY_INIT(1, 2, 3, 4, 5), "example uint8_t array")
 *      ARRAY(2, uint16_t, 3, ExampleUint16Array, ARRAY_INIT(100, 200, 300), "example uint16_t array")
 *      ARRAY(2, int32_t, 4, ExampleUint32Array, ARRAY_INIT(1, 0, 0, 0), "example int array")
 *      ARRAY(2, float, 3, ExampleFloatArray, ARRAY_INIT(1.1, 2.2, 3.3), "example float array")
 *      ARRAY(2, bool, 2, ExampleBoolArray, ARRAY_INIT(true, false), "example bool array")
 */

#define ARRAY_INIT(...) {__VA_ARGS__}

#ifndef PARAM
#define PARAM(...)
#endif

#ifndef ARRAY
#define ARRAY(...)
#endif

#ifndef SECURE_LEVEL
#define SECURE_LEVEL(...)
#endif

SECURE_LEVEL(0, "User level")
SECURE_LEVEL(1, "Maintenance")
SECURE_LEVEL(2, "User level")


PARAM(0, int32_t, SerialNumber, 0, "Device's unique identifier")
PARAM(2, int32_t, Brightness, 50, "brightness duh")
PARAM(2, uint32_t, Internval, 1000, "random interval")
PARAM(2, bool, SeriousMode, false, "Determines AIs tone of voice")
ARRAY(2, char, 32, Ssid, "fakessid", "WiFi ssid (name)")
ARRAY(2, char, 64, Password, "fakepass", "WiFi password")

#undef PARAM
#undef ARRAY