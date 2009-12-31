#ifndef __qMRMLSceneGeneratorButtonPlugin_h
#define __qMRMLSceneGeneratorButtonPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLSceneGeneratorButtonPlugin : public QObject,
                                                              public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLSceneGeneratorButtonPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;
};

#endif
