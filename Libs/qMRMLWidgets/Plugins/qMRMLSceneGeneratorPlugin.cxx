#include "qMRMLSceneGeneratorPlugin.h"
#include "qMRMLSceneGeneratorPlugin_p.h"

// qMRML includes
#include "qMRMLNodeFactory.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkEventBroker.h>

// QT includes
#include <QDebug>

// --------------------------------------------------------------------------
qMRMLSceneGeneratorPlugin::qMRMLSceneGeneratorPlugin(QObject *_parent)
        : QObject(_parent)
{
  qctk_d.setPublic(this); //QCTK_INIT_PRIVATE
}

// --------------------------------------------------------------------------
QWidget *qMRMLSceneGeneratorPlugin::createWidget(QWidget *_parent)
{
  qMRMLSceneGenerator* _widget = new qMRMLSceneGenerator(_parent);
  return _widget;
}

// --------------------------------------------------------------------------
QString qMRMLSceneGeneratorPlugin::domXml() const
{
  return "<widget class=\"qMRMLSceneGenerator\" \
          name=\"MRMLSceneGenerator\">\n"
          "</widget>\n";
}

// --------------------------------------------------------------------------
QString qMRMLSceneGeneratorPlugin::includeFile() const
{
  return "qMRMLSceneGeneratorPlugin.h";
}

// --------------------------------------------------------------------------
bool qMRMLSceneGeneratorPlugin::isContainer() const
{
  return false;
}

// --------------------------------------------------------------------------
QString qMRMLSceneGeneratorPlugin::name() const
{
  return "qMRMLSceneGenerator";
}

// --------------------------------------------------------------------------
// qMRMLSceneGenerator methods

// --------------------------------------------------------------------------
qMRMLSceneGenerator::qMRMLSceneGenerator(QWidget* parent)
  :QPushButton("Generate New Scene", parent)
{
  this->init();
}

// --------------------------------------------------------------------------
qMRMLSceneGenerator::~qMRMLSceneGenerator()
{
  this->clear();
}

// --------------------------------------------------------------------------
void qMRMLSceneGenerator::init()
{
  this->MRMLScene = 0;
  this->connect(this, SIGNAL(clicked()), SLOT(generateScene()));
}

// --------------------------------------------------------------------------
void qMRMLSceneGenerator::clear()
{
  if (this->MRMLScene)
    {
    emit mrmlSceneSet(0);
    this->MRMLScene->Delete();
    }
}

// --------------------------------------------------------------------------
void qMRMLSceneGenerator::generateScene()
{
  this->clear();
  this->MRMLScene = vtkMRMLScene::New();
  
  qMRMLNodeFactory factory(this);
  factory.setMRMLScene(this->MRMLScene);
  
  int numClasses = this->MRMLScene->GetNumberOfRegisteredNodeClasses();
  int numNodes = 15;
  QList<QString> nodeNames;
  for (int i = 0; i < numNodes ; ++i)
    {
    int classNumber = rand() % numClasses; 
    vtkMRMLNode* node = this->MRMLScene->GetNthRegisteredNodeClass(classNumber);
    QString classname = QLatin1String(node->GetClassName()); 
    if (classname.isEmpty())
      {
      qWarning() << "Class registered (#" << classNumber << "):"
                 << node << " has an empty classname";
      continue;
      }
    nodeNames << QLatin1String(node->GetClassName());
    factory.createNode(node->GetClassName());
    }

  emit this->randomMRMLNodeType(nodeNames.at(rand() % nodeNames.size()));
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
  qDebug() << "Scene generated; Number of nodes: " << this->MRMLScene->GetNumberOfNodes();
  emit mrmlSceneSet(this->MRMLScene);
}
