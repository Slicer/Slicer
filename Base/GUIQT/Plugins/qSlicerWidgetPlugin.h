#ifndef __qSlicerWidgetPlugin_h
#define __qSlicerWidgetPlugin_h

#include "qSlicerGUIQTPlugin.h"
#include "qSlicerGUIQTPluginWin32Header.h"

class Q_SLICER_GUIQT_PLUGIN_EXPORT qSlicerWidgetPlugin : 
  public QObject,
  public qSlicerGUIQTPlugin
{
  Q_OBJECT

public:
  qSlicerWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif

