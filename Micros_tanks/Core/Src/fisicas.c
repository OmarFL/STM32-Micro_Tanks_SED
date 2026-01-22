/*
 * fisicas.c
 *
 *  Created on: Jan 5, 2026
 *      Author: omarf
 */
#include "config.h"
#include "fisicas.h"
#include "fsm_juego.h"
#include "drivers_hardware.h"
#include <math.h>
#include <stdlib.h>

// --- MEMORIA DE VÍDEO (Privada de este módulo) ---
static volatile uint32_t framebuffer[16];


// --- VARIABLES FÍSICAS INTERNAS ---
static float bala_x, bala_y;
static float bala_vx, bala_vy;


// --- FUNCIONES PRIVADAS (Auxiliares) ---
static void SetPixel(int x, int y, int color) {
    if (x >= 0 && x < 32 && y >= 0 && y < 16) {
        if (color) framebuffer[y] |= (1UL << x);
        else       framebuffer[y] &= ~(1UL << x);
    }
}

static void PintarTanque(int x, int y, int direccion) {
    // 0=Vacío, 1=Pintar. Matriz 3x3.
    static const int FORMA[3][3] = {
        {0,0,1}, // Cañón (arriba)
        {1,1,0}, // Cuerpo
        {1,1,1}  // Orugas (abajo)
    };

    for (int f = 0; f < 3; f++) {
        for (int c = 0; c < 3; c++) {
            // Si direccion es 1 (Derecha/J1), pintamos normal.
            // Si es -1 (Izquierda/J2), invertimos columnas (espejo).
            int col_leida = (direccion == 1) ? c : (2 - c);

            if (FORMA[f][col_leida] == 1) {
                SetPixel(x + c, y + f, 1);
            }
        }
    }
}



// --- FUNCIONES PÚBLICAS (Las que pide el .h) ---

void Fisicas_Init(void) {
    Fisicas_LimpiarPantalla();
    bala_x = 0; bala_y = 0;
    bala_vx = 0; bala_vy = 0;
}

uint32_t* Fisicas_GetBuffer(void) {
    return (uint32_t*)framebuffer;
}

void Fisicas_LimpiarPantalla(void) {
    for(int i = 0; i < 16; i++) framebuffer[i] = 0;
}


// Dibuja suelo, muro y tanques (usando los parámetros que le pase la FSM)
void Fisicas_DibujarEscenario(int t1_x, int t1_y, int t2_x, int t2_y, Nube_t *nubes) {

    // Suelo
    for(int x = 0; x < 32; x++) SetPixel(x, 15, 1);

    // Muros
    for(int y = 15 - 5; y < 15; y++) { //he cambiado los muros pq antes eran demasiado altos
        SetPixel(13, y, 1); SetPixel(14, y, 1); // Muro 1
        SetPixel(17, y, 1); SetPixel(18, y, 1); // Muro 2
    }

    // Tanques
    PintarTanque(t1_x, t1_y, 1);  // J1 mira derecha
    PintarTanque(t2_x, t2_y, -1); // J2 mira izquierda


    //Nubes
    for(int i=0; i<MAX_NUBES; i++) {

        if(nubes[i].activa) {

            int nube_x_int = (int)nubes[i].x; //Cast a int
            int nube_y = nubes[i].y;
            int nube_w = nubes[i].ancho;
            int nube_h = nubes[i].alto;

            // Dibujar nube
            for(int y = nube_y; y < nube_y + nube_h; y++) {
                for(int x = nube_x_int; x < nube_x_int + nube_w; x++) {
                    // Comprobar límites array
                    if(x >= 0 && x < 32 && y >= 0 && y < 16) {
                        SetPixel(x, y, 1);
                    }
                }
            }
        }
    }


}




// --- FÍSICAS DE LA BALA ---

// Configura el disparo inicial
void Fisicas_PrepararDisparo(int jugador, float angulo, float fuerza, int origen_x, int origen_y) {

    float radianes = angulo * (3.14159f / 180.0f);

    // Factor de ajuste: Convertimos potencia (0-100) a velocidad de píxeles (ej: 0.5 a 5.0)
    float velocidad = fuerza / 20.0f;

    // Ajuste de posición de salida (para que salga del cañón y no del centro)
    if (jugador == JUGADOR_1) {
            bala_x = origen_x + 2;
            bala_y = origen_y;
            bala_vx = cosf(radianes) * velocidad;
            bala_vy = -sinf(radianes) * velocidad; // -Y es arriba
        } else {
            bala_x = origen_x;
            bala_y = origen_y;
            bala_vx = -cosf(radianes) * velocidad; // X negativa hacia izq
            bala_vy = -sinf(radianes) * velocidad;
        }
    }


void Fisicas_CalcularSiguientePosicion(void) {
    bala_vy += GRAVEDAD; // La gravedad aumenta la velocidad de caída
    bala_x += bala_vx;
    bala_y += bala_vy;
}

int Fisicas_DetectarColision(int objetivo_x, int objetivo_y, Nube_t *nubes) {
    int ix = (int)bala_x;
    int iy = (int)bala_y;

    // Salida de pantalla (Lados o suelo muy profundo)
    if (ix < 0 || ix > 32 || iy > 16) return COLISION_NADA; // FSM manejará la salida

    // Choque con Suelo
    if (iy >= 15) return COLISION_OBSTACULO;

    // Choque con Muros (Coordenadas fijas de los muros)
    if (iy >= 10 && iy < 15) {
        if ((ix >= 13 && ix <= 14) || (ix >= 17 && ix <= 18)) {
            return COLISION_OBSTACULO;
        }
    }

    // Choque con Nubes
    for(int i=0; i<MAX_NUBES; i++) {

        if(nubes[i].activa) {
            int nx = (int)nubes[i].x;

            if (ix >= nx && ix < (nx + nubes[i].ancho) &&
                iy >= nubes[i].y && iy < (nubes[i].y + nubes[i].alto)) {
                return COLISION_OBSTACULO;
            }
        }
    }


    // Choque con Tanque Objetivo (Hitbox 3x3)
    if (ix >= objetivo_x && ix <= (objetivo_x + 2) &&
        iy >= objetivo_y && iy <= (objetivo_y + 2)) {
        return COLISION_TANQUE;
    }

    return COLISION_NADA;
}


void Fisicas_PintarBala(void) {
    int ix = (int)bala_x;
    int iy = (int)bala_y;
    SetPixel(ix, iy, 1);
}

// Getters para que la FSM sepa dónde explotó
int Fisicas_GetBalaX(void) { return (int)bala_x; }
int Fisicas_GetBalaY(void) { return (int)bala_y; }





// --- ANIMACIONES ---


void Fisicas_DibujarExplosion(int x, int y, uint32_t tiempo_explosion) {
	//tiempo_explosion se inicia el momento en el que se produce la explosion
     SetPixel(x, y, 1);
     if(HAL_GetTick()-tiempo_explosion>900) {
             	     SetPixel(x+1, y+1, 1);
             	     SetPixel(x-1, y-1, 1);
             	     SetPixel(x+1, y-1, 1);
             	     SetPixel(x-1, y+1, 1);
                 	 SetPixel(x, y, 1);
                      SetPixel(x+2, y, 1);
                      SetPixel(x-2, y, 1);
                      SetPixel(x, y-2, 1);
                      SetPixel(x, y+2, 1);
                      SetPixel(x+2, y+2, 1);
                      SetPixel(x-2, y-2, 1);
                      SetPixel(x+2, y-2, 1);
                      SetPixel(x-2, y+2, 1);

                  }
     else if(HAL_GetTick()-tiempo_explosion>600) {
             	 SetPixel(x, y, 1);
                  SetPixel(x+1, y, 1);
                  SetPixel(x-1, y, 1);
                  SetPixel(x, y-1, 1);
                  SetPixel(x, y+1, 1);
                  SetPixel(x+2, y+2, 1);
                  SetPixel(x-1, y-2, 1);
                  SetPixel(x+2, y-2, 1);
                  SetPixel(x-2, y+2, 1);
              }
     else if(HAL_GetTick()-tiempo_explosion>300) {
    	 SetPixel(x, y, 1);
         SetPixel(x+1, y+1, 1);
         SetPixel(x-1, y-1, 1);
         SetPixel(x+1, y-1, 1);
         SetPixel(x-1, y+1, 1);
     }
}


// Función auxiliar para dibujar un carácter de 8x8 en una posición concreta
// Recibe el array de 8 bytes (bitmap) que define la letra
static void DibujarLetra8x8(int x_offset, int y_offset, const uint8_t *letra) {
    for (int fila = 0; fila < 8; fila++) {
        uint8_t linea = letra[fila];
        for (int col = 0; col < 8; col++) {
        	// Lee del bit menos significativo (Derecha) al más significativo (Izquierda)
            if (linea & (1 <<  col)) {
                SetPixel(x_offset + col, y_offset + fila, 1);
            }
        }
    }
}



void Fisicas_DibujarGameOver(void) {

		Fisicas_LimpiarPantalla();

	    // Definición de las letras (Mapas de bits 8x8)
	    // Cada byte es una fila, de arriba a abajo.

	    const uint8_t LETRA_G[] = { 0x3C, 0x66, 0xC0, 0xC0, 0xCE, 0xC6, 0x66, 0x3E };
	    const uint8_t LETRA_A[] = { 0x18, 0x3C, 0x66, 0xC3, 0xFF, 0xC3, 0xC3, 0xC3 };
	    const uint8_t LETRA_M[] = { 0xC3, 0xE7, 0xFF, 0xDB, 0xC3, 0xC3, 0xC3, 0xC3 };
	    const uint8_t LETRA_E[] = { 0xFF, 0xC0, 0xC0, 0xFF, 0xC0, 0xC0, 0xC0, 0xFF };

	    const uint8_t LETRA_O[] = { 0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C };
	    const uint8_t LETRA_V[] = { 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C, 0x18 };
	    const uint8_t LETRA_R[] = { 0xF8, 0xFC, 0xC6, 0xC6, 0xFC, 0xF8, 0xC6, 0xC6 };

	    // Dibujar "GAME" en la fila superior (Y=0)
	    // Matriz 0 (X=0), Matriz 1 (X=8), Matriz 2 (X=16), Matriz 3 (X=24)
	    DibujarLetra8x8(0,  0, LETRA_E);
	    DibujarLetra8x8(8,  0, LETRA_M);
	    DibujarLetra8x8(16, 0, LETRA_A);
	    DibujarLetra8x8(24, 0, LETRA_G);

	    // Dibujar "OVER" en la fila inferior (Y=8)
	    DibujarLetra8x8(0,  8, LETRA_R);
	    DibujarLetra8x8(8,  8, LETRA_E);
	    DibujarLetra8x8(16, 8, LETRA_V);
	    DibujarLetra8x8(24, 8, LETRA_O);
}


void Fisicas_DibujarInicio(uint32_t tiempo_actual) {

	//Fondo de pantalla inicial
	    for(int x = 0; x < 32; x++) SetPixel(x, 15, 1);
	    PintarTanque(2, 12, 1);
	    PintarTanque(27, 12, -1);

	    //Parpadeo
	    if ((tiempo_actual / 500) % 2 == 0) {

	        // Arrays INVERTIDOS
	    	const uint8_t L_S[] = { 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C, 0x00 };
	        const uint8_t L_T[] = { 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00 };
	        const uint8_t L_A[] = { 0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00 };
	        const uint8_t L_R[] = { 0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00 };

	        int y = 3; //pongo 3 pero se puede reajustar
	        DibujarLetra8x8(0,  y, L_T);
	        DibujarLetra8x8(6,  y, L_R);
	        DibujarLetra8x8(12, y, L_A);
	        DibujarLetra8x8(18, y, L_T);
	        DibujarLetra8x8(24, y, L_S);
	    }
}
