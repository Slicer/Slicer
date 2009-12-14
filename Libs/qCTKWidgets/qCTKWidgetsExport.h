//
// The qCTKWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 
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

#endif // __qCTKWidgetsExport_h
