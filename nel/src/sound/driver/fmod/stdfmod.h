
#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#	pragma include_alias(<fmod.h>, <fmod3\fmod.h>)
#endif

#include "nel/misc/common.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/fast_mem.h"
#include "nel/misc/debug.h"
#include "nel/misc/vector.h"

#include "../sound_driver.h"

/* MERGE: this is the result of merging branch_mtr_nostlport with trunk (NEL-16)
 */
