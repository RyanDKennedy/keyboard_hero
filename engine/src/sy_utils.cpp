#include "sy_utils.hpp"
#include "sy_macros.hpp"

#include <stdio.h>

char* sy_read_resource_file(const char *filepath, size_t *out_size)
{
    // get filepath with directory path prepended
    char new_filepath[256];
    snprintf(new_filepath, 256, "%s%s", "./resources/", filepath);

    FILE *fd = fopen(new_filepath, "r");
    if (fd == NULL)
    {
	return NULL;
    }

    // get size of file
    fseek(fd, 0, SEEK_END);
    size_t size = ftell(fd);
    rewind(fd);


    
    // allocate and read into buffer
    char *result = (char*) malloc(sizeof(char) * (size + 1));
    size_t data_read = 0;
    while (data_read < size)
    {
	data_read += fread(result+data_read, sizeof(char), size-data_read, fd);

	// check for errors
	if (ferror(fd) != 0)
	{
	    free(result);
	    fclose(fd);
	    return NULL;
	}
    }

    fclose(fd);

    result[data_read] = '\0';
    
    *out_size = data_read;    
    return result;
}
