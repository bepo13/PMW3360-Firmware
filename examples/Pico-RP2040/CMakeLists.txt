cmake_minimum_required(VERSION 3.13)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(pico-pmw3360)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# include headers
include_directories(
	../../src
)

# predefined symbles
add_compile_definitions(
	__PICO_SDK__
)

# rest of your project
add_executable(pico-pmw3360
	main.c
	../../src/PMW3360.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pico-pmw3360 pico_stdlib hardware_spi)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pico-pmw3360)