#ifndef __qSlicerEMSegmentWidgetPlugin_h
#define __qSlicerEMSegmentWidgetPlugin_h

#include "qSlicerEMSegmentWidgetsAbstractPlugin.h"

class Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT qSlicerEMSegmentWidgetPlugin :
  public QObject,
  public qSlicerEMSegmentWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qSlicerEMSegmentWidgetPlugin(QObject *newParent = 0);
  
  QWidget *createWidget(QWidget *newParent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif
