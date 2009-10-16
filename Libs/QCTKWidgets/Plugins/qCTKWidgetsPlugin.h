#ifndef __qCTKWidgetsPlugin_h
#define __qCTKWidgetsPlugin_h

#include <QDesignerCustomWidgetInterface>

class qCTKWidgetsPlugin : public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface)
public:

  qCTKWidgetsPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
