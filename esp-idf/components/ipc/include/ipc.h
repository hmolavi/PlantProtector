/**
 *
 */

#include <stdint.h>
#include <stdlib.h>

/* Master parameter database */
typedef struct Parameters_s {
#define PARAM(sec, w, x, y, z) w x;
#define PARAM_ARRAY(sec, t, s, n, d, h) t n[s];
#include "parameters.inc"
} Parameters_t;

const Parameters_t Params;

/* Global parameter store, fill in defaults */
#define PARAM(sec, w, x, y, z) .x = y,
#define PARAM_ARRAY(sec, t, s, n, d, h) .n = d,
const Parameters_t Params = {
#include "parameters.inc"
};

typedef enum IPCCommands_e {
#define SPICMD(name, code) name,
#include "../ipc_commands.inc"
    MAXCmd
} IPCCommands_t;
#undef SPICMD

void performAction(IPCCommands_t action);

/* Master parameter database */
typedef struct IPCCommands_s {
#define PARAM(sec, w, x, y, z) w x;
#define PARAM_ARRAY(sec, t, s, n, d, h) t n[s];
#include "parameters.inc"
} Parameters_t;

const Parameters_t Params;

// void IPC_read_RTC();

// void IPC_set_RTC()