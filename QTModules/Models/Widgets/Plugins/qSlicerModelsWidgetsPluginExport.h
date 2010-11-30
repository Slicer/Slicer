
// The qSlicerModelsWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qSlicerModelsWidgetsPluginExport_h
#define __qSlicerModelsWidgetsPluginExport_h

#if defined(WIN32) && !defined(qSlicerModelsWidgetsPlugin_STATIC)
 #if defined(qSlicerModelsWidgetsPlugin_EXPORTS)
  #define Q_SLICER_QTMODULES_MODELS_WIDGETS_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define Q_SLICER_QTMODULES_MODELS_WIDGETS_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define Q_SLICER_QTMODULES_MODELS_WIDGETS_PLUGIN_EXPORT
#endif

#endif //__qSlicerModelsWidgetsPluginExport_h
