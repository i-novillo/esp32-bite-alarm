#ifndef HMI_MANAGER_H
#define HMI_MANAGER_H

#define SENSITIVITY_LEVELS 10

//TODO: Add comments
typedef enum {
    SENS_LEVEL_1 = 0,
    SENS_LEVEL_2 = 1,
    SENS_LEVEL_3 = 2,
    SENS_LEVEL_4 = 3,
    SENS_LEVEL_5 = 4,
    SENS_LEVEL_6 = 5,
    SENS_LEVEL_7 = 6,
    SENS_LEVEL_8 = 7,
    SENS_LEVEL_9 = 8,
    SENS_LEVEL_10 = 9
} sensitivity_level_t;

void setup_hmi_manager(void);

void start_hmi_manager_task(void);

sensitivity_level_t get_sensitivity(void);

#endif // HMI_MANAGER_H