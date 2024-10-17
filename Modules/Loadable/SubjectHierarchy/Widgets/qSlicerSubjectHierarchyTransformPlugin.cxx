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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

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
  qMRMLSubjectHierarchyModel* Model;

  QMenu* TransformMenu;
  QAction* TransformInteractionInViewAction;
  QAction* TransformEditPropertiesAction;
  QAction* TransformHardenAction;
  QAction* CreateNewTransformAction;
  QAction* NoTransformAction;
  QActionGroup* TransformActionGroup;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformPluginPrivate::qSlicerSubjectHierarchyTransformPluginPrivate(
  qSlicerSubjectHierarchyTransformPlugin& object)
  : q_ptr(&object)
{
  this->Model = nullptr;
  this->TransformMenu = nullptr;
  this->TransformInteractionInViewAction = nullptr;
  this->TransformEditPropertiesAction = nullptr;
  this->TransformHardenAction = nullptr;
  this->NoTransformAction = nullptr;
  this->TransformActionGroup = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyTransformPlugin);

  // Transform
  this->TransformMenu = new QMenu();

  this->TransformInteractionInViewAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Interaction"), this->TransformMenu);
  this->TransformInteractionInViewAction->setCheckable(true);
  this->TransformInteractionInViewAction->setToolTip(qSlicerSubjectHierarchyTransformPlugin::tr(
    "Allow the transform to be modified interactively in the 2D and 3D views"));
  this->TransformMenu->addAction(this->TransformInteractionInViewAction);
  QObject::connect(
    this->TransformInteractionInViewAction, SIGNAL(toggled(bool)), q, SLOT(onTransformInteractionInViewToggled(bool)));

  this->TransformEditPropertiesAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Edit transform properties..."), this->TransformMenu);
  this->TransformEditPropertiesAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Edit properties of the current transform"));
  this->TransformMenu->addAction(this->TransformEditPropertiesAction);
  QObject::connect(this->TransformEditPropertiesAction, SIGNAL(triggered()), q, SLOT(onTransformEditProperties()));

  this->TransformHardenAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Harden transform"), this->TransformMenu);
  this->TransformHardenAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Harden current transform on this node and all children nodes"));
  this->TransformMenu->addAction(this->TransformHardenAction);
  QObject::connect(this->TransformHardenAction, SIGNAL(triggered()), q, SLOT(onHardenTransformOnBranchOfCurrentItem()));

  this->CreateNewTransformAction =
    new QAction(qSlicerSubjectHierarchyTransformPlugin::tr("Create new transform"), this->TransformMenu);
  this->CreateNewTransformAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Create and apply new transform"));
  this->TransformMenu->addAction(this->CreateNewTransformAction);
  QObject::connect(this->CreateNewTransformAction, SIGNAL(triggered()), q, SLOT(onCreateNewTransform()));

  this->NoTransformAction = new QAction(
    qSlicerSubjectHierarchyTransformPlugin::tr("None") /*: Displayed in the transforms submenu */, this->TransformMenu);
  this->NoTransformAction->setCheckable(true);
  this->NoTransformAction->setToolTip(
    qSlicerSubjectHierarchyTransformPlugin::tr("Remove parent transform from all the nodes in this branch"));
  this->TransformMenu->addAction(this->NoTransformAction);
  QObject::connect(this->NoTransformAction, SIGNAL(triggered()), q, SLOT(onRemoveTransformsFromBranchOfCurrentItem()));

  this->TransformActionGroup = new QActionGroup(this->TransformMenu);
  this->TransformActionGroup->addAction(this->NoTransformAction);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformPluginPrivate::~qSlicerSubjectHierarchyTransformPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformPlugin methods

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
vtkMRMLTransformNode* qSlicerSubjectHierarchyTransformPluginPrivate::appliedTransformToItem(vtkIdType itemID,
                                                                                            bool& commonToAllChildren)
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
  d->NoTransformAction->setVisible(true);

  vtkMRMLTransformableNode* node = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  bool allTransformsAreTheSame = false;
  vtkMRMLTransformNode* currentTransformNode = d->appliedTransformToItem(itemID, allTransformsAreTheSame);
  QList<QAction*> transformActions = d->TransformActionGroup->actions();
  foreach (QAction* transformAction, transformActions)
  {
    if (transformAction == d->NoTransformAction)
    {
      continue;
    }
    d->TransformActionGroup->removeAction(transformAction);
  }
  QSignalBlocker blocker1(d->NoTransformAction);
  d->NoTransformAction->setChecked(allTransformsAreTheSame && currentTransformNode == nullptr);
  std::vector<vtkMRMLNode*> transformNodes;
  scene->GetNodesByClass("vtkMRMLTransformNode", transformNodes);
  for (std::vector<vtkMRMLNode*>::iterator it = transformNodes.begin(); it != transformNodes.end(); ++it)
  {
    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(*it);

    if (!transformNode || transformNode->GetHideFromEditors())
    {
      continue;
    }
    if (transformNode == node)
    {
      // do not let apply transform to itself
      continue;
    }

    QAction* nodeAction = new QAction(transformNode->GetName(), d->TransformMenu);
    nodeAction->setData(QString(transformNode->GetID()));
    nodeAction->setCheckable(true);
    nodeAction->setVisible(true);

    if (allTransformsAreTheSame && transformNode == currentTransformNode)
    {
      nodeAction->setChecked(allTransformsAreTheSame && transformNode == currentTransformNode);
    }
    connect(nodeAction, SIGNAL(triggered()), this, SLOT(onTransformActionSelected()), Qt::DirectConnection);
    d->TransformActionGroup->addAction(nodeAction);
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
QList<QAction*> qSlicerSubjectHierarchyTransformPlugin::transformContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyTransformPlugin);

  QList<QAction*> actions;

  actions << d->TransformInteractionInViewAction << d->TransformEditPropertiesAction << d->TransformHardenAction
          << d->NoTransformAction;

  // Add separator between persistent actions and transform actions
  QAction* separator = new QAction();
  separator->setSeparator(true);
  actions << separator;

  QList<QAction*> transformActions = d->TransformActionGroup->actions();
  foreach (QAction* transformAction, transformActions)
  {
    actions << transformAction;
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
