/*!

LTC2983_table_coeffs.h:
Used for custom tables.


http://www.linear.com/product/LTC2983

http://www.linear.com/product/LTC2983#demoboards

$Revision: 1.2.3 $
$Date: July 5, 2016 $
Copyright (c) 2014, Linear Technology Corp.(LTC)
All rights reserved.

*/


#include <stdint.h>


struct table_coeffs 
{
  uint32_t measurement;
  uint32_t temperature;
};


struct channel_table 
{
  uint16_t channel;
  uint8_t is_a_temperature_measurement;
};

