/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: qSlicerBaseQWidgetsConfigure.h,v $
  Date:      $Date: 2006/01/06 17:56:51 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME qSlicerBaseQWidgetsConfigure - manage Windows system differences
// .SECTION Description
// The qSlicerBaseQWidgetsConfigure captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qCTKWidgetsWin32Header_h
#define __qCTKWidgetsWin32Header_h

#include <qCTKWidgetsConfigure.h>

#if defined(WIN32) && !defined(VTKSLICER_STATIC)
 #if defined(QCTKWidgets_EXPORTS)
  #define QCTK_WIDGETS_EXPORT __declspec( dllexport ) 
 #else
  #define QCTK_WIDGETS_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define QCTK_WIDGETS_EXPORT
#endif

#endif
