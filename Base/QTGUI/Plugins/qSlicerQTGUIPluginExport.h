
// The qSlicerQTGUIPluginExport captures some system differences between Unix
// and Windows operating systems. 

#ifndef __qSlicerQTGUIPluginExport_h
#define __qSlicerQTGUIPluginExport_h

#if defined(WIN32) && !defined(qSlicerBaseQTGUIPlugin_STATIC)
 #if defined(qSlicerBaseQTGUIPlugin_EXPORTS)
  #define Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT __declspec( dllexport ) 
 #else
  #define Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT __declspec( dllimport ) 
 #endif
#else
 #define Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT
#endif

#endif //__qSlicerQTGUIPluginExport_h
