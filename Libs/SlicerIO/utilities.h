/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/


#ifndef _UTILITIES_H
#define _UTILITIES_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>


/* Utility functions */
int formatStringLength(const char *format, ...);
int vaFormatStringLength(const char *format, va_list ap);


#ifdef __cplusplus
}
#endif


#endif /* _UTILITIES_H */


