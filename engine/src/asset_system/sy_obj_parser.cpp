#include "sy_obj_parser.hpp"
#include "sy_macros.hpp"
#include "sy_utils.hpp"

#include <string.h>

#define SY_OBJ_LINE_TYPE_NONE -1
#define SY_OBJ_LINE_TYPE_VERTEX 0
#define SY_OBJ_LINE_TYPE_TEXTURE 1
#define SY_OBJ_LINE_TYPE_NORMAL 2
#define SY_OBJ_LINE_TYPE_FACE 3

int classify_line(char *line)
{
    int result = SY_OBJ_LINE_TYPE_NONE;

    // NOTE have them in alphabetical order, v should come before vn, because "vn" will match with v
    const size_t prefixes_amt = 4;
    const char *prefixes[prefixes_amt];
    prefixes[SY_OBJ_LINE_TYPE_VERTEX] = "v";
    prefixes[SY_OBJ_LINE_TYPE_NORMAL] = "vn";
    prefixes[SY_OBJ_LINE_TYPE_TEXTURE] = "vt";
    prefixes[SY_OBJ_LINE_TYPE_FACE] = "f";
    
    for (int i = 0; i < prefixes_amt; ++i)
    {
	if (strlen(line) < strlen(prefixes[i]))
	    continue;

	if (memcmp(line, prefixes[i], strlen(prefixes[i])) == 0)
	{
	    result = i;
	}
    }
    
    return result;
}

void parse_space_seperated_floats(int amt, char *start, float *out_values)
{
    int out_index = 0;
    for (int i = 0; i < strlen(start); ++i)
    {
	if (start[i] == ' ')
	{
	    out_values[out_index++] = atof(&start[i+1]);
	    if (out_index == amt)
		break;
	}
    }
}

void parse_indices_from_face(int amt, char *start, uint32_t *out_values)
{
    int out_index = 0;
    for (int i = 0; i < strlen(start); ++i)
    {
	if (start[i] == ' ')
	{
	    out_values[out_index++] = (uint32_t)(atol(&start[i+1]) - 1);
	    if (out_index == amt)
		break;
	}
    }    
}

int sy_parse_obj(const char *obj_path, float **out_vertices, size_t *out_vertices_size, uint32_t **out_indices, size_t *out_indices_size)
{
    *out_vertices = NULL;
    *out_indices = NULL;
    *out_vertices_size = 0;
    *out_indices_size = 0;

    size_t file_contents_size = 0;
    char *file_contents = sy_read_resource_file(obj_path, &file_contents_size);
    if (file_contents == NULL)
	return -1;

    // split file_contents into lines
    size_t lines_amt = 0;
    char **lines = NULL;
    {
	for (size_t i = 0; i < file_contents_size; ++i)
	{
	    if (file_contents[i] == '\n')
		++lines_amt;
	}
	
	lines = (char**)calloc(lines_amt, sizeof(char*));
	{
	    size_t line_index = 0;
	    lines[line_index++] = file_contents;
	    for (size_t i = 0; i < file_contents_size - 1; ++i)
	    {
		if (file_contents[i] == '\n')
		{
		    file_contents[i] = '\0';
		    lines[line_index++] = &file_contents[i + 1];
		}
	    }
	}
    }

    float *vertices = NULL;
    size_t vertices_amt = 0;
    size_t vertices_size = 0;

    // Count Vertices
    for (size_t line_num = 0; line_num < lines_amt; ++line_num)
    {
	if (classify_line(lines[line_num]) == SY_OBJ_LINE_TYPE_VERTEX)
	    ++vertices_amt;
    }
    vertices_size = vertices_amt * 3;
    
    // Load Vertices
    {
	vertices = (float*)calloc(vertices_size, sizeof(float));

	size_t vertex_number = 0;
	for (size_t line_num = 0; line_num < lines_amt; ++line_num)
	{
	    char *current_line = lines[line_num];

	    if (classify_line(current_line) == SY_OBJ_LINE_TYPE_VERTEX)
	    {
		parse_space_seperated_floats(3, current_line + strlen("v"), &vertices[vertex_number++ * 3]);

		if (vertex_number == vertices_amt)
		    break;
	    }
	}
    }

    uint32_t *indices = NULL;
    size_t indices_size = 0;

    // Count indices
    for (size_t line_num = 0; line_num < lines_amt; ++line_num)
    {
	if (classify_line(lines[line_num]) == SY_OBJ_LINE_TYPE_FACE)
	    indices_size += 3;
    }    
    
    // Load indices
    {
	indices = (uint32_t*)calloc(indices_size, sizeof(uint32_t));

	size_t face_number = 0;
	for (size_t line_num = 0; line_num < lines_amt; ++line_num)
	{
	    char *current_line = lines[line_num];

	    if (classify_line(current_line) == SY_OBJ_LINE_TYPE_FACE)
	    {
		parse_indices_from_face(3, current_line + strlen("f"), &indices[face_number++ * 3]);

		if (face_number * 3 == indices_size)
		    break;
	    }
	}
    }

    free(lines);
    free(file_contents);

    *out_vertices_size = vertices_size;
    *out_vertices = vertices;
    *out_indices_size = indices_size;
    *out_indices = indices;

    return 0;
}
