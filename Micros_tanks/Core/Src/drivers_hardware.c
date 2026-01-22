/*
 * drivers_hardware.c
 *
 * Created on: Jan 5, 2026
 * Author: omarf
 */
#include "drivers_hardware.h"
#include "config.h"
#include <math.h>

// Variables externas del main.c (generadas por el .ioc)
extern SPI_HandleTypeDef hspi1;
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;

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

// Lee ÁNGULO (PA0) y POTENCIA (PA1) secuencialmente
void HW_LeerControles(float *angulo, float *potencia) {
    HAL_ADC_Start(&hadc1);

    uint32_t raw_angulo = 0;
    uint32_t raw_potencia = 0;

    // Rank 1: Canal 0 (PA0) -> Ángulo
    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        raw_angulo = HAL_ADC_GetValue(&hadc1);
    }

    // Rank 2: Canal 1 (PA1) -> Potencia
    if(HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        raw_potencia = HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    // Mapeo
    // Ángulo: 0 a 90 grados
    *angulo = (raw_angulo * 90.0f) / 4095.0f;

    // Potencia: 10 a 100 (Sumamos 10 para que nunca sea 0 y la bala avance)
    *potencia = ((raw_potencia * 90.0f) / 4095.0f) + 10.0f;
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
    // Reloj base ~1MHz tras prescaler
    uint32_t periodo = (uint32_t)(1000000 / freq);
    __HAL_TIM_SET_AUTORELOAD(&htim1, periodo);

    // Activar sonido (Ciclo 50%)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, periodo / 2);
}

void HW_Buzzer_Stop(void) {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}

// Función auxiliar interna para hacer beeps bloqueantes (necesaria para melodías simples)
void SonidoPWM_Beep(float freq, float duration_ms)
{
    uint32_t counter = HAL_GetTick();
    HW_Buzzer_Frecuencia(freq);

    while (HAL_GetTick() - counter < duration_ms)
    {
        HW_Buzzer_Frecuencia(freq);
    }

    HW_Buzzer_Stop();
}

void HW_Buzzer_Start(void) {

    SonidoPWM_Beep(987, 100);
    HAL_Delay(50);
    // moneda mario bros xd
    SonidoPWM_Beep(1318, 300);
}

void HW_Buzzer_Disparo(float pot) {
    float frec;
    frec = 500.0f + (pot / 100) * 2500.0f;
    SonidoPWM_Beep(frec, 200);
}

void HW_Buzzer_Impacto(void) {
    for(int f=300; f>100; f-=10) {
        HW_Buzzer_Frecuencia(f);
        HAL_Delay(5);
    }
    HW_Buzzer_Stop();
}

void HW_Buzzer_Victoria(void) {
    float A = 440;
    float F = 349.228231;
    float G = 391.995436;
    float tempo = 150; //bpm
    float t_negra = tempo/60 * 100; // ms
    float t_corchea = t_negra/4;
    float t_blanca = 2*t_negra;

    SonidoPWM_Beep(A,t_corchea);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(A,t_corchea);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(A,t_corchea);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(A,t_negra);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(F,t_negra);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(G,t_corchea);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(G,t_corchea);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(A,t_corchea);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(0,t_corchea);//silencio corchea
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(G,t_corchea);
    SonidoPWM_Beep(0,100);
    SonidoPWM_Beep(A,t_blanca);
    HW_Buzzer_Stop();
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



// ================= LEDS DE TURNO =================
void HW_LED_Turno(int jugador) {

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);

    if (jugador == 1) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET); // Enciende PC0 (J1)
    }
    else if (jugador == 2) {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET); // Enciende PC1 (J2)
    }
}


