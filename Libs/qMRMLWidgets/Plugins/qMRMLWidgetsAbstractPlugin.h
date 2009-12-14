#ifndef __qMRMLWidgetsAbstractPlugin_h
#define __qMRMLWidgetsAbstractPlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qMRMLWidgetsPluginExport.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLWidgetsAbstractPlugin : public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qMRMLWidgetsAbstractPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
