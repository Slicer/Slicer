/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D MRML
  Module:    $RCSfile: itkMGHImageIOWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME itkMGHImageIOWin32Header - manage Windows system differences
// .SECTION Description
// The itkMGHImageIOWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __itkMGHImageIOWin32Header_h
#define __itkMGHImageIOWin32Header_h

#include <itkMGHImageIOConfigure.h>

#if defined(WIN32) && !defined(MGHIO_STATIC)
#if defined(MGHIO_EXPORTS)
#define MGHImageIO_EXPORT __declspec( dllexport ) 
#else
#define MGHImageIO_EXPORT __declspec( dllimport ) 
#endif
#else
#define MGHImageIO_EXPORT
#endif

#endif
