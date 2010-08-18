#ifndef __qSlicerEMSegmentAnatomicalTreeWidgetPlugin_h
#define __qSlicerEMSegmentAnatomicalTreeWidgetPlugin_h

#include "qSlicerEMSegmentWidgetsAbstractPlugin.h"

class Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT qSlicerEMSegmentAnatomicalTreeWidgetPlugin :
  public QObject,
  public qSlicerEMSegmentWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qSlicerEMSegmentAnatomicalTreeWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif
