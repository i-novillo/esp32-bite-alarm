#include "config_manager.h"

static volatile sensitivity_level_t g_sensitivity = SENS_LEVEL_3;

void config_manager_set_sensitivity(sensitivity_level_t level)
{
    g_sensitivity = level;
}

sensitivity_level_t config_manager_get_sensitivity(void)
{
    return g_sensitivity;
}