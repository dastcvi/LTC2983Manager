/*!



LTC2983_support_functions.h:
This file contains all the support function prototypes used in the main program.


http://www.linear.com/product/LTC2983

http://www.linear.com/product/LTC2983#demoboards

$Revision: 1.3.4 $
$Date: October 5, 2016 $
Copyright (c) 2014, Linear Technology Corp.(LTC)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Linear Technology Corp.

The Linear Technology Linduino is not affiliated with the official Arduino team.
However, the Linduino is only possible because of the Arduino team's commitment
to the open-source community.  Please, visit http://www.arduino.cc and
http://store.arduino.cc , and consider a purchase that will help fund their
ongoing work.
*/

#include <stdint.h>

////These definitions were pulled from LT_SPI.h
// Macros
//! Set "pin" low
//! @param pin pin to be driven LOW
#define output_low(pin)   digitalWrite(pin, LOW)
//! Set "pin" high
//! @param pin pin to be driven HIGH
#define output_high(pin)  digitalWrite(pin, HIGH)
//! Return the state of pin "pin"
//! @param pin pin to be read (HIGH or LOW).
//! @return the state of pin "pin"
#define input(pin)        digitalRead(pin)



//void print_title();
void assign_channel(uint8_t chip_select, uint8_t channel_number, uint32_t channel_assignment_data);
//void write_custom_table(uint8_t chip_select, struct table_coeffs coefficients[64], uint16_t start_address, uint8_t table_length);
//void write_custom_steinhart_hart(uint8_t chip_select, uint32_t steinhart_hart_coeffs[6], uint16_t start_address);

float measure_channel(uint8_t chip_select, uint8_t channel_number, uint8_t channel_output);
void convert_channel(uint8_t chip_select, uint8_t channel_number);
void wait_for_process_to_finish(uint8_t chip_select);

float get_result(uint8_t chip_select, uint8_t channel_number, uint8_t channel_output);
float print_conversion_result(uint32_t raw_conversion_result, uint8_t channel_output);
//void read_voltage_or_resistance_results(uint8_t chip_select, uint8_t channel_number);
void print_fault_data(uint8_t fault_byte);
void LTC_sleep(uint8_t chip_select);


uint32_t transfer_four_bytes(uint8_t chip_select, uint8_t read_or_write, uint16_t start_address, uint32_t input_data);
uint8_t transfer_byte(uint8_t chip_select, uint8_t read_or_write, uint16_t start_address, uint8_t input_data);

uint16_t get_start_address(uint16_t base_address, uint8_t channel_number);
bool is_number_in_array(uint8_t number, uint8_t *array, uint8_t array_length);

void setSpiSup(uint8_t spi_port);

//This function was pulled from LT_SPI.H//////
void spi_transfer_block(uint8_t chip_select,     //!< Chip select pin
                        uint8_t *tx,        //!< Byte array to be transmitted
                        uint8_t *rx,        //!< Byte array to be received
                        uint8_t length      //!< Length of array
                       );

