#ifndef __qMRMLWidgetsPlugins_h
#define __qMRMLWidgetsPlugins_h

#include "qMRMLLinearTransformSliderPlugin.h"
#include "qMRMLMatrixWidgetPlugin.h"
#include "qMRMLNodeSelectorPlugin.h"
#include "qMRMLTransformSlidersPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>
#include "qMRMLWidgetsPluginWin32Header.h"

// \class Group the plugins in one library 
class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLWidgetsPlugins : public QObject,
                           public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qMRMLLinearTransformSliderPlugin
            << new qMRMLMatrixWidgetPlugin
            << new qMRMLNodeSelectorPlugin
            << new qMRMLTransformSlidersPlugin;
    return plugins;
    }
};

#endif
