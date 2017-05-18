#include <stdio.h>		// printf 
#include <semaphore.h> 	// sem_...
#include <pthread.h>	// pthread_...
#include <signal.h>		// sigevent, SIGRTMIN, sigset
#include "errors.h"
#include "sema.h"

sem_t semaphore; // Zur Synchronisation von Thread mit signal

/* signal-catcher */
void signal_catcher(int sig) {
	printf("received signal %d\n", sig);
	V(&semaphore);
}

/* thread routine - wartet 5 mal auf frei werden der semaphore */
void* sem_waiter(void* arg) {
	int id = (int)arg;
	for (int counter=1; counter <=5; counter++) {
		P(&semaphore);
		printf("Thread %d waking (%d)...\n", id, counter);
	}
	return NULL;
}

int main() {
	const int noThreads = 5;
	int i, status;
	pthread_t thread[noThreads];

	// initialisiere semaphore
	sem_init(&semaphore, 0, 0);

	// erzeuge threads, die auf die semaphore warten
	for (i=0; i<noThreads; i++) {
		status = pthread_create(&thread[i], NULL, sem_waiter, (void*)i);
		if (status != 0) {
			err_abort(status, "Create thread");
		}
	}
	// erzeuge ein timer-signal, das alle 2 sekunden "bimmelt"
	struct sigevent sig_event;
	struct sigaction sig_action;
	timer_t timer_id;
	sigset_t sig_mask;
	struct itimerspec timer_val;

	// definiere signal das der timer erzeugen soll
	sig_event.sigev_value.sival_int = 0;
	sig_event.sigev_signo = SIGRTMIN;
	sig_event.sigev_notify = SIGEV_SIGNAL;

	// erzeuge timer
	if (timer_create(CLOCK_REALTIME, &sig_event, &timer_id) == -1)
		errno_abort("Create timer");

	// Definiere behandlungsroutine für signal(e)
	sigemptyset(&sig_mask);
	sigaddset(&sig_mask, SIGRTMIN);
	sig_action.sa_handler = signal_catcher;
	sig_action.sa_mask = sig_mask;
	sig_action.sa_flags = 0;
	if (sigaction(SIGRTMIN, &sig_action, NULL) == -1)
		errno_abort("Set signal action");

	// Konfiguiere und starte den Timer
	timer_val.it_interval.tv_sec = 1; // wiederhole alle sekunde
	timer_val.it_interval.tv_nsec = 0;
	timer_val.it_value.tv_sec = 0; // starte nach 500 millisekunde
	timer_val.it_value.tv_nsec = 500000000;
	if (timer_settime(timer_id, 0, &timer_val, NULL) == -1)
		errno_abort("Set timer");

	/* Warte 10 Sekunden */
	struct timespec wait, rem;
	wait.tv_sec =  10;
	wait.tv_nsec = 0;

	printf("Now going to sleep for 10 seconds\n");
	while (nanosleep(&wait,&rem) == -1) { 
		if (errno != EINTR) { 
			errno_abort("nanosleep");
		}
		printf(".. interrupted, sleep again\n");
		wait = rem;
	}
	
	printf("Now wait for threads to finish\n");
	/* Warte bis alle threads fertig sind */
	for (i=0; i<noThreads; i++) {
		status = pthread_join(thread[i], NULL);
		if (status != 0) {
			err_abort(status, "Join thread");
		}
	}
}
