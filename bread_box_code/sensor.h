#ifndef SENSOR_H
#define	SENSOR_H

#ifdef	__cplusplus
extern "C" {
#endif

struct sensor_reading {
    float temperature;
    float humidity;
    // add status enum
};

struct sensor_reading read_sensor(uint8_t addr);


#ifdef	__cplusplus
}
#endif

#endif	/* SENSOR_H */

