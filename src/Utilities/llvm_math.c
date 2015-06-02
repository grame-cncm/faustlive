#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// THIS IS THE SOURCE CODE TO CREATE llvm_math.ll TO ADD MISSING FUNCTIONS IN WINDOWS
// clang -emit-llvm -O3 llvm_math.c -c -S -o llvm_math.ll

void* llvm_malloc(size_t size)
{ 
    printf("SIZE OF MALLOC = %zi\n", size);
    return malloc(size); 
}

void llvm_free(void *ptr)
{
    free(ptr);
}

float fabsf(float x)
{ 
    return (float)fabs((double)x); 
}

float acosf(float x)
{ 
    return (float)acos((double)x); 
}

float asinf(float x)
{ 
    return (float)asin((double)x); 
}

float atanf(float x)
{ 
    return (float)atan((double)x); 
}

float atan2f(float x, float y)
{ 
    return (float)atan2((double)x, (double)y); 
}

float ceilf(float x)
{ 
    return (float)ceil((double)x); 
}

float cosf(float x)
{ 
    return (float)cos((double)x); 
}

float coshf(float x)
{ 
    return (float)cosh((double)x); 
}

float expf(float x)
{ 
    return (float)exp((double)x); 
}

float floorf(float x)
{ 
    return (float)floor((double)x); 
}

float fmodf(float x, float y)
{ 
    return (float)fmod((double)x, (double)y); 
}

float logf(float x)
{ 
    return (float)log((double)x); 
}

float log10f(float x)
{ 
    return (float)log10((double)x); 
}

float powf(float x, float y)
{ 
    return (float)pow((double)x, (double)y); 
}

float roundf(float x)
{ 
    return (float)round((double)x); 
}

float sinf(float x)
{ 
    return (float)sin((double)x); 
}

float sinhf(float x)
{ 
    return (float)sinh((double)x); 
}

float sqrtf(float x)
{ 
    return (float)sqrt((double)x); 
}

float tanf(float x)
{ 
    return (float)tan((double)x); 
}

float tanhf(float x)
{ 
    return (float)tanh((double)x); 
}


