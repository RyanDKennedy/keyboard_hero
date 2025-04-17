#pragma once

#include "sy_ecs.hpp"

enum class SyCameraProjectionType
{
    orthographic,
    perspective
};

struct SyCameraSettings
{
    union
    {
	struct
	{
	    float fov;
	    float aspect_ratio;
	    float near_plane;
	    float far_plane;
	} perspective_settings;
	struct
	{
	    float left;
	    float right;
	    float top;
	    float bottom;
	    float near;
	    float far;
	} orthographic_settings;
    };

    SyEntityHandle active_camera;
    SyCameraProjectionType projection_type;

};
