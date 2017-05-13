/*--------------------------------------------------------------------------*/
/*  Grafiktest                                                              */
/*--------------------------------------------------------------------------*/

/*
 Das Testprogramm zeigt die Darstellung verschiedener Linien
 als Zeitdiagramm.

 */

#include "diagram.h"
#include <pthread.h>				//Thread

SDL_Surface *screen; //Window-Handler

int main() {
	int i; // Laufvariable
	float t; // laufende Zeit
	float y, y1; // y-Werte

	//--- Grafik initialisieren ----------------------------------------------

	InitScreen(&screen, "Grafiktest");
	DrawDiagram(&screen, "Zeit/sec", "y-Werte", 
			-1, 20, 1,  
			-1, 20, 1);

	//--- Startlinien zeichnen -----------------------------------------------

	DrawLine(&screen, 1, 0, 0, red);
	DrawLine(&screen, 2, 0, 0, blue);

	//--- Treppenfunktion und Linie zeichnen ---------------------------------

	y1=0;
	for (i=1; i<=10; i++) {
		t=i;
		y=i;
		DrawLine(&screen, 1, t, y1, red); // Treppe mit 2 Linien zeichnen
		DrawLine(&screen, 1, t, y, red);
		DrawLine(&screen, 2, t, y, blue); // Punkte durch Gerade verbinden
		y1=y;
	}
	//Bildschirm aktualisieren
	SDL_UpdateRect(screen, 0, 0, 0, 0);

	//Event-Handler fuer den Abbruch
	SDL_Event event;

	//Struktur für die Message-Queue Attribute
	struct timespec delay;
	delay.tv_sec = 0;
	delay.tv_nsec = 500000;

	int done = 0;

	printf("Beliebige Taste zum Beenden drücken\n");

	//zyklische Abbruchabfrage
	while (!done) {

		//Abfrageverzögerung abwarten
		nanosleep( &delay, NULL);

		//Events verarbeiten
		while (SDL_PollEvent(&event) ) {
			switch (event.type) {
			case SDL_MOUSEBUTTONDOWN: //Mausdruck
			case SDL_KEYDOWN: //Tastendruck
			case SDL_QUIT: //Quit-Event
				printf("Verlasse Programm\n");
				done = 1; //Beenden
				break;
			default:
				break;
			}
		}
	}
	return 0;
} // main
