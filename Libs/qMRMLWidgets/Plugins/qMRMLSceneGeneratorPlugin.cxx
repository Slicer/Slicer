#include "qMRMLSceneGeneratorPlugin.h"

#include <QDebug>
#include <vtkMRMLScene.h>
#include "qMRMLNodeFactory.h"

class qMRMLSceneGeneratorPrivate: public qCTKPrivate<qMRMLSceneGenerator>
{
public:
  QCTK_DECLARE_PUBLIC(qMRMLSceneGenerator);
  void init()
  {
    QCTK_P(qMRMLSceneGenerator);
    this->MRMLScene = 0;
    p->connect(p, SIGNAL(clicked()), p, SLOT(generateScene()));
  }
  
  vtkMRMLScene* MRMLScene;
};

qMRMLSceneGenerator::qMRMLSceneGenerator(QWidget* parent)
  :QPushButton("Generate New Scene", parent)
{
  QCTK_INIT_PRIVATE(qMRMLSceneGenerator);
  qctk_d()->init();
}

qMRMLSceneGenerator::~qMRMLSceneGenerator()
{
  QCTK_D(qMRMLSceneGenerator);
  if (d->MRMLScene)
    {
    d->MRMLScene = 0;
    emit mrmlSceneSet(d->MRMLScene);
    }
}

void qMRMLSceneGenerator::generateScene()
{
  QCTK_D(qMRMLSceneGenerator);
  if (d->MRMLScene)
    {
    d->MRMLScene->Delete();
    }
  d->MRMLScene = vtkMRMLScene::New();
  
  qMRMLNodeFactory factory(this);
  factory.setMRMLScene(d->MRMLScene);
  
  int numClasses = d->MRMLScene->GetNumberOfRegisteredNodeClasses();
  int numNodes = 15;
  for (int i = 0; i < numNodes ; ++i)
    {
    vtkMRMLNode* node = d->MRMLScene->GetNthRegisteredNodeClass(rand() % numClasses);
    factory.createNode(node->GetClassName());
    }
  
/*
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLCameraNode");

  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLLinearTransformNode");
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLCameraNode");
*/

/*
  factory.createNode("vtkMRMLNonlinearTransformNode");
  factory.createNode("vtkMRMLModelHierarchyNode");
  factory.createNode("vtkMRMLColorNode");
  factory.createNode("vtkMRMLLinearTransformNode");
  factory.createNode("vtkMRMLModelHierarchyNode");
  factory.createNode("vtkMRMLColorNode");
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLCameraNode");
  factory.createNode("vtkMRMLFiducialListNode");
  factory.createNode("vtkMRMLLayoutNode");
#ifdef MRML_USE_vtkTeem
  factory.createNode("vtkMRMLNRRDStorageNode");
#endif
*/
  qDebug() << "Scene generated; Number of nodes: " << d->MRMLScene->GetNumberOfNodes();
  emit mrmlSceneSet(d->MRMLScene);
}

qMRMLSceneGeneratorPlugin::qMRMLSceneGeneratorPlugin(QObject *_parent)
        : QObject(_parent)
{
}

QWidget *qMRMLSceneGeneratorPlugin::createWidget(QWidget *_parent)
{
  qMRMLSceneGenerator* _widget = new qMRMLSceneGenerator(_parent);
  return _widget;
}

QString qMRMLSceneGeneratorPlugin::domXml() const
{
  return "<widget class=\"qMRMLSceneGenerator\" \
          name=\"MRMLSceneGenerator\">\n"
          "</widget>\n";
}

QString qMRMLSceneGeneratorPlugin::includeFile() const
{
  return "qMRMLSceneGeneratorPlugin.h";
}

bool qMRMLSceneGeneratorPlugin::isContainer() const
{
  return false;
}

QString qMRMLSceneGeneratorPlugin::name() const
{
  return "qMRMLSceneGenerator";
}
