/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkFilterWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkFilterWin32Header - manage Windows system differences
// .SECTION Description
// The vtkFilterWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkMeshIOWin32Header_h
#define __vtkMeshIOWin32Header_h

#include <vtkMeshIOConfigure.h>

#if defined(WIN32) && !defined(VTKMIMXIO_STATIC)
#if defined(MimxMeshIO_EXPORTS)
#define VTK_MIMXIO_EXPORT __declspec( dllexport )
#else
#define VTK_MIMXIO_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_MIMXFILTER_EXPORT 
#endif

#endif
