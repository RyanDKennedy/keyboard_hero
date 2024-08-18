#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>
#include <dlfcn.h>

#include "sy_linux_window.hpp"
#include "sy_linux_input.hpp"

#include "sy_syengine.hpp"
#include "sy_macros.hpp"

size_t get_current_time_us();
double get_us_diff_in_ms(size_t a, size_t b);
void load_app_functions(SyPlatformInfo *platform_info, const char *dll_file);

int main(int argc, char *argv[])
{
    // parse command arguments
    const char *dll_file = "./libapp.so";

    for (size_t i = 1; i < argc; ++i)
    {
	const char *dllequals = "dll=";
	if (strlen(argv[i]) > strlen(dllequals) &&memcmp(argv[i], dllequals, strlen(dllequals)) == 0)
	{
	    dll_file = argv[i] + strlen(dllequals);
	}
    }

    SyPlatformInfo platform_info; // Stuff used to communicate platform <--> engine
    SyXCBInfo xcb_info;
    SyAppInfo app_info; // Stuff used to communicate engine <--> app

    // Linux Init
    init_window(&xcb_info, 600, 600, "Syengine");

    { // Platform Info Init
	// App Dyanamic function load
	platform_info.dll_handle = nullptr;
	platform_info.app_init = nullptr;
	platform_info.app_run = nullptr;
	platform_info.app_destroy = nullptr;
	load_app_functions(&platform_info, dll_file);
	SY_OUTPUT_INFO("Loaded the dll app functions.");

	// Flags Init
	platform_info.end_engine = false;
	platform_info.reload_dll = false;
    }
    
    { // App Info Init
	// Input Init
	memset(&app_info.input_info, 0, sizeof(SyInputInfo));
	poll_events(&xcb_info, &app_info.input_info);
	
	// Arena/Allocation Init
	SY_ERROR_COND(app_info.persistent_arena.initialize(4096) != 0, "Failed to allocate data for persistent arena.");
	SY_ERROR_COND(app_info.frame_arena.initialize(4096) != 0, "Failed to allocate data for frame arena.");
	
	// ECS Init
	app_info.ecs.initialize();
    }

    // Init Engine
    engine_init(&platform_info, &app_info);

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
		app_info.delta_time = get_us_diff_in_ms(delta_time_frame_start, old_start);
	    }
	    else
	    {
		delta_time_frame_start = delta_time_frame_end;
		app_info.delta_time = diff;
	    }
	}

	// Input
	poll_events(&xcb_info, &app_info.input_info);

	// Calls engine
	engine_run(&platform_info, &app_info);

	if (platform_info.reload_dll)
	{
	    platform_info.reload_dll = false;
	    load_app_functions(&platform_info, dll_file);
	    SY_OUTPUT_INFO("Reloaded the dll app functions.");
	    sleep(1);
	}


    }

    // Destroys engine
    engine_destroy(&platform_info, &app_info);

    
    { // Cleanup App Info
	// Arena/Allocation Cleanup
	app_info.persistent_arena.destroy();
	app_info.frame_arena.destroy();
    
	// ECS Cleanup
	app_info.ecs.destroy();
    }

    { // Cleanup Platform Info
	dlclose(platform_info.dll_handle);

    }

    // Cleanup Linux
    cleanup_window(&xcb_info);

    return 0;
}

void load_app_functions(SyPlatformInfo *platform_info, const char *dll_file)
{
    if (platform_info->dll_handle != nullptr)
    {
	dlclose(platform_info->dll_handle);
    }

    char *error;
    platform_info->dll_handle = dlmopen(LM_ID_NEWLM, dll_file, RTLD_NOW); // This needs to be dlmopen(LM_ID_NEWLM, ...) otherwise the gnu unique symbols inside of the elf .so make it marked as nodelete which means that when you do dlclose(...) it doesn't unload it, so then when you do dlopen(...) after it just gives you the same code instead of the new code. Read man ld section about unique/nounique
    SY_ERROR_COND(platform_info->dll_handle == NULL, "failed to load shared object. %s", dlerror());
    dlerror();

    platform_info->app_init = (void (*)(SyAppInfo*))dlsym(platform_info->dll_handle, "app_init");
    SY_ERROR_COND((error = dlerror()) != NULL, "failed to load symbol %s", error);

    platform_info->app_run = (void (*)(SyAppInfo*))dlsym(platform_info->dll_handle, "app_run");
    SY_ERROR_COND((error = dlerror()) != NULL, "failed to load symbol %s", error);

    platform_info->app_destroy = (void (*)(SyAppInfo*))dlsym(platform_info->dll_handle, "app_destroy");
    SY_ERROR_COND((error = dlerror()) != NULL, "failed to load symbol %s", error);


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
