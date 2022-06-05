#pragma once

#include <unistd.h>
#include <movement.h>

void command_init(void (*resp)(const char *msg));
void command_process(struct focuser_descriptor_s *desc, const char *cmd, size_t len);
