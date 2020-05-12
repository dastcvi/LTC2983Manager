/*
 *  LTC2983Manager.h
 *  Definition of a generalized class to manage the LTC2983 temperature chip
 *  Author: Alex St. Clair
 *  April 2018
 *
 *  This is a general class meant for use by multiple boards for the LASP Strateole 2
 *  payloads and also future projects to configure and control the LTC2983 temperature
 *  chip.
 *
 *  To use:
 *    0) Instantiate an object of the class. Assign the chip select and reset pins
 *       based on the board design. If there are sense resistors for thermistors or
 *       RTDs, assign the channels based on board design. If there isn't a sense
 *       resistor for either of those sensor types, assign it to channel 0, which
 *       specifies that it doesn't exist.
 *    1) Create sensor assignments in the channel_assignments[21] array. The index
 *       corresponds to the channel number, from 1 to 20 (index 0 is unused). Only the
 *       sensors in the Sensor_Type_t enum are supported, but it is easy to add
 *       sensors to this enum.
 *    2) Call InitializeAndConfigure(), this will assign channels based on
 *       channel_assignments[21]
 *    3) Read sensors by calling MeasureAllChannels() or MeasureChannel(uint8_t)
 *    4) Results for valid, requested channels will be in channel_temperatures[21],
 *       and MeasureChannel(uint8_t) will also return the result
 *
 *  Note: this class does not error check sensor configurations, ie. will not catch if a
 *        channel that is assigned to one sensor is then needed for differential input.

  September 2018, Updated by Marika Schubert to allow
 selection of SPI port
 */

#ifndef LTC2983MANAGER_H
#define LTC2983MANAGER_H

#include "LTC2983_configuration_constants.h"
#include "LTC2983_table_coeffs.h"
#include "LTC2983_support_functions.h"
#include "Arduino.h"
#include "HardwareSerial.h"
#include "WProgram.h"
#include "SPI.h"
#include <stdint.h>

#define TEMPERATURE_ERROR	-300.0f
#define LTC_POWERED_OFF		-888.0f
#define LTC_SENSOR_ERROR	-999.0f

#ifndef SPI_DISABLE
#define SPI_DISABLE (0x1<<30)
#endif

enum Sensor_Type_t {
	UNUSED_CHANNEL,
	SENSE_RESISTOR_1000,
	THERMISTOR_44006,
	RTD_PT_100
};


class LTC2983Manager {
public:
	// constructor and destructor
	LTC2983Manager(int cs_pin, int rst_pin, uint8_t therm_sense_ch, uint8_t rtd_sense_ch);
	~LTC2983Manager(void) { }; // nothing to destruct

	// basic methods
	void InitializeAndConfigure(void);
	void Sleep(void);
	void WakeUp(void);
	void MeasureAllChannels(void);
	uint8_t CheckStatusReg(void); //used for debugging SPI
	uint32_t ReadFullChannelData(uint8_t channel_number); // used to debug channel errors
	float MeasureChannel(uint8_t channel_number);

	// non-blocking methods
	void StartMeasurement(uint8_t channel_number);
	bool FinishedMeasurement(void);
	float ReadMeasurementResult(uint8_t channel_number);
	void InterruptHandler(void);

    // Depreciate; new resetSpi function assumes port0
    // allow for selection of SPI port
    //void setSpi(uint8_t port_number);

    // Reset Spi settings
    void resetSpi();

    // Reconnect spi with proper settings
    void connect();

	// array containing hardware channel setup information
	Sensor_Type_t channel_assignments[21]; // index corresponds to channel, 0 is unused

	// array containing channel temperature results
	float channel_temperatures[21]; // index corresponds to channel, 0 is unused


private:
	void Configure();

	// typical sensor methods for Strat2
	void AssignSenseResistor(uint8_t channel_number); // value: 1000
	void AssignThermistor(uint8_t channel_number); // must be 44006 10K@25C
	void AssignRTD(uint8_t channel_number); // must be PT-100

	// board specific settings
	uint8_t _rtd_sense_channel;
	uint8_t _therm_sense_channel;
    uint8_t _spi;
	int _chip_select_pin;
	int _reset_pin;

	bool _sleeping;
	bool _measurement_finished;
};

#endif