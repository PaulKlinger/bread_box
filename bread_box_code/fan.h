#ifndef FAN_H
#define	FAN_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    enum fan_state {
        FAN_OFF = 0,
        FAN_LOW = 2,
        FAN_HIGH = 4
    };
    

    void setup_fan();            
    void set_fan_low();
    void set_fan_high();
    void turn_off_fan();
    
    enum fan_state current_fan_state;


#ifdef	__cplusplus
}
#endif

#endif	/* FAN_H */

