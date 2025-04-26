// sy_macros.hpp

#pragma once

#include <stdio.h>

#define SY_ERROR_OUTPUT(TEXT, ...) fprintf(stderr, "\n\033[31mERROR\033[0m - %s [%d]:\n\033[31m" TEXT "\033[0m\n", __FILE__ , __LINE__  __VA_OPT__(,) __VA_ARGS__);

// requires importing stdio.h
#define SY_ERROR(TEXT, ...)						\
    {									\
        SY_ERROR_OUTPUT(TEXT, __VA_ARGS__);				\
	exit(1);							\
    }

// requires importing stdio.h
#define SY_OUTPUT_INFO(TEXT, ...)					\
    {									\
	fprintf(stdout, "\n\033[32mINFORMATION\033[0m:\n" TEXT "\033[0m\n" __VA_OPT__(,) __VA_ARGS__); \
    }


#define SY_ERROR_COND(condition, ...) if (condition) { SY_ERROR(__VA_ARGS__); }

#ifndef NDEBUG // Debug Mode

#define SY_ASSERT(cond, ...) if (!(cond)) { SY_ERROR(#cond" failed." __VA_OPT__(,) __VA_ARGS__) }

#define SY_OUTPUT_DEBUG(TEXT, ...)					\
    {									\
	fprintf(stdout, "\n\033[32mDEBUG\033[0m:\n" TEXT "\033[0m\n" __VA_OPT__(,) __VA_ARGS__); \
    }

#else // Release mode

#define SY_ASSERT(cond, ...)
#define SY_OUTPUT_DEBUG(...)

#endif

#define SY_ARRLEN(arr) (sizeof(arr) / sizeof(arr[0]))
