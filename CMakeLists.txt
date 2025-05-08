# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(CCACHE_ENABLE "1")
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

set(CONFIG_MY_PROJECT_KCONFIG "Kconfig")

set(KERNEL_ADD_DEPENDENCY_SLINT true)
# set(KERNEL_ADD_DEPENDENCY_LVGL true)
# Needed for lvgl
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE=1)
# May be needed for 5" display to display the screen properly when using lvgl
#add_compile_definitions(DISPLAY_NUM_FB=2)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(DISPLAYDEMOSLINT)

if(${KERNEL_ADD_DEPENDENCY_SLINT})
        set(SLINT_STYLE "fluent-light")
        set(SLINT_FONT_SIZES 12,24,48)

        SET(GCC_COVERAGE_LINK_FLAGS    "-z noexecstack")
        SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS} -Wl,--allow-multiple-definition")
endif()

if(DEFINED ENV{FLASHERDIR})
        # The following line is needed when creating eso files in the format of ESoPe GmbH. For this you need the Flasher tool of ESoPe GmbH.
        # This is needed to use the secure software update mechanisms implemented from ESoPe.

        if(IDF_TARGET STREQUAL "esp32p4")
                message("Build for SLD_C_W_P4_C6")
                add_custom_target(${PROJECT_NAME} ALL
                        COMMAND "$ENV{FLASHERDIR}\\Flasher.Cli.exe" "${project_dir}/config_p4.json"
                        COMMAND "$ENV{FLASHERDIR}\\Flasher.Cli.exe" "${project_dir}/config_p4_compressed.json"
                        )
        
                add_dependencies(${PROJECT_NAME} gen_project_binary)
        endif()

        if(IDF_TARGET STREQUAL "esp32s3")
                if(CONFIG_SLD_C_W_S3_BT817)
                        message("Build for SLD_C_W_S3_BT817")
                        add_custom_target(${PROJECT_NAME} ALL
                                COMMAND "$ENV{FLASHERDIR}\\Flasher.Cli.exe" "${project_dir}/config_s3_bt817.json"
                                COMMAND "$ENV{FLASHERDIR}\\Flasher.Cli.exe" "${project_dir}/config_s3_bt817_compressed.json"
                                )
                
                        add_dependencies(${PROJECT_NAME} gen_project_binary)
                else()
                        message("Build for SLD_C_W_S3")
                        add_custom_target(${PROJECT_NAME} ALL
                                COMMAND "$ENV{FLASHERDIR}\\Flasher.Cli.exe" "${project_dir}/config_s3.json"
                                COMMAND "$ENV{FLASHERDIR}\\Flasher.Cli.exe" "${project_dir}/config_s3_compressed.json"
                                )
                
                        add_dependencies(${PROJECT_NAME} gen_project_binary)
                endif()
        endif()
endif()