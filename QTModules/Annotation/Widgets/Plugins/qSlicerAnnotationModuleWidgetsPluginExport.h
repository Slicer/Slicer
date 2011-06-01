
// The qSlicerAnnotationModuleWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qSlicerAnnotationModuleWidgetsPluginExport_h
#define __qSlicerAnnotationModuleWidgetsPluginExport_h

#if defined(WIN32) && !defined(qSlicerAnnotationModuleWidgetsPlugin_STATIC)
 #if defined(qSlicerAnnotationModuleWidgetsPlugin_EXPORTS)
  #define Q_ANNOTATION_MODULE_WIDGETS_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define Q_ANNOTATION_MODULE_WIDGETS_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define Q_ANNOTATION_MODULE_WIDGETS_PLUGIN_EXPORT
#endif

#endif //__qSlicerAnnotationModuleWidgetsPlugin_h

