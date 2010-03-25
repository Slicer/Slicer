#ifndef __qCTKWidgetsPlugin_h
#define __qCTKWidgetsPlugin_h

#include "qCTKWidgetsPluginExport.h"
#include "qCTKCollapsibleButtonPlugin.h"
#include "qCTKCollapsibleGroupBoxPlugin.h"
#include "qCTKColorPickerButtonPlugin.h"
#include "qCTKCoordinatesWidgetPlugin.h"
#include "qCTKDoubleRangeSliderPlugin.h"
#include "qCTKDoubleSliderPlugin.h"
#include "qCTKDynamicSpacerPlugin.h"
#include "qCTKFittedTextBrowserPlugin.h"
#include "qCTKMenuButtonPlugin.h"
#include "qCTKRangeSliderPlugin.h"
#include "qCTKRangeWidgetPlugin.h"
#include "qCTKTitleComboBoxPlugin.h"
#include "qCTKTreeComboBoxPlugin.h"
#include "qCTKSliderSpinBoxWidgetPlugin.h"

#include <QDesignerCustomWidgetCollectionInterface>

// \class Group the plugins in one library
class QCTK_WIDGETS_PLUGIN_EXPORT qCTKWidgetsPlugin : public QObject,
                           public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);

public:
  QList<QDesignerCustomWidgetInterface*> customWidgets() const
    {
    QList<QDesignerCustomWidgetInterface *> plugins;
    plugins << new qCTKCollapsibleButtonPlugin
            << new qCTKCollapsibleGroupBoxPlugin
            << new qCTKColorPickerButtonPlugin
            << new qCTKCoordinatesWidgetPlugin
            << new qCTKDoubleRangeSliderPlugin
            << new qCTKDoubleSliderPlugin
            << new qCTKDynamicSpacerPlugin
            << new qCTKFittedTextBrowserPlugin
            << new qCTKMenuButtonPlugin
            << new qCTKRangeSliderPlugin
            << new qCTKRangeWidgetPlugin
            << new qCTKTitleComboBoxPlugin
            << new qCTKTreeComboBoxPlugin
            << new qCTKSliderSpinBoxWidgetPlugin;
    return plugins;
    }
};

#endif
