#ifndef __qSlicerQTGUIPlugins_h
#define __qSlicerQTGUIPlugins_h

#include "qSlicerWidgetPlugin.h"
#include "qSlicerModulePanelPlugin.h"
#include "qSlicerModuleSelectorWidgetPlugin.h"
#include "qSlicerQTGUIPluginWin32Header.h"
#include <QDesignerCustomWidgetCollectionInterface>

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
    plugins << new qSlicerModuleSelectorWidgetPlugin; 
    return plugins;
    }
};

#endif

