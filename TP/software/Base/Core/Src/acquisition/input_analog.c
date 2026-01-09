/*
 * analog_input.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "acquisition/input_analog.h"
#include <stdio.h>
#include <string.h>

extern ADC_HandleTypeDef hadc1; // il faut config l'ADC en trigger intern pour que le CH4 le front montant le trigger
uint16_t adc_buffer[10];

#define ADC_RESOLUTION      4096.0f
#define ADC_VREF            3.3f
#define SENSOR_SENSITIVITY  0.05f   // GO-SME 10 (0.05 V/A)
#define V_OFFSET_THEO       (ADC_VREF / 2.0f)


int cmd_current(h_shell_t* h_shell, int argc, char** argv) {
    int size;

    // Si aucun argument ou argument invalide
    if (argc < 2) {
        size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
                        "Usage: current [get|raw]\r\n");
        h_shell->drv.transmit(h_shell->print_buffer, size);
        return HAL_ERROR;
    }

    if (strcmp(argv[1], "get") == 0) {
        float courant = Calculer_Courant_Moyen();

        int partie_entiere = (int)courant;
        int partie_decimale = (int)((courant - partie_entiere) * 1000); // 3 décimales
        if (partie_decimale < 0) partie_decimale = -partie_decimale;

        size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
                        "Courant Phase U: %d.%03d A\r\n", partie_entiere, partie_decimale);

        h_shell->drv.transmit(h_shell->print_buffer, size);
        return HAL_OK;
    }

    // Commande : current raw (Pour debug, affiche la valeur brute moyenne 0-4095)
    else if (strcmp(argv[1], "raw") == 0) {
        uint32_t sum = 0;
        for (int i = 0; i < 10; i++) sum += adc_buffer[i];
        uint16_t raw_avg = sum / 10;

        size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
                        "Raw ADC Value (Avg): %u\r\n", raw_avg);
        h_shell->drv.transmit(h_shell->print_buffer, size);
        return HAL_OK;
    }

    return HAL_ERROR;
}

int input_analog_init() {
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 10) != HAL_OK) {
        return HAL_ERROR;
    }

    return shell_add(&hshell1, "current", cmd_current, "Get current sensor values");
}

float Calculer_Courant_Moyen(void) {
    uint32_t sum = 0;

    //Moyenne du buffer pour stabiliser la lecture
    for (int i = 0; i < 10; i++) {
        sum += adc_buffer[i];
    }
    float adc_raw_avg = (float)sum / 10.0f;

    //Calcul physique
    float voltage_mesure = (adc_raw_avg * ADC_VREF) / ADC_RESOLUTION;
    float delta_v = voltage_mesure - V_OFFSET_THEO;
    float courant = delta_v / SENSOR_SENSITIVITY;

    return courant;
}
