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

#ifndef __vtkFilterWin32Header_h
#define __vtkFilterWin32Header_h

#include <vtkFilterConfigure.h>

#if defined(WIN32) && !defined(VTKMIMXFilter_STATIC)
#if defined(mimxFilter_EXPORTS)
#define VTK_MIMXFILTER_EXPORT __declspec( dllexport ) 
#else
#define VTK_MIMXFILTER_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_MIMXFILTER_EXPORT 
#endif

#endif
