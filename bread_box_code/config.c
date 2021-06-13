#include "config.h"

#include "avr/eeprom.h"

typedef struct { 
    struct config cfg;
    uint8_t checksum;
} StoredConfig;
EEMEM StoredConfig stored_config;

struct config default_config = {
        .thresh_shutter = 50,
        .thresh_fan_low = 60,
        .thresh_fan_high = 65,
        .hysteresis = 5
    };

uint8_t calc_checksum(struct config cfg) {
    // add an offset, so the checksum is not correct when EEPROM is all zeros
    return 42 + cfg.hysteresis + cfg.thresh_fan_high + cfg.thresh_fan_low + cfg.thresh_shutter;
}

void save_config(struct config conf) {
    // use update to save time if the values are same as the old ones
    // (read is >10x faster than write)
    StoredConfig new_store = {
        .cfg = conf,
        .checksum = calc_checksum(conf),
    };
    eeprom_update_block(&new_store, &stored_config, sizeof(new_store));
}

struct config load_config() {
    StoredConfig loaded_data;
    eeprom_read_block(&loaded_data, &stored_config, sizeof(loaded_data));
    
    if (calc_checksum(loaded_data.cfg) != loaded_data.checksum){
        save_config(default_config);
        return default_config;
    } 
    return loaded_data.cfg;
}