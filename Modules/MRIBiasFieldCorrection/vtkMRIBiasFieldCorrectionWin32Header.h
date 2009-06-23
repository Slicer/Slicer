/*=auto==============================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All
  Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRIBiasFieldCorrectionWin32Header.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $
  Author:  $Nicolas Rannou (BWH), Sylvain Jaume (MIT)$

==============================================================auto=*/
// .NAME vtkMRIBiasFieldCorrectionWin32Header - manage Windows system
// differences
// .SECTION Description
// The vtkMRIBiasFieldCorrectionWin32Header captures some system
// differences between Unix and Windows operating systems.

#ifndef __vtkMRIBiasFieldCorrectionWin32Header_h
#define __vtkMRIBiasFieldCorrectionWin32Header_h

#include <vtkMRIBiasFieldCorrectionConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(MRIBiasFieldCorrection_EXPORTS)
#define VTK_MRIBiasFieldCorrection_EXPORT __declspec( dllexport )
#else
#define VTK_MRIBiasFieldCorrection_EXPORT __declspec( dllimport )
#endif
#else
#define VTK_MRIBiasFieldCorrection_EXPORT
#endif

#endif

