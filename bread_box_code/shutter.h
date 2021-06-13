#ifndef SHUTTER_H
#define	SHUTTER_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <stdbool.h>
    bool shutter_open;

    void setup_shutter();
    void open_shutter();
    void close_shutter();


#ifdef	__cplusplus
}
#endif

#endif	/* SHUTTER_H */

