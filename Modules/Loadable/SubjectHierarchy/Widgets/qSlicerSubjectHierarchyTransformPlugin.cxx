/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTransformPlugin.h"

// SubjectHierarchy logic includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

#include "qMRMLSubjectHierarchyModel.h"

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QMenu>

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "vtkSlicerApplicationLogic.h"

// VTK includes
#include <vtkObjectFactory.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLStorableNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyTransformPluginPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyTransformPlugin);

protected:
  qSlicerSubjectHierarchyTransformPlugin* const q_ptr;

public:
  qSlicerSubjectHierarchyTransformPluginPrivate(qSlicerSubjectHierarchyTransformPlugin& object);
  ~qSlicerSubjectHierarchyTransformPluginPrivate() override;
  void init();

  void applyTransformToItem(vtkIdType itemID, const char* transformNodeID);
  vtkMRMLTransformNode* appliedTransformToItem(vtkIdType itemID, bool& commonToAllChildren);
  vtkMRMLTransformNode* firstAppliedTransformToSelectedItems();

public:
  qMRMLSubjectHierarchyModel* Model{ nullptr };

  QAction* TransformInteractionInViewAction{ nullptr };
  QAction* TransformEditPropertiesAction{ nullptr };
  QAction* TransformHardenAction{ nullptr };
  QAction* CreateNewTransformAction{ nullptr };
  QAction* NoTransformAction{ nullptr };
  QActionGroup* TransformActionGroup{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformPluginPrivate::qSlicerSubjectHierarchyTransformPluginPrivate(qSlicerSubjectHierarchyTransformPlugin& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyTransformPlugin);

  this->TransformInteractionInViewAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Interaction"), q);
  this->TransformInteractionInViewAction->setObjectName("TransformInteraction");
  this->TransformInteractionInViewAction->setCheckable(true);
  this->TransformInteractionInViewAction->setToolTip(qSlicerSubjectHierarchyTransformPlugin::tr(
    "Allow the transform to be modified interactively in the 2D and 3D views"));
  QObject::connect(
    this->TransformInteractionInViewAction, SIGNAL(toggled(bool)), q, SLOT(onTransformInteractionInViewToggled(bool)));

  this->TransformEditPropertiesAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Edit transform properties..."), q);
  this->TransformEditPropertiesAction->setObjectName("TransformEditProperties");
  this->TransformEditPropertiesAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Edit properties of the current transform"));
  QObject::connect(this->TransformEditPropertiesAction, SIGNAL(triggered()), q, SLOT(onTransformEditProperties()));

  this->TransformHardenAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Harden transform"), q);
  this->TransformHardenAction->setObjectName("TransformHarden");
  this->TransformHardenAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Harden current transform on this node and all children nodes"));
  QObject::connect(this->TransformHardenAction, SIGNAL(triggered()), q, SLOT(onHardenTransformOnBranchOfCurrentItem()));

  this->CreateNewTransformAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Create new transform"), q);
  this->CreateNewTransformAction->setObjectName("TransformCreateNew");
  this->CreateNewTransformAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Create and apply new transform"));
  QObject::connect(this->CreateNewTransformAction, SIGNAL(triggered()), q, SLOT(onCreateNewTransform()));

  this->NoTransformAction = new QAction(
    qSlicerSubjectHierarchyTransformPlugin::tr("None") /*: Displayed in the transforms submenu */);
  this->NoTransformAction->setObjectName("Transform:None");
  q->setActionPosition(this->NoTransformAction, q->transformListSection());
  this->NoTransformAction->setCheckable(true);
  this->NoTransformAction->setData(QString()); // user data stores the transform node ID
  this->NoTransformAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Remove parent transform from all the nodes in this branch"));
  QObject::connect(this->NoTransformAction, SIGNAL(triggered()), q, SLOT(onRemoveTransformsFromBranchOfCurrentItem()));

  this->TransformActionGroup = new QActionGroup(q);
  this->TransformActionGroup->setExclusive(true);
  this->TransformActionGroup->addAction(this->NoTransformAction);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformPluginPrivate::~qSlicerSubjectHierarchyTransformPluginPrivate() = default;

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformPlugin::qSlicerSubjectHierarchyTransformPlugin(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSubjectHierarchyTransformPluginPrivate(*this))
{
  this->m_Name = QString("Transform");

  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformPlugin::~qSlicerSubjectHierarchyTransformPlugin() = default;

//------------------------------------------------------------------------------
vtkMRMLTransformNode* qSlicerSubjectHierarchyTransformPluginPrivate::appliedTransformToItem(vtkIdType itemID, bool& commonToAllChildren)
{
  commonToAllChildren = false;
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return nullptr;
  }

  // Get all the item IDs to apply the transform to (the item itself and all children recursively)
  std::vector<vtkIdType> itemIDsToTransform;
  shNode->GetItemChildren(itemID, itemIDsToTransform, true);
  itemIDsToTransform.insert(itemIDsToTransform.begin(), itemID);

  bool foundTransform = false;
  vtkMRMLTransformNode* commonTransformNode = nullptr;
  // Apply transform to the node and all its suitable children
  for (std::vector<vtkIdType>::iterator itemIDToTransformIt = itemIDsToTransform.begin();
       itemIDToTransformIt != itemIDsToTransform.end();
       ++itemIDToTransformIt)
  {
    vtkIdType itemIDToTransform = (*itemIDToTransformIt);
    vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(itemIDToTransform));
    if (!node)
    {
      // not transformable
      continue;
    }
    vtkMRMLTransformNode* currentTransformNode = node->GetParentTransformNode();
    if (!foundTransform)
    {
      // first transform
      foundTransform = true;
      commonTransformNode = currentTransformNode;
    }
    else
    {
      if (currentTransformNode != commonTransformNode)
      {
        // mismatch - not all nodes use the same transform
        return commonTransformNode;
      }
    }
  }
  // no mismatch was found
  commonToAllChildren = true;
  return commonTransformNode;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPluginPrivate::applyTransformToItem(vtkIdType itemID, const char* transformNodeID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  // Get all the item IDs to apply the transform to (the item itself and all children recursively)
  std::vector<vtkIdType> itemIDsToTransform;
  shNode->GetItemChildren(itemID, itemIDsToTransform, true);
  itemIDsToTransform.push_back(itemID);

  // Apply transform to the node and all its suitable children
  for (std::vector<vtkIdType>::iterator itemIDToTransformIt = itemIDsToTransform.begin();
       itemIDToTransformIt != itemIDsToTransform.end();
       ++itemIDToTransformIt)
  {
    vtkIdType itemIDToTransform = (*itemIDToTransformIt);
    vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(itemIDToTransform));
    if (!node)
    {
      // not transformable
      continue;
    }
    node->SetAndObserveTransformNodeID(transformNodeID);
  }
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* qSlicerSubjectHierarchyTransformPluginPrivate::firstAppliedTransformToSelectedItems()
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return nullptr;
  }
  QList<vtkIdType> currentItemIDs;
  currentItemIDs = qSlicerSubjectHierarchyPluginHandler::instance()->currentItems();

  foreach (vtkIdType itemID, currentItemIDs)
  {
    std::vector<vtkIdType> childItemIDs;
    shNode->GetItemChildren(itemID, childItemIDs, true);
    childItemIDs.insert(childItemIDs.begin(), itemID);

    // Apply transform to the node and all its suitable children
    for (std::vector<vtkIdType>::iterator childItemIDsIt = childItemIDs.begin(); childItemIDsIt != childItemIDs.end();
         ++childItemIDsIt)
    {
      vtkIdType childItemID = (*childItemIDsIt);
      vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(childItemID));
      if (!node)
      {
        // not transformable
        continue;
      }
      vtkMRMLTransformNode* currentTransformNode = node->GetParentTransformNode();
      if (!currentTransformNode)
      {
        continue;
      }
      return currentTransformNode;
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPlugin::showTransformContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
  }

  d->TransformInteractionInViewAction->setVisible(true);
  d->TransformEditPropertiesAction->setVisible(true);
  d->TransformHardenAction->setVisible(true);
  d->CreateNewTransformAction->setVisible(true);
  d->NoTransformAction->setVisible(true);

  vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  bool allTransformsAreTheSame = false;
  vtkMRMLTransformNode* currentTransformNode = d->appliedTransformToItem(itemID, allTransformsAreTheSame);
  std::string currentTransformNodeID;
  if (currentTransformNode)
  {
    currentTransformNodeID = currentTransformNode->GetID();
  }

  QList<QAction*> transformActions = d->TransformActionGroup->actions();
  foreach (QAction* transformAction, transformActions)
  {
    std::string actionTransformNodeID = transformAction->data().toString().toStdString();
    bool checked = allTransformsAreTheSame && currentTransformNodeID == actionTransformNodeID;
    if (transformAction->isChecked() != checked)
    {
      QSignalBlocker blocker1(transformAction);
      transformAction->setChecked(checked);
    }
    bool visible = false;
    if (transformAction == d->NoTransformAction)
    {
      visible = true;
    }
    else
    {
      vtkMRMLTransformNode* actionTransformNode = vtkMRMLTransformNode::SafeDownCast(scene->GetNodeByID(actionTransformNodeID.c_str()));
      if (actionTransformNode && !actionTransformNode->GetHideFromEditors()
        && actionTransformNode != node /* do not let apply transform to itself */)
      {
        visible = true;
      }
    }
    if (visible)
    {
      transformAction->setVisible(true);
    }
  }

  QSignalBlocker blocker2(d->TransformInteractionInViewAction);
  if (allTransformsAreTheSame && currentTransformNode != nullptr)
  {
    d->TransformInteractionInViewAction->setEnabled(true);
    bool interactionVisible = false;
    if (currentTransformNode && currentTransformNode->GetDisplayNode())
    {
      vtkMRMLTransformDisplayNode* displayNode =
        vtkMRMLTransformDisplayNode::SafeDownCast(currentTransformNode->GetDisplayNode());
      if (displayNode)
      {
        interactionVisible = displayNode->GetEditorVisibility();
      }
    }
    d->TransformInteractionInViewAction->setChecked(interactionVisible);
  }
  else
  {
    d->TransformInteractionInViewAction->setEnabled(false);
    d->TransformInteractionInViewAction->setChecked(false);
  }

  // Enable harden unless there is no applied transform at all (all transforms are nullptr)
  d->TransformHardenAction->setEnabled(!(allTransformsAreTheSame && currentTransformNode == nullptr));

  // Enable "Edit properties" if all transforms are the same (and not nullptr)
  d->TransformEditPropertiesAction->setEnabled(allTransformsAreTheSame && currentTransformNode != nullptr);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyTransformPlugin::transformContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyTransformPlugin);

  QList<QAction*> actions;

  actions << d->TransformInteractionInViewAction << d->TransformEditPropertiesAction << d->TransformHardenAction
          << d->NoTransformAction << d->CreateNewTransformAction;

  QList<QAction*> originalTransformActions = d->TransformActionGroup->actions();
  originalTransformActions.removeAll(d->NoTransformAction);

  QSignalBlocker blocker1(d->NoTransformAction);
  std::vector<vtkMRMLNode*> transformNodes;
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (scene)
  {
    scene->GetNodesByClass("vtkMRMLTransformNode", transformNodes);
  }
  for (std::vector<vtkMRMLNode*>::iterator it = transformNodes.begin(); it != transformNodes.end(); ++it)
  {
    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(*it);

    if (!transformNode || transformNode->GetHideFromEditors())
    {
      continue;
    }

    QAction* transformAction = nullptr;
    foreach(QAction * foundTransformAction, originalTransformActions)
    {
      if (foundTransformAction->data().toString().toStdString() == transformNode->GetID())
      {
        transformAction = foundTransformAction;
        break;
      }
    }

    QString transformNodeName;
    if (transformNode->GetName())
    {
      transformNodeName = QString::fromStdString(transformNode->GetName());
    }
    if (transformAction)
    {
      transformAction->setText(transformNodeName);
    }
    else
    {
      transformAction = new QAction(transformNodeName, d->NoTransformAction->parent());
      transformAction->setCheckable(true);
      transformAction->setVisible(false);
      this->setActionPosition(transformAction, this->transformListSection());
      transformAction->setData(QString(transformNode->GetID()));
      connect(transformAction, SIGNAL(triggered()), this, SLOT(onTransformActionSelected()), Qt::UniqueConnection);
      d->TransformActionGroup->addAction(transformAction);
    }
    actions << transformAction;
  }

  // Remove actions that belong to deleted transform nodes
  foreach(QAction * transformAction, originalTransformActions)
  {
    QString actionTransformNodeID = transformAction->data().toString();
    if (actionTransformNodeID.isEmpty())
    {
      // always keep the no transform action
      continue;
    }
    vtkMRMLTransformNode* actionTransformNode = vtkMRMLTransformNode::SafeDownCast(scene->GetNodeByID(actionTransformNodeID.toStdString().c_str()));
    if (actionTransformNode && !actionTransformNode->GetHideFromEditors())
    {
      // transform node still exists and visible in the scene
      continue;
    }
    d->TransformActionGroup->removeAction(transformAction);
    transformAction->deleteLater();
  }

  return actions;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPlugin::onTransformActionSelected()
{
  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  QAction* action = qobject_cast<QAction*>(this->sender());
  std::string selectedTransformNodeID = action->data().toString().toStdString();
  QList<vtkIdType> currentItemIDs;
  currentItemIDs = qSlicerSubjectHierarchyPluginHandler::instance()->currentItems();
  foreach (vtkIdType itemID, currentItemIDs)
  {
    d->applyTransformToItem(itemID, selectedTransformNodeID.c_str());
  }
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPlugin::onTransformEditProperties()
{
  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  vtkMRMLTransformNode* transformNode = d->firstAppliedTransformToSelectedItems();
  if (!transformNode)
  {
    return;
  }
  qSlicerApplication::application()->openNodeModule(transformNode);
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPlugin::onHardenTransformOnBranchOfCurrentItem()
{
  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  auto a = qSlicerSubjectHierarchyPluginHandler::instance()->currentTreeView();
  if (!a)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access node";
    return;
  }
  d->Model = a->model();
  if (!d->Model)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  d->Model->onHardenTransformOnBranchOfCurrentItem();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPlugin::onRemoveTransformsFromBranchOfCurrentItem()
{
  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  auto a = qSlicerSubjectHierarchyPluginHandler::instance()->currentTreeView();
  if (!a)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access node";
    return;
  }
  d->Model = a->model();
  if (!d->Model)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  d->Model->onRemoveTransformsFromBranchOfCurrentItem();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPlugin::onTransformInteractionInViewToggled(bool show)
{
  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  vtkMRMLTransformNode* transformNode = d->firstAppliedTransformToSelectedItems();
  if (!transformNode)
  {
    return;
  }
  transformNode->CreateDefaultDisplayNodes();
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  if (!displayNode)
  {
    return;
  }
  displayNode->SetEditorVisibility(show);
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPlugin::onCreateNewTransform()
{
  Q_D(qSlicerSubjectHierarchyTransformPlugin);
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
  }

  vtkMRMLTransformNode* newTransformNode =
    vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
  if (!newTransformNode)
  {
    qCritical() << Q_FUNC_INFO << ": failed to create new transform node";
    return;
  }
  QList<vtkIdType> currentItemIDs;
  currentItemIDs = qSlicerSubjectHierarchyPluginHandler::instance()->currentItems();
  foreach (vtkIdType itemID, currentItemIDs)
  {
    d->applyTransformToItem(itemID, newTransformNode->GetID());
  }
}

//------------------------------------------------------------------------------
int qSlicerSubjectHierarchyTransformPlugin::transformListSection()const
{
  // +20 to allow placing more items between default section and the transform list
  return qSlicerSubjectHierarchyAbstractPlugin::SectionDefault + 20;
}
