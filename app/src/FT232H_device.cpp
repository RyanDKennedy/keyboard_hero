#include "FT232H_device.hpp"

/*
  GREAT RESOURCES
  https://atadiat.com/en/e-ftdi-mpsse-engine-programming-basics-a-gui-example/
  https://github.com/adafruit/Adafruit_Python_GPIO/blob/master/Adafruit_GPIO/FT232H.py#L168
*/

#include <assert.h>

#include <ftd2xx.h>

typedef unsigned int uint;

extern "C"
int ft232h_create_device(FT232HDevice *out_device, const char **out_error)
{
    FT232HDevice device;
    device.direction_mask = 0;
    device.output_mask = 0;

    if (!FT_SUCCESS(FT_Open(0, &device.handle)))
    {
	*out_error =  "Failed to open device\nTo fix this try running \"rmmod ftdi_sio; rmmod usbserial\"\nAnother possible solution is to run this with privileges if it doesn't currently have privileges.";
	return -1;
    }

    FT_DEVICE device_info;
    if (!FT_SUCCESS(FT_GetDeviceInfo(device.handle, &device_info, NULL, NULL, NULL, NULL)))
    {
	*out_error =  "Failed to get device info";
	return -1;
    }

    if (device_info != FT_DEVICE_232H)
    {
	*out_error =  "Didn't open a ft232h device";
	return -1;
    }

    // don't specify mask because we will set pins later using mpsse engine commands
    if (!FT_SUCCESS(FT_SetBitMode(device.handle, 0, FT_BITMODE_MPSSE)))
    {
	*out_error = "failed to set bitmode to async bitbang";
	return -1;
    }

    FT_STATUS ft_status = 0;
    
    ft_status |= FT_SetUSBParameters(device.handle, 65535, 65535); //Set USB request transfer size
    ft_status |= FT_SetChars(device.handle, 0, 0, 0, 0); //Disable event and error characters
    ft_status |= FT_SetTimeouts(device.handle, 3000, 3000); //Sets the read and write timeouts in 3 sec for the FT2232H
    ft_status |= FT_SetLatencyTimer(device.handle, 1); //Set the latency 
    
    if (!FT_SUCCESS(ft_status))
    {
	*out_error =  "new code failed";
	return -1;
    }
    
    // send the bad code to sync with mpsse
    UCHAR bad_code = 0xAA;
    uint bytes_sent = 0;
    ft_status = FT_Write(device.handle, &bad_code, 1, &bytes_sent);
    
    if (!FT_SUCCESS(ft_status))
    {
	*out_error =  "failed to write bad code";
	return -1;
    }
    
    DWORD amt_in_queue = 0;
    do
    {
	ft_status = FT_GetQueueStatus(device.handle, &amt_in_queue);
    } while ((amt_in_queue == 0) && (ft_status == FT_OK));
    
    int command_echod = 0;
    char input_buf[256];
    uint bytes_read = 0;
    ft_status = FT_Read(device.handle, input_buf, 2, &bytes_read);
    for (uint dw_count = 0; dw_count < (bytes_read - 1); ++dw_count)
    {
	if (input_buf[dw_count] == '\xFA' && input_buf[dw_count + 1] == '\xAA')
	{
	    command_echod = 1;
	    break;
	}
    }
    
    if (command_echod == 0)
    {
	*out_error =  "failed to sync";
	return -1;
    }

    // set all pins to output
    for (int i = 0; i < 16; ++i)
    {
	ft232h_set_pin_state(&device, i, FT232H_PIN_DIRECTION_OUTPUT, FT232H_PIN_OUTPUT_LOW);
    }
    if (ft232h_upload_gpio_state(&device) != 0)
    {
	*out_error = "Failed to set all pins to output";
	return -1;
    }

    *out_device = device;

    return 0;
}

extern "C"
int ft232h_destroy_device(FT232HDevice *device, const char **out_error)
{
    int result = 0;

    if (!FT_SUCCESS(FT_SetBitMode(device->handle, 0, FT_BITMODE_RESET)))
    {
	*out_error =  "failed to set bitmode to reset";
	result = -1;
    }

    if (!FT_SUCCESS(FT_Close(device->handle)))
    {
	*out_error =  "Failed to close device";
	result = -1;
    }

    device->handle = (FT_HANDLE)FT_INVALID_HANDLE;
    device->direction_mask = 0;
    device->output_mask = 0;

    return result;

}

extern "C"
uint16_t ft232h_get_gpio_state(FT232HDevice *device)
{
    FT_STATUS ft_status = 0;
    UCHAR read_gpio_code[3] = "\x81\x83";

    uint bytes_sent = 0;
    FT_Write(device->handle, read_gpio_code, 2, &bytes_sent);
    
    uint amt_in_queue = 0;
    do
    {
	ft_status = FT_GetQueueStatus(device->handle, &amt_in_queue);
    } while ((amt_in_queue == 0 || amt_in_queue < 2) && (ft_status == FT_OK));
    

    UCHAR input_buf[2];
    uint bytes_read;
    FT_Read(device->handle, input_buf, 2, &bytes_read);
    
    return *((uint16_t*)input_buf);
}

extern "C"
void ft232h_set_pin_state(FT232HDevice *device, int pin, FT232HPinDirection dir, FT232HPinOutputState output_state)
{
    assert(pin >= 0 && pin <= 15 && "Invalid pin number. Pin number must be 0 <= pin <= 15.");

    if (dir == FT232H_PIN_DIRECTION_OUTPUT)
    {
	device->direction_mask |= 1 << pin;

	if (output_state == FT232H_PIN_OUTPUT_LOW)
	    device->output_mask &= ~(1 << pin);
	else
	    device->output_mask |= 1 << pin;

    }
    else
    {
	device->direction_mask &= ~(1 << pin);
    }
    
}

extern "C"
int ft232h_upload_gpio_state(FT232HDevice *device)
{
    FT_STATUS ft_status = 0;

    {
	uint8_t direction_mask = device->direction_mask & 0x00FF;
	uint8_t output_mask = device->output_mask & 0x00FF;

	UCHAR output_buf[4] = "\x80\x00\x00"; 
	output_buf[1] = output_mask;
	output_buf[2] = direction_mask;

	uint bytes_sent = 0;
	ft_status |= FT_Write(device->handle, output_buf, 3, &bytes_sent);	
    }

    {
	uint8_t direction_mask = (device->direction_mask & 0xFF00) >> 8;
	uint8_t output_mask = (device->output_mask & 0xFF00) >> 8;

	UCHAR output_buf[4] = "\x82\x00\x00"; 
	output_buf[1] = output_mask;
	output_buf[2] = direction_mask;
	
	uint bytes_sent = 0;
	ft_status |= FT_Write(device->handle, output_buf, 3, &bytes_sent);	
    }

    return (ft_status != 0);
}
