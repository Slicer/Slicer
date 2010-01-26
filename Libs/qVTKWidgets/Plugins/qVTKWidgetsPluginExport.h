
// The qVTKWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qVTKWidgetsPluginExport_h
#define __qVTKWidgetsPluginExport_h

#if defined(WIN32) && !defined(qVTKWidgetsPlugin_STATIC)
 #if defined(qVTKWidgetsPlugin_EXPORTS)
  #define QVTK_WIDGETS_PLUGIN_EXPORT __declspec( dllexport )
 #else
  #define QVTK_WIDGETS_PLUGIN_EXPORT __declspec( dllimport )
 #endif
#else
 #define QVTK_WIDGETS_PLUGIN_EXPORT
#endif

#endif //__qVTKWidgetsPluginExport_h
