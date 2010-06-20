
// Qt includes
#include <QHash>
#include <QSharedPointer>
#include <QDebug>

// qMRML includes
#include "qMRMLUtils.h"
#include "qMRMLNodeFactory.h"

// MRML includes
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLNodeFactoryPrivate: public ctkPrivate<qMRMLNodeFactory>
{
public:
  qMRMLNodeFactoryPrivate()
    {
    this->MRMLScene = 0; 
    }
  vtkMRMLScene * MRMLScene;
  QHash<QString, QString> Attributes;
};

//------------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qMRMLNodeFactory, QObject*);

//------------------------------------------------------------------------------
CTK_SET_CXX(qMRMLNodeFactory, vtkMRMLScene*, setMRMLScene, MRMLScene);
CTK_GET_CXX(qMRMLNodeFactory, vtkMRMLScene*, mrmlScene, MRMLScene);

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactory::createNode(const QString& className,
                                          const vtkMRMLNodeInitializer & initializer)
{
  CTK_D(qMRMLNodeFactory);
  
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

  // Extra initialization steps
  initializer(node);
  
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
                                          const vtkMRMLNodeInitializer & initializer,
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
  return factory->createNode(className, initializer);
}

//------------------------------------------------------------------------------
void qMRMLNodeFactory::addAttribute(const QString& name, const QString& value)
{
  ctk_d()->Attributes.insert(name, value);
}
