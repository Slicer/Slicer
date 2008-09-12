/*=========================================================================

  Program:   Open IGT Link Library
  Module:    $RCSfile: $
  Language:  C
  Date:      $Date: $
  Version:   $Revision: $

  Copyright (c) Insight Software Consortium. All rights reserved.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __IGTL_TYPES_H
#define __IGTL_TYPES_H


/* 8-bit integer type */
#if IGTL_SIZEOF_CHAR == 1
  typedef unsigned char igtl_uint8;
  typedef char          igtl_int8;
#else
  # error "No native data type can represent an 8-bit integer."
#endif

/* 16-bit integer type */
#if IGTL_SIZEOF_SHORT == 2
  typedef unsigned short igtl_uint16;
  typedef signed short   igtl_int16;
#elif IGTL_SIZEOF_INT == 2
  typedef unsigned int   igtl_uint16;
  typedef signed int     igtl_int16;
#else
  # error "No native data type can represent a 16-bit integer."
#endif

/* 32-bit integer type */
#if IGTL_SIZEOF_INT == 4
  typedef unsigned int   igtl_uint32;
  typedef signed int     igtl_int32;
#elif IGTL_SIZEOF_LONG == 4
  typedef unsigned long  igtl_uint32;
  typedef signed long    igtl_int32;
#else
  # error "No native data type can represent a 32-bit integer."
#endif

/* 64-bit integer type */
#if defined(IGTL_TYPE_USE_LONG_LONG) && IGTL_SIZEOF_LONG_LONG == 8
  typedef unsigned long long igtl_uint64;
  typedef signed long long   igtl_int64;
#elif IGTL_SIZEOF_INT == 8
  typedef unsigned int       igtl_uint64;
  typedef signed int         igtl_int64;
#elif IGTL_SIZEOF_LONG == 8
  typedef unsigned long      igtl_uint64;
  typedef signed long        igtl_64;
#elif defined(IGTL_TYPE_USE___INT64) && IGTL_SIZEOF___INT64 == 8
  typedef unsigned __int64   igtl_uint64;
  typedef signed __int64     igtl_int64;
#elif defined(IGTL_TYPE_USE_INT64_T) && IGTL_SIZEOF_INT64_T == 8
  typedef unsigned int64_t   igtl_uint64;
  typedef signed int64_t     igtl_int64;
#else
  # error "No native data type can represent a 64-bit integer."
#endif

/* 32-bit floating point type */
#if IGTL_SIZEOF_FLOAT == 4
  typedef float              igtl_float32;
#else
# error "No native data type can represent a 32-bit floating point value."
#endif

/* 64-bit floating point type */
#if IGTL_SIZEOF_DOUBLE == 8
  typedef double             igtl_float64;
#else
# error "No native data type can represent a 64-bit floating point value."
#endif


#endif /* __IGTL_TYPES_H */
