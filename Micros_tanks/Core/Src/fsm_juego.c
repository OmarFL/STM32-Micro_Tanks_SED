/*
 * fsm_juego.c
 *
 *  Created on: Jan 5, 2026
 *      Author: omarf
 */

#include "fsm_juego.h"
#include "drivers_hardware.h" // Para leer botones y pots
#include "fisicas.h"          // Para mover balas y detectar choques
#include "main.h"             // Para HAL_GetTick()
#include <stdlib.h>


// Banderas externas
extern volatile uint8_t flag_btn_j1;
extern volatile uint8_t flag_btn_j2;



// Función para iniciar nubes
void GenerarNubes(Juego_t *juego) {

    for(int i=0; i<MAX_NUBES; i++) {

        juego->nubes[i].activa = 1;

        // DIMENSIONES
        juego->nubes[i].ancho = (rand() % 4) + 2; // 2 a 4 píxeles
        juego->nubes[i].alto = 2;  // 2 píxeles


       // POSICIONAMIENTO SEGÚN TIPO

       // NUBE ESTÁTICA JUGADOR 1 (entre X=6 y X=15)
       if (i == 0) {
            juego->nubes[i].x = (float)((rand() % 9) + 6);
            juego->nubes[i].y = (rand() % 5) + 1; // Altura aleatoria dentro de los límites
            juego->nubes[i].velocidad = 0.0f; // ESTÁTICA
       }

       // NUBE ESTÁTICA JUGADOR 2 (entre X=18 y X=27)
       else if (i == 1) {
            juego->nubes[i].x = (float)((rand() % 9) + 18);
            juego->nubes[i].y = (rand() % 5) + 1; // Altura aleatoria dentro de los límites
            juego->nubes[i].velocidad = 0.0f; // ESTÁTICA
       }

       // NUBES DINÁMICAS (aleatorio dentro de las posiciones válidas)
       else {

            int posicion_valida = 0;

            while (!posicion_valida) {

                float cand_x = (float)(rand() % 29); // 0 a 28
                int cand_y = rand() % 6;             // 0 a 5

                // Límites (<5 o >26) -> REPETIR
                if (cand_y == 0 && (cand_x < 5 || cand_x > 26)) {

                     posicion_valida = 0;

                }
                else {

                     juego->nubes[i].x = cand_x;
                     juego->nubes[i].y = cand_y;
                     posicion_valida = 1;
                }
           }

           // Velocidad de entre 0.02 y 0.08 píxeles por frame
           juego->nubes[i].velocidad = ((rand() % 6) + 2) / 100.0f;
      }

    }
}


// Función para controlar la distribución aleatoria de las nubes
void FSM_ActualizarNubes(Juego_t *juego) {
    for(int i=0; i<MAX_NUBES; i++) {
        // Solo mover nubes no estáticas
        if (juego->nubes[i].activa && juego->nubes[i].velocidad > 0) {
            juego->nubes[i].x += juego->nubes[i].velocidad;

            // Si se sale por la derecha (> 32), vuelve a entrar por la izquierda
            if (juego->nubes[i].x > 32.0f) {
                juego->nubes[i].x = -4.0f; // Empieza fuera de pantalla por la izq
                juego->nubes[i].y = (rand() % 5) + 1; // Altura aleatoria evitando los límites
            }
        }
    }
}


// Inicio
void FSM_Init(Juego_t *juego) {
    juego->estado_actual = ESTADO_INICIO;
    juego->ganador = 0;
    juego->angulo_actual = 0.0f;
    juego->vidas_j1 = 2;
    juego->vidas_j2 = 2;
    juego->musica_gameover_sonada = 0;

    flag_btn_j1 = 0;
    flag_btn_j2 = 0;

    // Generar las nubes al arrancar la partida
    GenerarNubes(juego);

    // Hardware en estado inicial
    Fisicas_Init();
    HW_Buzzer_Stop();
    HW_LED_Turno(0);
}



// CONTROL DEL JUEGO
void FSM_Actualizar(Juego_t *juego) {

	FSM_ActualizarNubes(juego);


    switch (juego->estado_actual) {

        // ----------------------------------------------------------------
        // 1. ESTADO_INICIO
        // ----------------------------------------------------------------
        case ESTADO_INICIO:
            // Esperar a que alguien pulse un botón para empezar
            if (flag_btn_j1 || flag_btn_j2) {
                flag_btn_j1 = 0;
                flag_btn_j2 = 0;

                HW_Buzzer_Start();
                Fisicas_LimpiarPantalla();
                //FSM_Init(juego);

                // Transición -> Turno J1
                juego->estado_actual = ESTADO_J1_APUNTANDO;
                HW_LED_Turno(1);
            }
            break;

        // ----------------------------------------------------------------
        // 2. ESTADO_J1_APUNTANDO
        // ----------------------------------------------------------------
        case ESTADO_J1_APUNTANDO:
            // Leer potenciómetro continuamente
            //juego->angulo_actual = HW_LeerAngulo(JUGADOR_1);

            // Leer los 2 potenciómetros a la vez
             HW_LeerControles(&juego->angulo_actual, &juego->potencia_actual);

            // Transición: Si J1 pulsa disparo
            if (flag_btn_j1) {
                flag_btn_j1 = 0; // Limpiar bandera
                flag_btn_j2 = 0; // Hay que limpair las dos

                // Configurar el disparo en el motor de físicas
                Fisicas_PrepararDisparo(JUGADOR_1, juego->angulo_actual, juego->potencia_actual, T1_X_INI, T1_Y_INI);

                // Efecto de sonido
                HW_Buzzer_Disparo(juego->potencia_actual);

                // Cambio de estado
                juego->estado_actual = ESTADO_J1_DISPARO;
            }
            break;

        // ----------------------------------------------------------------
        // 3. ESTADO_J1_DISPARO
        // ----------------------------------------------------------------
        case ESTADO_J1_DISPARO:

            // Acción 1: Calcular movimiento (física)
            Fisicas_CalcularSiguientePosicion();

            // Acción 2: Verificar choques (Uso las coordenadas del J2 como objetivo)
            int colision = Fisicas_DetectarColision(T2_X_INI, T2_Y_INI, juego->nubes);

            // Transiciones
            if (colision == COLISION_TANQUE) {

            	// Ha dado al J2
            	juego->vidas_j2--;
            	HW_Buzzer_Impacto();

            	// Parpadear el LED del juagador alcanzado
            	for(int k=0; k<3; k++){

            	    HW_LED_Turno(2); HAL_Delay(100);
            	    HW_LED_Turno(0); HAL_Delay(100);
            	}

            	if (juego->vidas_j2 <= 0) {
            	    // Muerte definitiva
            	    juego->ganador = JUGADOR_1;
            	    juego->estado_actual = ESTADO_GAME_OVER;
            	}
            	else {
            	    // animacion de explosion y cambio de turno
            	    juego->timer_animacion = HAL_GetTick();
            	    juego->estado_actual = ESTADO_J1_IMPACTO;
            	}

            }
            else if (colision == COLISION_OBSTACULO) {
                juego->timer_animacion = HAL_GetTick();
                juego->estado_actual = ESTADO_J1_IMPACTO;
            }
            else if (Fisicas_GetBalaX() > 32 || Fisicas_GetBalaX() < 0 || Fisicas_GetBalaY() > 16) {
                // Se salió de la pantalla -> Cambio de turno directo
                juego->estado_actual = ESTADO_J2_APUNTANDO;
                HW_LED_Turno(2);
            }
            break;

        // ----------------------------------------------------------------
        // 4. ESTADO_J1_IMPACTO (Animación Explosión)
        // ----------------------------------------------------------------
        case ESTADO_J1_IMPACTO:
            // El main llama a Fisicas_DibujarExplosion() usando el timer

            // Transición: Esperar 500ms y cambiar turno
            if (HAL_GetTick() - juego->timer_animacion > 500) {
                juego->estado_actual = ESTADO_J2_APUNTANDO;
                HW_LED_Turno(2);
            }
            break;

        // ----------------------------------------------------------------
        // 5. ESTADO_J2_APUNTANDO
        // ----------------------------------------------------------------
        case ESTADO_J2_APUNTANDO:
            //juego->angulo_actual = HW_LeerAngulo(JUGADOR_2);

            // Leer los 2 potenciómetros a la vez
             HW_LeerControles(&juego->angulo_actual, &juego->potencia_actual);

            if (flag_btn_j2) {
                flag_btn_j2 = 0;
                flag_btn_j1 = 0;

                Fisicas_PrepararDisparo(JUGADOR_2, juego->angulo_actual, juego->potencia_actual, T2_X_INI, T2_Y_INI);
                HW_Buzzer_Disparo(juego->potencia_actual);
                juego->estado_actual = ESTADO_J2_DISPARO;
            }
            break;

        // ----------------------------------------------------------------
        // 6. ESTADO_J2_DISPARO
        // ----------------------------------------------------------------
        case ESTADO_J2_DISPARO:
            Fisicas_CalcularSiguientePosicion();

            // Verificar choque contra J1
            int colision_j2 = Fisicas_DetectarColision(T1_X_INI, T1_Y_INI, juego->nubes);


            // Transiciones
            if (colision_j2 == COLISION_TANQUE) {

            	// Ha dado al J2
                juego->vidas_j1--;
                HW_Buzzer_Impacto();

                // Parpadear el LED del juagador alcanzado
                for(int k=0; k<3; k++){

                	HW_LED_Turno(1); HAL_Delay(100);
                    HW_LED_Turno(0); HAL_Delay(100);
                }

                if (juego->vidas_j1 <= 0) {
                // Muerte definitiva
                    juego->ganador = JUGADOR_2;
                    juego->estado_actual = ESTADO_GAME_OVER;
                }
                else {
                // animacion de explosion y cambio de turno
                    juego->timer_animacion = HAL_GetTick();
                    juego->estado_actual = ESTADO_J2_IMPACTO;
                }
            }
            else if (colision_j2 == COLISION_OBSTACULO) {
                juego->timer_animacion = HAL_GetTick();
                juego->estado_actual = ESTADO_J2_IMPACTO;
            }
            else if (Fisicas_GetBalaX() < 0 || Fisicas_GetBalaX() > 32 || Fisicas_GetBalaY() > 16) {
                juego->estado_actual = ESTADO_J1_APUNTANDO;
                HW_LED_Turno(1);
            }
            break;

        // ----------------------------------------------------------------
        // 7. ESTADO_J2_IMPACTO
        // ----------------------------------------------------------------
        case ESTADO_J2_IMPACTO:
            if (HAL_GetTick() - juego->timer_animacion > 500) {
                juego->estado_actual = ESTADO_J1_APUNTANDO;
                HW_LED_Turno(1);
            }
            break;

        // ----------------------------------------------------------------
        // 8. ESTADO_GAME_OVER
        // ----------------------------------------------------------------
        case ESTADO_GAME_OVER:
            // El main dibuja el texto del ganador
        	HW_LED_Turno(0);

        	//Flag de musica final
        	if (juego->musica_gameover_sonada == 0) {

        	    HW_Buzzer_Victoria();
        	    juego->musica_gameover_sonada = 1;
        	}

            // Transición: Reiniciar si pulsan botón
            if (flag_btn_j1 || flag_btn_j2) {
                flag_btn_j1 = 0;
                flag_btn_j2 = 0;
                FSM_Init(juego); // Vuelta a empezar
            }
            break;
    }
}

