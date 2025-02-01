#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>
#include <dlfcn.h>

#include "sy_linux_window.hpp"
#include "sy_linux_input.hpp"

#include "sy_syengine.hpp"
#include "sy_macros.hpp"

#define SY_MICROSECOND 1000000

size_t get_current_time_us();
void load_app_functions(SyPlatformInfo *platform_info, const char *dll_file);
int go_to_root_path();

int main(int argc, char *argv[])
{
    // parse command arguments
    const char *dll_file = "./libapp.so";

    {
	opterr = 0;
	option long_options[] =
	{
	    {"dll", required_argument, 0, 'd'},
	    {0, 0, 0, 0}
	};
	int option_index = 0;
	int flag_value;
	while ( (flag_value = getopt_long(argc, argv, "d:", long_options, &option_index)) != -1 )
	{
	    switch (flag_value)
	    {
		case 'd':
		{
		    dll_file = optarg;
		    break;
		}

		case '?':
		{
		    SY_ERROR("Invalid Argument: -%c", optopt);
		    break;
		}

	    }
	}
    }

    // Goto root path
    SY_ERROR_COND(go_to_root_path() == -1, "Failed to go to root path.");

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

	// Input Init
	memset(&platform_info.input_info, 0, sizeof(SyInputInfo));
	poll_events(&xcb_info, &platform_info.input_info);
    }

    // Init Engine
    engine_init(&platform_info, &app_info);

    // RENDER LOOP

    // Delta time stuff
    size_t frame_limit_frame_time = SY_MICROSECOND / 60;
    size_t delta_time_frame_start = get_current_time_us();
    size_t delta_time_frame_end;

    while (platform_info.end_engine == false)
    {
	
	{ // Enforce Frame Rate
	    delta_time_frame_end = get_current_time_us();
	    size_t diff = delta_time_frame_end - delta_time_frame_start;
	    if (diff < frame_limit_frame_time)
	    {
		usleep(frame_limit_frame_time - diff);
		size_t old_start = delta_time_frame_start;
		delta_time_frame_start = get_current_time_us();
		platform_info.delta_time = delta_time_frame_start - old_start;
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
    return timev.tv_sec * SY_MICROSECOND + timev.tv_usec;
}

int go_to_root_path()
{
    char path[257];

    // Get the executable path
    {
	size_t root_path_len = readlink("/proc/self/exe", path, 256);
	if (root_path_len == -1)
	{
	    return -1;
	}
	path[root_path_len] = '\0';
    }
    
    char *last_slash = strrchr(path, '/');

    *last_slash = '\0';

    return chdir(path);
}

