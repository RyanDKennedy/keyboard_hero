#include "sy_linux_init.hpp"
#include "render/sy_render_settings.hpp"
#include "sy_macros.hpp"

void create_instance(SyRenderInfo *render_info);
bool check_validation_layer_support();
bool check_extension_support();
void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info);
static VKAPI_ATTR VkBool32 VKAPI_CALL validation_layers_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data);

void setup_debug_messenger(SyRenderInfo *render_info);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

void create_surface(SyRenderInfo *render_info, SyXCBInfo *xcb_info);


int sy_render_init(SyXCBInfo *xcb_info, SyRenderInfo *render_info)
{
    create_instance(render_info);
    setup_debug_messenger(render_info);
    create_surface(render_info, xcb_info);

    SY_OUTPUT_INFO("Initialized Renderer");

    return 0;
}

int sy_render_deinit(SyRenderInfo *render_info)
{
    vkDestroySurfaceKHR(render_info->instance, render_info->surface, NULL);

    if (sy_g_render_use_validation_layers == true)
    {
	DestroyDebugUtilsMessengerEXT(render_info->instance, render_info->debug_messenger, NULL);
    }

    vkDestroyInstance(render_info->instance, NULL);

    SY_OUTPUT_INFO("Deinitialized Renderer");

    return 0;
}

void create_instance(SyRenderInfo *render_info)
{
    // Make sure validation layers are supported if enabled
    bool supports_validation_layers = check_validation_layer_support();
    SY_ERROR_COND(sy_g_render_use_validation_layers == true && supports_validation_layers == false, "RENDERER: Vulkan Validation layers active but not supported.");

    // Make sure extensions are supported
    SY_ERROR_COND(check_extension_support() == 0, "RENDERER: Vulkan Extensions not supported");
    
    /* Application Info
       Use: GPU developers use this to identify the running application so that they can
            optimize their cards for the specific application. e.g. Fortnite, Adobe Premier Pro
     */
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Application";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "syengine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;//VK_API_VERSION_1_0;
    app_info.pNext = NULL;

    // Instance Create Info
    VkInstanceCreateInfo create_info;
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.flags = 0;

    // extensions
    create_info.enabledExtensionCount = sy_g_render_vulkan_extension_amt;
    create_info.ppEnabledExtensionNames = sy_g_render_vulkan_extensions;

    // layers
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info;
    if (sy_g_render_use_validation_layers)
    {
	create_info.enabledLayerCount = sy_g_render_vulkan_layer_amt;
	create_info.ppEnabledLayerNames = sy_g_render_validation_layers;
	
	populate_debug_messenger_create_info(&debug_create_info);
	create_info.pNext = &debug_create_info;
    }
    else
    {
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
	create_info.pNext = NULL;
    }

    SY_ERROR_COND(vkCreateInstance(&create_info, NULL, &render_info->instance) != VK_SUCCESS, "RENDERER: Failed to create vulkan instance.");
}

bool check_validation_layer_support()
{
    VkResult ok = VK_SUCCESS;

    uint32_t layer_count;
    ok = vkEnumerateInstanceLayerProperties(&layer_count, NULL);
    SY_ERROR_COND(ok != VK_SUCCESS, "RENDERER: vulkan vkEnumerateInstanceLayerProperties(...) failed.");

    VkLayerProperties *available_layers = (VkLayerProperties*)calloc(layer_count, sizeof(VkLayerProperties));
    ok = vkEnumerateInstanceLayerProperties(&layer_count, available_layers);
    SY_ERROR_COND(ok != VK_SUCCESS, "RENDERER: vulkan vkEnumerateInstanceLayerProperties(...) failed.");

    bool result = true;

    for (int i = 0; i < sy_g_render_vulkan_layer_amt; ++i)
    {
	const char *layer_name = sy_g_render_validation_layers[i];

	bool layer_found = false;
	for (size_t available_i = 0; available_i < layer_count; ++available_i)
	{
	    if (strcmp(layer_name, available_layers[available_i].layerName) == 0)
	    {
		layer_found = true;
		break;
	    }
	}

	if (layer_found == false)
	{
	    result = false;
	    break;
	}
    }

    free(available_layers);

    return result;
}

bool check_extension_support()
{
    bool result = true;
    VkResult ok = VK_SUCCESS;

    uint32_t extension_count;
    ok = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
    SY_ERROR_COND(ok != VK_SUCCESS, "ERROR: vkEnumerateInstanceExtensionProperties(...) failed.");    

    VkExtensionProperties *available_extensions = (VkExtensionProperties*)calloc(extension_count, sizeof(VkExtensionProperties));
    ok = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, available_extensions);
    SY_ERROR_COND(ok != VK_SUCCESS, "ERROR: vkEnumerateInstanceExtensionProperties(...) failed.");    

    for (int i = 0; i < sy_g_render_vulkan_extension_amt; ++i)
    {
	const char *extension_name = sy_g_render_vulkan_extensions[i];

	bool extension_found = false;
	for (int available_i = 0; available_i < extension_count; ++available_i)
	{
	    if (strcmp(extension_name, available_extensions[available_i].extensionName) == 0)
	    {
		extension_found = true;
	    }
	}

	if (extension_found == false)
	{
	    result = false;
	    SY_OUTPUT_INFO("Coudn't find vulkan extension \"%s\"", extension_name);
	    break;
	}
    }

    free(available_extensions);

    return result;    
}

// Shared functions
void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info)
{
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

    create_info->messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
//	                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
	                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    create_info->flags = 0;
    create_info->pfnUserCallback = validation_layers_debug_callback;
    create_info->pNext = NULL;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL validation_layers_debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *p_user_data)
{
    const char *error_label = "ERROR";
    const char *warning_label = "WARNING";
    const char *verbose_label = "VERBOSE";

    const char *used_label = "NO LABEL";

    if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
	SY_ERROR_OUTPUT("RENDERER: validation layers - WARNING %s", p_callback_data->pMessage);
    }
    else if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
	SY_ERROR_OUTPUT("RENDERER: validation layers - ERROR %s", p_callback_data->pMessage);
    }
    else if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
    {
	SY_OUTPUT_DEBUG("RENDERER: validation layers - VERBOSE %s", p_callback_data->pMessage);
    }

    return VK_FALSE;
}

void setup_debug_messenger(SyRenderInfo *render_info)
{
    if (sy_g_render_use_validation_layers == false) return;
    
    VkDebugUtilsMessengerCreateInfoEXT create_info;
    populate_debug_messenger_create_info(&create_info);
    
    SY_ERROR_COND(CreateDebugUtilsMessengerEXT(render_info->instance, &create_info, NULL, &render_info->debug_messenger) != VK_SUCCESS, "RENDER: Failed to setup debug messenger.")
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
	return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
	return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
	func(instance, debugMessenger, pAllocator);
    }
}

void create_surface(SyRenderInfo *render_info, SyXCBInfo *xcb_info)
{
    VkXcbSurfaceCreateInfoKHR create_info;
    create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    create_info.pNext = NULL;
    create_info.flags = 0;
    create_info.connection = xcb_info->conn;
    create_info.window = xcb_info->win;

    SY_ERROR_COND(vkCreateXcbSurfaceKHR(render_info->instance, &create_info, NULL, &render_info->surface) != VK_SUCCESS, "RENDER: Failed to create vulkan surface (xcb).");
}

