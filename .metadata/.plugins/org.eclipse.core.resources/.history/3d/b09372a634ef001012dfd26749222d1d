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

// --- POTENCIÓMETROS (ADC) ---
// Devuelve el ángulo (0-90) para el jugador indicado (1 o 2)
float HW_LeerAngulo(int jugador);

// --- ZUMBADOR (PWM) ---
void HW_Buzzer_Init(void);
void HW_Buzzer_Stop(void);
void HW_Buzzer_Frecuencia(float freq); // Pone un tono
void HW_Buzzer_Disparo(void);          // Efecto de sonido "Pium"
void HW_Buzzer_Victoria(void);         // Melodía completa

// --- BOTONES ---
// Devuelve 1 si es una pulsación válida (con debounce), 0 si es ruido
uint8_t HW_DebounceBoton(void);

#endif /* INC_DRIVERS_HARDWARE_H_ */
