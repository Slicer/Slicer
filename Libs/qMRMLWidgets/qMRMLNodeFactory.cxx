#include "qMRMLNodeFactory.h"
#include "qMRMLUtils.h"

// MRML includes
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkSmartPointer.h"

// QT includes
#include <QHash>
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
  QHash<QString, QString> Attributes;
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
  
  vtkMRMLNode * nodeCreated = this->Internal->MRMLScene->AddNode(node);
  qDebug() << "createAndAddNodeToSceneByClass - Set name to:" 
           << nodeCreated->GetName() << "(" << nodeCreated->GetID() << ")";

  Q_ASSERT(nodeCreated);
  
  // Set node attributes
  QHashIterator<QString, QString> i(this->Internal->Attributes);
  while (i.hasNext())
    {
    i.next();
    nodeCreated->SetAttribute(i.key().toLatin1(), i.value().toLatin1());
    }
  return nodeCreated; 
}

//------------------------------------------------------------------------------
void qMRMLNodeFactory::addAttribute(const QString& name, const QString& value)
{
  this->Internal->Attributes.insert(name, value);
}
