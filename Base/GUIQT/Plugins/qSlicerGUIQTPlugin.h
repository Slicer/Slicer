#ifndef __qSlicerGUIQTPlugin_h
#define __qSlicerGUIQTPlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qSlicerGUIQTPluginWin32Header.h"

class Q_SLICER_GUIQT_PLUGIN_EXPORT qSlicerGUIQTPlugin : 
  public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface)
public:

  qSlicerGUIQTPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
