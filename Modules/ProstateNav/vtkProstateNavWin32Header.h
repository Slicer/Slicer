/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkProstateNavWin32Header_h
#define __vtkProstateNavWin32Header_h

#include <vtkProstateNavConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(ProstateNav_EXPORTS)
#define VTK_PROSTATENAV_EXPORT __declspec( dllexport ) 
#else
#define VTK_PROSTATENAV_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_PROSTATENAV_EXPORT 
#endif
#endif
