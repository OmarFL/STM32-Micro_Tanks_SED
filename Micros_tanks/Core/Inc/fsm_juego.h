/*
 * fsm_juego.h
 *
 *  Created on: Jan 5, 2026
 *      Author: omarf
 */

#ifndef INC_FSM_JUEGO_H_
#define INC_FSM_JUEGO_H_

#include <stdint.h>
#include "config.h" // Necesitamos saber qué es T1_X_INI, etc.

// Definición de los ESTADOS del diagrama
typedef enum {
    ESTADO_INICIO,
    ESTADO_J1_APUNTANDO,
    ESTADO_J1_DISPARO,
    ESTADO_J1_IMPACTO,   // Animación explosión J1
    ESTADO_J2_APUNTANDO,
    ESTADO_J2_DISPARO,
    ESTADO_J2_IMPACTO,   // Animación explosión J2
    ESTADO_GAME_OVER
} Estado_t;


// Objeto principal del juego
typedef struct {
    Estado_t estado_actual;

    // Datos de la partida
    int ganador;            // 0=Nadie, 1=J1, 2=J2
    float angulo_actual;    // Guardar el ángulo mientras se apunta
    float potencia_actual;

    // Control de tiempos para animaciones
    uint32_t timer_animacion;
    int musica_gameover_sonada;

    // Vidas
    int vidas_j1;
    int vidas_j2;

    // Array de Nubes
    Nube_t nubes[MAX_NUBES];

} Juego_t;


void FSM_Init(Juego_t *juego);
void FSM_Actualizar(Juego_t *juego);
void FSM_ActualizarNubes(Juego_t *juego);


#endif /* INC_FSM_JUEGO_H_ */
