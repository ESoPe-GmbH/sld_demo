#include <stdio.h>
#include "sdkconfig.h"

#if CONFIG_IDF_TARGET_ESP32S3 && CONFIG_SLD_C_W_S3_BT817 && !KERNEL_USES_SLINT && !KERNEL_USES_LVGL

#include "board/board.h"
#include "module/version/version.h"
#include "module/gui/eve_ui/screen.h"
#include "module/gui/eve_ui/button.h"
#include "module/gui/eve_ui/image.h"
#include "module/gui/eve_ui/text.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal definitions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

struct screen_main_s
{
    /// Counter that is incremented with a button.
    uint32_t counter;
    /// Runtime counter, that is incremented every second
    uint32_t runtime_seconds;

    char str_counter[16];

    char str_runtime[16];

    char str_display[16];

    char str_resolution[30];

    char str_version[16];

    text_t text_counter_title;

    text_t text_counter_value;

    text_t text_runtime_title;

    text_t text_runtime_value;

    text_t text_display;

    text_t text_resolution;

    text_t text_title;

    text_t text_version;

    button_t button_increment;

    button_t button_image;

    button_t button_info;

    image_t image_logo;

    image_t image_button_image;

    image_t image_button_info;
};

struct screen_image_s
{
    /// Image that is shown on the screen
    image_t image;
    /// Back button to go back to the main screen
    button_t button_back;
};

struct screen_info_s
{
    /// QR code that is shown on the screen
    image_t image_qr_code;
    /// Back button to go back to the main screen
    button_t button_back;

    text_t text_title;

    text_t text_subtitle;

    text_t text_powered_by;

    image_t image_powered_by;
};

typedef struct screen_data_s
{
    /// @brief Data for main screen
    struct screen_main_s main; 
    /// @brief Data for image screen
    struct screen_image_s image;
    /// @brief Data for info screen
    struct screen_info_s info;
    
}screen_data_t;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal structures and enums
//-----------------------------------------------------------------------------------------------------------------------------------------------------------
typedef enum
{
    /// @brief The main screen, shows a counter, runtime and buttons to enter the other screens.
    LCD_ACTIVE_SCREEN_MAIN,

    /// @brief The image screen shows an image with a back button
    LCD_ACTIVE_SCREEN_IMAGE,

    /// @brief The info screen shows a qr code and an info text
    LCD_ACTIVE_SCREEN_INFO,

    /// @brief Limiter of the enum
    LCD_ACTIVE_SCREEN_MAX
}LCD_ACTIVE_SCREEN_T;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Prototypes
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/**
 * @brief Create the initial image that is drawn
 */
static void _ui_init(void);
/**
 * @brief Create a screen based on the enum value.
 * 
 * @param screen    Screen that should be created.
 */
static void _show_screen(LCD_ACTIVE_SCREEN_T screen);
/**
 * @brief Create the screen according to @c LCD_ACTIVE_SCREEN_MAIN 
 */
static void _create_screen_main(void);
/**
 * @brief Create the screen according to @c LCD_ACTIVE_SCREEN_IMAGE 
 */
static void _create_screen_image(void);
/**
 * @brief Create the screen according to @c LCD_ACTIVE_SCREEN_INFO 
 */
static void _create_screen_info(void);
/**
 * @brief Default button handle to show a screen. The user data of the event is a casted @c LCD_ACTIVE_SCREEN_T value that should be shown.
 * 
 * @param e     Pointer to the event data
 */
static void _button_handler(button_t* e);
/**
 * @brief Button used to increment the counter that is shown.
 * 
 * @param e     Pointer to the event data
 */
static void _button_increment_handler(button_t* e);
/**
 * @brief Callback set on a 1s timer thath increments the runtime
 * 
 * @param tmr           Timer that was triggered
 */
static int _timer_runtime_handle(struct pt* pt);

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal variables
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

/// Currently shown screen.
static LCD_ACTIVE_SCREEN_T _active_screen = LCD_ACTIVE_SCREEN_MAX;

static screen_t _screens[LCD_ACTIVE_SCREEN_MAX] = {0};

static system_task_t _task_runtime = {0};

static screen_data_t* _screen_data = NULL;

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// External Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

bool app_ui_init(void)
{
    if(board_screen_device.eve.status != EVE_STATUS_OK)
    {
        DBG_ERROR("Invalid display handle\n");
        return false;
    }

    _screen_data = mcu_heap_calloc(1, sizeof(screen_data_t));
    if(_screen_data == NULL)
    {
        DBG_ERROR("No memory for screen data\n");
        return false;
    }

    system_task_init_protothread(&_task_runtime, true, _timer_runtime_handle, _screen_data);

    // TODO: Initialize the screen device

    _ui_init();

    return true;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
// Internal Functions
//-----------------------------------------------------------------------------------------------------------------------------------------------------------

static void _ui_init(void)
{
    _create_screen_main();
    _create_screen_info();
    _create_screen_image();

    _show_screen(LCD_ACTIVE_SCREEN_MAIN);

    screen_device_on(&board_screen_device);
    screen_device_set_dimming(&board_screen_device, 60);
}

static void _show_screen(LCD_ACTIVE_SCREEN_T screen)
{
    if(_active_screen != screen)
    {
        DBG_INFO("Show %d\n", screen);
        screen_paint(&_screens[screen], 0);
        _active_screen = screen;
    }
}

static void _create_screen_main(void)
{
    struct screen_main_s* data = &_screen_data->main;
    screen_t* scr = &_screens[LCD_ACTIVE_SCREEN_MAIN];

    uint32_t w = screen_device_get_width(&board_screen_device);
    uint32_t h = screen_device_get_height(&board_screen_device);

    screen_init_object(scr, color_get(COLOR_WHITE), NULL, NULL);

    text_init(&data->text_display, w - 5, 5, data->str_display);
    string_nprintf(data->str_display, sizeof(data->str_display), "Display: %s\"", board_screen_device.eve.sld_edid.screen_diagonal);
    text_set_horizontal_alignment(&data->text_display, TEXT_H_ALIGNMENT_RIGHT);
    screen_add_component(scr, &data->text_display.component);
    // TODO: Create the screen
    // // Clean the screen
    // lv_obj_t* scr = lv_screen_active();
    // lv_obj_clean(scr);
    // lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // // Create the logo at the top left
    // lv_obj_t* image_logo = lv_img_create(scr);
    // lv_obj_align(image_logo, LV_ALIGN_TOP_LEFT, 5, 5);
    // lv_img_set_src(image_logo, &schukat_logo);
    // // Create the display information on the top right
    // lv_obj_t* label_display_size = lv_label_create(scr);
    // lv_label_set_text_fmt(label_display_size, "Display: %s\"", board_lcd->screen_diagonal);
    // lv_obj_set_style_text_font(label_display_size, &lv_font_montserrat_10, LV_STATE_DEFAULT);
    // lv_obj_align(label_display_size, LV_ALIGN_TOP_RIGHT, -5, 5);
    // lv_obj_t* label_display_resolution = lv_label_create(scr);
    // lv_label_set_text_fmt(label_display_resolution, "Resolution: %d x %d", (int)display_device_get_width(board_lcd->display), (int)display_device_get_height(board_lcd->display));
    // lv_obj_set_style_text_font(label_display_resolution, &lv_font_montserrat_10, LV_STATE_DEFAULT);
    // lv_obj_align(label_display_resolution, LV_ALIGN_TOP_RIGHT, -5, 20);
    // // Create the title in the middle
    // lv_obj_t* label = lv_label_create(scr);
    // lv_label_set_text(label, "LVGL Demo");
    // lv_obj_set_style_text_font(label, &lv_font_montserrat_24, LV_STATE_DEFAULT);
    // lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 80);
    // // Create the runtime labels
    // lv_obj_t* label_runtime_title = lv_label_create(scr);
    // lv_label_set_text(label_runtime_title, "Runtime:");
    // lv_obj_align(label_runtime_title, LV_ALIGN_LEFT_MID, 10, -10);
    // _lbl_runtime = lv_label_create(scr);
    // lv_label_set_text_fmt(_lbl_runtime, "%02u:%02u min", (unsigned int)(_runtime_seconds / 60), (unsigned int)(_runtime_seconds % 60));
    // lv_obj_align(_lbl_runtime, LV_ALIGN_RIGHT_MID, -10, -10);
    // // Create the counter labels
    // lv_obj_t* label_counter_title = lv_label_create(scr);
    // lv_label_set_text(label_counter_title, "Counter:");
    // lv_obj_align(label_counter_title, LV_ALIGN_LEFT_MID, 10, 10);
    // _lbl_counter = lv_label_create(scr);
    // lv_label_set_text_fmt(_lbl_counter, "%u", (unsigned int)_counter);
    // lv_obj_align(_lbl_counter, LV_ALIGN_RIGHT_MID, -10, 10);
    // // Create the buttons
    // lv_obj_t* buttons[3] = {0};
    // // Create the button that increments the counter
    // buttons[0] = _create_button(scr);
    // lv_obj_add_event_cb(buttons[0], _button_increment_handler, LV_EVENT_CLICKED, NULL);
    // lv_obj_align(buttons[0], LV_ALIGN_BOTTOM_LEFT, 5, -35);
    // lv_obj_t* button_increment_label = lv_label_create(buttons[0]);
    // lv_label_set_text(button_increment_label, "+");
    // lv_obj_set_style_text_color(button_increment_label, lv_color_hex(0x000000), LV_PART_MAIN);
    // lv_obj_center(button_increment_label);
    // // Create the button that switches to LCD_ACTIVE_SCREEN_IMAGE
    // buttons[1] = _create_button(scr);
    // lv_obj_add_event_cb(buttons[1], _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_IMAGE);
    // lv_obj_align(buttons[1], LV_ALIGN_BOTTOM_MID, 0, -35);
    // lv_obj_remove_flag(buttons[1], LV_OBJ_FLAG_PRESS_LOCK);
    // lv_obj_t* image_button_image = lv_img_create(buttons[1]);
    // lv_obj_center(image_button_image);
    // lv_img_set_src(image_button_image, &button_landscape);
    // // Create the button that switches to LCD_ACTIVE_SCREEN_INFO
    // buttons[2] = _create_button(scr);
    // lv_obj_add_event_cb(buttons[2], _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_INFO);
    // lv_obj_align(buttons[2], LV_ALIGN_BOTTOM_RIGHT, -5, -35);
    // lv_obj_remove_flag(buttons[2], LV_OBJ_FLAG_PRESS_LOCK);
    // lv_obj_t* image_button_info = lv_img_create(buttons[2]);
    // lv_obj_center(image_button_info);
    // lv_img_set_src(image_button_info, &esope);
    // // Make all three buttons the same size
    // for(int i = 0; i < 3; i++)
    // {
    //     lv_obj_set_width(buttons[i], display_device_get_width(board_lcd->display) / 4);
    //     lv_obj_set_height(buttons[i], 35);
    // }
    // // Create the version label
    // lv_obj_t* label_version = lv_label_create(scr);
    // lv_label_set_text_fmt(label_version, "Version: %s", version_get_string());
    // lv_obj_align(label_version, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
}

static void _create_screen_image(void)
{
    // TODO: Create the screen
    // // Clean the screen
    // lv_obj_t* scr = lv_screen_active();
    // lv_obj_clean(scr);
    // lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // // Image shown in the center
    // lv_obj_t* image = lv_img_create(scr);
    // lv_obj_center(image);
    // lv_img_set_src(image, &landscape);
    // // Button for back
    // lv_obj_t* button_back = _create_button(scr);
    // lv_obj_add_event_cb(button_back, _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_MAIN);
    // lv_obj_align(button_back, LV_ALIGN_TOP_LEFT, 5, 5);
    // lv_obj_remove_flag(button_back, LV_OBJ_FLAG_PRESS_LOCK);
    // // Label for the back button
    // lv_obj_t* button_label = lv_label_create(button_back);
    // lv_label_set_text(button_label, "<");
    // lv_obj_set_style_text_color(button_label, lv_color_hex(0x000000), LV_PART_MAIN);
    // lv_obj_center(button_label);
}

static void _create_screen_info(void)
{
    // TODO: Create the screen
    // // Clean the screen
    // lv_obj_t* scr = lv_screen_active();
    // lv_obj_clean(scr);
    // lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    // // Button for back
    // lv_obj_t* button_back = _create_button(scr);
    // lv_obj_add_event_cb(button_back, _button_handler, LV_EVENT_CLICKED, (void*)LCD_ACTIVE_SCREEN_MAIN);
    // lv_obj_align(button_back, LV_ALIGN_TOP_LEFT, 5, 5);
    // lv_obj_remove_flag(button_back, LV_OBJ_FLAG_PRESS_LOCK);
    // // Label for the back button
    // lv_obj_t* button_label = lv_label_create(button_back);
    // lv_label_set_text(button_label, "<");
    // lv_obj_set_style_text_color(button_label, lv_color_hex(0x000000), LV_PART_MAIN);
    // lv_obj_center(button_label);
    // // Show the header and make it wrap
    // lv_obj_t* label = lv_label_create(scr);
    // lv_label_set_text(label, "Demo Software and Description");
    // lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    // lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    // lv_obj_set_width(label, display_device_get_width(board_lcd->display) - 10);
    // lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 20);
    // // Show the description and make it wrap
    // lv_obj_t* label2 = lv_label_create(scr);
    // lv_label_set_text(label2, "Scan the QR-Code for the GitHub Link to this Demo.");
    // lv_label_set_long_mode(label2, LV_LABEL_LONG_WRAP);
    // lv_obj_set_style_text_align(label2, LV_TEXT_ALIGN_CENTER, 0);
    // lv_obj_set_width(label2, display_device_get_width(board_lcd->display) - 10);
    // lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 40);
    // // Show the QR-Code
    // lv_obj_t* image = lv_img_create(scr);
    // lv_img_set_src(image, &qr_sld_demo);
    // lv_obj_align(image, LV_ALIGN_BOTTOM_MID, 0, -40);
    // // Show powered by ESoPe
    // lv_obj_t* label_powered_by = lv_label_create(scr);
    // lv_label_set_text(label_powered_by, "powered by");
    // lv_obj_align(label_powered_by, LV_ALIGN_BOTTOM_RIGHT, -75, -5);
    // // Create the image of ESoPe
    // lv_obj_t* image_esope = lv_img_create(scr);
    // lv_obj_align(image_esope, LV_ALIGN_BOTTOM_RIGHT, -5, -5);
    // lv_img_set_src(image_esope, &esope);
}

static void _button_handler(button_t* e)
{
    LCD_ACTIVE_SCREEN_T screen = (LCD_ACTIVE_SCREEN_T)e->component.user;
    _show_screen(screen);
}

static void _button_increment_handler(button_t* e)
{
    screen_t* screen = (screen_t*)screen_get_from_component(&e->component);
    screen_data_t* data = (screen_data_t*)screen->user;

    data->main.counter++;
    string_nprintf(data->main.str_counter, sizeof(data->main.str_counter), "%u", data->main.counter);
    screen_repaint_by_component(&screen->component);
}

static int _timer_runtime_handle(struct pt* pt)
{
    screen_data_t* data = pt->obj;
    PT_BEGIN(pt);
    while(true)
    {
        PT_YIELD_MS(pt, 1000);
        data->main.runtime_seconds++;
        string_nprintf(data->main.str_runtime, sizeof(data->main.str_runtime), "%02u:%02u min", (unsigned int)(data->main.runtime_seconds / 60), (unsigned int)(data->main.runtime_seconds % 60));
        // Only repaint the screen if it is the main screen
        if(_active_screen == LCD_ACTIVE_SCREEN_MAIN)
        {
            screen_repaint(&board_screen_device);
        }
    }
    PT_END(pt);
}

#endif