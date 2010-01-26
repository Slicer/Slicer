#ifndef __qVTKWidgetsPlugin_h
#define __qVTKWidgetsPlugin_h

#include "qVTKRenderViewPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>
#include "qVTKWidgetsPluginExport.h"

// \class Group the plugins in one library
class QVTK_WIDGETS_PLUGIN_EXPORT qVTKWidgetsPlugin : public QObject,
                           public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qVTKRenderViewPlugin;
    return plugins;
    }
};

#endif
