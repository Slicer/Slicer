#ifndef __qCTKWidgetsBasePlugin_h
#define __qCTKWidgetsBasePlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qCTKWidgetsPluginWin32Header.h"

class QCTK_WIDGETS_PLUGIN_EXPORT qCTKWidgetsBasePlugin : public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface)
public:

  qCTKWidgetsBasePlugin();
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
