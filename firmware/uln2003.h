#pragma once

#include <stdbool.h>

void uln2003_init(void);

void uln2003_make_step(void);
void uln2003_set_dir(bool dir);
void uln2003_unforce(void);
