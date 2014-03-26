
#ifndef __qMRMLWidgetsExport_h
#define __qMRMLWidgetsExport_h

#if defined(WIN32) && !defined(qMRMLWidgets_STATIC)
 #if defined(qMRMLWidgets_EXPORTS)
  #define QMRML_WIDGETS_EXPORT __declspec( dllexport )
 #else
  #define QMRML_WIDGETS_EXPORT __declspec( dllimport )
 #endif
#else
 #define QMRML_WIDGETS_EXPORT
#endif

#endif //__qMRMLWidgetsExport_h
