/*
 * drivers_hardware.h
 *
 *  Created on: Jan 5, 2026
 *      Author: omarf
 */

#ifndef INC_DRIVERS_HARDWARE_H_
#define INC_DRIVERS_HARDWARE_H_

#include <stdint.h>
#include "main.h" // Necesario para HAL

// --- PANTALLA (SPI) ---
void HW_MAX7219_Init(void);
void HW_UpdateDisplay(uint32_t *buffer_pantalla);

// --- LEDS de TURNO ---
void HW_LED_Turno(int jugador); // 1=J1, 2=J2, 0=Apagar

// --- POTENCIÓMETROS (ADC) ---
// Devuelve el ángulo (0-90) para el jugador indicado (1 o 2)
//float HW_LeerAngulo(int jugador);
// Lee el estado de la mesa de control (Angulo y Potencia)
// Pasa las variables por referencia para guardarlas
void HW_LeerControles(float *angulo, float *potencia);

// --- ZUMBADOR (PWM) ---
void HW_Buzzer_Start(void);
void HW_Buzzer_Init(void);
void HW_Buzzer_Stop(void);
void HW_Buzzer_Frecuencia(float freq);
void HW_Buzzer_Disparo(float pot);
void SonidoPWM_Beep(float freq, float duration_ms); //play sonido
void HW_Buzzer_Victoria(void);         // Melodía completa
void HW_Buzzer_Impacto(void);

// --- BOTONES ---
// Devuelve 1 si es una pulsación válida (con debounce), 0 si es ruido
uint8_t HW_DebounceBoton(void);

#endif /* INC_DRIVERS_HARDWARE_H_ */
