/**
 * 	@file 	board.h
 * 	@copyright Urheberrecht 2018-2023 ESoPe GmbH, Alle Rechte vorbehalten
 *
 * 	@author Tim Koczwara
 *
 *  @brief	Offers one function, that initializes all periphery that is assigned to the board inside board.c and board_specific.h.
 *
 *  @version  	1.02 (16.02.2013)
 *		- Converted comments in english.
 *  @version  	1.01 (03.04.2011)
 *		- board.h splitted to board_specific.h and board.h.
 *		- board.h contains only the board_init function.
 *  @version  	1.00
 *  	- Initial release
 *
 ******************************************************************************/
#ifndef BOARD_HEADER_FIRST_INCLUDE_GUARD
#define BOARD_HEADER_FIRST_INCLUDE_GUARD


#include "module_public.h"
#include "module_include_public.h"
#include "module/comm/i2c/i2c.h"
#include "module/display/sld/display_sld.h"
#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817
#include "module/gui/eve/eve.h"
#include "module/gui/eve_ui/screen.h"
#endif

/// @brief Handle for SLD connection. Contains the display, the touch and the pwm handle (Backlight).
extern display_sld_handle_t board_lcd;
/// @brief Handle for the uart that can be used for external communication.
extern mcu_uart_t board_uart_peripheral;
/// @brief Comm handle for the peripheral communication, using board_uart_peripheral.
extern comm_t board_comm_peripheral;

#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817

extern screen_device_t board_screen_device;

#endif


/**
 * @brief 		Initializes the mcu and all periphery assigned to the used board. Is automatically called in sys.c at first step of the main.
 */
void board_init(void);
/**
 * @brief       Sets the backlight brightness in percent.
 * 
 * @param pwm   Valid values 0.0 to 100.0 for the brightness of the backlight.
 */
void board_set_backlight(float pwm);

#endif
