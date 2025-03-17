#include "sy_utils.hpp"
#include "sy_macros.hpp"

#include <stdio.h>

char* sy_read_resource_file(const char *filepath, size_t *size)
{
    // Open file
    char new_filepath[256];
    snprintf(new_filepath, 256, "%s%s", "./resources/", filepath);

    FILE *fd = fopen(new_filepath, "r");

    SY_ERROR_COND(fd == NULL, "Failed to read file %s", new_filepath);

    fseek(fd, 0, SEEK_END);
    *size = ftell(fd);
    rewind(fd);
    
    char *result = (char*) malloc(sizeof(char) * *size);
    fread(result, sizeof(char), *size, fd);
    
    fclose(fd);

    return result;
}
