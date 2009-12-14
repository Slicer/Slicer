#ifndef __qSlicerQTGUIAbstractPlugin_h
#define __qSlicerQTGUIAbstractPlugin_h

// QT includes
#include <QDesignerCustomWidgetInterface>

#include "qSlicerQTGUIPluginExport.h"

class Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT qSlicerQTGUIAbstractPlugin :
  public QDesignerCustomWidgetInterface
{
  Q_INTERFACES(QDesignerCustomWidgetInterface);
public:

  qSlicerQTGUIAbstractPlugin();
  // Don't reimplement this method.
  QString group() const;
  // You can reimplement these methods
  virtual QIcon icon() const;
  virtual QString toolTip() const;
  virtual QString whatsThis() const;
};

#endif
