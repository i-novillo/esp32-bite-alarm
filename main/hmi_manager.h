#ifndef HMI_MANAGER_H
#define HMI_MANAGER_H

#define SENSITIVITY_LEVELS 10

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

/**
 * @brief Sets up the HMI manager to handle physical user inputs.
 */
void setup_hmi_manager(void);

/**
 * @brief Starts the HMI manager task.
 */
void start_hmi_manager_task(void);

/**
 * @brief Getter for the sensitivity level configured from user input (to be used by bite detection modules).
 * @return Current sensitivity level
 */
sensitivity_level_t get_sensitivity(void);

#endif // HMI_MANAGER_H