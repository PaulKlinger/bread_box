#include "twi.h"
#include "sensor.h"
#include <util/delay.h>


struct sensor_reading read_sensor(uint8_t addr) {
    // start write transaction
    TWI0_start_write(addr);
    TWI0_write(0x24);
    TWI0_write(0x00);
    TWI0_stop();
    
    // wait for measurement to complete
    //(max time needed for high repeatability should be 30ms)
    _delay_ms(50);
    
    TWI0_start_read(addr);
    uint8_t temp_msb = TWI0_read(1);
    uint8_t temp_lsb = TWI0_read(1);
    uint8_t temp_crc = TWI0_read(1);
    
    uint8_t hum_msb = TWI0_read(1);
    uint8_t hum_lsb = TWI0_read(1);
    uint8_t hum_crc = TWI0_read(0);
    TWI0_stop();
    
    uint16_t temp_int = (temp_msb<<8) | temp_lsb;
    uint16_t hum_int = (hum_msb<<8) | hum_lsb;
    
    struct sensor_reading result;
    
    result.temperature = -45.0 + (175.0 / 65535.0) * temp_int;
    result.humidity = (100.0 / 65535.0) * hum_int;
    
    return result;
}
