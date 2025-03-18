#pragma once

#include <stdlib.h>

/**
 * @brief reads a file contents from the resources directory
 * @param filepath[in] the filepath to read from relative to the resources directory
 * @param out_size[out] the size of the buffer returned
 * @returns NULL on error
 * @returns a buffer that needs to be freed that contains the file's contents
 */
char* sy_read_resource_file(const char *filepath, size_t *out_size);
