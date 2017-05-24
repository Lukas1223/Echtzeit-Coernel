/* module diagram
 * @brief module for drawing a user scaled diagram on a SDL-screen
 * 
 * @author	Jens Ziegler
 
 * @date	20.08.2007
 * @version	V0.5a
 */
 
#ifndef DIAGRAM_H_
#define DIAGRAM_H_

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#define WIDTH	960
#define HEIGHT	720

typedef struct {
	Uint8 red;
	Uint8 green;
	Uint8 blue;
} color;
extern const color red;  
extern const color blue; 
extern const color cyan; 
extern const color green; 
extern const color yellow; 
extern const color purple;


/* Initialize a screen for drawing */
int InitScreen(SDL_Surface** screen, const char* const title);
/* Clear entire screen */
void ClearScreen(SDL_Surface** screen);
/* Draw a coordniate system and prepare viewport for line drawing */
int DrawDiagram(SDL_Surface **screen, 
		const char* const xtitle, const char* const ytitle, 
		const int xmin, const int xmax, const int xscale, 
		const int ymin, const int ymax, const int yscale);
/* draw a line from previous point to (x,y) */
void DrawLine ( SDL_Surface** screen, const int lineNr, const double x, const double y, const Uint8 rr, const Uint8 gg, const Uint8 bb);
/* draw a line from previous point to (x,y) */
void DrawLine ( SDL_Surface** screen, const int lineNr, const double x, const double y, const color& c);


#endif /*DIAGRAM_H_*/
