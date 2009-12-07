#ifndef __qSlicerModuleSelectorWidgetPlugin_h
#define __qSlicerModuleSelectorWidgetPlugin_h

#include "qSlicerQTGUIAbstractPlugin.h"

class Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT qSlicerModuleSelectorWidgetPlugin : 
  public qSlicerQTGUIAbstractPlugin
{
public:
  qSlicerModuleSelectorWidgetPlugin();
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif
