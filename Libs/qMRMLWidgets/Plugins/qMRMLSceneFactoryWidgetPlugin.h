#ifndef __qMRMLSceneFactoryWidgetPlugin_h
#define __qMRMLSceneFactoryWidgetPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLSceneFactoryWidgetPlugin : public QObject,
                                                              public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLSceneFactoryWidgetPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif
