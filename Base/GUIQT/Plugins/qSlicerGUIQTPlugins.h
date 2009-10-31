#ifndef __qSlicerGUIQTPlugins_h
#define __qSlicerGUIQTPlugins_h

#include "qSlicerWidgetPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>

// \class Group the plugins in one library 
class qSlicerGUIQTPlugins : public QObject,
                         public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qSlicerWidgetPlugin;
    return plugins;
    }
};

#endif

