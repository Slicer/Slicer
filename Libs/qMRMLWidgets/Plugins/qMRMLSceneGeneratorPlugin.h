#ifndef __qMRMLSceneGeneratorPlugin_h
#define __qMRMLSceneGeneratorPlugin_h

#include "qMRMLWidgetsAbstractPlugin.h"
#include <qCTKPimpl.h>
#include <QPushButton>

class qMRMLSceneGeneratorPrivate;
class vtkMRMLScene;

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLSceneGenerator : public QPushButton
{
  Q_OBJECT
public:
  qMRMLSceneGenerator(QWidget* parent = 0);
  ~qMRMLSceneGenerator();
public slots:
  void generateScene();

signals:
  void mrmlSceneSet(vtkMRMLScene* scene);
private:
  QCTK_DECLARE_PRIVATE(qMRMLSceneGenerator);
};

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
  
};

#endif
