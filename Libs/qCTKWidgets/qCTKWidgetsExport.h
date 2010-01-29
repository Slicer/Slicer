/*=========================================================================

  Library:   qCTK

  Copyright (c) Kitware Inc. 
  All rights reserved.
  Distributed under a BSD License. See LICENSE.txt file.

  This software is distributed "AS IS" WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the above copyright notice for more information.

=========================================================================*/

//
/// The qCTKWidgetsPluginExport captures some system differences between Unix
/// and Windows operating systems. 
//

#ifndef __qCTKWidgetsExport_h
#define __qCTKWidgetsExport_h

#if defined(WIN32) && !defined(qCTKWidgets_STATIC)
 #if defined(qCTKWidgets_EXPORTS)
  #define QCTK_WIDGETS_EXPORT __declspec( dllexport ) 
 #else
  #define QCTK_WIDGETS_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define QCTK_WIDGETS_EXPORT
#endif

#endif /// __qCTKWidgetsExport_h
