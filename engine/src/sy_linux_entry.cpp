/*
  the projectile shit
  splash splash splash splash everywhere
  big poop in my sock
 */


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

#include "render/sy_linux_init.hpp"

#define SY_US_SECOND 1000000

#ifndef NDEBUG
void load_app_functions(SyPlatformInfo *platform_info, const char *dll_file);
#endif

size_t get_current_time_us();
int go_to_root_path();

int main(int argc, char *argv[])
{
    // parse command arguments
    const char *dll_file = "./bin/debug/libapp.so";

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


    SyXCBInfo *xcb_info = (SyXCBInfo*)calloc(1, sizeof(SyXCBInfo));
    SyPlatformInfo *platform_info = (SyPlatformInfo*)calloc(1, sizeof(SyPlatformInfo)); // Stuff used to communicate platform <--> engine
    SyAppInfo *app_info = (SyAppInfo*)calloc(1, sizeof(SyAppInfo)); // Stuff used to communicate engine <--> app
    SyEngineState *engine_state = (SyEngineState*)calloc(1, sizeof(SyEngineState));

    int status;

    // Linux Init
    init_window(xcb_info, 600, 600, "Syengine");

    { // Platform Info Init
	// App Dyanamic function load
#ifndef NDEBUG
	platform_info->dll_handle = nullptr;
	platform_info->app_init = nullptr;
	platform_info->app_run = nullptr;
	platform_info->app_destroy = nullptr;
	platform_info->app_dll_init = nullptr;
	platform_info->app_dll_exit = nullptr;
	platform_info->reload_dll = false;
	load_app_functions(platform_info, dll_file);
	SY_OUTPUT_INFO("Loaded the dll app functions.");
#endif

	// Flags Init
	platform_info->end_engine = false;

	// Input Init
	memset(&platform_info->input_info, 0, sizeof(SyInputInfo));
	poll_events(xcb_info, &platform_info->input_info);

	// Init render system
	status = sy_render_init(xcb_info, &platform_info->render_info);
	// FIXME CHECK STATUS VAR

    }

    // Init Engine
    engine_init(platform_info, app_info, engine_state);

    // RENDER LOOP

    // Delta time stuff
    size_t frame_limit_frame_time = SY_US_SECOND / 240;
    size_t delta_time_frame_start = get_current_time_us();
    size_t delta_time_frame_end;

    while (platform_info->end_engine == false)
    {
	
	{ // Enforce Frame Rate
	    delta_time_frame_end = get_current_time_us();
	    size_t diff = delta_time_frame_end - delta_time_frame_start;
	    if (diff < frame_limit_frame_time)
	    {
		usleep(frame_limit_frame_time - diff);
		size_t old_start = delta_time_frame_start;
		delta_time_frame_start = get_current_time_us();
		platform_info->delta_time = delta_time_frame_start - old_start;
	    }
	    else
	    {
		delta_time_frame_start = delta_time_frame_end;
		platform_info->delta_time = diff;
	    }
	}

	// Input
	poll_events(xcb_info, &platform_info->input_info);

	// Calls engine
	engine_run(platform_info, app_info, engine_state);

#ifndef NDEBUG
	if (platform_info->reload_dll)
	{
	    platform_info->reload_dll = false;
	    platform_info->dll_first_run = true;
	    load_app_functions(platform_info, dll_file);
	    SY_OUTPUT_INFO("Reloaded the dll app functions.");
	    sleep(1);
	}
#endif


    }

    // Destroys engine
    engine_destroy(platform_info, app_info, engine_state);

    { // Cleanup Platform Info
#ifndef NDEBUG
	dlclose(platform_info->dll_handle);
#endif

	sy_render_deinit(&platform_info->render_info);
    }

    // Cleanup Linux
    cleanup_window(xcb_info);

    free(engine_state);
    free(app_info);
    free(platform_info);
    free(xcb_info);

    return 0;
}

#ifndef NDEBUG
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

    platform_info->app_dll_init = (void (*)(SyAppInfo*))dlsym(platform_info->dll_handle, "app_dll_init");
    SY_ERROR_COND((error = dlerror()) != NULL, "failed to load symbol %s", error);

    platform_info->app_dll_exit = (void (*)(SyAppInfo*))dlsym(platform_info->dll_handle, "app_dll_exit");
    SY_ERROR_COND((error = dlerror()) != NULL, "failed to load symbol %s", error);

}
#endif

size_t get_current_time_us()
{
    timeval timev;
    gettimeofday(&timev, NULL);
    return timev.tv_sec * SY_US_SECOND + timev.tv_usec;
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
    
    for (int i = 0; i < 3; ++i)
    {
	char *last_slash = strrchr(path, '/');
	*last_slash = '\0';
    }

    SY_OUTPUT_INFO("Root Directory = %s", path);

    return chdir(path);
}

