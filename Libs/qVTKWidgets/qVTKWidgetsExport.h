
/// The qVTKWidgetsExport captures some system differences between Unix
/// and Windows operating systems. 

#ifndef __qVTKWidgetsExport_h
#define __qVTKWidgetsExport_h

#if defined(WIN32) && !defined(qVTKWidgets_STATIC)
 #if defined(qVTKWidgets_EXPORTS)
  #define QVTK_WIDGETS_EXPORT __declspec( dllexport )
 #else
  #define QVTK_WIDGETS_EXPORT __declspec( dllimport )
 #endif
#else
 #define QVTK_WIDGETS_EXPORT
#endif

#endif
