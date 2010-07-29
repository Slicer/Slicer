#ifndef __qSlicerQTGUIPlugins_h
#define __qSlicerQTGUIPlugins_h

#include "qSlicerWidgetPlugin.h"
#include "qSlicerModulePanelPlugin.h"

// QT includes
#include <QDesignerCustomWidgetCollectionInterface>

#include "qSlicerQTGUIPluginExport.h"

// \class Group the plugins in one library
class Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT qSlicerQTGUIPlugins : public QObject,
                         public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qSlicerWidgetPlugin;
    plugins << new qSlicerModulePanelPlugin;
    return plugins;
    }
};

#endif

