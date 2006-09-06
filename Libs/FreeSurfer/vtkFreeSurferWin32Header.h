/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D MRML
  Module:    $RCSfile: vtkFreeSurferWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkFreeSurferWin32Header - manage Windows system differences
// .SECTION Description
// The vtkFreeSurferWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkFreeSurferWin32Header_h
#define __vtkFreeSurferWin32Header_h

#include <FreeSurferConfigure.h>

#if defined(WIN32) && !defined(FreeSurfer_STATIC)
#if defined(FreeSurfer_EXPORTS)
#define VTK_FreeSurfer_EXPORT __declspec( dllexport ) 
#else
#define VTK_FreeSurfer_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_FreeSurfer_EXPORT
#endif

#endif
