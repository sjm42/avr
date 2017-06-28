// global.h

#ifndef GLOBAL_H
#define GLOBAL_H

#include <inttypes.h>
#include <stdint.h>

// global AVRLIB defines
#include "avrlibdefs.h"
// global AVRLIB types definitions
#include "avrlibtypes.h"

// project/system dependent defines

#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	// cpu cycles per microsecond

#endif
// EOF
