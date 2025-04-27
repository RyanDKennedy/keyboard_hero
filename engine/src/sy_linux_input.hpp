#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "sy_linux_window.hpp"
#include "sy_input_info.hpp"

void poll_events(SyXCBInfo *xcb_info, SyInputInfo *input_info);

void clear_input_info(SyInputInfo *input_info);
