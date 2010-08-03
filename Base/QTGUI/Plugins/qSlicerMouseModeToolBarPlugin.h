#ifndef __qSlicerMouseModeToolBarPlugin_h
#define __qSlicerMouseModeToolBarPlugin_h

#include "qSlicerQTGUIAbstractPlugin.h"

class Q_SLICER_BASE_QTGUI_PLUGIN_EXPORT qSlicerMouseModeToolBarPlugin :
  public qSlicerQTGUIAbstractPlugin
{
public:
  qSlicerMouseModeToolBarPlugin();

  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif
