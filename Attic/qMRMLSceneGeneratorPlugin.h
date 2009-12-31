#ifndef __qMRMLSceneGeneratorPlugin_h
#define __qMRMLSceneGeneratorPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"

// qCTK includes
#include <qCTKPimpl.h>

class qMRMLSceneGenerator;
class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLSceneGeneratorPlugin : public QObject,
                                                              public qMRMLWidgetsAbstractPlugin
{
  Q_OBJECT

public:
  qMRMLSceneGeneratorPlugin(QObject *_parent = 0);
  
  QWidget *createWidget(QWidget *_parent);
  QString domXml() const;
  QString includeFile() const;
  bool isContainer() const;
  QString name() const;

private:
  friend class qMRMLSceneGenerator;
  qCTKPrivateInterface<qMRMLSceneGeneratorPlugin, qMRMLSceneGenerator> qctk_d;
};

#endif
