#ifndef __ADC_PARAMETERS_HPP__
#define __ADC_PARAMETERS_HPP__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "math.h"
#ifdef __cplusplus
}
#endif



/*
 *	MPXV7002DP pitot tube
 *
 *	Vout = VS x (0.2 x P(kPa)+0.5) -> P = ((Vout/Vs)-0.5)/0.2 [Kpa]
 *	Vs = 5 V , P = dynamics Pressure
 *
 *	v = sqrt(2*dynamics_pressure(Pa)/density of air (kg/m3)) [m/s]
 *
 *
 */

class measurement_analog_value_c {

private:

	ADC_HandleTypeDef *hadc;
	uint32_t adc_channel_u32;
	uint16_t adc_resolotion_u16;

	double analog_value_d64;
	double analog_voltage_d64;
	double pressure_d64;


	double density_of_air_d64;
	double Vs_voltage_d64;
	double real_voltage_out_d64;
	double reference_voltage_d64;
	double voltage_divider_ratio;
	double wind_speed_ofset_d64;


public:

	double wind_speed_d64;
	double filtered_wind_speed_d64;


	measurement_analog_value_c(ADC_HandleTypeDef *__hadc,
			uint32_t __adc_channel, uint16_t adc_resolation) {

		this->hadc = __hadc;
		this->adc_channel_u32 = __adc_channel;
		this->adc_resolotion_u16 = adc_resolation;

		this->Vs_voltage_d64 = 5;
		this->voltage_divider_ratio = 0.5;
		this->reference_voltage_d64 = 3.3;


		this->analog_value_d64 = 0;
		this->analog_voltage_d64 = 0;
		this->real_voltage_out_d64 = 0;
		this->pressure_d64 = 0;
		this->wind_speed_d64 = 0;

		this->density_of_air_d64 = 1.293;

		this->wind_speed_ofset_d64 = 0;
		this->filtered_wind_speed_d64 = 0;

	}

	void read_analog_value() {

		ADC_ChannelConfTypeDef sConfig = { 0 };
		uint32_t adc_sample_period_u32 = ADC_SAMPLETIME_480CYCLES;
		sConfig.Channel = this->adc_channel_u32;
		sConfig.Rank = ADC_REGULAR_RANK_1;
		sConfig.SamplingTime = adc_sample_period_u32;

		if (HAL_ADC_ConfigChannel(this->hadc, &sConfig) != HAL_OK) {
			Error_Handler();
		}

		HAL_ADC_Start(this->hadc);
		HAL_ADC_PollForConversion(this->hadc, HAL_MAX_DELAY);
		this->analog_value_d64 = HAL_ADC_GetValue(this->hadc);
		HAL_ADC_Stop(this->hadc);

	}


	void get_offset(){


		for(int i = 0; i<2000; i++){

			this->get_wind_speed();

			this->wind_speed_ofset_d64 += this->wind_speed_d64;

		}

		this->wind_speed_ofset_d64 = this->wind_speed_ofset_d64/2000;


	}

	void get_wind_speed() {

		this->read_analog_value();

		this->analog_voltage_d64 = (this->analog_value_d64
				/ (this->adc_resolotion_u16 - 1)) * this->reference_voltage_d64;

		this->real_voltage_out_d64 = this->analog_voltage_d64 / this->voltage_divider_ratio;

		this->pressure_d64 = (((this->real_voltage_out_d64 / this->Vs_voltage_d64)
				- 0.5) / 0.2)*1000;

		if(this->pressure_d64 >= 0)
		{
			this->wind_speed_d64 = sqrt(
					2 * this->pressure_d64 / this->density_of_air_d64) - this->wind_speed_ofset_d64; //m/s
		}

		else{

			this->wind_speed_d64 = 0;

		}


	}

};

typedef measurement_analog_value_c *measurement_analog_value_c_ptr;

#endif
