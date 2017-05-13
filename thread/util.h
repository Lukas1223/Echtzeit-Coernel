#ifndef UTIL_H_
#define UTIL_H_

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
	while (nanosleep(&wait,&rem) == -1) {
		if (errno != EINTR) {
			errno_abort("nanosleep");
		}
		wait = rem;
	}
}

#endif /*UTIL_H_*/
