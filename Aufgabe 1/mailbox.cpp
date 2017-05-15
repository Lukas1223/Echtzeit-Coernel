/* program Beispiel
 * @brief Simulation eines Fertigungsprozesses, 
 * 		1 Erzeuger mit Puffer, 1 Verbraucher
 * 		Synchronisation durch Semaphore
 * 		1 Zeichenthread
 * 		Synchronisation durch mailbox
 * @author	Leon Urbas, Lukas Bachmann, Hannes Kath, Lukas Ludwig, Gregor Just
 * @date	15.05.2017
 * @version	V0.1
 * 
 * benötigt Verweise auf Projekt diagramm
 * benötigt libpthread, librt, libSDL und libSDL_gfx
 */
#include <iostream>
#include <stdio.h>          // printf
#include <pthread.h>		// pthread_...
#include <semaphore.h>		// sem_...
#include <sys/time.h>		// struct timespec
#include <mqueue.h>			//Message Queue
#include "diagram.h"		// SDL, Draw...
#include "errors.h"        	// errno_abort
#include "util.h"			// Hilfsfunktionen
/* Simulationsparameter */
const int erzeugerA_dauer_sec = 2; // Dauer Teileerzeugung in sec
const int erzeugerA_dauer_nsec = 0; // + Dauer Teileerzeugung in nsec
const int erzeugerB_dauer_sec = 5; // Dauer Teileerzeugung in sec
const int erzeugerB_dauer_nsec = 0; // + Dauer Teileerzeugung in nsec
const int erzeugerC_dauer_sec = 7; // Dauer Teileerzeugung in sec
const int erzeugerC_dauer_nsec = 0; // + Dauer Teileerzeugung in nsec
const int erzeugerA_puffer = 1; // kein Puffer
const int erzeugerB_puffer = 1; // kein Puffer
const int erzeugerC_puffer = 1; // kein Puffer
const int verbraucher_dauer_sec = 3; // Dauer Verpackung in sec
const int verbraucher_dauer_nsec = 0; // + Dauer Verpackung in nsec

/* Task Variablen */
pthread_t erzeugerTask; // Erzeugertask
sem_t erzeugerPufferSemaphore; // Produktzwischenpuffer als erzeugerPufferSemaphore
pthread_t verbraucherTask; // Verbrauchertask

pthread_t grafikTask; // Zeichenthread
mqd_t grafikMailbox; // Mailbox für Zeichenkommandos
char* gMbName = "/mboxG"; // Name der Mailbox für Zeichenkommandos

/* Parametrierung der Oberfläche */
SDL_Surface *screen; //Window-Handler
#define XMIN 	-10
#define XMAX    200
#define XSCALE 	 20
#define YMIN  	 -5
#define YMAX 	 20
#define YSCALE 	  5

/* Eine Klasse für zeitbasierte Ereignisse */
enum eventType {
	Produziert_A,
	Produziert_B,
	Produziert_C,
	Verpackt,
	Undefiniert
};

class Event {
	static timeval base; // Startzeit

	const struct timeval timestamp; // Ereigniszeit
	const eventType type; // Ereignistyp
public:
	// Konstruktor für ein undefiniertes Ereignis
	Event() :
			timestamp(mygettimeofday()), type(Undefiniert) {
	}
	// Konstruktor für ein typisiertes Ereignis
	Event(eventType eArg) :
			timestamp(mygettimeofday()), type(eArg) {
	}
	// lese relative Zeit seit Start in msec */
	int getRTmsec() const {
		return (timestamp.tv_sec - base.tv_sec) * 1000
				+ (timestamp.tv_usec - base.tv_usec + 500) / 1000;
	}
	// lese Ereignistyp aus
	eventType getType() const {
		return type;
	}
};

/* Gib relative Zeit und Ereignistyp des Events aus */
std::ostream& operator<<(std::ostream& os, const Event& evt) {
	os << evt.getRTmsec() << ":" << evt.getType();
	return os;
}

// Initialisiere die Klassenvariable Startzeit 
struct timeval Event::base = mygettimeofday();

/* function cleanup
 * @brief Threads und Synchronisationsmittel freigeben
 * @param	void
 * @return	void
 * 
 * Diese Funktion gibt alle Strukturen, Synchronisationsmittel und erzeugten
 * Tasks frei
 */
void cleanup() {
	if (&erzeugerPufferSemaphore != NULL)
		sem_destroy(&erzeugerPufferSemaphore);
	if (&erzeugerTask != NULL)
		pthread_cancel(erzeugerTask);
	if (&verbraucherTask != NULL)
		pthread_cancel(verbraucherTask);
	if (&grafikMailbox != NULL) {
		mq_close(grafikMailbox);
		mq_unlink(gMbName);
	}
}

/* function Erzeuger
 * @brief 	Funktion für Erzeugertask
 * @param	void*	Zeit in Sekunden
 * @return	void*	NULL
 * 
 * Diese Funktion wird im Erzeugertask abgearbeitet
 */
/*void * Erzeuger(void *argument) {
	struct argA = *(struct argument_A *) argument;
	struct timespec delay = argA->tsErzeugerA;
	mqd_t mbG = mq_open(gMbName, O_RDWR);

	for (;;) {
		//Produktionsdauer abwarten
		guarded_nanosleep(delay);
		// std::cout << "part produced..." << std::endl << std::flush;
		// Erzeugungsmeldung an Grafik senden
		switch (type)
		case Produziert_A:
		mq_send(mbG, (char *) new Event(Produziert_A), sizeof(Event), 1);
		break;
		case Produziert_B:
		mq_send(mbG, (char *) new Event(Produziert_B), sizeof(Event), 1);
		break;
		case Produziert_C:
		mq_send(mbG, (char *) new Event(Produziert_C), sizeof(Event), 1);
		break;

		// Produkt in Puffer schieben wenn Platz frei, sonst warten
		sem_wait(&erzeugerPufferSemaphore);
	}
	return NULL;
}
*/
void * ErzeugerA(void *arg) {
	struct timespec delay = *(struct timespec *)arg;
	mqd_t mbG_A = mq_open(gMbName, O_RDWR);

	for (;;) {
		//Produktionsdauer abwarten
		guarded_nanosleep(delay);
		// std::cout << "part produced..." << std::endl << std::flush;
		// Erzeugungsmeldung an Grafik senden
		mq_send(mbG_A, (char *) new Event(Produziert_A), sizeof(Event), 1);
		// Produkt in Puffer schieben wenn Platz frei, sonst warten
		sem_wait(&erzeugerPufferSemaphore);
	}
	return NULL;
}
void * ErzeugerB(void *arg) {
	struct timespec delay = *(struct timespec *)arg;
	mqd_t mbG_B = mq_open(gMbName, O_RDWR);

	for (;;) {
		guarded_nanosleep(delay);
		// std::cout << "part produced..." << std::endl << std::flush;
		mq_send(mbG_B, (char *) new Event(Produziert_B), sizeof(Event), 1);
		sem_wait(&erzeugerPufferSemaphore);
	}
	return NULL;
}
void * ErzeugerC(void *arg) {
	struct timespec delay = *(struct timespec *)arg;
	mqd_t mbG_C = mq_open(gMbName, O_RDWR);

	for (;;) {
		guarded_nanosleep(delay);
		// std::cout << "part produced..." << std::endl << std::flush;
		// Erzeugungsmeldung an Grafik senden
		mq_send(mbG_C, (char *) new Event(Produziert_C), sizeof(Event), 1);
		sem_wait(&erzeugerPufferSemaphore);
	}
	return NULL;
}
/* function Verbraucher
 * @brief Task fuer Verbraucher
 * @param	void*	Zeit in Sekunden
 * @return	void*	NULL
 * 
 * Diese Funktion wird in der Verbrauchertask ausgeführt
 */
void * Verbraucher(void *arg) {
	struct timespec delay = *(struct timespec *) arg;
	mqd_t mbG = mq_open(gMbName, O_RDWR);

	while (1) {
		int sval;
		// Teste, ob es etwas zum Verpacken gibt 
		sem_getvalue(&erzeugerPufferSemaphore, &sval);
		if (sval != erzeugerA_puffer) { // Alle Plätze sind frei
			// Hole das Teil aus dem Puffer
			sem_post(&erzeugerPufferSemaphore);
			// und verpacke
			guarded_nanosleep(delay);
			// std::cout << "part wrapped..." << std::endl << std::flush;
			// Verpackungsmeldung an Grafik senden 
			mq_send(mbG, (const char*) new Event(Verpackt), sizeof(Event), 1);
		}
	}
	return NULL;
}

/* function Grafik
 * @brief Task fuer das Zeichnen
 * @param	void*	NULL
 * @return	void*	NULL
 * 
 * Diese Funktion wird in der Grafiktask ausgeführt
 */
void * Grafik(void *arg) {
	int pCounterA = 0, vCounter = 0; // Zähler
	int pCounterB = 0;
	int pCounterC = 0;
	Event evt; // Speicher für Ereignis
	char text[256];
	mqd_t mbG = mq_open(gMbName, O_RDWR); // Queue

	DrawDiagram(&screen, "Zeit [1/10 sec]", "Anzahl der Produkte",
	XMIN, XMAX, XSCALE,
	YMIN, YMAX, YSCALE);

	while (1) {
		if (mq_receive(mbG, (char *) &evt, sizeof(Event), NULL) != -1) {
			// std::cout << "Received: " << evt << std::endl << std::flush;
			switch (evt.getType()) {
			case Produziert_A:
				// Zeichne Stufe für die produzierten Stücke von A
				DrawLine(&screen, 0, (double) evt.getRTmsec() / 100, pCounterA,
						cyan);
				DrawLine(&screen, 0, (double) evt.getRTmsec() / 100, ++pCounterA,
						cyan);
				// Anzahl der Produkte
				sprintf(text, "Anzahl der Produkte von A: %d", pCounterA);
				// vorher den Bereich löschen
				boxRGBA(screen, 100, 40, 400, 49, 0, 0, 0, 255);
				// Textausgabe
				stringRGBA(screen, 100, 40, text, 0, 255, 255, 255);
				break;
			case Produziert_B:
				// Zeichne Stufe für die produzierten Stücke von B
				DrawLine(&screen, 2, (double) evt.getRTmsec() / 100, pCounterB,
						yellow);
				DrawLine(&screen, 2, (double) evt.getRTmsec() / 100, ++pCounterB,
						yellow);
				sprintf(text,  "Anzahl der Produkte von B: %d", pCounterB);
				boxRGBA(screen, 100, 50, 400, 59, 0, 0, 0, 255);
				stringRGBA(screen, 100, 50, text, 0, 255, 255, 255);
				break;
			case Produziert_C:
				// Zeichne Stufe für die produzierten Stücke von C
				DrawLine(&screen, 3, (double) evt.getRTmsec() / 100, pCounterC,
						green);
				DrawLine(&screen, 3, (double) evt.getRTmsec() / 100, ++pCounterC,
						green);
				sprintf(text, "Anzahl der Produkte von C: %d", pCounterC);
				boxRGBA(screen, 100, 60, 400, 59, 0, 0, 0, 255);
				stringRGBA(screen, 100, 60, text, 0, 255, 255, 255);
				break;
			case Verpackt:
				// Zeichne Stufe für die verpackten Stücke
				DrawLine(&screen, 1, (double) evt.getRTmsec() / 100, vCounter,
						red);
				DrawLine(&screen, 1, (double) evt.getRTmsec() / 100, ++vCounter,
						red);
				sprintf(text, "Anzahl der Verpackungen: %d", vCounter);
				boxRGBA(screen, 100, 70, 500, 79, 0, 0, 0, 255);
				stringRGBA(screen, 100, 70, text, 255, 0, 0, 255);
				break;
			case Undefiniert:
				// Zeichne Linie weiter
				DrawLine(&screen, 0, (double) evt.getRTmsec() / 100, pCounterA,
						cyan);
				DrawLine(&screen, 2, (double) evt.getRTmsec() / 100, pCounterB,
						yellow);
				DrawLine(&screen, 3, (double) evt.getRTmsec() / 100, pCounterC,
						green);
				DrawLine(&screen, 1, (double) evt.getRTmsec() / 100, vCounter,
						red);
				break;
			default:
				break;
			}
			SDL_UpdateRect(screen, 0, 0, 0, 0);
		} else {
			std::cout << "Received: " << strerror(errno) << std::endl
					<< std::flush;
		}
	}
}

/* function main
 * @brief	Hauptprogramm
 * @param	void
 * @return	int		Rückkehr-Identifizierer für BS
 */
int main() {
	// Öffne Grafikbildschirm
	InitScreen(&screen, "Erzeuger - Verbraucher");

	// Initialisiere Zeiten der Tasks

	struct timespec tsErzeugerA;
	tsErzeugerA.tv_sec = erzeugerA_dauer_sec;
	tsErzeugerA.tv_nsec = erzeugerA_dauer_nsec;
	struct timespec tsErzeugerB;
	tsErzeugerB.tv_sec = erzeugerB_dauer_sec;
	tsErzeugerB.tv_nsec = erzeugerB_dauer_nsec;
	struct timespec tsErzeugerC;
	tsErzeugerC.tv_sec = erzeugerC_dauer_sec;
	tsErzeugerC.tv_nsec = erzeugerC_dauer_nsec;
	struct timespec tsVerbraucher;
	tsVerbraucher.tv_sec = verbraucher_dauer_sec;
	tsVerbraucher.tv_nsec = verbraucher_dauer_nsec;

	typedef struct {
		timespec tsErzeugerA;
		eventType Produziert_A;
	} argument_A;
	typedef struct {
		timespec tsErzeugerB;
		eventType Produziert_B;
		} argument_B;

	typedef struct {
		timespec tsErzeugerC;
		eventType Produziert_C;
		} argument_C;



	// Initialisiere erzeugerPufferSemaphoren fuer den Erzeugerpuffer
	sem_init(&erzeugerPufferSemaphore, 0, erzeugerA_puffer);
	sem_init(&erzeugerPufferSemaphore, 0, erzeugerB_puffer);
	sem_init(&erzeugerPufferSemaphore, 0, erzeugerC_puffer);

	// Die mailbox soll bis zu 10 Elemente vom Typ Event aufnehmen können 
	struct mq_attr attr;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(Event);
	attr.mq_flags = 0;

	// Mailbox anlegen
	grafikMailbox = mq_open(gMbName, O_RDWR | O_CREAT, 0600, &attr);

	// Tasks anlegen und starten
	pthread_create(&erzeugerTask, NULL, ErzeugerA, (void *) &tsErzeugerA);
	pthread_create(&erzeugerTask, NULL, ErzeugerB, (void *) &tsErzeugerB);
	pthread_create(&erzeugerTask, NULL, ErzeugerC, (void *) &tsErzeugerC);
	pthread_create(&verbraucherTask, NULL, Verbraucher,
			(void *) &tsVerbraucher);
	pthread_create(&grafikTask, NULL, Grafik, NULL);

	// Warte auf Ereignis
	SDL_Event event;
	struct timespec tvPollEvent;
	tvPollEvent.tv_sec = 0;
	tvPollEvent.tv_nsec = 100000000;
	while (1) {
		// anliegende SDL-Ereignisse verarbeiten
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN: //Mausdruck
			case SDL_KEYDOWN: //Tastendruck
			case SDL_QUIT: //Quit-Event
				cleanup(); //Aufräumen
				exit(0);
				break;
			default:
				break;
			}
		}
		// Schicke für einen glatten Eindruck ein "Undefiniert" Ereignis an die Grafik
		mq_send(grafikMailbox, (char *) new Event(Undefiniert), sizeof(Event),
				1);
		nanosleep(&tvPollEvent, NULL);
	}
	// Aufräumen
	cleanup();
	return 0;
}