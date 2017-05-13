/* module timer
 * @brief	Generieren und behandeln eines Signals
 * 
 * @author	L.Urbas
 * @date	2008
 * @version	0.1
 */
#include <stdio.h>		// printf 
#include <time.h>		// nanosleep
#include <signal.h>		// sigevent, SIGRTMIN, sigset
#include "errors.h"		// errno_abort

/* int counter
 * @brief Anzahl empfangener Signale
 */
int counter = 0;

/* function signal_catcher
 * @brief	Funktion zur Signalbehandlung
 * 
 * @param	int	Signalnummer
 * @return	void 
 */
void signal_catcher(int sig) {
	printf("received signal %d, counter=%d\n", sig, counter++);
}

/* function main
 * @brief Parametriert und startet einen timer
 */
int main() {
	struct sigevent sig_event;	// Struktur für Signaldefinition
	
	struct sigaction sig_action;// Struktur für Signalbehandlung
	sigset_t sig_mask;
	
	timer_t timer_id;			// Handle für Timer
	struct itimerspec timer_val;

	// Definiere das Signal SIGRTMIN (erstes freies Signal für den Benutzer) für den timer
	sig_event.sigev_value.sival_int = 0;
	sig_event.sigev_signo = SIGRTMIN;
	sig_event.sigev_notify = SIGEV_SIGNAL;
	
	// Definiere Behandlungsroutine für signal(e)
	sigemptyset(&sig_mask);			// Erzeuge leere Signalmaske
	sigaddset(&sig_mask, SIGRTMIN);	// Füge SIGRTMIN dazu
	sig_action.sa_mask = sig_mask;	
	sig_action.sa_flags = 0;
	sig_action.sa_handler = signal_catcher;
	
	// Installiere Behandlungsroutine
	if (sigaction(SIGRTMIN, &sig_action, NULL) == -1)
		errno_abort("Set signal action");

	// Erzeuge timer 
	if (timer_create(CLOCK_REALTIME, &sig_event, &timer_id) == -1)
		errno_abort("Create timer");

	// Konfiguriere und starte den Timer
	timer_val.it_interval.tv_sec  = 0; // wiederhole alle 1/2 sec
	timer_val.it_interval.tv_nsec = 500000000;
	timer_val.it_value.tv_sec  = 0;	// starte nach einer nanosekunde
	timer_val.it_value.tv_nsec = 1;
	if (timer_settime(timer_id, 0, &timer_val, NULL) == -1)
		errno_abort("Set timer");

	// Warte 5 Sekunden
	struct timespec wait, rem;
	wait.tv_sec =  5;
	wait.tv_nsec = 0;
	
	printf("Now going to sleep for 5 seconds\n");
	// nanosleep wird ggf. von Signalen unterbrochen. Das wird durch EINTR in
	// errno angezeigt, rem enthält dann den noch zu wartenden Anteil.
	while (nanosleep(&wait,&rem) == -1) { 
		if (errno != EINTR) { 
			errno_abort("nanosleep");
		}
		printf(".. interrupted, sleep again\n");
		wait = rem;
	}
	printf("Hello Again\n");
	
	// Ressourcen werden vom Betriebssystem freigegeben
}
