#ifndef __qSlicerModulePanelPlugin_h
#define __qSlicerModulePanelPlugin_h

#include "qSlicerQTGUIAbstractPlugin.h"

class Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT qSlicerModulePanelPlugin : 
  public qSlicerQTGUIAbstractPlugin
{
public:
  qSlicerModulePanelPlugin();
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif
