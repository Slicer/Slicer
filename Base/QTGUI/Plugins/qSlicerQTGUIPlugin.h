#ifndef __qSlicerQTGUIPlugin_h
#define __qSlicerQTGUIPlugin_h

#include <QDesignerCustomWidgetInterface>
#include "qSlicerQTGUIPluginWin32Header.h"

class Q_SLICER_QTGUI_PLUGIN_EXPORT qSlicerQTGUIPlugin : 
  public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface)
public:

  qSlicerQTGUIPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;

};

#endif
