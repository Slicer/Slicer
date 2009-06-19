/*==========================================================================

Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDAnalysis/vtkFourDAnalysisWin32Header.h $
Date:      $Date: 2009-01-05 13:23:32 -0500 (Mon, 05 Jan 2009) $
Version:   $Revision: 3499 $

==========================================================================*/

#ifndef __vtkFourDAnalysisWin32Header_h
#define __vtkFourDAnalysisWin32Header_h

#include <vtkFourDAnalysisConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
#if defined(FourDAnalysis_EXPORTS)
#define VTK_FourDAnalysis_EXPORT __declspec( dllexport ) 
#else
#define VTK_FourDAnalysis_EXPORT __declspec( dllimport ) 
#endif
#else
#define VTK_FourDAnalysis_EXPORT 
#endif
#endif
