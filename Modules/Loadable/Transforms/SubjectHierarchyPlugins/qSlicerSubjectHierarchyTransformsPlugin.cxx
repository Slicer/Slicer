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

#include "qSlicerApplication.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTransformsPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Subject Hierarchy includes
#include "qMRMLSubjectHierarchyModel.h"
#include "qMRMLSubjectHierarchyTreeView.h"
#include <vtkSlicerSubjectHierarchyModuleLogic.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLTransformDisplayNode.h>

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkMatrix4x4.h>

// Qt includes
#include <QAction>
#include <QActionGroup>
#include <QDebug>
#include <QMenu>
#include <QMessageBox>
#include <QStandardItem>

// Transforms logic includes
#include <vtkSlicerTransformLogic.h>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyTransformsPluginPrivate : public QObject
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyTransformsPlugin);

protected:
  qSlicerSubjectHierarchyTransformsPlugin* const q_ptr;

public:
  qSlicerSubjectHierarchyTransformsPluginPrivate(qSlicerSubjectHierarchyTransformsPlugin& object);
  ~qSlicerSubjectHierarchyTransformsPluginPrivate() override;
  void init();

  // Transforms menu
  void applyTransformToItem(vtkIdType itemID, const char* transformNodeID);
  vtkMRMLTransformNode* appliedTransformToItem(vtkIdType itemID, bool& commonToAllChildren);
  vtkMRMLTransformNode* firstAppliedTransformToSelectedItems();

public:
  QIcon TransformIcon;

  QAction* InvertAction;
  QAction* IdentityAction;

  QAction* ResetCenterOfTransformLocalAction;

  QAction* ResetCenterOfTransformAllTransformedNodesBoundsAction;
  QAction* ResetCenterOfTransformTransformedNodeBoundsAction;

  QAction* ResetCenterOfTransformMenuAction;
  QMenu* ResetCenterOfTransformationMenu;

  QActionGroup* ResetCenterOfTransformationNodeGroup;

  QAction* ToggleInteractionAction;

  QAction* ToggleInteractionTranslationVisibleAction{ nullptr };
  QAction* ToggleInteractionRotationVisibleAction{ nullptr };
  QAction* ToggleInteractionScaleVisibleAction{ nullptr };
  QMenu* InteractionOptionsMenu{ nullptr };
  QAction* InteractionOptionsAction{ nullptr };

  QVariantMap ViewContextMenuEventData;

  vtkMRMLTransformNode* transformNodeFromViewContextMenuEventData();

  vtkMRMLTransformNode* getTransformNodeForAction();
  void resetCenterOfTransformationLocal(vtkMRMLTransformNode* transformNode);
  void resetCenterOfTransformationWorld(vtkMRMLTransformNode* transformNode);
  void resetCenterOfTransformationAllTransformedNodeBounds(vtkMRMLTransformNode* transformNode);

  void removeResetCenterOfTransformationForTransformedNodesActions();
  void showResetCenterOfTransformationForTransformedNodesActions(vtkMRMLTransformNode* transformNode);

  static const char* INTERACTION_HANDLE_TYPE_PROPERTY;
  enum INTERACTION_HANDLE_TYPE
  {
    INTERACTION_HANDLE_TYPE_TRANSLATION,
    INTERACTION_HANDLE_TYPE_ROTATION,
    INTERACTION_HANDLE_TYPE_SCALE,
  };

  // Transforms menu
  QAction* TransformInteractionInViewAction{ nullptr };
  QAction* TransformEditPropertiesAction{ nullptr };
  QAction* TransformHardenAction{ nullptr };
  QAction* CreateNewTransformAction{ nullptr };
  QAction* NoTransformAction{ nullptr };
  QActionGroup* TransformActionGroup{ nullptr };
};

//-----------------------------------------------------------------------------
const char* qSlicerSubjectHierarchyTransformsPluginPrivate::INTERACTION_HANDLE_TYPE_PROPERTY = "InteractionHandleType";

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPluginPrivate::qSlicerSubjectHierarchyTransformsPluginPrivate(qSlicerSubjectHierarchyTransformsPlugin& object)
  : q_ptr(&object)
{
  this->TransformIcon = QIcon(":Icons/Transform.png");
  this->ToggleInteractionAction = nullptr;
  this->ToggleInteractionTranslationVisibleAction = nullptr;
  this->ToggleInteractionRotationVisibleAction = nullptr;
  this->ToggleInteractionScaleVisibleAction = nullptr;
  this->InteractionOptionsMenu = nullptr;
  this->InteractionOptionsAction = nullptr;
  this->InvertAction = nullptr;
  this->IdentityAction = nullptr;
  this->ResetCenterOfTransformLocalAction = nullptr;
  this->ResetCenterOfTransformAllTransformedNodesBoundsAction = nullptr;
  this->ResetCenterOfTransformationMenu = nullptr;
  this->ResetCenterOfTransformMenuAction = nullptr;
  this->ResetCenterOfTransformationNodeGroup = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyTransformsPlugin);

  int transformWeight = 0;
  int transformOffset = 0;

  this->InvertAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Invert transform"), q);
  this->InvertAction->setObjectName("InvertTransform");
  q->setActionPosition(this->InvertAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, transformWeight, transformOffset++);
  QObject::connect(this->InvertAction, SIGNAL(triggered()), q, SLOT(invert()));

  this->IdentityAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Reset transform to identity"), q);
  this->IdentityAction->setObjectName("ResetToIdentity");
  q->setActionPosition(this->IdentityAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, transformWeight, transformOffset++);
  QObject::connect(this->IdentityAction, SIGNAL(triggered()), q, SLOT(identity()));

  this->ResetCenterOfTransformLocalAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Transform origin"), q);
  this->ResetCenterOfTransformLocalAction->setObjectName("ResetCenterOfTransformationLocal");
  QObject::connect(this->ResetCenterOfTransformLocalAction, SIGNAL(triggered()), q, SLOT(resetCenterOfTransformationLocal()));

  this->ResetCenterOfTransformAllTransformedNodesBoundsAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Transformed nodes"), q);
  QObject::connect(this->ResetCenterOfTransformAllTransformedNodesBoundsAction, SIGNAL(triggered()), q, SLOT(resetCenterOfTransformationAllTransformedNodeBounds()));
  this->ResetCenterOfTransformAllTransformedNodesBoundsAction->setObjectName("ResetCenterOfTransformationAllTransformedNodesBounds");

  this->ResetCenterOfTransformationMenu = new QMenu();
  this->ResetCenterOfTransformationMenu->addAction(this->ResetCenterOfTransformLocalAction);
  this->ResetCenterOfTransformationMenu->addAction(this->ResetCenterOfTransformAllTransformedNodesBoundsAction);
  this->ResetCenterOfTransformationMenu->addSeparator();

  this->ResetCenterOfTransformMenuAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Reset center of transformation"));
  this->ResetCenterOfTransformMenuAction->setObjectName("ResetCenterOfTransformationMenu");
  q->setActionPosition(this->ResetCenterOfTransformMenuAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, transformWeight, transformOffset++);
  this->ResetCenterOfTransformMenuAction->setMenu(this->ResetCenterOfTransformationMenu);

  this->ResetCenterOfTransformationNodeGroup = new QActionGroup(this->ResetCenterOfTransformMenuAction);

  this->ToggleInteractionAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Interaction"), q);
  this->ToggleInteractionAction->setObjectName("ToggleInteractionAction");
  q->setActionPosition(this->ToggleInteractionAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, transformWeight, transformOffset++);
  QObject::connect(this->ToggleInteractionAction, SIGNAL(toggled(bool)), q, SLOT(toggleInteractionBox(bool)));
  this->ToggleInteractionAction->setCheckable(true);
  this->ToggleInteractionAction->setChecked(false);

  this->ToggleInteractionTranslationVisibleAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Translate"));
  this->ToggleInteractionTranslationVisibleAction->setObjectName("ToggleInteractionTranslationVisibleAction");
  this->ToggleInteractionTranslationVisibleAction->setCheckable(true);
  this->ToggleInteractionTranslationVisibleAction->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, INTERACTION_HANDLE_TYPE_TRANSLATION);
  QObject::connect(this->ToggleInteractionTranslationVisibleAction, SIGNAL(toggled(bool)), q, SLOT(toggleCurrentItemHandleTypeVisibility(bool)));

  this->ToggleInteractionRotationVisibleAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Rotate"));
  this->ToggleInteractionRotationVisibleAction->setObjectName("ToggleInteractionRotationVisibleAction");
  this->ToggleInteractionRotationVisibleAction->setCheckable(true);
  this->ToggleInteractionRotationVisibleAction->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, INTERACTION_HANDLE_TYPE_ROTATION);
  QObject::connect(this->ToggleInteractionRotationVisibleAction, SIGNAL(toggled(bool)), q, SLOT(toggleCurrentItemHandleTypeVisibility(bool)));

  this->ToggleInteractionScaleVisibleAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Scale"));
  this->ToggleInteractionScaleVisibleAction->setObjectName("ToggleInteractionScaleVisibleAction");
  this->ToggleInteractionScaleVisibleAction->setCheckable(true);
  this->ToggleInteractionScaleVisibleAction->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, INTERACTION_HANDLE_TYPE_SCALE);
  QObject::connect(this->ToggleInteractionScaleVisibleAction, SIGNAL(toggled(bool)), q, SLOT(toggleCurrentItemHandleTypeVisibility(bool)));

  this->InteractionOptionsMenu = new QMenu();
  this->InteractionOptionsMenu->setObjectName("InteractionOptionsMenu");
  this->InteractionOptionsMenu->addAction(this->ToggleInteractionTranslationVisibleAction);
  this->InteractionOptionsMenu->addAction(this->ToggleInteractionRotationVisibleAction);
  this->InteractionOptionsMenu->addAction(this->ToggleInteractionScaleVisibleAction);

  this->InteractionOptionsAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Interaction options"));
  this->InteractionOptionsAction->setObjectName("InteractionOptionsMenuAction");
  q->setActionPosition(this->InteractionOptionsAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode, transformWeight, transformOffset++);
  this->InteractionOptionsAction->setMenu(this->InteractionOptionsMenu);

  // Transforms menu
  this->TransformInteractionInViewAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Interaction"), q);
  this->TransformInteractionInViewAction->setObjectName("TransformInteraction");
  this->TransformInteractionInViewAction->setCheckable(true);
  this->TransformInteractionInViewAction->setToolTip(qSlicerSubjectHierarchyTransformsPlugin::tr("Allow the transform to be modified interactively in the 2D and 3D views"));
  QObject::connect(this->TransformInteractionInViewAction, SIGNAL(toggled(bool)), q, SLOT(onTransformInteractionInViewToggled(bool)));

  this->TransformEditPropertiesAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Edit transform properties..."), q);
  this->TransformEditPropertiesAction->setObjectName("TransformEditProperties");
  this->TransformEditPropertiesAction->setToolTip(qSlicerSubjectHierarchyTransformsPlugin::tr("Edit properties of the current transform"));
  QObject::connect(this->TransformEditPropertiesAction, SIGNAL(triggered()), q, SLOT(onTransformEditProperties()));

  this->TransformHardenAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Harden transform"), q);
  this->TransformHardenAction->setObjectName("TransformHarden");
  this->TransformHardenAction->setToolTip(qSlicerSubjectHierarchyTransformsPlugin::tr("Harden current transform on this node and all children nodes"));
  QObject::connect(this->TransformHardenAction, SIGNAL(triggered()), q, SLOT(onHardenTransformOnBranchOfCurrentItem()));

  this->CreateNewTransformAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Create new transform"), q);
  this->CreateNewTransformAction->setObjectName("TransformCreateNew");
  this->CreateNewTransformAction->setToolTip(qSlicerSubjectHierarchyTransformsPlugin::tr("Create and apply new transform"));
  QObject::connect(this->CreateNewTransformAction, SIGNAL(triggered()), q, SLOT(onCreateNewTransform()));

  this->NoTransformAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("None") /*: Displayed in the transforms submenu */);
  this->NoTransformAction->setObjectName("Transform:None");
  q->setActionPosition(this->NoTransformAction, q->transformListSection());
  this->NoTransformAction->setCheckable(true);
  this->NoTransformAction->setData(QString()); // user data stores the transform node ID
  this->NoTransformAction->setToolTip(qSlicerSubjectHierarchyTransformsPlugin::tr("Remove parent transform from all the nodes in this branch"));
  QObject::connect(this->NoTransformAction, SIGNAL(triggered()), q, SLOT(onRemoveTransformsFromBranchOfCurrentItem()));

  this->TransformActionGroup = new QActionGroup(q);
  this->TransformActionGroup->setExclusive(true);
  this->TransformActionGroup->addAction(this->NoTransformAction);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPluginPrivate::~qSlicerSubjectHierarchyTransformsPluginPrivate() = default;

//-----------------------------------------------------------------------------
vtkMRMLTransformNode* qSlicerSubjectHierarchyTransformsPluginPrivate::transformNodeFromViewContextMenuEventData()
{
  Q_Q(qSlicerSubjectHierarchyTransformsPlugin);
  if (this->ViewContextMenuEventData.find("NodeID") == this->ViewContextMenuEventData.end())
  {
    return nullptr;
  }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
  {
    return nullptr;
  }

  // Get transform node
  QString nodeID = this->ViewContextMenuEventData["NodeID"].toString();
  vtkMRMLNode* node = scene->GetNodeByID(nodeID.toUtf8().constData());
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(node);
  return transformNode;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::resetCenterOfTransformationLocal(vtkMRMLTransformNode* transformNode)
{
  if (!transformNode)
  {
    return;
  }
  transformNode->SetCenterOfTransformation(0, 0, 0);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::resetCenterOfTransformationWorld(vtkMRMLTransformNode* transformNode)
{
  if (!transformNode)
  {
    return;
  }

  vtkNew<vtkGeneralTransform> transformFromWorld;
  transformNode->GetTransformFromWorld(transformFromWorld);

  double center_Local[3] = { 0.0, 0.0, 0.0 };
  transformFromWorld->TransformPoint(center_Local, center_Local);
  transformNode->SetCenterOfTransformation(center_Local);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::resetCenterOfTransformationAllTransformedNodeBounds(vtkMRMLTransformNode* transformNode)
{
  if (!transformNode)
  {
    return;
  }

  vtkMRMLScene* scene = transformNode->GetScene();
  std::vector<vtkMRMLDisplayableNode*> transformedNodes;
  vtkSlicerTransformLogic::GetTransformedNodes(scene, transformNode, transformedNodes);

  bool boundsValid = false;
  double bounds_World[6] = { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, VTK_DOUBLE_MIN };

  for (vtkMRMLDisplayableNode* transformedNode : transformedNodes)
  {
    double nodeBounds_World[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    transformedNode->GetRASBounds(nodeBounds_World);

    if (!vtkMath::AreBoundsInitialized(nodeBounds_World))
    {
      // Skip nodes with uninitialized bounds
      continue;
    }

    boundsValid = true;

    for (int i = 0; i < 6; i += 2)
    {
      if (nodeBounds_World[i] < bounds_World[i])
      {
        bounds_World[i] = nodeBounds_World[i];
      }
      if (nodeBounds_World[i + 1] > bounds_World[i + 1])
      {
        bounds_World[i + 1] = nodeBounds_World[i + 1];
      }
    }
  }

  if (!boundsValid)
  {
    // No valid bounds.
    return;
  }

  double center_World[3] = { (bounds_World[0] + bounds_World[1]) / 2.0, (bounds_World[2] + bounds_World[3]) / 2.0, (bounds_World[4] + bounds_World[5]) / 2.0 };

  vtkNew<vtkGeneralTransform> transformFromWorld;
  transformNode->GetTransformFromWorld(transformFromWorld);

  double center_Local[3] = { 0.0, 0.0, 0.0 };
  transformFromWorld->TransformPoint(center_World, center_Local);
  transformNode->SetCenterOfTransformation(center_Local);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::showResetCenterOfTransformationForTransformedNodesActions(vtkMRMLTransformNode* transformNode)
{
  Q_Q(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLScene* scene = transformNode->GetScene();
  std::vector<vtkMRMLDisplayableNode*> transformedNodes;
  vtkSlicerTransformLogic::GetTransformedNodes(scene, transformNode, transformedNodes);

  for (vtkMRMLDisplayableNode* transformNode : transformedNodes)
  {
    double bounds_World[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
    transformNode->GetRASBounds(bounds_World);
    if (!vtkMath::AreBoundsInitialized(bounds_World))
    {
      // Skip nodes with uninitialized bounds
      continue;
    }

    QAction* action = this->ResetCenterOfTransformationMenu->addAction(transformNode->GetName());
    action->setData(QVariant(transformNode->GetID()));
    this->ResetCenterOfTransformationNodeGroup->addAction(action);
    QObject::connect(action, SIGNAL(triggered()), q, SLOT(resetCenterOfTransformationTransformedNodeBounds()));
  }

  if (transformedNodes.empty())
  {
    this->ResetCenterOfTransformAllTransformedNodesBoundsAction->setVisible(false);
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::removeResetCenterOfTransformationForTransformedNodesActions()
{
  QList<QAction*> transformActions = this->ResetCenterOfTransformationNodeGroup->actions();
  foreach (QAction* transformAction, transformActions)
  {
    this->ResetCenterOfTransformationNodeGroup->removeAction(transformAction);
    this->ResetCenterOfTransformationMenu->removeAction(transformAction);
  }
}

//------------------------------------------------------------------------------
vtkMRMLTransformNode* qSlicerSubjectHierarchyTransformsPluginPrivate::appliedTransformToItem(vtkIdType itemID, bool& commonToAllChildren)
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
  for (std::vector<vtkIdType>::iterator itemIDToTransformIt = itemIDsToTransform.begin(); itemIDToTransformIt != itemIDsToTransform.end(); ++itemIDToTransformIt)
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
void qSlicerSubjectHierarchyTransformsPluginPrivate::applyTransformToItem(vtkIdType itemID, const char* transformNodeID)
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
  for (std::vector<vtkIdType>::iterator itemIDToTransformIt = itemIDsToTransform.begin(); itemIDToTransformIt != itemIDsToTransform.end(); ++itemIDToTransformIt)
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
vtkMRMLTransformNode* qSlicerSubjectHierarchyTransformsPluginPrivate::firstAppliedTransformToSelectedItems()
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
    for (std::vector<vtkIdType>::iterator childItemIDsIt = childItemIDs.begin(); childItemIDsIt != childItemIDs.end(); ++childItemIDsIt)
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

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPlugin::qSlicerSubjectHierarchyTransformsPlugin(QObject* parent)
  : Superclass(parent)
  , d_ptr(new qSlicerSubjectHierarchyTransformsPluginPrivate(*this))
{
  this->m_Name = QString("Transforms");

  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPlugin::~qSlicerSubjectHierarchyTransformsPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyTransformsPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID) const
{
  if (!itemID || !parentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
  }

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  if (transformNode)
  {
    // If parent item is transform then can reparent
    return 1.0;
  }

  return 0.0;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyTransformsPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  if (!itemID || !parentItemID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return false;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
  }

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  if (!transformNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access transform node from parent item " << parentItemID;
    return false;
  }

  // Ask the user if any child node in the branch is transformed with a transform different from the chosen one
  bool hardenExistingTransforms = true;
  if (shNode->IsAnyNodeInBranchTransformed(itemID))
  {
    QMessageBox::StandardButton answer = QMessageBox::question(nullptr,
                                                               tr("Some nodes in the branch are already transformed"),
                                                               tr("Do you want to harden all already applied transforms before setting the new one?\n\n"
                                                                  "  Note: If you choose no, then the applied transform will simply be replaced."),
                                                               QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                                               QMessageBox::Yes);
    if (answer == QMessageBox::No)
    {
      hardenExistingTransforms = false;
    }
    else if (answer == QMessageBox::Cancel)
    {
      return false;
    }
  }

  // Transform all items in branch
  vtkSlicerSubjectHierarchyModuleLogic::TransformBranch(shNode, itemID, transformNode, hardenExistingTransforms);

  // Actual reparenting will never happen, only setting of the transform
  return false;
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyTransformsPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkIdType parentItemID /*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/) const
{
  Q_UNUSED(parentItemID);
  if (!node)
  {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr";
    return 0.0;
  }
  else if (node->IsA("vtkMRMLTransformNode"))
  {
    // Node is a transform
    return 0.5;
  }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyTransformsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID) const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return 0.0;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return 0.0;
  }

  // Transform
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLTransformNode"))
  {
    return 0.5; // There are other plugins that can handle special transform nodes better, thus the relatively low value
  }

  if (associatedNode && associatedNode->IsA("vtkMRMLDisplayableNode") //
      && !associatedNode->IsA("vtkMRMLMarkupsNode"))                  // Markups are handled by the Markups plugin
  {
    return 0.1; // Most other plugins can handle displayable nodes better.
  }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyTransformsPlugin::roleForPlugin() const
{
  return "Transform";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyTransformsPlugin::tooltip(vtkIdType itemID) const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QString("Invalid");
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString("Error");
  }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (transformNode)
  {
    QString transformInfo = QString("\n") + tr("Transform to parent:") + QString(" %1\n").arg(transformNode->GetTransformToParentInfo()) + tr("Transform from parent:")
                            + QString(" %1").arg(transformNode->GetTransformFromParentInfo());
    tooltipString.append(transformInfo);
  }

  return tooltipString;
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTransformsPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
  }

  // Transform
  if (this->canOwnSubjectHierarchyItem(itemID))
  {
    return d->TransformIcon;
  }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyTransformsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::itemContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyTransformsPlugin);

  QList<QAction*> actions;
  actions << d->InvertAction << d->IdentityAction << d->ResetCenterOfTransformMenuAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    // There are no scene actions in this plugin
    return;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  d->removeResetCenterOfTransformationForTransformedNodesActions();

  if (this->canOwnSubjectHierarchyItem(itemID))
  {
    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (transformNode)
    {
      d->InvertAction->setVisible(true);
      vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
      if (tnode && tnode->IsLinear())
      {
        d->IdentityAction->setVisible(true);
      }
      d->ResetCenterOfTransformLocalAction->setVisible(true);
      d->ResetCenterOfTransformMenuAction->setVisible(true);

      d->ResetCenterOfTransformAllTransformedNodesBoundsAction->setVisible(true);
      d->showResetCenterOfTransformationForTransformedNodesActions(transformNode);
    }
  }
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::visibilityContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyTransformsPlugin);

  QList<QAction*> actions;
  actions << d->ToggleInteractionAction << d->InteractionOptionsAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    return;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  if (this->canOwnSubjectHierarchyItem(itemID))
  {
    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (!transformableNode)
    {
      return;
    }

    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (!transformNode)
    {
      transformNode = transformableNode->GetParentTransformNode();
    }

    vtkMRMLTransformDisplayNode* displayNode = transformNode ? vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode()) : nullptr;

    d->ToggleInteractionAction->setVisible(true);

    QSignalBlocker toggleInteractionBlocker(d->ToggleInteractionAction);
    d->ToggleInteractionAction->setChecked(displayNode ? displayNode->GetEditorVisibility() : false);

    d->InteractionOptionsAction->setVisible(displayNode != nullptr);
    if (displayNode)
    {
      QSignalBlocker toggleInteractionRotationVisibleBlocker(d->ToggleInteractionRotationVisibleAction);
      d->ToggleInteractionRotationVisibleAction->setChecked(displayNode->GetEditorRotationEnabled());

      QSignalBlocker toggleInteractionTranslationVisibleBlocker(d->ToggleInteractionTranslationVisibleAction);
      d->ToggleInteractionTranslationVisibleAction->setChecked(displayNode->GetEditorTranslationEnabled());

      QSignalBlocker toggleInteractionScaleVisibleBlocker(d->ToggleInteractionScaleVisibleAction);
      d->ToggleInteractionScaleVisibleAction->setChecked(displayNode->GetEditorScalingEnabled());
    }
  }
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::viewContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyTransformsPlugin);

  QList<QAction*> actions;
  actions << d->InvertAction << d->IdentityAction << d->ResetCenterOfTransformMenuAction << d->ToggleInteractionAction << d->InteractionOptionsAction;
  return actions;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  // make sure we don't use metadata from some previous view context menu calls
  d->ViewContextMenuEventData.clear();
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return;
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!transformNode)
  {
    return;
  }

  d->ViewContextMenuEventData = eventData;
  d->ViewContextMenuEventData["NodeID"] = QVariant(transformNode->GetID());

  d->InvertAction->setVisible(true);
  d->IdentityAction->setVisible(true);
  d->ResetCenterOfTransformLocalAction->setVisible(true);
  d->ResetCenterOfTransformAllTransformedNodesBoundsAction->setVisible(true);
  d->ResetCenterOfTransformMenuAction->setVisible(true);
  d->ToggleInteractionAction->setVisible(true);

  d->removeResetCenterOfTransformationForTransformedNodesActions();
  d->showResetCenterOfTransformationForTransformedNodesActions(transformNode);

  vtkMRMLTransformDisplayNode* displayNode = transformNode ? vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode()) : nullptr;

  QSignalBlocker toggleInteractionBlocker(d->ToggleInteractionAction);
  d->ToggleInteractionAction->setChecked(displayNode ? displayNode->GetEditorVisibility() : false);

  d->InteractionOptionsAction->setVisible(displayNode != nullptr);
  if (displayNode)
  {
    QSignalBlocker toggleInteractionRotationVisibleBlocker(d->ToggleInteractionRotationVisibleAction);
    d->ToggleInteractionRotationVisibleAction->setChecked(displayNode->GetEditorRotationEnabled());

    QSignalBlocker toggleInteractionTranslationVisibleBlocker(d->ToggleInteractionTranslationVisibleAction);
    d->ToggleInteractionTranslationVisibleAction->setChecked(displayNode->GetEditorTranslationEnabled());

    QSignalBlocker toggleInteractionScaleVisibleBlocker(d->ToggleInteractionScaleVisibleAction);
    d->ToggleInteractionScaleVisibleAction->setChecked(displayNode->GetEditorScalingEnabled());
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::invert()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (transformNode)
  {
    MRMLNodeModifyBlocker blocker(transformNode);
    transformNode->Inverse();
    transformNode->InverseName();
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::identity()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);

  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (transformNode && transformNode->IsLinear())
  {
    vtkNew<vtkMatrix4x4> matrix; // initialized to identity by default
    transformNode->SetMatrixTransformToParent(matrix.GetPointer());
  }
}

//---------------------------------------------------------------------------
vtkMRMLTransformNode* qSlicerSubjectHierarchyTransformsPluginPrivate::getTransformNodeForAction()
{
  vtkMRMLTransformNode* transformNode = this->transformNodeFromViewContextMenuEventData();
  if (transformNode)
  {
    return transformNode;
  }

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return nullptr;
  }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return nullptr;
  }

  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  transformNode = vtkMRMLTransformNode::SafeDownCast(transformableNode);
  if (!transformNode && transformableNode)
  {
    transformNode = transformableNode->GetParentTransformNode();
  }
  return transformNode;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::resetCenterOfTransformationLocal()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (!transformNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access transform node";
    return;
  }
  d->resetCenterOfTransformationLocal(transformNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::resetCenterOfTransformationWorld()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (!transformNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access transform node";
    return;
  }
  d->resetCenterOfTransformationWorld(transformNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::resetCenterOfTransformationAllTransformedNodeBounds()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (!transformNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access transform node";
    return;
  }
  d->resetCenterOfTransformationAllTransformedNodeBounds(transformNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::resetCenterOfTransformationTransformedNodeBounds()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (!transformNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access transform node";
    return;
  }

  QAction* action = qobject_cast<QAction*>(QObject::sender());
  if (!action)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access action";
    return;
  }

  vtkMRMLScene* scene = transformNode->GetScene();
  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access scene";
    return;
  }

  QString nodeID = action->data().toString();
  vtkMRMLNode* node = scene->GetNodeByID(nodeID.toUtf8().constData());
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(node);
  if (!displayableNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access displayable node";
    return;
  }

  double bounds_World[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
  displayableNode->GetRASBounds(bounds_World);

  double center_World[3] = { (bounds_World[0] + bounds_World[1]) / 2.0, (bounds_World[2] + bounds_World[3]) / 2.0, (bounds_World[4] + bounds_World[5]) / 2.0 };

  vtkNew<vtkGeneralTransform> transformFromWorld;
  transformNode->GetTransformFromWorld(transformFromWorld);

  double center_Local[3] = { 0.0, 0.0, 0.0 };
  transformFromWorld->TransformPoint(center_World, center_Local);

  transformNode->SetCenterOfTransformation(center_Local);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::toggleInteractionBox(bool visible)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }

  vtkMRMLTransformableNode* transformableNode = nullptr;
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    transformableNode = vtkMRMLTransformableNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  }

  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (!transformNode)
  {
    transformNode = transformableNode->GetParentTransformNode();
  }

  if (!transformNode && transformableNode)
  {
    std::stringstream transformNameSS;
    transformNameSS << "Interaction_" << transformableNode->GetName();
    transformNode = vtkMRMLTransformNode::SafeDownCast(transformableNode->GetScene()->AddNewNodeByClass("vtkMRMLTransformNode", transformNameSS.str()));
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(transformableNode);
    if (displayableNode)
    {
      double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
      displayableNode->GetBounds(bounds);
      transformNode->SetCenterOfTransformation((bounds[1] + bounds[0]) / 2.0, (bounds[3] + bounds[2]) / 2.0, (bounds[5] + bounds[4]) / 2.0);
    }

    transformNode->CreateDefaultDisplayNodes();
    transformableNode->SetAndObserveTransformNodeID(transformNode->GetID());
  }

  if (!transformNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get or create transform node";
    return;
  }
  transformNode->CreateDefaultDisplayNodes();

  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  if (!displayNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get transform display node";
    return;
  }
  displayNode->SetEditorVisibility(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::toggleCurrentItemHandleTypeVisibility(bool toggle)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);

  vtkMRMLTransformNode* transformNode = d->getTransformNodeForAction();
  if (!transformNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get transform node";
    return;
  }

  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
  if (!displayNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get display node";
    return;
  }

  QAction* sender = qobject_cast<QAction*>(QObject::sender());
  if (!sender)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get sender";
    return;
  }

  int componentType = sender->property(qSlicerSubjectHierarchyTransformsPluginPrivate::INTERACTION_HANDLE_TYPE_PROPERTY).toInt();
  switch (componentType)
  {
    case qSlicerSubjectHierarchyTransformsPluginPrivate::INTERACTION_HANDLE_TYPE_TRANSLATION:
      displayNode->SetEditorTranslationEnabled(toggle);
      displayNode->SetEditorTranslationSliceEnabled(toggle);
      break;
    case qSlicerSubjectHierarchyTransformsPluginPrivate::INTERACTION_HANDLE_TYPE_ROTATION:
      displayNode->SetEditorRotationEnabled(toggle);
      displayNode->SetEditorRotationSliceEnabled(toggle);
      break;
    case qSlicerSubjectHierarchyTransformsPluginPrivate::INTERACTION_HANDLE_TYPE_SCALE:
      displayNode->SetEditorScalingEnabled(toggle);
      displayNode->SetEditorScalingSliceEnabled(toggle);
      break;
    default: break;
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::showTransformContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
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
      if (actionTransformNode && !actionTransformNode->GetHideFromEditors() //
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
      vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(currentTransformNode->GetDisplayNode());
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
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::transformContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyTransformsPlugin);

  QList<QAction*> actions;

  actions << d->TransformInteractionInViewAction << d->TransformEditPropertiesAction << d->TransformHardenAction << d->NoTransformAction << d->CreateNewTransformAction;

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
    foreach (QAction* foundTransformAction, originalTransformActions)
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
  foreach (QAction* transformAction, originalTransformActions)
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
void qSlicerSubjectHierarchyTransformsPlugin::onTransformActionSelected()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
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
void qSlicerSubjectHierarchyTransformsPlugin::onTransformEditProperties()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLTransformNode* transformNode = d->firstAppliedTransformToSelectedItems();
  if (!transformNode)
  {
    return;
  }
  qSlicerApplication::application()->openNodeModule(transformNode);
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::onHardenTransformOnBranchOfCurrentItem()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  auto a = qSlicerSubjectHierarchyPluginHandler::instance()->currentTreeView();
  if (!a)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access node";
    return;
  }
  if (!a->model())
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  a->model()->onHardenTransformOnBranchOfCurrentItem();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::onRemoveTransformsFromBranchOfCurrentItem()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  auto a = qSlicerSubjectHierarchyPluginHandler::instance()->currentTreeView();
  if (!a)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access node";
    return;
  }
  if (!a->model())
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  a->model()->onRemoveTransformsFromBranchOfCurrentItem();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::onTransformInteractionInViewToggled(bool show)
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
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
void qSlicerSubjectHierarchyTransformsPlugin::onCreateNewTransform()
{
  Q_D(qSlicerSubjectHierarchyTransformsPlugin);
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();

  if (!scene)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
  }

  vtkMRMLTransformNode* newTransformNode = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
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
int qSlicerSubjectHierarchyTransformsPlugin::transformListSection() const
{
  // +20 to allow placing more items between default section and the transform list
  return qSlicerSubjectHierarchyAbstractPlugin::SectionDefault + 20;
}

#include "qSlicerSubjectHierarchyTransformsPlugin.moc"
