/* program Beispiel
 * @brief Simulation eines Fertigungsprozesses, 
 * 			1 Erzeuger mit Puffer, 1 Verbraucher
 * 			Synchronisation durch Semaphore
 * 
 * @author	Leon Urbas
 * @date	09.05.2008
 * @version	V0.1
 * 
 * benötigt libpthread, librt
 */

#include <stdio.h>          // printf
#include <pthread.h>		// pthread_...
#include <semaphore.h>		// sem_...
#include <time.h>			// struct timespec
#include "errors.h"        	// errno_abort

const struct timespec tvVerbraucher = { 1, 500000000 };
const struct timespec tvErzeuger    = { 1, 0 };
const int bufferSize = 4;

pthread_t hTaskA;	// Erzeuger
pthread_t hTaskB; 	// Verbraucher
sem_t semaphore;	// Synchronisation über Puffer
s

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

/* function cleanup
 * @brief Threads und Synchronisationsmittel freigeben
 * @param	void
 * @return	void
 * 
 * Diese Funktion gibt alle Strukturen, Synchronisationsmittel und erzeugten
 * Tasks frei
 */
void cleanup() {
	if (&semaphore != NULL)
		sem_destroy(&semaphore);
	if (&hTaskA != NULL)
		pthread_cancel(hTaskA);
	if (&hTaskB != NULL)
		pthread_cancel(hTaskB);
}

int pCounter=0;	// Zähler für erzeugte Produkte 
/* function Erzeuger
 * @brief 	Funktion für Erzeugertask
 * @param	void*	Zeit in Sekunden
 * @return	void*	NULL
 * 
 * Diese Funktion wird im Erzeugertask abgearbeitet
 */
void * Erzeuger(void *arg) {
	struct timespec delay = *(struct timespec *)arg;

	for (;;) {
		//Produktionsdauer abwarten
		printf("Produziere Stück %d ...\n", ++pCounter);
		guarded_nanosleep( delay );
		printf("Sende Stück %d ...\n", pCounter);
		//Sende an Puffer wenn Platz frei, sonst warte
		sem_wait(&semaphore);
		printf("Stück %d gesendet...\n", pCounter);
	}
	return NULL;
}

int vCounter = 0; // Zähler für verpackte Produkte
/* function Verbraucher
 * @brief Task fuer Verbraucher
 * @param	void*	Zeit in Sekunden
 * @return	void*	NULL
 * 
 * Diese Funktion wird in der Verbrauchertask ausgeführt
 */
void * Verbraucher(void *arg) {
	struct timespec delay = *(struct timespec *)arg;

	for (;;) {
		int sval;
		// Teste, ob es etwas zum Verpacken gibt 
		sem_getvalue(&semaphore, &sval);
		if (sval != bufferSize) {	// Alle Plätze sind frei
			// Hole das Teil aus dem Puffer
			sem_post(&semaphore);
			// und verpacke
			printf("Verpacke Stück %d ...\n", ++vCounter);
			guarded_nanosleep( delay );
			printf("Stück %d fertig\n", vCounter);
		} 
	}
	return NULL;
}

/* function main
 * @brief	Hauptprogramm
 * @param	void
 * @return	int		Rückkehr-Identifizierer für BS
 */
int main() {
	// Initialisiere Semaphoren fuer den Erzeugerpuffer
	sem_init( &semaphore, 0, bufferSize);
	
	// Anwendertasks anlegen und starten
	pthread_create( &hTaskA, NULL, Verbraucher, (void *) &tvVerbraucher);
	pthread_create( &hTaskB, NULL, Erzeuger,    (void *) &tvErzeuger);

	// Warte auf Tastendruck
	printf("Press Enter to finish\n");
	char c = getchar();
	c = c;
	// Aufräumen
	cleanup(); 
	return 0;
}
