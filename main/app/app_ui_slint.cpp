
#if defined(KERNEL_USES_SLINT)

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <esp_err.h>
#include <slint-esp.h>

#include "slint_string.h"
#include <slint_timer.h>

#include "appwindow.h"

using namespace std::chrono_literals;

#if CONFIG_IDF_TARGET_ESP32P4
slint::Image _get_image_image();
slint::Image _get_camera_image();
#endif

extern "C"
{
    #include "board/board.h"
    #include "esp_lcd_panel_ops.h"
    #include "module/version/version.h"
    #include "mcu/sys.h"
    #include "module/comm/dbg.h"
    #include "module/lcd_touch/lcd_touch_esp32.h"
    #include "app_camera.h"
#if CONFIG_IDF_TARGET_ESP32P4
    #include "driver/ppa.h"
    #include "esp_partition.h"
#endif

    static void _task_window(void* param);
    
#if CONFIG_IDF_TARGET_ESP32P4

    static void _camera_capture_cb(camera_buffer_t* buffer);
#endif

    /// @brief Handle of the UI Task @c _task_window
    TaskHandle_t _task_handle_ui = NULL;

#if CONFIG_IDF_TARGET_ESP32P4
    static camera_buffer_t _buffer = 
    {
        .user = NULL,
        .buffer = NULL,
        .width = 0,
        .height = 0,
        .bytes_per_pixel = 3,
        .f = _camera_capture_cb,
    };

    static void* _image_buffer = NULL;

    static ppa_client_handle_t _ppa_handle = NULL;
#endif
}

static std::vector<slint::Rgb8Pixel>* buffer_rgb8;
static std::vector<slint::platform::Rgb565Pixel>* buffer_rgb565;
// static std::vector<slint::platform::Rgb565Pixel>* buffer2_rgb565;

extern "C" bool app_ui_init(void)
{
    if(board_lcd == NULL || board_lcd->display == NULL)
    {
        DBG_ERROR("No display connected\n");
        return false;
    }

    uint32_t width = display_device_get_width(board_lcd->display);
    uint32_t height = display_device_get_height(board_lcd->display);

#if CONFIG_IDF_TARGET_ESP32P4
    _buffer.width = display_device_get_width(board_lcd->display);
    _buffer.height = display_device_get_height(board_lcd->display);
    _buffer.buffer = mcu_heap_calloc(_buffer.width * _buffer.height, _buffer.bytes_per_pixel);

    _image_buffer = mcu_heap_calloc(width * height, 3);

    // ppa_client_config_t ppa_config = 
    // {
    //     .oper_type = PPA_OPERATION_SRM,
    //     .max_pending_trans_num = 1,
    //     .data_burst_length = PPA_DATA_BURST_LENGTH_16
    // };

    // ppa_register_client(&ppa_config, &_ppa_handle);
    // if (_ppa_handle == NULL) 
    // {
    //     DBG_ERROR("Cannot register ppa client\n");
    //     return false;
    // }
#endif

    DBG_INFO("Initialize %d x %d\n", width, height);

    // Print current information about the panel rotation
    bool swap_xy, mirror_x, mirror_y;
    display_device_get_swap_xy(board_lcd->display, &swap_xy);
    display_device_get_mirror(board_lcd->display, &mirror_x, &mirror_y);
    DBG_INFO("Display: Swap=%d MirrorX=%d MirrorY=%d\n", swap_xy, mirror_x, mirror_y);
    

    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_touch_handle_t touch_handle = NULL;
    // Create the esp panel used in slint
    display_get_esp_panel_handle(board_lcd->display, &panel_handle);

    if(board_lcd->touch)
    {
        // Print current touch flags
        struct lcd_touch_flags_s flags;
        lcd_touch_get_flags(board_lcd->touch, &flags);
        DBG_INFO("Touch: Swap=%d MirrorX=%d MirrorY=%d\n", flags.swap_xy, flags.mirror_x, flags.mirror_y);

        // Create the Touch handle to use in slint
        lcd_touch_esp32_create(board_lcd->touch, &touch_handle);
    }

    if(board_lcd->data_width == 24)
    {
        // Allocate a drawing buffer
        buffer_rgb8 = new std::vector<slint::Rgb8Pixel>(width * height);
        // Initialize Slint's ESP platform support
        slint_esp_init(SlintPlatformConfiguration<slint::Rgb8Pixel>{
            .size = slint::PhysicalSize({ width, height }), 
            .panel_handle = panel_handle,
            .touch_handle = touch_handle, 
            .buffer1 = *buffer_rgb8,
            .byte_swap = true
            });
    }
    else if(board_lcd->data_width == 16)
    {
        // Allocate a drawing buffer
        buffer_rgb565 = new std::vector<slint::platform::Rgb565Pixel>(width * height);
        // buffer2_rgb565 = new std::vector<slint::platform::Rgb565Pixel>(width * height);
        // Initialize Slint's ESP platform support
        slint_esp_init(SlintPlatformConfiguration<slint::platform::Rgb565Pixel>{
            .size = slint::PhysicalSize({ width, height }), 
            .panel_handle = panel_handle,
            .touch_handle = touch_handle, 
            .buffer1 = *buffer_rgb565,
            // .buffer2 = *buffer2_rgb565,
            .color_swap_16 = false
            });
    }

    board_set_backlight(60.0);

    xTaskCreate(_task_window, "DISP", 8192 * 2, NULL, 15, &_task_handle_ui);

    // return eve_found;
    return true;
}

extern "C" void app_ui_stop(void)
{
    if(_task_handle_ui)
    {
        vTaskDelete(_task_handle_ui);
        _task_handle_ui = NULL;
    }
}

extern "C" 
{
    static void _task_window(void* param)
    {
        auto ui = AppWindow::create();
        // Use this to make the UI look bigger on the 5" display when software should be scaled for multiple displays.
        // if (display_device_get_width(board_lcd->display) > 500) 
        // {
        //     ui->window().dispatch_scale_factor_change_event(2.);
        // }
        /* Show it on the screen and run the event loop */
        ui->global<Logic>().set_version(slint::SharedString(version_get_string()));
        ui->global<Logic>().set_display_size(slint::SharedString(board_lcd->screen_diagonal));
        ui->global<Logic>().set_image_image(_get_image_image());

#if CONFIG_IDF_TARGET_ESP32P4
        ui->global<Logic>().on_start_camera([&ui](float width, float height){
            _buffer.width = (size_t)width;
            _buffer.height = (size_t)height;
            app_camera_start(&_buffer);
            ui->global<Logic>().set_is_capturing(app_camera_is_capturing());
        });
        ui->global<Logic>().on_stop_camera([&ui](){
            app_camera_stop();
            ui->global<Logic>().set_is_capturing(app_camera_is_capturing());
        });
        ui->global<Logic>().set_has_camera(app_camera_is_initialized());
        ui->global<Logic>().on_camera_update([&ui](){
            if(app_camera_is_capturing())
            {
                if(app_camera_has_frame_captured())
                {
                    ui->global<Logic>().set_camera_image(_get_camera_image());
                    app_camera_capture_frame();
                }
            }
        });
        ui->global<Logic>().on_image_update([&ui](){
            ui->global<Logic>().set_image_image(_get_image_image());
        });
#endif

        slint::Timer timer_update_runtime;
        timer_update_runtime.start(slint::TimerMode::Repeated, 1s, [&ui]() {
            uint32_t seconds = system_get_tick_count() / 1000;
            DBG_VERBOSE((char*)"seconds = %d\n", seconds);
            ui->global<Logic>().set_runtime_minutes(slint::SharedString(std::format("{:02d}:{:02d}", seconds / 60, seconds % 60)));
        });
        ui->run();
    }

#if CONFIG_IDF_TARGET_ESP32P4
    static void _camera_capture_cb(camera_buffer_t* buffer)
    {
        // display_device_draw_bitmap(board_lcd->display, 0, 0, buffer->width, buffer->height, buffer->buffer);
    }
#endif
}

#if CONFIG_IDF_TARGET_ESP32P4
slint::Image _get_image_image()
{
    DBG_INFO("_get_image\n");
    if(_image_buffer)
    {
        static int count = 0;

        uint32_t address = count * (1024 * 600 * 3);

        uint32_t width = display_device_get_width(board_lcd->display);
        uint32_t height = display_device_get_height(board_lcd->display);
        
        const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "fdata");
        if (!partition) 
        {
            DBG_ERROR("Error loading partition\n");
            return slint::Image();
        }

        // Buffer for reading the flash
        uint8_t buffer_read[1024];
        // Source pointer RGB565
        uint8_t* src = buffer_read;
        // Destination pointer RGB888
        uint8_t* ptr = (uint8_t*)_image_buffer;
        // Offset inside the currently read buffer
        size_t buffer_offset = 0;
        // Offset for calculating the read address
        size_t offset = 0;

        size_t max_size = 1024 * 600 * 3;

        for(int i = 0; i < max_size; i += sizeof(buffer_read))
        {
            esp_err_t err = esp_partition_read(partition, address + i, buffer_read, sizeof(buffer_read));
            if(err != ESP_OK)
            {
                DBG_ERROR("Error reading flash\n");
                return slint::Image();
            }

            memcpy(ptr, src, sizeof(buffer_read));

            ptr += sizeof(buffer_read);
        }

        // for(int i = 0; i < width; i++)
        // {
        //     for(int j = 0; j < height; j++)
        //     {
        //         if(buffer_offset == 0)
        //         {
        //             esp_err_t err = esp_partition_read(partition, address + offset, buffer_read, sizeof(buffer_read));
        //             if(err != ESP_OK)
        //             {
        //                 DBG_ERROR("Error reading flash\n");
        //                 return slint::Image();
        //             }
        //         }

        //         // uint16_t pix = ((uint16_t)src[0] << 8) | src[1];
        //         // // Take pixel and swap r and b because of slint
        //         // uint8_t r5 = (pix >> (6+5)) & 0x01F;
        //         // uint8_t g6 = (pix >> 5) & 0x03F;
        //         // uint8_t b5 = pix & 0x01F;
        //         // // Shift the color to the higher positions

        //         // uint8_t r8 = (r5 << 3) | (r5 >> 2); // 5 -> 8 Bit
        //         // uint8_t g8 = (g6 << 2) | (g6 >> 4); // 6 -> 8 Bit
        //         // uint8_t b8 = (b5 << 3) | (b5 >> 2); // 5 -> 8 Bit

        //         // ptr[1] = r8;
        //         // ptr[2] = g8;
        //         // ptr[0] = b8;

        //         // // ptr[0] <<= 3;
        //         // // ptr[1] <<= 2;
        //         // // ptr[2] <<= 3;

        //         // // Increment pointer
        //         // ptr += 3;
        //         // src += 2;
        //         // Increment offset in the buffer to detect overflow
        //         buffer_offset += 2;
        //         if(buffer_offset >= sizeof(buffer_read))
        //         {
        //             buffer_offset = 0;
        //             src = buffer_read;
        //             offset += sizeof(buffer_read);
        //         }
        //     }
        // }

        // TODO: Convert RGB565 from image to RGB888
    
        // ppa_srm_oper_config_t srm_config = 
        // {
        //     .in = {
        //         .buffer = NULL, // TODO:
        //         .pic_w = 1024,
        //         .pic_h = 600,
        //         .block_w = _buffer.width,
        //         .block_h = _buffer.height,
        //         .block_offset_x = 0,
        //         .block_offset_y = 0,
        //         .srm_cm = PPA_SRM_COLOR_MODE_RGB565,
        //         .yuv_range = PPA_COLOR_RANGE_FULL,
        //         .yuv_std = PPA_COLOR_CONV_STD_RGB_YUV_BT601,
        //     },
        //     .out = {
        //         .buffer = _image_buffer,
        //         .buffer_size = width * height * 3,
        //         .pic_w = width,
        //         .pic_h = height,
        //         .block_offset_x = 0,
        //         .block_offset_y = 0,
        //         .srm_cm = PPA_SRM_COLOR_MODE_RGB888,
        //         .yuv_range = PPA_COLOR_RANGE_FULL,
        //         .yuv_std = PPA_COLOR_CONV_STD_RGB_YUV_BT601,
        //     },
        //     .rotation_angle = PPA_SRM_ROTATION_ANGLE_180,
        //     .scale_x = 1.0f,
        //     .scale_y = 1.0f,
        //     .mirror_x = false,
        //     .mirror_y = false,

        //     .mode = PPA_TRANS_MODE_BLOCKING,
        // };
        // ppa_do_scale_rotate_mirror(_ppa_handle, &srm_config);
        // Test image red / green / blue
        // uint8_t* ptr = (uint8_t*)_image_buffer;

        // for(int i = 0; i < width; i++)
        // {
        //     for(int j = 0; j < height; j++)
        //     {
        //         for(int k = 0; k < 3; k++)
        //         {
        //             ptr[k] = k == count ? 0xFF : 0;
        //         }
        //         ptr += 3;
        //     }
        // }

        // count = (count + 1) % 3;
        count = (count + 1) % 2;
    
        // Create an RGB8 pixel buffer for Slint
        slint::SharedPixelBuffer<slint::Rgb8Pixel> pixel_buffer(width, height, (slint::Rgb8Pixel*)_image_buffer);
    
        return slint::Image(pixel_buffer);
    }
    else
    {
        return slint::Image();
    }
}
slint::Image _get_camera_image()
{
    // Create an RGB8 pixel buffer for Slint
    slint::SharedPixelBuffer<slint::Rgb8Pixel> pixel_buffer((uint32_t)_buffer.width, (uint32_t)_buffer.height, (slint::Rgb8Pixel*)_buffer.buffer);

    // // Clear to white
    // std::fill(pixel_buffer.begin(), pixel_buffer.end(), slint::Rgb8Pixel{0xFF, 0xFF, 0xFF});

    // slint::Rgb8Pixel* raw_data = pixel_buffer.begin();

    // auto set_pixel = [&](int x, int y, slint::Rgb8Pixel color) {
    //     if (x >= 0 && x < width && y >= 0 && y < height) {
    //         raw_data[y * width + x] = color;
    //     }
    // };

    // slint::Rgb8Pixel red = {0xFF, 0x00, 0x00};
    // slint::Rgb8Pixel half_red = {0xFF, 0x80, 0x80}; // Light red (halfway blended with white)

    // for (int x = 0; x < width; ++x)
    // {
    //     int16_t temp = 0;
    //     if (ringbuffer_get(soldering_curve_buffer, &temp, x) == FUNCTION_RETURN_OK)
    //     {
    //         temp = std::clamp<int16_t>(temp, 0, height - 1);
    //         int y = height - 1 - temp;  // Flip Y axis, so that 0 is at the bottom

    //         // Center and adjacent pixels (full red)
    //         set_pixel(x, y, red);
    //         set_pixel(x, y - 1, red);
    //         set_pixel(x, y + 1, red);
    //         set_pixel(x - 1, y, red);
    //         set_pixel(x + 1, y, red);

    //         // Diagonal pixels (half red, anti-aliased)
    //         set_pixel(x - 1, y - 1, half_red);
    //         set_pixel(x + 1, y - 1, half_red);
    //         set_pixel(x - 1, y + 1, half_red);
    //         set_pixel(x + 1, y + 1, half_red);
    //     }
    // }

    return slint::Image(pixel_buffer);
}
#endif

#endif // #if defined(KERNEL_USES_SLINT)