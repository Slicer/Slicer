#ifndef __qCTKWidgetsPlugins_h
#define __qCTKWidgetsPlugins_h

#include "qCTKWidgetsPluginWin32Header.h"
#include "qCTKCollapsibleGroupBoxPlugin.h"
#include "qCTKCollapsibleWidgetPlugin.h"
#include "qCTKColorPickerButtonPlugin.h"
#include "qCTKCoordinatesWidgetPlugin.h"
#include "qCTKTitleComboBoxPlugin.h"
#include "qCTKNumericInputPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>

// \class Group the plugins in one library 
class QCTK_WIDGETS_PLUGIN_EXPORT qCTKWidgetsPlugins : public QObject,
                           public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qCTKCollapsibleGroupBoxPlugin
            << new qCTKCollapsibleWidgetPlugin
            << new qCTKColorPickerButtonPlugin
            << new qCTKCoordinatesWidgetPlugin
            << new qCTKTitleComboBoxPlugin
            << new qCTKNumericInputPlugin; 
    return plugins;
    }
};

#endif
