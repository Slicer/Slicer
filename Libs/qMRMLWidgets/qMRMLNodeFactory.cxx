
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
  QHash<QString, QString> BaseNames;
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
  QString baseName;
  if (d->BaseNames.contains(className) &&
      !d->BaseNames[className].isEmpty())
    {
    baseName = d->BaseNames[className];
    }
  else
    {
    baseName = d->MRMLScene->GetTagByClassName(className.toLatin1());
    }
  node->SetName(d->MRMLScene->GetUniqueNameByString(baseName.toLatin1()));

  if (node->GetSingletonTag() != 0 && node->GetID() == 0)
    {
    qDebug() << "Problem creating node [" << className << "] - "
                "Can't add a node with a SingletonTag and an empty ID";
    return 0;
    }

  // Extra initialization steps
  initializer(node);

  // Set node attributes
  // Attributes must be set before adding the node into the scene as the node
  // combobox filter might hide the node if the attributes are not set yet.
  // Ideally the qMRMLSortFilterProxyModel should listen the all the nodes and
  // when the attribute property is changed, make sure that it doesn't change
  // it's visibility
  foreach (const QString& attributeName, d->Attributes.keys())
    {
    node->SetAttribute(attributeName.toLatin1(),
                       d->Attributes[attributeName].toLatin1());
    }

  vtkMRMLNode * nodeCreated = d->MRMLScene->AddNode(node);
  Q_ASSERT(nodeCreated);

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

//------------------------------------------------------------------------------
void qMRMLNodeFactory::setBaseName(const QString& className, const QString& baseName)
{
  CTK_D(qMRMLNodeFactory);
  d->BaseNames[className] = baseName;
}

//------------------------------------------------------------------------------
QString qMRMLNodeFactory::baseName(const QString& className)const
{
  CTK_D(const qMRMLNodeFactory);
  return d->BaseNames.contains(className) ? d->BaseNames[className] : QString();
}
