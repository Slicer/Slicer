#ifndef __qSlicerQTGUIPlugins_h
#define __qSlicerQTGUIPlugins_h

#include "qSlicerWidgetPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>

// \class Group the plugins in one library 
class qSlicerQTGUIPlugins : public QObject,
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

