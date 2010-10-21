/* These checks will cause exceptions if certain parameters do not
 * make sense.
 */
#define Memory_BoundaryChecks 1
#define Memory_PointerChecks  1

/* Show an error in case you forgot to initialize a module's
 * exception manager.
 */
#define Exception_Safety 1

/* Save the file name and line number from which an exception was
 * raised.
 */
#define Exception_SaveOrigin 1

/* Include a trace with 20 elements at most. */
#define Exception_SaveTrace  1
#define Exception_TraceSize 20

/* Support formatting call traces. */
#define Backtrace_HasBFD

/* Log everything by default. */
#define Logger_DisabledLevels 0

#import "Manifest.h"
