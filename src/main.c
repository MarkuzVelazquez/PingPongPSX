#include <stdio.h>
#include <stdlib.h>
#include <psx.h>
#include <psxgpu.h>

#define pad1Check(a) padbuf & a
#define pad2Check(a) padbuf2 & a
#define clamp(x, a, b) (x < a) ? a : (x > b) ? b : x
#define abs(a) (a < 0) ? (-(a)) : a
#define irandom(a, b) ((rand()%((distancia(a, 0, b, 0))+1))+a)
#define max(a, b) (a > b) ? a : b
#define min(a, b) (a < b) ? a : b

#define instanceCreate(a, b, c, d) \
	if (d.Ini == NULL) { \
		d.Ini = (struct c *) malloc(sizeof(struct c)); \
		d.Ini->Ant = NULL; \
		d.Fin = d.Ini; \
	} \
	else { \
		d.Fin->Sig = (struct c *) malloc(sizeof(struct c)); \
		d.Fin->Sig->Ant = d.Fin; \
		d.Fin = d.Fin->Sig; \
	} \
	d.Fin->Sig = NULL; \
	d.Fin->x = a; \
	d.Fin->y = b;

#define instanceDestroy(a, b) \
	if (b.Ini->Sig == NULL) { \
		free(b.Ini); \
		b.Ini = NULL; \
		b.Fin = NULL; \
	} \
	else if (a->Sig == b.Fin) { \
		b.Fin->Ant = a->Ant; \
		*a = *b.Fin; \
		free(b.Fin); \
		b.Fin = a; \
	} \
	else if (a->Sig != NULL) { \
		a->Sig->Ant = a->Ant; \
		*a = *a->Sig; \
		free(a->Sig->Ant); \
		a->Sig->Ant = a;\
	} \
	else { \
		b.Fin = b.Fin->Ant; \
		free(b.Fin->Sig); \
		b.Fin->Sig = NULL; \
	}

#define CrearPelota() \
	instanceCreate(irandom(100, 200), irandom(20, 140), oPelota, globalP); \
	globalP.Fin->hspeed = (irandom(0, 1) == 0) ? -Speed : Speed; \
	globalP.Fin->vspeed = (irandom(0, 1) == 0) ? -Speed : Speed; \
	globalP.Fin->rect.x = globalP.Fin->x; \
	globalP.Fin->rect.y = globalP.Fin->y; \
	globalP.Fin->rect.w = 8; \
	globalP.Fin->rect.h = 8; \
	globalP.Fin->rect.r = irandom(0, 255); \
	globalP.Fin->rect.g = irandom(0, 255); \
	globalP.Fin->rect.b = irandom(0, 255); \
	globalP.Fin->rect.attribute = 0;

#define controlesPlayer(a, b, c) \
	/*player[a].rect.y += ((_Bool)(b & PAD_DOWN)-(_Bool)(b & PAD_UP))*c;*/ \
	if (b & PAD_DOWN) { \
		player[a].rect.y = clamp(player[a].rect.y + c, 20, 200); \
	} \
	else if (b & PAD_UP) { \
		player[a].rect.y = clamp(player[a].rect.y - c, 20, 200); \
	} \
	GsSortRectangle(&player[a].rect);



unsigned int primeList[0*4000];
volatile int displayOld = 1;
volatile int timeCounter = 0;
int dbuf = 0;
unsigned short padbuf;
unsigned short padbuf2;

struct oPelota {
	int x;
	int y;
	int hspeed;
	int vspeed;
	GsRectangle rect;

	struct oPelota *Sig;
	struct oPelota *Ant;
};

struct lPelota {
	struct oPelota *Ini;
	struct oPelota *Fin;
} globalP;

struct {
	int y;
	GsRectangle rect;
} player[2];

int seg = 0;
unsigned short int Speed = 1;
unsigned short int room = 0;
int opcion = 0;

_Bool press_left = false;
_Bool press_right = false;

void blankHandler();
void logicaPelota();
unsigned int distancia(int, int, int, int);
_Bool collisionRectangle(struct oPelota *, GsRectangle *);
unsigned short int p1Score = 0;
unsigned short int p2Score = 0;
int checarPosicionP(int, int, int, int, int);

int main() {
	PSX_Init();
	GsInit();
	GsSetList(primeList);
	GsClearMem();
	GsSetVideoMode(320, 240, VMODE_PAL);
	GsLoadFont(768, 0, 768, 256);
	SetVBlankHandler(blankHandler);

	globalP.Ini = NULL;

	player[0].rect.x = 10;
    player[0].rect.y = 100;
    player[0].rect.w = 10;
    player[0].rect.h = 40;
    player[0].rect.r = 255;
    player[0].rect.g = 0;
    player[0].rect.b = 0;
    player[0].rect.attribute = 0;

    player[1].rect.x = 300;
    player[1].rect.y = 100;
    player[1].rect.w = 10;
    player[1].rect.h = 40;
    player[1].rect.r = 0;
    player[1].rect.g = 0;
    player[1].rect.b = 255;
    player[1].rect.attribute = 0;

	while(1) {
		if(displayOld) {
			dbuf = !dbuf;
			GsSetDispEnvSimple(0, dbuf ? 0: 256);
			GsSetDrawEnvSimple(0, dbuf ? 256 : 0, 320, 240);
			GsSortCls(0, 0, 0);
            PSX_ReadPad(&padbuf, &padbuf2);

			switch(room) {
				case 0:
					seg++;
					if (seg < 30) {
						GsPrintFont(100, 150, "Precione START");
					}
					seg = seg % 60;
					if (pad1Check(PAD_START)) {
						seg = -1;
						Speed = 1;
						room = 1;
					}
				break;
				case 1:
					if (seg == -1) {CrearPelota();}
					if (globalP.Ini == NULL) {
						opcion = 0;
						room = 2;
					}
					// Dificultad
					seg++;
					if (seg == 60 * 30) {				
		 				Speed = clamp(Speed++, 1, 9);	
					}
					else if (seg == 60*60) {
						seg = 0;
						CrearPelota();	
			
					}

					// Dibuja y controla al player
					controlesPlayer(0, padbuf, 5);
					controlesPlayer(1, padbuf2, 5);

					// Dibuja y controla las pelotas
					logicaPelota();

					// DIbuja el score
					/*
					GsPrintFont(10, 10, "P1: %d", p1Score);
					GsPrintFont(170, 10, "P2: %d", p2Score);*/
				break;
				case 2:
					GsPrintFont(100, 140, "Continuar??");
					GsPrintFont(120, 150, "SI  NO");
					GsPrintFont(110 + (opcion*30), 150, ">");

					if (pad1Check(PAD_LEFT)) {
						if (!press_left) {
							press_left = true;
							opcion++;
						}
					}
					else {
						_Bool press_left = false;
					}

					if (pad1Check(PAD_RIGHT)) {
						if (!press_right) {
							press_right = true;
							opcion--;
						}
					}
					else {
						_Bool press_right = false;
					}
					opcion = clamp(opcion, 0, 1);
				
					if (pad1Check(PAD_START)) {
						switch(opcion) {
							case 0:
								room = 1;
								seg = -1;
							break;
							case 1:
								room = 0;
							break;
						}
					}
				break;
			}
			GsDrawList();
			while(GsIsDrawing());
			displayOld = 0;
		}
	}
}

void blankHandler() {
	displayOld = 1;
	timeCounter++;
}

unsigned int distancia(int x1, int y1, int x2, int y2) {
	unsigned int DisX, DisY;
	DisX = abs(x1-x2);
	DisY = abs(y1-y2);
	return DisX + DisY;
}

_Bool collisionRectangle(struct oPelota *Temp, GsRectangle *Temp2) {
	return (Temp->rect.x+Temp->hspeed  < Temp2->x+Temp2->w 
	&& Temp->rect.x+Temp->hspeed+Temp->rect.w > Temp2->x 
	&& Temp->rect.y+Temp->vspeed < Temp2->y+Temp2->h 
	&& Temp->rect.y+Temp->hspeed+Temp->rect.h > Temp2->y);
}

void logicaPelota() {
	struct oPelota *Temp = globalP.Ini;
	while(Temp != NULL) {
		// Collision con pelotas
		struct oPelota *Temp2 = globalP.Ini;
		while(Temp2 != NULL) {
			if (Temp != Temp2 && distancia(
							Temp->rect.x+Temp->hspeed, Temp->rect.y+Temp->vspeed, 
												Temp2->rect.x, Temp2->rect.y) <= 8) {
				if (Temp->hspeed == -Temp2->hspeed) {
					Temp->hspeed *= -1;
					Temp2->hspeed *= -1;
				}
				if (Temp->vspeed == -Temp2->vspeed) {
					Temp->vspeed *= -1;
					Temp2->vspeed *= -1;
				}
			}
			Temp2 = Temp2->Sig;
		}
	
		// Collision con player 1
		if (collisionRectangle(Temp, &player[0].rect)) {
			if (Temp->rect.x < player[0].rect.x+player[0].rect.w) {
				instanceDestroy(Temp, globalP);
				p2Score++;
			}
			else {
				Temp->rect.x = player[0].rect.x+player[0].rect.w;
				Temp->hspeed *= -1;
			}
		}
	
		// Collision con player 2
		else if (collisionRectangle(Temp, &player[1].rect)) {
			if (Temp->rect.x+Temp->rect.w > player[1].rect.x) {
				instanceDestroy(Temp, globalP);
				p1Score++;
			}
			else {
				Temp->rect.x = player[1].rect.x-Temp->rect.w;
				Temp->hspeed *= -1;
			}
		}
	
		// Rebote de pantalla
		if (Temp->rect.y+Temp->vspeed < 20 || Temp->rect.y+Temp->rect.h+Temp->vspeed > 240) {
			Temp->vspeed *= -1;
		}

		// Sale de la pantalla
		else if (Temp->rect.x+Temp->hspeed < 0) {
			instanceDestroy(Temp, globalP);
			p2Score++;
		}
		else if (Temp->rect.x+Temp->rect.w+Temp->hspeed > 320) {
			instanceDestroy(Temp, globalP);
			p1Score++;
		}
	
		// Dibuja Pelota
		GsSortRectangle(&Temp->rect);

		// Mover
		Temp->rect.x += Temp->hspeed;
		Temp->rect.y += Temp->vspeed;

		Temp = Temp->Sig;
	}
}

/**
 * Prototio para AI
 * Checa donde intersecta la pelota con el x
 */
int checarPosicionP(int x1, int y1, int x2, int y2, int x) {
	int a, b, c;
	a = (y2-y1);
	b = (x2-x1);
	c = -((x1 * a) - (y1 * b));
	return (b*x) + c / (a);
}
