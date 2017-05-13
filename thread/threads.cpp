/* modul threads
 * @brief 	Nebenläufige Abarbeitung von threads
 * @author	Leon Urbas
 * @date	09.05.2008
 * @version	V0.1
 * 
 * benötigt libpthread, librt
 *  */
#include <stdio.h>          // printf
#include <pthread.h>		// pthread_...
#include <time.h>			// struct timespec
#include "errors.h"        	// errno_abort
#include "util.h"			// guarded_nanosleep

/* struct thread_arg
 * @brief	Struktur zur Übergabe thread-spezifischer Daten
 */
struct thread_arg {
	char *	name;
	struct timespec	ruhezeit;	
};

pthread_t hTaskA;	// Handle für Thread A
pthread_t hTaskB; 	// Handle für Thread B


/* function cleanup
 * @brief Threads und Synchronisationsmittel freigeben
 * @param	void
 * @return	void
 * 
 * Diese Funktion gibt alle Strukturen, Synchronisationsmittel und erzeugten
 * Tasks frei
 */
void cleanup() {
	if (&hTaskA != NULL)
		pthread_cancel(hTaskA);
	if (&hTaskB != NULL)
		pthread_cancel(hTaskB);
}

/* function threadFunction
 * @brief 	Funktion, die in einem Thread ausgeführt wird
 * @param	void*	(struct thread_arg *)-Pointer mit individuellen Parametern 
 * @return	void*	NULL
 * 
 * Diese Funktion wird in einem thread abgearbeitet
 */
void * threadFunction(void *arg) {
	struct thread_arg* t = (struct thread_arg*) arg;
	int counter = 0;

	while (1) {
		printf("Hier %s, counter = %d\n", t->name, ++counter);
		guarded_nanosleep( t->ruhezeit );
	}
	return NULL;
}

/* function main
 * @brief	Hauptprogramm
 * @param	void
 * @return	int		Rückkehr-Identifizierer für BS
 */
int main() {
	struct thread_arg thread_arg_A;
	thread_arg_A.name = "Thread A";
	thread_arg_A.ruhezeit.tv_sec = 1;

	struct thread_arg thread_arg_B;
	thread_arg_B.name = "Thread B";
	thread_arg_B.ruhezeit.tv_sec = 1;
	thread_arg_B.ruhezeit.tv_nsec = 500000000;

	// Anwendertasks anlegen und starten
	pthread_create( &hTaskA, NULL, threadFunction, (void *) &thread_arg_A);
	pthread_create( &hTaskB, NULL, threadFunction, (void *) &thread_arg_B);

	// Warte auf Tastendruck
	printf("Press Enter to finish\n");
	char c = getchar();
	c = c;
	// Aufräumen	
	cleanup(); 
	printf("Finished\n");
	return 0;
}
