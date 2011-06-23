/*==========================================================================

Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: $
Date:      $Date: $
Version:   $Revision: $

==========================================================================*/

#ifndef __vtkFourDImageWin32Header_h
#define __vtkFourDImageWin32Header_h

#include <vtkFourDImageConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(FourDImage_EXPORTS)
#define VTK_FourDImage_EXPORT __declspec( dllexport ) 
#else
#define VTK_FourDImage_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_FourDImage_EXPORT 
#endif
#endif
