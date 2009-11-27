#include "qMRMLNodeFactory.h"
#include "qMRMLUtils.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkSmartPointer.h"

// QT includes
#include <QDebug>

//-----------------------------------------------------------------------------
struct qMRMLNodeFactory::qInternal
{
public:
  qInternal()
    {
    this->MRMLScene = 0; 
    }
  vtkMRMLScene * MRMLScene; 
};

// --------------------------------------------------------------------------
qMRMLNodeFactory::qMRMLNodeFactory(QObject* parent) : Superclass(parent)
{
  this->Internal = new qInternal;
}

// --------------------------------------------------------------------------
qMRMLNodeFactory::~qMRMLNodeFactory()
{
  delete this->Internal; 
}

// --------------------------------------------------------------------------
void qMRMLNodeFactory::setMRMLScene(vtkMRMLScene* mrmlScene)
{
  this->Internal->MRMLScene = mrmlScene;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLNodeFactory::mrmlScene()
{
  return this->Internal->MRMLScene; 
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactory::createNode(const char* className)
{
  return this->createNode(QString::fromAscii(className) );
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactory::createNode(const QString& className)
{
  Q_ASSERT(this->Internal->MRMLScene);
  Q_ASSERT(!className.isEmpty());
  if (!this->Internal->MRMLScene || className.isEmpty())
    {
    return 0; 
    }

  vtkSmartPointer<vtkMRMLNode> node; 
  node.TakeReference( this->Internal->MRMLScene->CreateNodeByClass( className.toLatin1().data() ) );
  
  Q_ASSERT(node);
  if (node == NULL)
    {
    return 0;
    }
  
  node->SetScene( this->Internal->MRMLScene );
  node->SetName( this->Internal->MRMLScene->GetUniqueNameByString(
    this->Internal->MRMLScene->GetTagByClassName(className.toLatin1().data()) ) );
  /*
    // If there is a Attribute Name-Value specified, then set that
    // attribute on the node
    for (int c=0; c < this->GetNumberOfNodeClasses(); c++)
      {
      if (!strcmp(this->GetNodeClass(c), className))
        {
        if (this->GetNodeAttributeName(c) != NULL)
          {
          node->SetAttribute(this->GetNodeAttributeName(c),
                            this->GetNodeAttributeValue(c));
          }
        break;
        }
      }
  */
  
  vtkMRMLNode * nodeCreated = this->Internal->MRMLScene->AddNode(node);
  qDebug() << "createAndAddNodeToSceneByClass - Set name to:" 
           << nodeCreated->GetName() << "(" << nodeCreated->GetID() << ")"; 
  return nodeCreated; 
}
