#ifndef __qSlicerEMSegmentWidgetsPlugin_h
#define __qSlicerEMSegmentWidgetsPlugin_h

#include "qSlicerEMSegmentWidgetPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>
#include "qSlicerEMSegmentWidgetsPluginExport.h"

// \class Group the plugins in one library
class Q_SLICER_QTMODULES_EMSEGMENT_WIDGETS_PLUGIN_EXPORT qSlicerEMSegmentWidgetsPlugin : 
  public QObject,
  public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qSlicerEMSegmentWidgetPlugin;
    return plugins;
    }
};

#endif
