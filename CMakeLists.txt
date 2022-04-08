cmake_minimum_required(VERSION 3.13)
project(kobuki)

# defulat settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE true)
add_compile_options(-Wall -W -Wshadow -Wextra)
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})

# set application version
set(VERSION 0.1)
add_compile_definitions(_VERSION_=\"${VERSION}\")

set(TARGET_APP kobuki)
add_executable(${TARGET_APP})
target_include_directories(${TARGET_APP} PRIVATE ${PROJECT_ROOT}/src)
# target_link_directories(${TARGET_APP} PRIVATE ...)
target_sources(${TARGET_APP} PRIVATE
    src/kobuki-driver.c
    src/kobuki-func.c
    src/kobuki-udp.c
)

set_target_properties(${TARGET_APP} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT}/output)
