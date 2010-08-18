#ifndef __qSlicerEMSegmentWidgetsAbstractPlugin_h
#define __qSlicerEMSegmentWidgetsAbstractPlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qSlicerEMSegmentWidgetsPluginExport.h"

class Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT qSlicerEMSegmentWidgetsAbstractPlugin : 
  public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qSlicerEMSegmentWidgetsAbstractPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
