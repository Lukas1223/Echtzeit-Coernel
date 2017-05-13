#ifndef SEMA_H_
#define SEMA_H_
#include <semaphore.h>
#include "errors.h"
inline void P(sem_t* s) { 
	while (sem_wait(s) == -1) { 
		if (errno != EINTR) { 
			errno_abort("Wait on semaphore");
		}
	}
};
inline void V(sem_t * s) {
	if (sem_post(s) == -1) {
		errno_abort("Post semaphore");
	}
};


#endif /*SEMA_H_*/
