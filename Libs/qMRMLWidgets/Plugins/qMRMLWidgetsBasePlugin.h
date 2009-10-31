#ifndef __qMRMLWidgetsBasePlugin_h
#define __qMRMLWidgetsBasePlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qMRMLWidgetsPluginWin32Header.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLWidgetsBasePlugin : public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface)
public:

  qMRMLWidgetsBasePlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
