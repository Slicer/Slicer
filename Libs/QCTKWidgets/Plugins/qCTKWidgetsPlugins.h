#ifndef __qCTKWidgetsPlugins_h
#define __qCTKWidgetsPlugins_h

#include "qCTKCollapsibleGroupBoxPlugin.h"
#include "qCTKCollapsibleWidgetPlugin.h"
#include "qCTKColorPickerButtonPlugin.h"
#include "qCTKCoordinatesWidgetPlugin.h"
#include "qCTKTitleComboBoxPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>

// \class Group the plugins in one library 
class qCTKWidgetsPlugins : public QObject,
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
            << new qCTKTitleComboBoxPlugin;
    return plugins;
    }
};

#endif
