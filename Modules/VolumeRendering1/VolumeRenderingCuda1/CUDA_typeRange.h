#ifndef CUDAGETTYPERANGE_H
#define CUDAGETTYPERANGE_H

#include <float.h>
#include <limits.h>


template <typename Type>
void GetTypeRange(float& min, float& max);

template <>
void GetTypeRange<unsigned char>(float& min, float& max)
{
  min = 0; 
  max = UCHAR_MAX;
}

template <>
void GetTypeRange<char>(float& min, float& max)
{
  min = CHAR_MIN; 
  max = CHAR_MAX;
}

template<>
void GetTypeRange<short>(float& min, float& max)
{
  min = SHRT_MIN; 
  max = SHRT_MAX;
}

template<>
void GetTypeRange<unsigned short>(float& min, float& max)
{
  min = 0; 
  max = USHRT_MAX;
}

template<>
void GetTypeRange<int>(float& min, float& max)
{
  min = INT_MIN;
  max = INT_MAX;
}

template <>
void GetTypeRange<float>(float& min, float& max)
{
  min = FLT_MIN;
  max = FLT_MAX;
}

/*
template <>
void GetTypeRange<double>(float& min, float& max)
{
  min = DBL_MIN;
  max = DBL_MAX;
}
*/

#endif
