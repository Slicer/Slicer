
// The qSlicerVolumeRenderingModuleWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qSlicerVolumeRenderingModuleWidgetsPluginExport_h
#define __qSlicerVolumeRenderingModuleWidgetsPluginExport_h

#if defined(WIN32) && !defined(qSlicerVolumeRenderingModuleWidgetsPlugin_STATIC)
 #if defined(qSlicerVolumeRenderingModuleWidgetsPlugin_EXPORTS)
  #define Q_VOLUMERENDERING_MODULE_WIDGETS_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define Q_VOLUMERENDERING_MODULE_WIDGETS_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define Q_VOLUMERENDERING_MODULE_WIDGETS_PLUGIN_EXPORT
#endif

#endif //__qSlicerVolumeRenderingModuleWidgetsPlugin_h

