#include <avr/io.h>

#ifndef CONFIG_H
#define	CONFIG_H

#ifdef	__cplusplus
extern "C" {
#endif

#define F_CPU 16000000

struct config {
    uint8_t thresh_shutter, thresh_fan_low, thresh_fan_high, hysteresis;
};

#define SENSOR_ADDR 0x44

#ifdef	__cplusplus
}
#endif

#endif	/* CONFIG_H */

