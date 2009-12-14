
// The qMRMLWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qMRMLWidgetsPluginExport_h
#define __qMRMLWidgetsPluginExport_h

#if defined(WIN32) && !defined(qMRMLWidgetsPlugin_STATIC)
 #if defined(qMRMLWidgetsPlugin_EXPORTS)
  #define QMRML_WIDGETS_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define QMRML_WIDGETS_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define QMRML_WIDGETS_PLUGIN_EXPORT
#endif

#endif //__qMRMLWidgetsPluginExport_h
