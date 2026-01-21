/*
 * analog_input.c
 *
 * Created on: Nov 11, 2025
 * Author: nicolas
 */


#include "acquisition/input_analog.h"

/** @brief DMA buffer for current sensor */
volatile uint16_t adc_buffer[10]; //DMA buffer current sensor
extern ADC_HandleTypeDef hadc1; // il faut config l'ADC en trigger intern pour que le CH4 le front montant le trigger

/**
 *
 * @brief Shell command to read current sensor.
 *
 * This function handles the 'current' command in the shell.
 * It provides two sub-commands:
 * - 'get': Displays the physical current value in Amperes.
 * - 'raw': Displays the raw average ADC value (0-4095) for debug.
 *
 * @param h_shell The pointer to the shell instance.
 * @param argc The number of command arguments.
 * @param argv The array of command arguments.
 * @return HAL_OK if the operation is valid, HAL_ERROR otherwise.
 */
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
		/*
        int partie_entiere = (int)courant;
        int partie_decimale = (int)((courant - partie_entiere) * 1000); // 3 décimales
        if (partie_decimale < 0) partie_decimale = -partie_decimale;

        size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
                        "Courant Phase U: %d.%03d A\r\n", partie_entiere, partie_decimale);*/

		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
				"Courant Phase U: %.2f A\r\n", courant);
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

/**
 *
 * @brief Initializes the analog input module.
 *
 * This function starts the ADC in DMA mode to fill the buffer continuously
 * and adds the 'current' function to the shell command list.
 *
 * @return HAL_OK if initialization is successful, HAL_ERROR otherwise.
 */
int input_analog_init() {
	if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 10) != HAL_OK) {
		return HAL_ERROR;
	}

	return shell_add(&hshell1, "current", cmd_current, "Get current sensor values");
}

/**
 *
 * @brief Computes the average current from the DMA buffer.
 *
 * This function performs a moving average on the 10 samples of the ADC buffer.
 * It converts the raw value to voltage, removes the theoretical offset,
 * and applies the sensor sensitivity to return a value in Amperes.
 *
 * @return The calculated current in Amperes.
 */
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


/**
 *
 * @brief ADC DMA conversion complete callback.
 *
 * This function is called when the DMA has finished transferring the ADC data.
 * It sets the flag current_asserv to 1 to indicate new data is available.
 *
 * @param hadc Pointer to the ADC handle.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){

	current_asserv = 1 ;


}
