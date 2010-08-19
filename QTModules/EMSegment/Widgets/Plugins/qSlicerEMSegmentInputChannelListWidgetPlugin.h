#ifndef __qSlicerEMSegmentInputChannelListWidgetPlugin_h
#define __qSlicerEMSegmentInputChannelListWidgetPlugin_h

#include "qSlicerEMSegmentWidgetsAbstractPlugin.h"

class Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT qSlicerEMSegmentInputChannelListWidgetPlugin :
  public QObject,
  public qSlicerEMSegmentWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qSlicerEMSegmentInputChannelListWidgetPlugin(QObject * newParent = 0);
  
  QWidget *createWidget(QWidget *newParent);
  QString  domXml() const;
  QIcon    icon() const;
  QString  includeFile() const;
  bool     isContainer() const;
  QString  name() const;
  
};

#endif
