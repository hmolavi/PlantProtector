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
 * Macros:
 * - PARAM(secure_level, type, name, default, description, PascalName)
 *   Example:
 *   PARAM(2, char, exampleChar, 'A', "example char", ExampleChar)
 *   PARAM(2, uint8_t, exampleUint8, 255, "example uint8_t", ExampleUint8)
 *   PARAM(2, uint16_t, exampleUint16, 65535, "example uint16_t", ExampleUint16)
 *   PARAM(2, uint32_t, exampleUint32, 4294967295, "example uint32_t", ExampleUint32)
 *   PARAM(2, int32_t, exampleInt32, -2147483648, "example int32_t", ExampleInt32)
 *   PARAM(2, float, exampleFloat, 3.14, "example float", ExampleFloat)
 * 
 * - ARRAY(secure_level, type, size, name, default, description, PascalName)
 *   Example:
 *   ARRAY(2, char, 32, exampleCharArray, "example char array", "example char array", ExampleCharArray)
 *   ARRAY(2, uint8_t, 5, exampleUint8Array, ARRAY_INIT(1, 2, 3, 4, 5), "example uint8_t array", ExampleUint8Array)
 *   ARRAY(2, uint16_t, 3, exampleUint16Array, ARRAY_INIT(100, 200, 300), "example uint16_t array", ExampleUint16Array)
 *   ARRAY(2, int32_t, 4, exampleUint32Array, ARRAY_INIT(1, 0, 0, 0), "example int array", MyArray)
 *   ARRAY(2, float, 3, exampleFloatArray, ARRAY_INIT(1.1, 2.2, 3.3), "example float array", ExampleFloatArray)
 *   ARRAY(2, bool, 2, exampleBoolArray, ARRAY_INIT(true, false), "example bool array", ExampleBoolArray)
 */

#define ARRAY_INIT(...) {__VA_ARGS__}

#ifndef PARAM
#define PARAM(...)
#endif

#ifndef ARRAY
#define ARRAY(...)
#endif

PARAM(2, int32_t, brightness, 50, "brightness duh", Brightness)
PARAM(2, uint32_t, interval, 1000, "random interval", Internval)
PARAM(2, bool, seriousmode, false, "Determines AIs tone of voice", SeriousMode)
ARRAY(2, char, 32, ssid, "fakessid", "WiFi ssid (name)", Ssid)
ARRAY(2, char, 64, password, "fakepass", "WiFi password", Password)

#undef PARAM
#undef ARRAY