/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D MRML
  Module:    $RCSfile: itkCommandIOWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME itkCommandIOWin32Header - manage Windows system differences
// .SECTION Description
// The itkCommandIOWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __itkCommandIOWin32Header_h
#define __itkCommandIOWin32Header_h

#include <itkCommandIOConfigure.h>

#if defined(WIN32) && !defined(ITKCOMMANDIO_STATIC)
#if defined(ITKCommandIO_EXPORTS)
#define ITKCOMMANDIO_EXPORT __declspec( dllexport ) 
#else
#define ITKCOMMANDIO_EXPORT __declspec( dllimport ) 
#endif
#else
#define ITKCOMMANDIO_EXPORT
#endif

#endif
