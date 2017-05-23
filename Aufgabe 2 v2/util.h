#ifndef UTIL_H_
#define UTIL_H_
#include <sys/time.h>	// struct timeval, struct timespec
#include <stdlib.h>		// NULL
#include "errors.h"		// errno_abort
/* Hilfsfunktion, um gettimeofday für die Elementinitialisierung verwenden zu können */
/* function mygettimeofday
 * @brief	Erzeuge eine Kopie der aktuellen Zeit 
 * @return	struct timeval
 * 
 * Wrapper, um gettimeofday in einer Elementinitialisierungsliste aufrufen zu können
 */
inline struct timeval mygettimeofday() {
	struct timeval x;
	gettimeofday(&x, NULL);
	return x;
}

/* function guarded_nanosleep
 * @brief	Warte eine definierte Zeit
 * @param	struct timespec	wait (call by value!)
 * @return	void
 * 
 * Diese Funktion wartet mit nanosleep. Unterbrechungen durch
 * Signale werden abgefangen
 */
inline void guarded_nanosleep(struct timespec wait) {
	struct timespec rem;
	while (nanosleep(&wait, &rem) == -1) {
		if (errno != EINTR) {
			errno_abort("nanosleep");
		}
		wait = rem;
	}
}
#endif /*UTIL_H_*/
