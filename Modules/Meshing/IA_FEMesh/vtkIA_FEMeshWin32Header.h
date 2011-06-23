/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkIA_FEMeshWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkIA_FEMeshWin32Header - manage Windows system differences
// .SECTION Description
// The vtkIA_FEMeshWin32Header captures some system differences between Unix
// and Windows operating systems. 

#ifndef __vtkIA_FEMeshWin32Header_h
#define __vtkIA_FEMeshWin32Header_h

//#include <vtkIA_FEMeshConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(IA_FEMesh_EXPORTS)
#define VTK_IA_FEMesh_EXPORT __declspec( dllexport ) 
#else
#define VTK_IA_FEMesh_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_IA_FEMesh_EXPORT 
#endif

#endif
