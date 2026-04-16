#ifndef BUZZER_MANAGER_H
#define BUZZER_MANAGER_H

typedef enum {
    BUZZER_OFF = 0,
    BUZZER_ON_POSSIBLE_BITE = 1,
    BUZZER_ON_ALARM = 2
} buzzer_status_t;

void setup_buzzer_manager(void);

void start_buzzer_task(void);

void buzzer_set_state(buzzer_status_t state);

#endif // BUZZER_MANAGER_H