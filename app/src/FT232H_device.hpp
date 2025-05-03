#pragma once

#include <stdint.h>

typedef struct
{
    void *handle;
    uint16_t direction_mask; // bitmask 0 == output, 1 == input
    uint16_t output_mask; // bitmask that specifies the level at each output pin, 0 == low, 1 == high
} FT232HDevice;

typedef enum
{
    FT232H_PIN_DIRECTION_OUTPUT,
    FT232H_PIN_DIRECTION_INPUT,
} FT232HPinDirection;

typedef enum
{
    FT232H_PIN_OUTPUT_LOW = 0,
    FT232H_PIN_OUTPUT_HIGH = 1,
} FT232HPinOutputState;

/**
 * @brief creates a ft232h device for use with this library
 * @param[out] out_device a pointer to the outputted device
 * @param[out] out_error a pointer to a const char* where the error will be stored, THIS CANNOT BE NULL, THIS MUST BE A VALID POINTER
 * @returns 0 on success
 * @returns non-zero on error
 */
extern "C"
int ft232h_create_device(FT232HDevice *out_device, const char **out_error);

/**
 * @brief destroys a ft232h device
 * @param[in] device a pointer to the device that is going to be destroyed
 * @param[out] out_error a pointer to a const char* where the error will be stored, THIS CANNOT BE NULL, THIS MUST BE A VALID POINTER
 * @returns 0 on success
 * @returns non-zero on error
 */
extern "C"
int ft232h_destroy_device(FT232HDevice *device, const char **out_error);

/**
 * @brief gets the gpio state of the device
 * @param[in] device a pointer to the device
 * @returns a bitmask of the gpio
 */
extern "C"
uint16_t ft232h_get_gpio_state(FT232HDevice *device);

/**
 * @brief sets a gpio pin's state on a ft232h device. this doesn't actually send any data to the physical device, to do that make a call to ft232h_upload_gpio_state
 * @param[in] device a pointer to the device
 * @param[in] pin the number associated with the pin 0-7 corresponds with the d pins
 * @param[in] dir the direction you want the pin to have, input or output
 * @param[in] output_state if dir is output then this value is the output value of the pin, if dir is not output then this argument is ignored
 */
extern "C"
void ft232h_set_pin_state(FT232HDevice *device, int pin, FT232HPinDirection dir, FT232HPinOutputState output_state);

/**
 * @brief uploads the stored gpio state to the physical device
 * @param[in] device a pointer to the device
 * @returns 0 on success
 * @returns non-zero on error
 */
extern "C"
int ft232h_upload_gpio_state(FT232HDevice *device);
