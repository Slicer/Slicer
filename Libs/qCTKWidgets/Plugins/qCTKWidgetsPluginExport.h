//
// The qCTKWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 
//

#ifndef __qCTKWidgetsPluginExport_h
#define __qCTKWidgetsPluginExport_h

#if defined(WIN32) && !defined(qCTKWidgetsPlugin_STATIC)
 #if defined(qCTKWidgetsPlugin_EXPORTS)
  #define QCTK_WIDGETS_PLUGIN_EXPORT __declspec( dllexport )
 #else
  #define QCTK_WIDGETS_PLUGIN_EXPORT __declspec( dllimport )
 #endif
#else
 #define QCTK_WIDGETS_PLUGIN_EXPORT
#endif

#endif // __qCTKWidgetsPluginExport_h
