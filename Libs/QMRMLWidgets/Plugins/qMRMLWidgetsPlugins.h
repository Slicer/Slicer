#ifndef __qMRMLWidgetsPlugins_h
#define __qMRMLWidgetsPlugins_h

#include "qMRMLLinearTransformSliderPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>

// \class Group the plugins in one library 
class qMRMLWidgetsPlugins : public QObject,
                           public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qMRMLLinearTransformSliderPlugin;
    return plugins;
    }
};

#endif
