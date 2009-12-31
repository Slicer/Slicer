#ifndef __qMRMLSceneGeneratorPlugin_p_h
#define __qMRMLSceneGeneratorPlugin_p_h

#include "qMRMLSceneGeneratorPlugin.h"

// qCTK includes
#include <qCTKPimpl.h>

// QT includes
#include <QPushButton>

#include "qMRMLWidgetsPluginExport.h"

class vtkMRMLScene;

class QMRML_WIDGETS_PLUGIN_EXPORT qMRMLSceneGenerator :
      public QPushButton, public qCTKPrivate<qMRMLSceneGeneratorPlugin>
{
  Q_OBJECT
public:
  qMRMLSceneGenerator(QWidget* parent = 0);
  ~qMRMLSceneGenerator();

  void init();
  void clear();
  
public slots:
  void generateScene();

signals:
  void mrmlSceneSet(vtkMRMLScene* scene);
  void randomMRMLNodeType(const QString& name);

public:
  vtkMRMLScene*  MRMLScene;
};

#endif
