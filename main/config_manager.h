#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#define SENSITIVITY_LEVELS 5

//TODO: Add comments

typedef enum {
    SENS_LEVEL_0 = 0,
    SENS_LEVEL_1 = 1,
    SENS_LEVEL_2 = 2,
    SENS_LEVEL_3 = 3,
    SENS_LEVEL_4 = 4
} sensitivity_level_t;

void config_manager_set_sensitivity(sensitivity_level_t level);
sensitivity_level_t config_manager_get_sensitivity(void);

#endif // CONFIG_MANAGER_H