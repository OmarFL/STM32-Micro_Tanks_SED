/*
 * drivers_hardware.c
 *
 *  Created on: Jan 5, 2026
 *      Author: omarf
 */
#include "drivers_hardware.h"
#include "config.h"
#include <math.h>

// Variables externas del main.c (generadas por el .ioc)
extern SPI_HandleTypeDef hspi1;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1; // OJO: Ahora es TIM1

// ================= PANTALLA (SPI) =================
static void MAX7219_SendAll(uint8_t address, uint8_t data) {
    uint8_t paquete[2] = {address, data};
    HAL_GPIO_WritePin(CS_MATRIZ_GPIO_Port, CS_MATRIZ_Pin, GPIO_PIN_RESET);
    for(int i = 0; i < NUM_MATRICES; i++) {
        HAL_SPI_Transmit(&hspi1, paquete, 2, 10);
    }
    HAL_GPIO_WritePin(CS_MATRIZ_GPIO_Port, CS_MATRIZ_Pin, GPIO_PIN_SET);
}

void HW_MAX7219_Init(void) {
    MAX7219_SendAll(0x0C, 0x01); // Shutdown -> Normal Operation
    MAX7219_SendAll(0x09, 0x00); // Decode Mode -> No decode
    MAX7219_SendAll(0x0B, 0x07); // Scan Limit -> All digits
    MAX7219_SendAll(0x0A, 0x01); // Intensity -> Min
    MAX7219_SendAll(0x0F, 0x00); // Display Test -> Off
}

void HW_UpdateDisplay(uint32_t *buffer) {
    for (int row = 0; row < 8; row++) {
        HAL_GPIO_WritePin(CS_MATRIZ_GPIO_Port, CS_MATRIZ_Pin, GPIO_PIN_RESET);

        // Parte Inferior (Filas 8-15)
        uint32_t linea_inf = buffer[row + 8];
        for(int m = 3; m >= 0; m--) {
            uint8_t data[2] = { row + 1, (linea_inf >> (m * 8)) & 0xFF };
            HAL_SPI_Transmit(&hspi1, data, 2, 10);
        }

        // Parte Superior (Filas 0-7)
        uint32_t linea_sup = buffer[row];
        for(int m = 3; m >= 0; m--) {
            uint8_t data[2] = { row + 1, (linea_sup >> (m * 8)) & 0xFF };
            HAL_SPI_Transmit(&hspi1, data, 2, 10);
        }
        HAL_GPIO_WritePin(CS_MATRIZ_GPIO_Port, CS_MATRIZ_Pin, GPIO_PIN_SET);
    }
}

// ================= POTENCIÓMETROS (ADC) =================
float HW_LeerAngulo(int jugador) {
    HAL_ADC_Start(&hadc1);
    uint32_t val1 = 0, val2 = 0;

    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) val1 = HAL_ADC_GetValue(&hadc1);
    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) val2 = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    uint32_t raw = (jugador == 1) ? val1 : val2;
    // Mapeo: 0-4095 -> 0-90 grados
    return (raw * 90.0f) / 4095.0f;
}

// ================= ZUMBADOR (PWM) =================
void HW_Buzzer_Init(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
}

void HW_Buzzer_Frecuencia(float freq) {
    if (freq < 10) {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); // Silencio
        return;
    }
    // Asumiendo reloj base aprox 1MHz (Prescaler 100 en 100MHz)
    // Periodo = 1.000.000 / Freq
    uint32_t periodo = (uint32_t)(1000000 / freq);
    __HAL_TIM_SET_AUTORELOAD(&htim1, periodo);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, periodo / 2); // 50% ciclo
}

void HW_Buzzer_Stop(void) {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

void HW_Buzzer_Disparo(void) {
    for(int f=800; f>200; f-=50) { // Barrido de frecuencia descendente
        HW_Buzzer_Frecuencia(f);
        HAL_Delay(5);
    }
    HW_Buzzer_Stop();
}

void HW_Buzzer_Victoria(void) {
    // Melodía simple
    float notas[] = {523.25, 659.25, 783.99, 1046.50}; // Do, Mi, Sol, Do alto
    for(int i=0; i<4; i++) {
        HW_Buzzer_Frecuencia(notas[i]);
        HAL_Delay(150);
        HW_Buzzer_Stop();
        HAL_Delay(50);
    }
}

// ================= BOTONES =================
uint8_t HW_DebounceBoton(void) {
    static uint32_t ultimo_tick = 0;
    if (HAL_GetTick() - ultimo_tick > 200) { // 200ms debounce
        ultimo_tick = HAL_GetTick();
        return 1;
    }
    return 0;
}

