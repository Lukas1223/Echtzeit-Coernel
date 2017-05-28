/* Aufgabe 1
 * Simulation eines Fertigungsprozesses mit drei Erzeugern und einem Verpacker
 * keine Erzeugerpuffer
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
const int gErzeugerA_Dauer_sec = 2; // Dauer Teileerzeugung in sec
const int gErzeugerA_Dauer_nsec = 0; // + Dauer Teileerzeugung in nsec

const int gErzeugerB_Dauer_sec = 5;
const int gErzeugerB_Dauer_nsec = 0;

const int gErzeugerC_Dauer_sec = 7;
const int gErzeugerC_Dauer_nsec = 0;

const int gSemPuffer = 1; // Zwischenpuffer des Verbrauchers
const int gSemInitWert = 1;	//Wert, mit dem wir die Semaphore initialisieren
const int gMaxPufferA = 0; //Produktpuffer der Erzeuger
const int gMaxPufferB = 0;
const int gMaxPufferC = 0;

const int verbraucher_dauer_sec = 3; // Dauer Verpackung in sec
const int verbraucher_dauer_nsec = 0; // + Dauer Verpackung in nsec

/* Task Variablen */
pthread_t erzeugerTaskA; // Erzeugertask
pthread_t erzeugerTaskB;
pthread_t erzeugerTaskC;

sem_t gVerbraucherSemaphore; // Produktzwischenpuffer als gVerbraucherSemaphore
pthread_t verbraucherTask; // Verbrauchertask

pthread_t grafikTask; // Zeichenthread
mqd_t grafikMailbox; // Mailbox für Zeichenkommandos
char* gMbName = "/mboxG"; // Name der Mailbox für Zeichenkommandos

/* Parametrierung der Oberfläche */
SDL_Surface *screen; //Window-Handler
#define XMIN 	-10
#define XMAX    300
#define XSCALE 	 20
#define YMIN  	 -5
#define YMAX 	 50
#define YSCALE 	  5

/* Eine Klasse für zeitbasierte Ereignisse */
enum eventType {
	ProduziertA, ProduziertB, ProduziertC, Verpackt, Undefiniert
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
	if (&gVerbraucherSemaphore != NULL)
		sem_destroy(&gVerbraucherSemaphore);
	if (&erzeugerTaskA != NULL)
		pthread_cancel(erzeugerTaskA);
	if (&erzeugerTaskB != NULL)
		pthread_cancel(erzeugerTaskB);
	if (&erzeugerTaskC != NULL)
		pthread_cancel(erzeugerTaskC);
	if (&verbraucherTask != NULL)
		pthread_cancel(verbraucherTask);
	if (&grafikMailbox != NULL) {
		mq_close(grafikMailbox);
		mq_unlink(gMbName);
	}
}

void* WartenImErzeuger(void* arg) {					//Funkzion durch Thread im ErzeugerX aufgerufen
	sem_wait(&gVerbraucherSemaphore);				//
	sem_post((sem_t*) arg);
	return NULL;
}

/* function Erzeuger
 * @brief 	Funktion für Erzeugertask
 * @param	void*	Zeit in Sekunden
 * @return	void*	NULL
 * 
 * Diese Funktion wird im Erzeugertask abgearbeitet
 */
void * ErzeugerA(void *arg) {						  //mit void Pointer kann Typsicherheit von C++ umgangen werden
	struct timespec delay = *(struct timespec *) arg; //Übergabeargument war ursprünglich vom Typ timespec, wird jetzt wieder zurück konvertiert
	pthread_t warteTaskA; //wenn ein Produkt fertig produziert wurde, wird einer dieser Threads erstellt, der dann darauf wartet, dass das Produkt übergeben werden kann
	mqd_t mbG = mq_open(gMbName, O_RDWR); //diese m_queue ist für die Grafik, daran haben wir nichts geändert

	sem_t erzeugerPufferSem;
	sem_init(&erzeugerPufferSem, 0, gMaxPufferA + 1);

	for (;;) { //diese Schleife läuft unendlich lange, bzw bis der Erzeugerthread beendet wird (wenn wir das Programm schließen)
			   //die ErzeugerPufferSemaphore ist quasi der Puffer eines einzelnen Erzeugers, und man darf nur produzieren, wenn noch Platz in der Semaphore/im Puffer ist
		sem_wait(&erzeugerPufferSem);
		guarded_nanosleep(delay);	//hier vergeht die Produktionszeit
		std::cout << "part produced A..." << std::endl << std::flush;//Ausgabe an die Konsole, dass ein Teil von A gebaut wurde
		mq_send(mbG, (char *) new Event(ProduziertA), sizeof(Event), 1);//Meldung an die Grafik, dass ein Teil von A gebaut wurde
		pthread_create(&warteTaskA, NULL, WartenImErzeuger,
				(void *) &erzeugerPufferSem); //dieser Thread wartet darauf, das Produkt an den Verbraucher weiter zu geben. Er bekommt als Übergabe eine Referenz an den momentanen
		// Puffer/die Semaphore, damit er ihn verringern kann, sodass neue Teile produziert werden können
	}
	return NULL;
}

void * ErzeugerB(void *arg) {	//Kommentare siehe A
	struct timespec delay = *(struct timespec *) arg;
	pthread_t warteTaskB;
	mqd_t mbG = mq_open(gMbName, O_RDWR);

	sem_t erzeugerPufferSem;
	sem_init(&erzeugerPufferSem, 0, gMaxPufferB + 1);

	for (;;) {
		sem_wait(&erzeugerPufferSem);
		guarded_nanosleep(delay);
		std::cout << "part produced B..." << std::endl << std::flush;
		mq_send(mbG, (char *) new Event(ProduziertB), sizeof(Event), 1);
		pthread_create(&warteTaskB, NULL, WartenImErzeuger,
				(void *) &erzeugerPufferSem);
	}
	return NULL;
}

void * ErzeugerC(void *arg) {	//Kommentare siehe A
	struct timespec delay = *(struct timespec *) arg;
	pthread_t warteTaskC;
	mqd_t mbG = mq_open(gMbName, O_RDWR);

	sem_t erzeugerPufferSem;
	sem_init(&erzeugerPufferSem, 0, gMaxPufferC + 1);

	for (;;) {
		sem_wait(&erzeugerPufferSem);
		guarded_nanosleep(delay);
		std::cout << "part produced C..." << std::endl << std::flush;
		mq_send(mbG, (char *) new Event(ProduziertC), sizeof(Event), 1);
		pthread_create(&warteTaskC, NULL, WartenImErzeuger,
				(void *) &erzeugerPufferSem);
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
void * Verbraucher(void *arg) {	//hieran haben wir kaum etwas geändert, außer den guarded_nanosleep vor das sem_post zu schieben
	struct timespec delay = *(struct timespec *) arg;
	mqd_t mbG = mq_open(gMbName, O_RDWR);

	while (1) {
		int sval;
		// Teste, ob es etwas zum Verpacken gibt 
		sem_getvalue(&gVerbraucherSemaphore, &sval);
		if (sval != gSemPuffer) { // Alle Plätze sind frei
			// und verpacke
			guarded_nanosleep(delay);
			// Hole das Teil aus dem Puffer
			sem_post(&gVerbraucherSemaphore);
			std::cout << "part wrapped..." << std::endl << std::flush;
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
void * Grafik(void *arg) {//hier haben wir die Fälle B und C hinzugefügt, damit sie ebenfalls gezeichnet werden
	int pCounterA = 0;
	int pCounterB = 0;
	int pCounterC = 0;
	int vCounter = 0; // Zähler
	Event evt; // Speicher für Ereignis
	char text[256];
	mqd_t mbG = mq_open(gMbName, O_RDWR); // Queue	

	DrawDiagram(&screen, "Zeit [1/10 sec]", "Anzahl der Produkte",
	XMIN, XMAX, XSCALE,
	YMIN, YMAX, YSCALE);

	while (1) {
		if (mq_receive(mbG, (char *) &evt, sizeof(Event), NULL) != -1) {
			//std::cout << "Received: " << evt << std::endl << std::flush;
			switch (evt.getType()) {
			case ProduziertA:
				// Zeichne Stufe für die produzierten Stücke
				DrawLine(&screen, 0, (double) evt.getRTmsec() / 100, pCounterA,
						cyan);
				DrawLine(&screen, 0, (double) evt.getRTmsec() / 100,
						++pCounterA, cyan);
				// Anzahl der Produkte
				sprintf(text, "Anzahl der Produkte: %d", pCounterA);
				// vorher den Bereich löschen 
				boxRGBA(screen, 100, 40, 300, 49, 0, 0, 0, 255);
				// Textausgabe
				stringRGBA(screen, 100, 40, text, 0, 255, 255, 255);
				break;

			case ProduziertB:
				// Zeichne Stufe für die produzierten Stücke
				DrawLine(&screen, 2, (double) evt.getRTmsec() / 100, pCounterB,
						green);
				DrawLine(&screen, 2, (double) evt.getRTmsec() / 100,
						++pCounterB, green);
				// Anzahl der Produkte
				sprintf(text, "Anzahl der Produkte: %d", pCounterB);
				// vorher den Bereich löschen
				boxRGBA(screen, 100, 50, 300, 59, 0, 0, 0, 255);
				// Textausgabe
				stringRGBA(screen, 100, 50, text, 0, 255, 0, 255);
				break;

			case ProduziertC:
				// Zeichne Stufe für die produzierten Stücke
				DrawLine(&screen, 3, (double) evt.getRTmsec() / 100, pCounterC,
						blue);
				DrawLine(&screen, 3, (double) evt.getRTmsec() / 100,
						++pCounterC, blue);
				// Anzahl der Produkte
				sprintf(text, "Anzahl der Produkte: %d", pCounterC);
				// vorher den Bereich löschen
				boxRGBA(screen, 100, 60, 300, 69, 0, 0, 0, 255);
				// Textausgabe
				stringRGBA(screen, 100, 60, text, 0, 0, 255, 255);
				break;

			case Verpackt:
				// Zeichne Stufe für die verpackten Stücke
				DrawLine(&screen, 1, (double) evt.getRTmsec() / 100, vCounter,
						red);
				DrawLine(&screen, 1, (double) evt.getRTmsec() / 100, ++vCounter,
						red);
				sprintf(text, "Anzahl der Verpackungen: %d", vCounter);
				boxRGBA(screen, 100, 70, 330, 79, 0, 0, 0, 255);
				stringRGBA(screen, 100, 70, text, 255, 0, 0, 255);
				break;
			case Undefiniert:
				// Zeichne Linie weiter
				DrawLine(&screen, 0, (double) evt.getRTmsec() / 100, pCounterA,
						cyan);
				DrawLine(&screen, 2, (double) evt.getRTmsec() / 100, pCounterB,
						green);
				DrawLine(&screen, 3, (double) evt.getRTmsec() / 100, pCounterC,
						blue);
				DrawLine(&screen, 1, (double) evt.getRTmsec() / 100, vCounter,
						red);
				break;
			default:
				break;
			}
			SDL_UpdateRect(screen, 0, 0, 0, 0);
		} else {
			//std::cout << "Received: " << strerror(errno) << std::endl
			//		<< std::flush;
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
// Öffne Grafikbildschirm
	InitScreen(&screen, "Erzeuger - Verbraucher");

// Initialisiere Zeiten der Tasks
	struct timespec tsErzeugerA;
	tsErzeugerA.tv_sec = gErzeugerA_Dauer_sec;
	tsErzeugerA.tv_nsec = gErzeugerA_Dauer_nsec;
	struct timespec tsErzeugerB;
	tsErzeugerB.tv_sec = gErzeugerB_Dauer_sec;
	tsErzeugerB.tv_nsec = gErzeugerB_Dauer_nsec;
	struct timespec tsErzeugerC;
	tsErzeugerC.tv_sec = gErzeugerC_Dauer_sec;
	tsErzeugerC.tv_nsec = gErzeugerC_Dauer_nsec;

	struct timespec tsVerbraucher;
	tsVerbraucher.tv_sec = verbraucher_dauer_sec;
	tsVerbraucher.tv_nsec = verbraucher_dauer_nsec;

// Initialisiere gVerbraucherSemaphoren fuer den Erzeugerpuffer
	sem_init(&gVerbraucherSemaphore, 0, gSemInitWert);

// Die mailbox soll bis zu 10 Elemente vom Typ Event aufnehmen können
	struct mq_attr attr;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(Event);
	attr.mq_flags = 0;

// Mailbox anlegen
	grafikMailbox = mq_open(gMbName, O_RDWR | O_CREAT, 0600, &attr);

// Tasks anlegen und starten
// zuerst die Threads der Erzeuger
	pthread_create(&erzeugerTaskA, NULL, ErzeugerA, (void *) &tsErzeugerA);
	pthread_create(&erzeugerTaskB, NULL, ErzeugerB, (void *) &tsErzeugerB);
	pthread_create(&erzeugerTaskC, NULL, ErzeugerC, (void *) &tsErzeugerC);

// dann den Thread des Verbrauchers und der Grafik
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
