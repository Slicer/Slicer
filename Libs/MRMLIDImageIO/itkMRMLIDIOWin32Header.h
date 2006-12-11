/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D MRML
  Module:    $RCSfile: itkMRMLIDIOWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME itkMRMLIDIOWin32Header - manage Windows system differences
// .SECTION Description
// The itkMRMLIDIOWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __itkMRMLIDIOWin32Header_h
#define __itkMRMLIDIOWin32Header_h

#include <itkMRMLIDImageIOConfigure.h>

#if defined(WIN32) && !defined(MRMLIDIO_STATIC)
#if defined(MRMLIDIO_EXPORTS)
#define MRMLIDImageIO_EXPORT __declspec( dllexport ) 
#else
#define MRMLIDImageIO_EXPORT __declspec( dllimport ) 
#endif
#else
#define MRMLIDImageIO_EXPORT
#endif

#endif
