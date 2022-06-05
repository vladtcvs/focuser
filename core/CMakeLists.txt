add_library(focuser_core STATIC movement.c command.c)
target_include_directories(focuser_core PUBLIC .)
