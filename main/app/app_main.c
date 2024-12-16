/**
 * @file app_main.c
 * @copyright Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten
 */
#include "module/comm/dbg.h"
#include "module/version/version.h"
#include "module/console/dbg/debug_console.h"
#include "module/flash_info/flash_info.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Initialize the graphical framework and start an internal task for handling the display.
 * 
 * @return true     Initialization was successfull and UI is shown to the user.
 * @return false    Initialization failed.
 */
extern bool app_ui_init(void);
#if defined(KERNEL_USES_SLINT)
/**
 * @brief Stops the UI task
 */
extern void app_ui_stop(void);
#endif
/**
 * @brief Callback function that is called upon "test start" to disable the logic of the application.
 * 
 * @param obj           Custom object pointer from debug_console_test_t.
 * @param data          Pointer to the console.
 * @param args          List of arguments.
 * @param args_len      Number of arguments.
 */
static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Handler for test start.
static debug_console_test_t _dbc_test;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

void app_main_init(void)
{
    version_set(24002, 2);

    DBG_INFO("Init SLD_Demo (Version %s Serial %u [" __DATE__ " " __TIME__ "]). Urheberrecht 2018-2024 ESoPe GmbH, Alle Rechte vorbehalten\n", version_get_string(), flash_info_get_hardware_id());

    app_ui_init();

    debug_console_register_test_callback(&_dbc_test, NULL, _dbc_test_handle);
}

#if SYSTEM_ENABLE_APP_MAIN_HANDLE
void app_main_handle(void)
{
}
#endif
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _dbc_test_handle(void* obj, console_data_t* data, char** args, uint8_t args_len)
{
	DBG_INFO("Enter testmode\n");
#if defined(KERNEL_USES_SLINT)
    app_ui_stop();
#endif
    // Reset all I/O to use them as GPIO in test
    for(int i = 1; i < 22; i++)
    {
        mcu_io_reset(i);
    }
    for(int i = 39; i < 43; i++)
    {
        mcu_io_reset(i);
    }
    for(int i = 45; i < 49; i++)
    {
        mcu_io_reset(i);
    }
}