/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>

// qMRML includes
#include "qMRMLNodeFactory.h"

// MRML includes
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkSmartPointer.h>

//------------------------------------------------------------------------------
class qMRMLNodeFactoryPrivate
{
public:
  qMRMLNodeFactoryPrivate()
    {
    this->MRMLScene = nullptr;
    }
  vtkMRMLScene * MRMLScene;
  QHash<QString, QString> BaseNames;
  QHash<QString, QString> Attributes;
};

//------------------------------------------------------------------------------
qMRMLNodeFactory::qMRMLNodeFactory(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLNodeFactoryPrivate)
{
}

//------------------------------------------------------------------------------
qMRMLNodeFactory::~qMRMLNodeFactory() = default;

//------------------------------------------------------------------------------
CTK_SET_CPP(qMRMLNodeFactory, vtkMRMLScene*, setMRMLScene, MRMLScene);
CTK_GET_CPP(qMRMLNodeFactory, vtkMRMLScene*, mrmlScene, MRMLScene);

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactory::createNode(const QString& className)
{
  Q_D(qMRMLNodeFactory);

  if (!d->MRMLScene || className.isEmpty())
    {
    return nullptr;
    }
  vtkSmartPointer<vtkMRMLNode> node;
  node.TakeReference( d->MRMLScene->CreateNodeByClass( className.toUtf8() ) );

  Q_ASSERT_X(node, "createNode",
             QString("Failed to create node of type [%1]").arg(className).toUtf8());

  if (node == nullptr)
    {
    return nullptr;
    }

  emit this->nodeInstantiated(node);

  QString baseName;
  if (d->BaseNames.contains(className) &&
      !d->BaseNames[className].isEmpty())
    {
    baseName = d->BaseNames[className];
    }
  else
    {
    baseName = d->MRMLScene->GetTagByClassName(className.toUtf8());
    }
  node->SetName(d->MRMLScene->GetUniqueNameByString(baseName.toUtf8()));

  // Set node attributes
  // Attributes must be set before adding the node into the scene as the node
  // combobox filter might hide the node if the attributes are not set yet.
  // Ideally the qMRMLSortFilterProxyModel should listen the all the nodes and
  // when the attribute property is changed, make sure that it doesn't change
  // it's visibility
  foreach (const QString& attributeName, d->Attributes.keys())
    {
    node->SetAttribute(attributeName.toUtf8(),
                       d->Attributes[attributeName].toUtf8());
    }

  emit this->nodeInitialized(node);
  // maybe the node has been added into the scene by slots connected
  // to nodeInitialized.
  // If node is initialized from a default node then scene may be set but the node
  // is not added to the scene yet, therefore we check if the node is actually present
  // in the scene (and add it if it is not present).
  if (!node->GetScene() || !node->GetScene()->IsNodePresent(node))
    {
    vtkMRMLNode* nodeAdded = d->MRMLScene->AddNode(node);
    Q_ASSERT(nodeAdded == node ||
             node->GetSingletonTag() != nullptr);
    node = nodeAdded;
    }
  emit this->nodeAdded(node);

  return node;
}

//------------------------------------------------------------------------------
vtkMRMLNode* qMRMLNodeFactory::createNode(vtkMRMLScene* scene, const QString& className,
                                          const QHash<QString,QString>& attributes)
{
  Q_ASSERT(scene);
  QScopedPointer<qMRMLNodeFactory> factory(new qMRMLNodeFactory());
  factory->setMRMLScene(scene);
  // Loop over attribute map and update the factory
  foreach(const QString& key, attributes.keys())
    {
    factory->addAttribute(key, attributes.value(key));
    }
  // Instantiate and return the requested node
  return factory->createNode(className);
}

//------------------------------------------------------------------------------
void qMRMLNodeFactory::addAttribute(const QString& name, const QString& value)
{
  Q_D(qMRMLNodeFactory);
  d->Attributes.insert(name, value);
}

//------------------------------------------------------------------------------
void qMRMLNodeFactory::removeAttribute(const QString& name)
{
  Q_D(qMRMLNodeFactory);
  d->Attributes.remove(name);
}

//------------------------------------------------------------------------------
QString qMRMLNodeFactory::attribute(const QString& name)const
{
  Q_D(const qMRMLNodeFactory);
  return d->Attributes[name];
}

//------------------------------------------------------------------------------
void qMRMLNodeFactory::setBaseName(const QString& className, const QString& baseName)
{
  Q_D(qMRMLNodeFactory);
  d->BaseNames[className] = baseName;
}

//------------------------------------------------------------------------------
QString qMRMLNodeFactory::baseName(const QString& className)const
{
  Q_D(const qMRMLNodeFactory);
  if (!d->BaseNames.contains(className))
    {
    qWarning("qMRMLNodeFactory::baseName failed: class name %s not found", qPrintable(className));
    return QString();
    }
  return d->BaseNames[className];
}
