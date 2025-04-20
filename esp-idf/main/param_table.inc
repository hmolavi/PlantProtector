/**
 * @file param_table.inc
 * @author Hossein Molavi (hmolavi@uwaterloo.ca)
 * 
 * @brief Defined the parameters used in the PlantProtector project.
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
#undef SECURE_LEVEL