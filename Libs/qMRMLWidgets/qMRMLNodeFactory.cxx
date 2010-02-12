#include "qMRMLNodeFactory.h"

// qMRML includes
#include "qMRMLUtils.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

// QT includes
#include <QHash>
#include <QSharedPointer>
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLNodeFactoryPrivate: public qCTKPrivate<qMRMLNodeFactory>
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
QCTK_CONSTRUCTOR_1_ARG_CXX(qMRMLNodeFactory, QObject*);

// --------------------------------------------------------------------------
QCTK_SET_CXX(qMRMLNodeFactory, vtkMRMLScene*, setMRMLScene, MRMLScene);
QCTK_GET_CXX(qMRMLNodeFactory, vtkMRMLScene*, mrmlScene, MRMLScene);

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

  Q_ASSERT_X(node, "createNode",
             QString("Failed to create node of type [%1]").arg(className).toLatin1());
             
  if (node == 0)
    {
    return 0;
    }
  
  node->SetScene( d->MRMLScene );
  node->SetName( d->MRMLScene->GetUniqueNameByString(
    d->MRMLScene->GetTagByClassName(className.toLatin1().data()) ) );

  if (node->GetSingletonTag() != 0 && node->GetID() == 0)
    {
    qDebug() << "Problem creating node [" << className << "] - "
                "Can't add a node with a SingletonTag and an empty ID";
    return 0;
    }
  
  vtkMRMLNode * nodeCreated = d->MRMLScene->AddNode(node);
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
vtkMRMLNode* qMRMLNodeFactory::createNode(vtkMRMLScene* scene, const QString& className,
  const QHash<QString,QString>& attributes)
{
  Q_ASSERT(scene);
  QSharedPointer<qMRMLNodeFactory> factory =
    QSharedPointer<qMRMLNodeFactory>(new qMRMLNodeFactory());
  factory->setMRMLScene(scene);
  // Loop over attribute map and update the factory
  foreach(const QString& key, attributes.keys())
    {
    factory->addAttribute(key, attributes.value(key));
    }
  // Instanciate and return the requested node
  return factory->createNode(className); 
}

//------------------------------------------------------------------------------
void qMRMLNodeFactory::addAttribute(const QString& name, const QString& value)
{
  qctk_d()->Attributes.insert(name, value);
}
