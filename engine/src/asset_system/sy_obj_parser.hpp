#pragma once


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/**
 * @brief parses an obj file from a path relative to the resources directory into indices and vertices
 * @param obj_path[in] the path of the obj file relative to the resources directory
 * @param out_vertices[out] the output for the vertices, this needs to be freed
 * @param out_vertices_size[out] the output for the size of the out_vertices parameter
 * @param out_indices[out] the output for the indices, this needs to be freed
 * @param out_indices_size[out] the output for the size of the out_indices parameter
 * @returns 0 on success
 * @returns non-zero on error
 */
int sy_parse_obj(const char *obj_path, float **out_vertices, size_t *out_vertices_size, uint32_t **out_indices, size_t *out_indices_size);
