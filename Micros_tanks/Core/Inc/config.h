/*
 * config.h
 *
 *  Created on: Jan 5, 2026
 *      Author: omarf
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

// --- PANTALLA ---
#define NUM_MATRICES 8

// --- FÍSICA ---
#define GRAVEDAD 0.15f        // le he metido más gravedad
#define VELOCIDAD_DISPARO 3.5f

// --- JUGADORES ---
#define JUGADOR_1 1
#define JUGADOR_2 2

// Posiciones iniciales
#define T1_X_INI  1
#define T1_Y_INI  12
#define T2_X_INI  28
#define T2_Y_INI  12

// --- CÓDIGOS DE COLISIÓN ---
#define COLISION_NADA 0
#define COLISION_OBSTACULO 1
#define COLISION_TANQUE 2

// --- NUBES ---
#define MAX_NUBES 4

typedef struct {
    float x;        // Float para movimiento
    int y;          // Altura
    int ancho;      // 2, 3 o 4
    int alto;       // 1, 2 o 3
    float velocidad; // 0 = Estática, >0 = Móvil
    int activa;     // 1 = Sí, 0 = No
} Nube_t;

#endif /* INC_CONFIG_H_ */

