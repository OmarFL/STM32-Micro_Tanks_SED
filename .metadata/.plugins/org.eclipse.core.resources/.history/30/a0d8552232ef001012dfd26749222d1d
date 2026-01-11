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


// Banderas externas
extern volatile uint8_t flag_btn_j1;
extern volatile uint8_t flag_btn_j2;


// Función de limpieza inicial
void FSM_Init(Juego_t *juego) {
    juego->estado_actual = ESTADO_INICIO;
    juego->ganador = 0;
    juego->angulo_actual = 0.0f;

    // Limpiamos flags por si se pulsó algo al arrancar
    flag_btn_j1 = 0;
    flag_btn_j2 = 0;

    // Hardware en estado inicial
    Fisicas_Init();
    HW_Buzzer_Stop();
}

// EL CEREBRO DEL JUEGO
void FSM_Actualizar(Juego_t *juego) {

    switch (juego->estado_actual) {

        // ----------------------------------------------------------------
        // 1. ESTADO_INICIO
        // ----------------------------------------------------------------
        case ESTADO_INICIO:
            // Esperar a que alguien pulse un botón para empezar
            if (flag_btn_j1 || flag_btn_j2) {
                flag_btn_j1 = 0;
                flag_btn_j2 = 0;

                // Preparar escenario
                Fisicas_LimpiarPantalla();

                // Transición -> Turno J1
                juego->estado_actual = ESTADO_J1_APUNTANDO;
            }
            break;

        // ----------------------------------------------------------------
        // 2. ESTADO_J1_APUNTANDO
        // ----------------------------------------------------------------
        case ESTADO_J1_APUNTANDO:
            // Leer potenciómetro continuamente
            juego->angulo_actual = HW_LeerAngulo(JUGADOR_1);

            // Transición: Si J1 pulsa disparo
            if (flag_btn_j1) {
                flag_btn_j1 = 0; // Limpiar bandera

                // Configurar el disparo en el motor de físicas
                Fisicas_PrepararDisparo(JUGADOR_1, juego->angulo_actual, T1_X_INI, T1_Y_INI);

                // Efecto de sonido
                HW_Buzzer_Disparo();

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
            int colision = Fisicas_DetectarColision(T2_X_INI, T2_Y_INI);

            // Transiciones
            if (colision == COLISION_TANQUE) {
                juego->ganador = JUGADOR_1;
                juego->estado_actual = ESTADO_GAME_OVER;
                HW_Buzzer_Victoria();
            }

            else if (colision == COLISION_OBSTACULO) {
                // Guardar el tiempo actual para la animación
                juego->timer_animacion = HAL_GetTick();
                juego->estado_actual = ESTADO_J1_IMPACTO;
            }
            else if (Fisicas_GetBalaX() > 32 || Fisicas_GetBalaX() < 0 || Fisicas_GetBalaY() > 16) {
                // Se salió de la pantalla -> Cambio de turno directo
                juego->estado_actual = ESTADO_J2_APUNTANDO;
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
            }
            break;

        // ----------------------------------------------------------------
        // 5. ESTADO_J2_APUNTANDO
        // ----------------------------------------------------------------
        case ESTADO_J2_APUNTANDO:
            juego->angulo_actual = HW_LeerAngulo(JUGADOR_2);

            if (flag_btn_j2) {
                flag_btn_j2 = 0;

                Fisicas_PrepararDisparo(JUGADOR_2, juego->angulo_actual, T2_X_INI, T2_Y_INI);
                HW_Buzzer_Disparo();
                juego->estado_actual = ESTADO_J2_DISPARO;
            }
            break;

        // ----------------------------------------------------------------
        // 6. ESTADO_J2_DISPARO
        // ----------------------------------------------------------------
        case ESTADO_J2_DISPARO:
            Fisicas_CalcularSiguientePosicion();

            // Verificar choque contra J1
            int colision_j2 = Fisicas_DetectarColision(T1_X_INI, T1_Y_INI);

            if (colision_j2 == COLISION_TANQUE) {
                juego->ganador = JUGADOR_2;
                juego->estado_actual = ESTADO_GAME_OVER;
                HW_Buzzer_Victoria();
            }
            else if (colision_j2 == COLISION_OBSTACULO) {
                juego->timer_animacion = HAL_GetTick();
                juego->estado_actual = ESTADO_J2_IMPACTO;
            }
            else if (Fisicas_GetBalaX() < 0 || Fisicas_GetBalaX() > 32 || Fisicas_GetBalaY() > 16) {
                juego->estado_actual = ESTADO_J1_APUNTANDO;
            }
            break;

        // ----------------------------------------------------------------
        // 7. ESTADO_J2_IMPACTO
        // ----------------------------------------------------------------
        case ESTADO_J2_IMPACTO:
            if (HAL_GetTick() - juego->timer_animacion > 500) {
                juego->estado_actual = ESTADO_J1_APUNTANDO;
            }
            break;

        // ----------------------------------------------------------------
        // 8. ESTADO_GAME_OVER
        // ----------------------------------------------------------------
        case ESTADO_GAME_OVER:
            // El main dibuja el texto del ganador

            // Transición: Reiniciar si pulsan botón
            if (flag_btn_j1 || flag_btn_j2) {
                flag_btn_j1 = 0;
                flag_btn_j2 = 0;
                FSM_Init(juego); // Vuelta a empezar
            }
            break;
    }
}

