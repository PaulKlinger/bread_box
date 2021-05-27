#include "config.h"
#include "sensor.h"
#include <avr/io.h>


#ifndef UI_H
#define	UI_H

#ifdef	__cplusplus
extern "C" {
#endif

    void setup_ui();
    void update_ui(struct sensor_reading reading, struct config *cfg);
    


#ifdef	__cplusplus
}
#endif

#endif	/* UI_H */

