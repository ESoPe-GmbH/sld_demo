# SLD Demo

This software demonstrate the usage of the SLD display series from Schukat using the [esopublic](https://github.com/ESoPe-GmbH/esopublic) library. With the SLD_C_W_S3 microcontroller board of ESoPe.

The displays have can be connected to the board and used without any software change, regardless of the display size. The SLD_C_W_S3 supports the displays from 2.4" (320 x 240) to 5" (800 x 480). The board itself is driven by ESP32-S3-WROOM-1(U)-N16R8, which is a dual core Xtensa MCU with up to 240 MHz clock frequency. The board has a connector for power supply, a connector for the display and a connector for a piggy back PCB with 2 GPIOs. This enable the piggy back to make use of UART, RS485, CAN (TWAI) or I2C.

## Setting up the Development Environment

For setup instructions, refer to the [English setup guide](docs/ide-setup-en.md) or the [German setup guide](docs/ide-setup-de.md).

## Build

The software is built using ESP-IDF 5.2 or 5.4. We recommend using VsCode with the ESP-IDF Extension for easy installation and usage of the espressif compiler. You can compile it either with the vscode function for compiling the esp-idf or by using the esp-idf prompt with the `idf.py build` command.

The default sdkconfig is set to ESP-IDF 5.2. There are also sdkconfig samples for ESP-IDF 5.4 as well as an sdkconfig for ESP32-P4 board (work in progress).

## UI generation

This demo shows how to use UI frameworks [Slint](https://slint.dev/) and [LVGL](https://lvgl.io/) with the display series. Both have there advantages and you can decide for yourself which one you want to use. The default configuration of this project uses Slint.

### Slint

To use slint you have to make the following configuration:

/CMakeList.txt

```Cmake

# Make sure this is defined and set to true
set(KERNEL_ADD_DEPENDENCY_SLINT true)

```

main/idf_component.yml

```yml

dependencies:
  esope-gmbh/esopublic: "^24.1.0"
  slint/slint: "^1.8.0"
  idf:
    version: ">=5.2.0"

```

By enabling this configuration, the CMakeList.txt of esopublic sets a compiler define "KERNEL_USES_SLINT", which is checked internally and enables the main/app_ui_slint.cpp to use for the UI configuration. The main/ui directory contains the slint files, which are used to create the user interface.

### LVGL

To use LVGL you have to make the following configuration:

/CMakeList.txt

```Cmake

# Make sure this line is commented or deleted
# set(KERNEL_ADD_DEPENDENCY_SLINT true)
# Is needed for generated image files to work directly
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE=1)
# This is needed for 5" display, to work propertly
add_compile_definitions(DISPLAY_NUM_FB=2)

```

main/idf_component.yml

```yml

dependencies:
  esope-gmbh/esopublic: "^24.1.0"
  lvgl/lvgl: "^9.1.0"
  idf:
    version: ">=5.2.0"

```

sdkconfig:

```bash

CONFIG_LCD_RGB_RESTART_IN_VSYNC=y
CONFIG_GDMA_CTRL_FUNC_IN_IRAM=y
CONFIG_COMPILER_OPTIMIZATION_PERF=y
CONFIG_LV_FONT_MONTSERRAT_10=y
CONFIG_LV_FONT_MONTSERRAT_14=y
CONFIG_LV_FONT_MONTSERRAT_24=y

```

By enabling this configuration, the CMakeList.txt of esopublic sets a compiler define "KERNEL_USES_LVGL", which is checked internally and enables the main/app_ui_lvgl.c to use for the UI configuration. The main/ui_lvgl directory contains assets to use for creating the lvgl user interface.

To prevent the display "drifting" (especially with 5" display), you should apply the sdkconfig changes above.
