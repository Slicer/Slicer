
// The qSlicerEMSegmentWorkflowWidgetsPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qSlicerEMSegmentWorkflowWidgetsPluginExport_h
#define __qSlicerEMSegmentWorkflowWidgetsPluginExport_h

#if defined(WIN32) && !defined(qSlicerEMSegmentWorkflowWidgetsPlugin_STATIC)
 #if defined(qSlicerEMSegmentWorkflowWidgetsPlugin_EXPORTS)
  #define Q_SLICER_QTMODULES_EMSEGMENT_WORKFLOW_WIDGETS_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define Q_SLICER_QTMODULES_EMSEGMENT_WORKFLOW_WIDGETS_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define Q_SLICER_QTMODULES_EMSEGMENT_WORKFLOW_WIDGETS_PLUGIN_EXPORT
#endif

#endif //__qSlicerEMSegmentWorkflowWidgetsPluginExport_h
