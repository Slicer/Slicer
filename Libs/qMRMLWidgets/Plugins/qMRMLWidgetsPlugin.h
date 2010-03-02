#ifndef __qMRMLWidgetsPlugin_h
#define __qMRMLWidgetsPlugin_h

#include "qMRMLLinearTransformSliderPlugin.h"
#include "qMRMLListWidgetPlugin.h"
#include "qMRMLMatrixWidgetPlugin.h"
#include "qMRMLNodeComboBoxPlugin.h"
#include "qMRMLNodeSelectorPlugin.h"
#include "qMRMLNodeTreeSelectorPlugin.h"
#include "qMRMLTransformSlidersPlugin.h"
#include "qMRMLTreeWidgetPlugin.h"
#include "qMRMLSceneFactoryWidgetPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>
#include "qMRMLWidgetsPluginExport.h"

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
            << new qMRMLListWidgetPlugin
            << new qMRMLMatrixWidgetPlugin
            << new qMRMLNodeComboBoxPlugin
            << new qMRMLNodeSelectorPlugin
            << new qMRMLNodeTreeSelectorPlugin
            << new qMRMLTransformSlidersPlugin
            << new qMRMLTreeWidgetPlugin
            << new qMRMLSceneFactoryWidgetPlugin;
    return plugins;
    }
};

#endif
