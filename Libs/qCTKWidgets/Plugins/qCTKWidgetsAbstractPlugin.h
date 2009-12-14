#ifndef __qCTKWidgetsAbstractPlugin_h
#define __qCTKWidgetsAbstractPlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qCTKWidgetsPluginExport.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKWidgetsAbstractPlugin : public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qCTKWidgetsAbstractPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;
  virtual void initialize(QDesignerFormEditorInterface *formEditor);
protected:
  bool Initialized;
};

#endif
