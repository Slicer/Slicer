
// The qSlicerEMSegmentWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qSlicerEMSegmentWidgetsPluginExport_h
#define __qSlicerEMSegmentWidgetsPluginExport_h

#if defined(WIN32) && !defined(qSlicerEMSegmentWidgetsPlugin_STATIC)
 #if defined(qSlicerEMSegmentWidgetsPlugin_EXPORTS)
  #define Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT
#endif

#endif //__qSlicerEMSegmentWidgetsPluginExport_h
