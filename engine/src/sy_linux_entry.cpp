#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/time.h>

#include "sy_linux_window.hpp"
#include "sy_linux_input.hpp"

#include "sy_syengine.hpp"
#include "sy_macros.hpp"

size_t get_current_time_us();
double get_us_diff_in_ms(size_t a, size_t b);

int main(int argc, char *argv[])
{
    SyenginePlatformInfo platform_info;
    SyXCBInfo xcb_info;

    // Linux Init
    init_window(&xcb_info, 600, 600, "Syengine");

    { // Platform Info Init
	// Input Init
	memset(&platform_info.input_info, 0, sizeof(SyInputInfo));
	poll_events(&xcb_info, &platform_info.input_info);
	
	// Arena/Allocation Init
	SY_ERROR_COND(platform_info.persistent_arena.initialize(4096) != 0, "Failed to allocate data for persistent arena.");
	SY_ERROR_COND(platform_info.frame_arena.initialize(4096) != 0, "Failed to allocate data for frame arena.");
	
	// ECS Init
	platform_info.ecs.initialize();
	
	// Flags Init
	platform_info.end_engine = false;
    }
    
    // Init Engine
    init_engine(&platform_info);

    // RENDER LOOP

    // Delta time stuff
    double frame_limit_frame_time_in_ms = 16.66667;
    size_t delta_time_frame_start = get_current_time_us();
    size_t delta_time_frame_end;

    while (platform_info.end_engine == false)
    {
	
	{ // Enforce Frame Rate
	    delta_time_frame_end = get_current_time_us();
	    double diff = get_us_diff_in_ms(delta_time_frame_end, delta_time_frame_start);
	    if (diff < frame_limit_frame_time_in_ms)
	    {
		usleep(frame_limit_frame_time_in_ms * 1000.0 - (delta_time_frame_end - delta_time_frame_start));
		size_t old_start = delta_time_frame_start;
		delta_time_frame_start = get_current_time_us();
		platform_info.delta_time = get_us_diff_in_ms(delta_time_frame_start, old_start);
	    }
	    else
	    {
		delta_time_frame_start = delta_time_frame_end;
		platform_info.delta_time = diff;
	    }
	}

	// Input
	poll_events(&xcb_info, &platform_info.input_info);

	// Calls engine
	run_engine(&platform_info);
    }

    // Destroys engine
    destroy_engine(&platform_info);

    
    { // Cleanup Platform Info
	// Arena/Allocation Cleanup
	platform_info.persistent_arena.destroy();
	platform_info.frame_arena.destroy();
    
	// ECS Cleanup
	platform_info.ecs.destroy();
    }

    // Cleanup Linux
    cleanup_window(&xcb_info);

    return 0;
}

size_t get_current_time_us()
{
    timeval timev;
    gettimeofday(&timev, NULL);
    return timev.tv_sec * 1000000 + timev.tv_usec;
}

double get_us_diff_in_ms(size_t a, size_t b)
{
    return (double)(a - b) / 1000.0;
}
