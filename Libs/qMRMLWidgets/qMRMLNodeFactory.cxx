#include "qMRMLNodeFactory.h"

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// QT includes
#include <QHash>
#include <QDebug>

//-----------------------------------------------------------------------------
struct qMRMLNodeFactoryPrivate: public qCTKPrivate<qMRMLNodeFactory>
{
public:
  qMRMLNodeFactoryPrivate()
    {
    this->MRMLScene = 0; 
    }
  vtkMRMLScene * MRMLScene;
  QHash<QString, QString> Attributes;
};

// --------------------------------------------------------------------------
qMRMLNodeFactory::qMRMLNodeFactory(QObject* parent) : Superclass(parent)
{
  QCTK_INIT_PRIVATE(qMRMLNodeFactory);
}

// --------------------------------------------------------------------------
QCTK_SET_CXX(qMRMLNodeFactory, vtkMRMLScene*, setMRMLScene, MRMLScene);
QCTK_GET_CXX(qMRMLNodeFactory, vtkMRMLScene*, mrmlScene, MRMLScene);

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactory::createNode(const char* className)
{
  return this->createNode(QString::fromAscii(className) );
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactory::createNode(const QString& className)
{
  QCTK_D(qMRMLNodeFactory);
  
  Q_ASSERT(d->MRMLScene);
  Q_ASSERT(!className.isEmpty());
  if (!d->MRMLScene || className.isEmpty())
    {
    return 0; 
    }
  vtkSmartPointer<vtkMRMLNode> node; 
  node.TakeReference( d->MRMLScene->CreateNodeByClass( className.toLatin1().data() ) );
  
  Q_ASSERT(node);
  if (node == 0)
    {
    return 0;
    }
  
  node->SetScene( d->MRMLScene );
  node->SetName( d->MRMLScene->GetUniqueNameByString(
    d->MRMLScene->GetTagByClassName(className.toLatin1().data()) ) );
  
  vtkMRMLNode * nodeCreated = d->MRMLScene->AddNode(node);
  qDebug() << "createAndAddNodeToSceneByClass - Set name to:" 
           << nodeCreated->GetName() << "(" << nodeCreated->GetID() << ")";

  Q_ASSERT(nodeCreated);
  
  // Set node attributes
  QHashIterator<QString, QString> i(d->Attributes);
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
  qctk_d()->Attributes.insert(name, value);
}
