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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
//#include <QDomElement>
#include <QDebug>

// CTK includes
#include <ctkVTKAbstractView.h>

// MRMLWidgets includes
#include "qMRMLLayoutManager.h"
#include "qMRMLLayoutViewFactory.h"
#include "qMRMLWidget.h"

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLLayoutLogic.h>
#include <vtkMRMLLayoutNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRendererCollection.h>

//------------------------------------------------------------------------------
// qMRMLLayoutViewFactoryPrivate methods
class qMRMLLayoutViewFactoryPrivate
{
  Q_DECLARE_PUBLIC(qMRMLLayoutViewFactory);
public:
  qMRMLLayoutViewFactoryPrivate(qMRMLLayoutViewFactory& obj);
  virtual void init();

  vtkMRMLAbstractViewNode* viewNodeByName(const QString& viewName)const;
  vtkMRMLAbstractViewNode* viewNodeByLayoutLabel(const QString& layoutLabel)const;

  vtkMRMLLayoutLogic::ViewAttributes attributesFromXML(QDomElement viewElement)const;
  vtkMRMLLayoutLogic::ViewProperties propertiesFromXML(QDomElement viewElement)const;
  vtkMRMLLayoutLogic::ViewProperty propertyFromXML(QDomElement propertyElement)const;

  QList<qMRMLWidget*> mrmlWidgets()const;

protected:
  qMRMLLayoutViewFactory* q_ptr;

  qMRMLLayoutManager* LayoutManager;
  QHash<vtkMRMLAbstractViewNode*, QWidget*> Views;

  vtkMRMLScene* MRMLScene;
  vtkMRMLAbstractViewNode* ActiveViewNode;
};

//------------------------------------------------------------------------------
qMRMLLayoutViewFactoryPrivate::qMRMLLayoutViewFactoryPrivate(qMRMLLayoutViewFactory& object)
  : q_ptr(&object)
  , LayoutManager(nullptr)
  , MRMLScene(nullptr)
  , ActiveViewNode(nullptr)
{
}

//------------------------------------------------------------------------------
void qMRMLLayoutViewFactoryPrivate::init()
{
  Q_Q(qMRMLLayoutViewFactory);
  q->setUseCachedViews(false);
}

//------------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLLayoutViewFactoryPrivate
::viewNodeByName(const QString& viewName)const
{
  foreach(vtkMRMLAbstractViewNode* viewNode, this->Views.keys())
    {
    if (viewName == QString(viewNode->GetName()))
      {
      return viewNode;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLLayoutViewFactoryPrivate
::viewNodeByLayoutLabel(const QString& layoutLabel)const
{
  foreach(vtkMRMLAbstractViewNode* viewNode, this->Views.keys())
    {
    if (layoutLabel == QString(viewNode->GetLayoutLabel()))
      {
      return viewNode;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewAttributes qMRMLLayoutViewFactoryPrivate
::attributesFromXML(QDomElement viewElement)const
{
  vtkMRMLLayoutLogic::ViewAttributes attributes;
  QDomNamedNodeMap elementAttributes = viewElement.attributes();
  const int attributeCount = elementAttributes.count();
  for (int i = 0; i < attributeCount; ++i)
    {
    QDomNode attribute = elementAttributes.item(i);
    attributes[attribute.nodeName().toStdString()] =
      viewElement.attribute(attribute.nodeName()).toStdString();
    }
  return attributes;
}

//------------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewProperties qMRMLLayoutViewFactoryPrivate
::propertiesFromXML(QDomElement viewElement)const
{
  vtkMRMLLayoutLogic::ViewProperties properties;
  for (QDomElement childElement = viewElement.firstChildElement();
       !childElement.isNull();
       childElement = childElement.nextSiblingElement())
    {
    properties.push_back(this->propertyFromXML(childElement));
    }
  return properties;
}

//------------------------------------------------------------------------------
vtkMRMLLayoutLogic::ViewProperty qMRMLLayoutViewFactoryPrivate
::propertyFromXML(QDomElement propertyElement)const
{
  vtkMRMLLayoutLogic::ViewProperty property;
  QDomNamedNodeMap elementAttributes = propertyElement.attributes();
  const int attributeCount = elementAttributes.count();
  for (int i = 0; i < attributeCount; ++i)
    {
    QDomNode attribute = elementAttributes.item(i);
    property[attribute.nodeName().toStdString()] =
      propertyElement.attribute(attribute.nodeName()).toStdString();
    }
  property["value"] = propertyElement.text().toStdString();
  return property;
}


//------------------------------------------------------------------------------
QList<qMRMLWidget*> qMRMLLayoutViewFactoryPrivate
::mrmlWidgets()const
{
  QList<qMRMLWidget*> res;
  foreach(QWidget* viewWidget, this->Views.values())
    {
    qMRMLWidget* mrmlWidget = qobject_cast<qMRMLWidget*>(viewWidget);
    if (mrmlWidget)
      {
      res << mrmlWidget;
      }
    }
  return res;
}

//------------------------------------------------------------------------------
// qMRMLLayoutViewFactory methods

// --------------------------------------------------------------------------
qMRMLLayoutViewFactory::qMRMLLayoutViewFactory(QObject* parentObject)
  : Superclass(parentObject)
  , d_ptr(new qMRMLLayoutViewFactoryPrivate(*this))
{
  Q_D(qMRMLLayoutViewFactory);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLLayoutViewFactory::~qMRMLLayoutViewFactory()
{
  Q_D(qMRMLLayoutViewFactory);
  while(this->viewCount())
    {
    this->deleteView(d->Views.keys()[0]);
    }
}

// --------------------------------------------------------------------------
QString qMRMLLayoutViewFactory::viewClassName()const
{
  return QString();
}

// --------------------------------------------------------------------------
bool qMRMLLayoutViewFactory::isElementSupported(QDomElement layoutElement)const
{
  if (!this->Superclass::isElementSupported(layoutElement))
    {
    return false;
    }
  vtkMRMLAbstractViewNode* viewNode = this->viewNodeFromXML(layoutElement);
  return this->isViewNodeSupported(viewNode);
}

// --------------------------------------------------------------------------
bool qMRMLLayoutViewFactory::isViewNodeSupported(vtkMRMLAbstractViewNode* viewNode)const
{
  return viewNode && viewNode->IsA(this->viewClassName().toUtf8());
}

// --------------------------------------------------------------------------
qMRMLLayoutManager* qMRMLLayoutViewFactory::layoutManager()const
{
  Q_D(const qMRMLLayoutViewFactory);
  return d->LayoutManager;
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::setLayoutManager(qMRMLLayoutManager* layoutManager)
{
  Q_D(qMRMLLayoutViewFactory);
  d->LayoutManager = layoutManager;
}

// --------------------------------------------------------------------------
vtkMRMLScene* qMRMLLayoutViewFactory::mrmlScene()const
{
  Q_D(const qMRMLLayoutViewFactory);
  return d->MRMLScene;
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::setMRMLScene(vtkMRMLScene* scene)
{
  Q_D(qMRMLLayoutViewFactory);

  if (d->MRMLScene == scene)
    {
    return;
    }
  while(this->viewCount())
    {
    this->deleteView(d->Views.keys()[0]);
    }
  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeAddedEvent,
                      this, SLOT(onNodeAdded(vtkObject*,vtkObject*)));

  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::NodeRemovedEvent,
                     this, SLOT(onNodeRemoved(vtkObject*,vtkObject*)));

  this->qvtkReconnect(d->MRMLScene, scene, vtkMRMLScene::EndBatchProcessEvent,
                      this, SLOT(onSceneModified()));

  d->MRMLScene = scene;

  this->onSceneModified();
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutViewFactory::viewWidget(int id)const
{
  Q_D(const qMRMLLayoutViewFactory);
  if (id < 0 || id >= d->Views.size())
    {
    return nullptr;
    }
  return d->Views.values().at(id);
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutViewFactory::viewWidget(vtkMRMLAbstractViewNode* node)const
{
  Q_D(const qMRMLLayoutViewFactory);
  return d->Views.value(node, 0);
}

//------------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLLayoutViewFactory::viewNode(QWidget* widget)const
{
  Q_D(const qMRMLLayoutViewFactory);
  return d->Views.key(widget, 0);
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutViewFactory::viewWidget(const QString& name)const
{
  Q_D(const qMRMLLayoutViewFactory);
  vtkMRMLAbstractViewNode* viewNode = d->viewNodeByName(name);
  return this->viewWidget(viewNode);
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutViewFactory::viewWidgetByLayoutLabel(const QString& layoutLabel)const
{
  Q_D(const qMRMLLayoutViewFactory);
  vtkMRMLAbstractViewNode* viewNode = d->viewNodeByLayoutLabel(layoutLabel);
  return this->viewWidget(viewNode);
}

//------------------------------------------------------------------------------
QStringList qMRMLLayoutViewFactory::viewNodeNames() const
{
  Q_D(const qMRMLLayoutViewFactory);

  QStringList res;
  foreach (vtkMRMLAbstractViewNode* viewNode, d->Views.keys())
    {
    res << viewNode->GetName();
    }

  return res;
}

//------------------------------------------------------------------------------
int qMRMLLayoutViewFactory::viewCount()const
{
  Q_D(const qMRMLLayoutViewFactory);
  return d->Views.size();
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::beginSetupLayout()
{
  Q_D(qMRMLLayoutViewFactory);
  this->Superclass::beginSetupLayout();
  foreach(vtkMRMLAbstractViewNode* viewNode, d->Views.keys())
    {
    viewNode->SetMappedInLayout(0);
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::onNodeAdded(vtkObject* scene, vtkObject* node)
{
  vtkMRMLScene* mrmlScene = vtkMRMLScene::SafeDownCast(scene);
  if (!mrmlScene || mrmlScene->IsBatchProcessing())
    {
    // We have to leave because maybe all the nodes required by the view node
    // (e.g. composite slice node) have not yet been added to the scene.
    return;
    }

  Q_UNUSED(scene);
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(node);
  if (viewNode && !viewNode->GetParentLayoutNode())
    {
    // No explicit parent layout node means that view is handled by the main Slicer layout
    this->onViewNodeAdded(viewNode);
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::onNodeRemoved(vtkObject* scene, vtkObject* node)
{
  Q_UNUSED(scene);
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    this->onViewNodeRemoved(viewNode);
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::onNodeModified(vtkObject* node)
{
  vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(node);
  if (viewNode)
    {
    this->onViewNodeModified(viewNode);
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::onViewNodeAdded(vtkMRMLAbstractViewNode* node)
{
  Q_D(qMRMLLayoutViewFactory);
  if (!this->isViewNodeSupported(node))
    {
    return;
    }
  if (this->viewWidget(node))
    { // The view already exists, no need to create it again.
    return;
    }
  QWidget* viewWidget = this->createViewFromNode(node);
  if (!viewWidget)
    { // The factory cannot create such view, do nothing about it
    return;
    }

  // Do not show until mapped into a view (the widget is shown/hidden only
  // if it is part of the layout, but if the widget was not yet part of any layout
  // then it would show up in the top-left corner of the viewport)
  viewWidget->setVisible(false);

  d->Views[node] = viewWidget;

  // For now, the active view is the first one
  if (this->viewCount() == 1)
    {
    this->setActiveViewNode(node);
    }
  this->qvtkConnect(node, vtkCommand::ModifiedEvent,
                    this, SLOT(onNodeModified(vtkObject*)));
  emit viewCreated(viewWidget);
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::onViewNodeRemoved(vtkMRMLAbstractViewNode* node)
{
  this->deleteView(node);
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::onViewNodeModified(vtkMRMLAbstractViewNode* node)
{
  this->viewWidget(node)->setVisible(node->IsViewVisibleInLayout());
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::onSceneModified()
{
  Q_D(qMRMLLayoutViewFactory);
  if (!d->MRMLScene)
    {
    return;
    }
  std::vector<vtkMRMLNode*> viewNodes;
  d->MRMLScene->GetNodesByClass("vtkMRMLAbstractViewNode", viewNodes);
  for (unsigned int i = 0; i < viewNodes.size(); ++i)
    {
    vtkMRMLAbstractViewNode* viewNode =
      vtkMRMLAbstractViewNode::SafeDownCast(viewNodes[i]);
    Q_ASSERT(viewNode);
    this->onViewNodeAdded(viewNode);
    }
}

// --------------------------------------------------------------------------
QWidget* qMRMLLayoutViewFactory::createViewFromNode(vtkMRMLAbstractViewNode* viewNode)
{
  Q_UNUSED(viewNode);
  return nullptr;
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::deleteView(vtkMRMLAbstractViewNode* viewNode)
{
  Q_D(qMRMLLayoutViewFactory);

  this->qvtkDisconnect(viewNode, vtkCommand::ModifiedEvent,
                       this, SLOT(onNodeModified(vtkObject*)));

  QWidget* widgetToDelete = this->viewWidget(viewNode);

  // Remove slice widget
  if (!widgetToDelete)
    {
    return;
    }
  qMRMLWidget* mrmlWidgetToDelete = qobject_cast<qMRMLWidget*>(widgetToDelete);
  if (mrmlWidgetToDelete)
    {
    mrmlWidgetToDelete->setMRMLScene(nullptr);
    }
  this->unregisterView(widgetToDelete);
  d->Views.remove(viewNode);
  widgetToDelete->deleteLater();
  if (this->activeViewNode() == viewNode)
    {
    this->setActiveViewNode(nullptr);
    }
}

// --------------------------------------------------------------------------
void qMRMLLayoutViewFactory::setActiveViewNode(vtkMRMLAbstractViewNode* node)
{
  Q_D(qMRMLLayoutViewFactory);
  if (d->ActiveViewNode == node)
    {
    return;
    }
  d->ActiveViewNode = node;

  emit this->activeViewNodeChanged(d->ActiveViewNode);
}

// --------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLLayoutViewFactory::activeViewNode()const
{
  Q_D(const qMRMLLayoutViewFactory);
  return d->ActiveViewNode;
}

// --------------------------------------------------------------------------
vtkRenderer* qMRMLLayoutViewFactory::activeRenderer()const
{
  QWidget* activeViewWidget = this->viewWidget(this->activeViewNode());
  if (!activeViewWidget)
    {
    return nullptr;
    }
  ctkVTKAbstractView* view = activeViewWidget->findChild<ctkVTKAbstractView*>();
  vtkRenderWindow* renderWindow = view ? view->renderWindow() : nullptr;
  vtkRendererCollection* renderers =
    renderWindow ? renderWindow->GetRenderers() : nullptr;
  return renderers ? renderers->GetFirstRenderer() : nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLLayoutViewFactory::viewNodeFromXML(QDomElement viewElement)const
{
  Q_D(const qMRMLLayoutViewFactory);
  vtkMRMLLayoutLogic::ViewAttributes attributes =
    d->attributesFromXML(viewElement);
  // convert Qt xml element into vtkMRMLLayoutLogic attributes
  vtkMRMLNode* viewNode = this->layoutManager()->layoutLogic()->GetViewFromAttributes(attributes);
  return vtkMRMLAbstractViewNode::SafeDownCast(viewNode);
}

//------------------------------------------------------------------------------
QWidget* qMRMLLayoutViewFactory::createViewFromXML(QDomElement viewElement)
{
  Q_D(qMRMLLayoutViewFactory);
  vtkMRMLAbstractViewNode* viewNode = this->viewNodeFromXML(viewElement);
  Q_ASSERT(viewNode);
  // Usually, the view is automatically created if:
  //  (1) a view node associated with the factory is added (See onViewNodeAdded)
  //  (2) a new Scene is set on the factory (See setMRMLScene / onSceneModified)
  //  (3) a batch process ends (See EndBatchProcessEvent / onSceneModified)
  QWidget* view = this->viewWidget(viewNode);
  if (!view)
    {
    // The following call will take care of creating the view when a sceneView is
    // restored. In that case, vtkMRMLLayoutLogic::OnMRMLSceneEndRestore was called first
    // without giving a chance to the factory to create the missing views.
    this->onViewNodeAdded(viewNode);
    view = this->viewWidget(viewNode);
    }
  Q_ASSERT(view);

  vtkMRMLLayoutLogic::ViewProperties properties = d->propertiesFromXML(viewElement);
  this->layoutManager()->layoutLogic()->ApplyProperties(properties, viewNode, "relayout");
  return view;
}

//------------------------------------------------------------------------------
QList<vtkMRMLAbstractViewNode*> qMRMLLayoutViewFactory
::viewNodesFromXML(QDomElement viewElement)const
{
  Q_D(const qMRMLLayoutViewFactory);
  vtkMRMLLayoutLogic::ViewAttributes attributes =
    d->attributesFromXML(viewElement);
  vtkCollection* viewNodes =
    this->layoutManager()->layoutLogic()->GetViewsFromAttributes(attributes);

  QList<vtkMRMLAbstractViewNode*> res;
  for (vtkMRMLAbstractViewNode* node = nullptr;
       (node = vtkMRMLAbstractViewNode::SafeDownCast(viewNodes->GetNextItemAsObject()));)
    {
    res << node;
    }
  viewNodes->Delete();
  return res;
}

//------------------------------------------------------------------------------
QList<QWidget*> qMRMLLayoutViewFactory::createViewsFromXML(QDomElement viewElement)
{
  Q_D(qMRMLLayoutViewFactory);
  QList<vtkMRMLAbstractViewNode*> viewNodes = this->viewNodesFromXML(viewElement);

  QList<QWidget*> res;
  foreach(vtkMRMLAbstractViewNode* viewNode, viewNodes)
    {
    res << this->viewWidget(viewNode);
    vtkMRMLLayoutLogic::ViewProperties properties = d->propertiesFromXML(viewElement);
    this->layoutManager()->layoutLogic()->ApplyProperties(properties, viewNode, "relayout");
    }
  return res;
}

//------------------------------------------------------------------------------
void qMRMLLayoutViewFactory::setupView(QDomElement viewElement, QWidget* view)
{
  this->Superclass::setupView(viewElement, view);
  vtkMRMLAbstractViewNode* viewNode = this->viewNode(view);
  Q_ASSERT(viewNode);
  viewNode->SetMappedInLayout(1);
  view->setVisible(viewNode->GetVisibility());
  view->setWindowTitle(viewNode->GetName());
}
