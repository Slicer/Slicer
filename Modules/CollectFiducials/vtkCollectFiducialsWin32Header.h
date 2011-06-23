/*==========================================================================

Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: $
Date:      $Date: $
Version:   $Revision: $

==========================================================================*/

#ifndef __vtkCollectFiducialsWin32Header_h
#define __vtkCollectFiducialsWin32Header_h

#include <vtkCollectFiducialsConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(CollectFiducials_EXPORTS)
#define VTK_CollectFiducials_EXPORT __declspec( dllexport ) 
#else
#define VTK_CollectFiducials_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_CollectFiducials_EXPORT 
#endif
#endif
