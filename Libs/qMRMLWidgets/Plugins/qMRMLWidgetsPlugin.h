#ifndef __qMRMLWidgetsPlugin_h
#define __qMRMLWidgetsPlugin_h

#include "qMRMLLinearTransformSliderPlugin.h"
#include "qMRMLMatrixWidgetPlugin.h"
#include "qMRMLNodeSelectorPlugin.h"
#include "qMRMLNodeTreeSelectorPlugin.h"
#include "qMRMLTransformSlidersPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>
#include "qMRMLWidgetsPluginWin32Header.h"

// \class Group the plugins in one library
class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLWidgetsPlugin : public QObject,
                           public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qMRMLLinearTransformSliderPlugin
            << new qMRMLMatrixWidgetPlugin
            << new qMRMLNodeSelectorPlugin
            << new qMRMLNodeTreeSelectorPlugin
            << new qMRMLTransformSlidersPlugin;
    return plugins;
    }
};

#endif
