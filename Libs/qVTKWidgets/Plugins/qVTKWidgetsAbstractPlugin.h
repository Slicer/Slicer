#ifndef __qVTKWidgetsAbstractPlugin_h
#define __qVTKWidgetsAbstractPlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qVTKWidgetsPluginExport.h"

class QVTK_WIDGETS_PLUGIN_EXPORT qVTKWidgetsAbstractPlugin : public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qVTKWidgetsAbstractPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
