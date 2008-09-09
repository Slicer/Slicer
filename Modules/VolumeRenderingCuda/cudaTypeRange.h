
#include <float.h>
#include <limits.h>


template <typename Type>
void GetTypeRange(Type& min, Type& max);

template <>
__device__ void GetTypeRange<unsigned char>(unsigned char& min, unsigned char& max)
{
    min = (unsigned char)0; 
    max = UCHAR_MAX;
}

template <>
__device__ void GetTypeRange<char>(char& min, char& max)
{
    min = CHAR_MIN; 
    max = CHAR_MAX;
}

template<>
__device__ void GetTypeRange<short>(short& min, short& max)
{
    min = SHRT_MIN; 
    max = SHRT_MAX;
}

template<>
__device__ void GetTypeRange<unsigned short>(unsigned short& min, unsigned short& max)
{
    min = (unsigned short)0; 
    max = USHRT_MAX;
}

template<>
__device__ void GetTypeRange<int>(int& min, int& max)
{
    min = INT_MIN;
    max = INT_MAX;
}

template <>
__device__ void GetTypeRange<float>(float& min, float& max)
{
    min = FLT_MIN;
    max = FLT_MAX;
}

template <>
__device__ void GetTypeRange<double>(double& min, double& max)
{
    min = DBL_MIN;
    max = DBL_MAX;
}
