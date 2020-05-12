/*
 *  LTC2983Manager.cpp
 *  Implementation of a generalized class to manage the LTC2983 temperature chip
 *  Author: Alex St. Clair
 *  April 2018
 *
 *  This is a general class meant for use by multiple boards for the LASP Strateole 2
 *  payloads and also future projects to configure and control the LTC2983 temperature
 *  chip.
 *
 *  To use:
 *    0) Instantiate an object of the class. Assign the chip select and reset pins
 *       based on the board design. If there is a sense resistor for thermistors and/or
 *       one for RTDs, assign the channels based on board design. If there isn't a sense
 *       resistor for either of those sensor types, assign that resistor to channel 0,
 *       which specifies that it doesn't exist.
 *    1) Create sensor assignments in the channel_assignments[20] array. The index
 *       corresponds to the channel number, from 1 to 20 (index 0 is unused). Only the
 *       sensors in the Sensor_Type_t enum are supported, but it is easy to add
 *       sensors to this enum.
 *    2) Call Initialize(), this will assign channels based on channel_assignments
 *    3) Read sensors by calling MeasureAllChannels() or MeasureChannel(uint8_t)
 *    4) Results for valid, requested channels will be in channel_temperatures[20]
 *
 *  Note: this class does not error check sensor configurations, ie. will not catch if a
 *        channel that is assigned to one sensor is then needed for differential input.
 
 September 2018, Updated by Marika Schubert to allow 
 selection of SPI port 
 
 */

#include "LTC2983Manager.h"

LTC2983Manager::LTC2983Manager(int cs_pin, int rst_pin, uint8_t therm_sense_ch, uint8_t rtd_sense_ch) {
	_chip_select_pin = cs_pin;
	_reset_pin = rst_pin;
	_sleeping = false;
	_measurement_finished = false;
    _spi = 0;
    setSpiSup(_spi);

	// initialize all channels to unused, and all temperature results to error values
	uint8_t channel;
	for (channel = 0; channel < 21; channel++) {
		channel_assignments[channel] = UNUSED_CHANNEL;
		channel_temperatures[channel] = TEMPERATURE_ERROR;
	}

	// if there's a thermistor sense resistor, assign it
	if (therm_sense_ch > 0 && therm_sense_ch < 21) {
		_therm_sense_channel = therm_sense_ch;
		channel_assignments[_therm_sense_channel] = SENSE_RESISTOR_1000;
	} else {
		_therm_sense_channel = 0;
	}

	// if there's an rtd sense resistor, assign it
	if (rtd_sense_ch > 0 && rtd_sense_ch < 21) {
		_rtd_sense_channel = rtd_sense_ch;
		channel_assignments[_rtd_sense_channel] = SENSE_RESISTOR_1000;
	} else {
		_rtd_sense_channel = 0;
	}
}

void LTC2983Manager::InitializeAndConfigure(void) {
	// GPIO setup
	pinMode(_chip_select_pin, OUTPUT);
	pinMode(_reset_pin, OUTPUT);
	digitalWrite(_reset_pin, HIGH);
	delay(100);

	// start SPI
    if(_spi == 0){
        SPI.begin();
        SPI.setClockDivider(SPI_CLOCK_DIV128);
    }  else if (_spi == 1){
        SPI1.begin();
        SPI1.setClockDivider(SPI_CLOCK_DIV128);
    } else if (_spi == 2){
        SPI2.begin();
        SPI2.setClockDivider(SPI_CLOCK_DIV128);
    }

	delay(100);

	Configure();
}

// TODO: sleep after measurements?
void LTC2983Manager::Sleep(void) {
	transfer_byte(_chip_select_pin, WRITE_TO_RAM, COMMAND_STATUS_REGISTER, SLEEP_BYTE);
	_sleeping = true;
}

void LTC2983Manager::WakeUp(void) {
	digitalWrite(_reset_pin, LOW);
	delay(100);
	digitalWrite(_reset_pin, HIGH);
	delay(200);
	_sleeping = false;

	Configure();
}

uint8_t LTC2983Manager::CheckStatusReg(void) {
	return transfer_byte(_chip_select_pin, READ_FROM_RAM, 0x0000,0);
}

uint32_t LTC2983Manager::ReadFullChannelData(uint8_t channel_number) {
	convert_channel(_chip_select_pin, channel_number);
	uint16_t start_address = get_start_address(CONVERSION_RESULT_MEMORY_BASE,channel_number);
	return transfer_four_bytes(_chip_select_pin,READ_FROM_RAM,start_address,0);
}

void LTC2983Manager::MeasureAllChannels(void) {
	uint8_t channel;

	for (channel = 1; channel < 21; channel++) {
		MeasureChannel(channel);
	}
}

float LTC2983Manager::MeasureChannel(uint8_t channel_number) {
	if (_sleeping) WakeUp();
	Sensor_Type_t assignment = channel_assignments[channel_number];
	float temp = TEMPERATURE_ERROR;

	if (assignment == THERMISTOR_44006 || assignment == RTD_PT_100) {
		temp =  measure_channel(_chip_select_pin, channel_number, TEMPERATURE);
	}

	channel_temperatures[channel_number] = temp;
	return temp;
}

// non-blocking methods -------------------------------------------------------
void LTC2983Manager::StartMeasurement(uint8_t channel_number)
{
	_measurement_finished = false;
	transfer_byte(_chip_select_pin, WRITE_TO_RAM, COMMAND_STATUS_REGISTER, CONVERSION_CONTROL_BYTE | channel_number);
}

bool LTC2983Manager::FinishedMeasurement(void)
{
	uint8_t status_byte = 0;
	
	// get the status byte
	status_byte = transfer_byte(_chip_select_pin, READ_FROM_RAM, COMMAND_STATUS_REGISTER, 0);
	
	// if bit 6 is set, the measurement is finished
	return (status_byte & 0x40);
}

float LTC2983Manager::ReadMeasurementResult(uint8_t channel_number)
{
	_measurement_finished = false; // reset the flag

	// verify that the device is ready to read
	if (!FinishedMeasurement()) return TEMPERATURE_ERROR;

	return get_result(_chip_select_pin, channel_number, TEMPERATURE);
}

void LTC2983Manager::InterruptHandler(void)
{
	_measurement_finished = true;
}

// old methods ----------------------------------------------------------------
// Depreciate; new resetSpi function assumes port0
/* void LTC2983Manager::setSpi(uint8_t port_number){
    if (port_number <= 2){
        _spi = port_number;
        setSpiSup(port_number);
    }
} */

/* this function doesn't compile: SPI_DISABLE undefined */
// void LTC2983Manager::resetSpi(){
//     SPI0_SR |= SPI_DISABLE;
//     SPI0_CTAR0 = 0x38004005;
//     SPI0_SR &= ~(SPI_DISABLE);
// }

void LTC2983Manager::connect(){
    SPI.setCS(_chip_select_pin);
}

// Private methods ------------------------------------------------------------
void LTC2983Manager::Configure(void) {// chip configurations
	if (_sleeping) WakeUp();
	transfer_byte(_chip_select_pin, WRITE_TO_RAM, 0xF0, TEMP_UNIT__C | REJECTION__50_60_HZ);
	transfer_byte(_chip_select_pin, WRITE_TO_RAM, 0xFF, 0); // conversion delay = 0 us

	// channel configuration
	uint8_t channel;
	for (channel = 1; channel < 21; channel++) {
		switch (channel_assignments[channel]) {
		case UNUSED_CHANNEL:
			break; // nothing to do
		case SENSE_RESISTOR_1000:
			AssignSenseResistor(channel);
			break;
		case THERMISTOR_44006:
			AssignThermistor(channel);
			break;
		case RTD_PT_100:
			AssignRTD(channel);
			break;
		default:
			Serial.println("LTC2983Manager error: unknown channel assignment");
			break;
		}
	}
}

void LTC2983Manager::AssignSenseResistor(uint8_t channel_number) {
	uint32_t channel_assignment_data;

	channel_assignment_data = 
		SENSOR_TYPE__SENSE_RESISTOR | 
		SENSE_RESISTOR_1K;   // sense resistor - value: 1000.

	assign_channel(_chip_select_pin, channel_number, channel_assignment_data);
}

void LTC2983Manager::AssignThermistor(uint8_t channel_number) {
	uint32_t channel_assignment_data;

	channel_assignment_data = 
		SENSOR_TYPE__THERMISTOR_44006_10K_25C |
		(_therm_sense_channel << THERMISTOR_RSENSE_CHANNEL_LSB) |
		THERMISTOR_DIFFERENTIAL |
		THERMISTOR_EXCITATION_MODE__SHARING_NO_ROTATION |
		THERMISTOR_EXCITATION_CURRENT__AUTORANGE;

	assign_channel(_chip_select_pin, channel_number, channel_assignment_data);
}

void LTC2983Manager::AssignRTD(uint8_t channel_number) {
	uint32_t channel_assignment_data;

	channel_assignment_data = 
		SENSOR_TYPE__RTD_PT_100 |
		(_rtd_sense_channel << RTD_RSENSE_CHANNEL_LSB) |
		RTD_NUM_WIRES__2_WIRE |
		RTD_EXCITATION_MODE__NO_ROTATION_SHARING |
		RTD_EXCITATION_CURRENT__50UA |
		RTD_STANDARD__AMERICAN;

	assign_channel(_chip_select_pin, channel_number, channel_assignment_data);
}