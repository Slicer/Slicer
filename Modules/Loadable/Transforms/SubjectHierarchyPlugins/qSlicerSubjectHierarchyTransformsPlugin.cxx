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
#include "qSlicerSubjectHierarchyTransformsPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Subject Hierarchy includes
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
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyTransformsPlugin);
protected:
  qSlicerSubjectHierarchyTransformsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyTransformsPluginPrivate(qSlicerSubjectHierarchyTransformsPlugin& object);
  ~qSlicerSubjectHierarchyTransformsPluginPrivate() override;
  void init();
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
  QAction* ToggleInteractionItemAction;

  QVariantMap ViewContextMenuEventData;

  vtkMRMLTransformNode* transformNodeFromViewContextMenuEventData();

  vtkMRMLTransformNode* getTransformNodeForAction();
  void resetCenterOfTransformationLocal(vtkMRMLTransformNode* transformNode);
  void resetCenterOfTransformationWorld(vtkMRMLTransformNode* transformNode);
  void resetCenterOfTransformationAllTransformedNodeBounds(vtkMRMLTransformNode* transformNode);

  void removeResetCenterOfTransformationForTransformedNodesActions();
  void showResetCenterOfTransformationForTransformedNodesActions(vtkMRMLTransformNode* transformNode);
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPluginPrivate::qSlicerSubjectHierarchyTransformsPluginPrivate(qSlicerSubjectHierarchyTransformsPlugin& object)
  : q_ptr(&object)
{
  this->TransformIcon = QIcon(":Icons/Transform.png");
  this->ToggleInteractionAction = nullptr;
  this->ToggleInteractionItemAction = nullptr;
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

  this->InvertAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Invert transform"), q);
  QObject::connect(this->InvertAction, SIGNAL(triggered()), q, SLOT(invert()));
  this->InvertAction->setObjectName("InvertTransform");

  this->IdentityAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Reset transform to identity"), q);
  QObject::connect(this->IdentityAction, SIGNAL(triggered()), q, SLOT(identity()));
  this->IdentityAction->setObjectName("ResetToIdentity");

  this->ResetCenterOfTransformLocalAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Transform origin"), q);
  QObject::connect(this->ResetCenterOfTransformLocalAction, SIGNAL(triggered()), q, SLOT(resetCenterOfTransformationLocal()));
  this->ResetCenterOfTransformLocalAction->setObjectName("ResetCenterOfTransformationLocal");

  this->ResetCenterOfTransformAllTransformedNodesBoundsAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Transformed nodes"), q);
  QObject::connect(this->ResetCenterOfTransformAllTransformedNodesBoundsAction, SIGNAL(triggered()), q,
    SLOT(resetCenterOfTransformationAllTransformedNodeBounds()));
  this->ResetCenterOfTransformAllTransformedNodesBoundsAction->setObjectName("ResetCenterOfTransformationAllTransformedNodesBounds");

  this->ResetCenterOfTransformationMenu = new QMenu();
  this->ResetCenterOfTransformationMenu->addAction(this->ResetCenterOfTransformLocalAction);
  this->ResetCenterOfTransformationMenu->addAction(this->ResetCenterOfTransformAllTransformedNodesBoundsAction);
  this->ResetCenterOfTransformationMenu->addSeparator();

  this->ResetCenterOfTransformMenuAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Reset center of transformation"));
  this->ResetCenterOfTransformMenuAction->setObjectName("HandleInteractionOptions");
  this->ResetCenterOfTransformMenuAction->setMenu(this->ResetCenterOfTransformationMenu);

  this->ResetCenterOfTransformationNodeGroup = new QActionGroup(this->ResetCenterOfTransformMenuAction);

  this->ToggleInteractionAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Interaction"), q);
  QObject::connect(this->ToggleInteractionAction, SIGNAL(toggled(bool)), q, SLOT(toggleInteractionBox(bool)));
  this->ToggleInteractionAction->setCheckable(true);
  this->ToggleInteractionAction->setChecked(false);

  this->ToggleInteractionItemAction = new QAction(qSlicerSubjectHierarchyTransformsPlugin::tr("Interaction"), q);
  QObject::connect(this->ToggleInteractionItemAction, SIGNAL(toggled(bool)), q, SLOT(toggleInteractionBox(bool)));
  this->ToggleInteractionItemAction->setCheckable(true);
  this->ToggleInteractionItemAction->setChecked(false);
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

  // Get markups node
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

  double center_World[3] = { (bounds_World[0] + bounds_World[1]) / 2.0,
                             (bounds_World[2] + bounds_World[3]) / 2.0,
                             (bounds_World[4] + bounds_World[5]) / 2.0 };

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
  foreach(QAction * transformAction, transformActions)
  {
    this->ResetCenterOfTransformationNodeGroup->removeAction(transformAction);
    this->ResetCenterOfTransformationMenu->removeAction(transformAction);
  }
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
double qSlicerSubjectHierarchyTransformsPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const
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
    QMessageBox::StandardButton answer =
      QMessageBox::question(nullptr, tr("Some nodes in the branch are already transformed"),
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
double qSlicerSubjectHierarchyTransformsPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
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
double qSlicerSubjectHierarchyTransformsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  if (associatedNode && associatedNode->IsA("vtkMRMLDisplayableNode"))
  {
    return 0.1; // Most other plugins can handle displayable nodes better.
  }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyTransformsPlugin::roleForPlugin()const
{
  return "Transform";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyTransformsPlugin::tooltip(vtkIdType itemID)const
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
    QString transformInfo = QString("\n")
      + tr("Transform to parent:")
      + QString(" %1\n").arg(transformNode->GetTransformToParentInfo())
      + tr("Transform from parent:")
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
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::itemContextMenuActions()const
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
  actions << d->ToggleInteractionItemAction;
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

    d->ToggleInteractionItemAction->setVisible(true);
    bool wasBlocked = d->ToggleInteractionItemAction->blockSignals(true);
    d->ToggleInteractionItemAction->setChecked(displayNode ? displayNode->GetEditorVisibility() : false);
    d->ToggleInteractionItemAction->blockSignals(wasBlocked);
  }
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::viewContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyTransformsPlugin);

  QList<QAction*> actions;
  actions << d->InvertAction << d->IdentityAction << d->ResetCenterOfTransformMenuAction;
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

  vtkMRMLTransformNode* associatedNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!associatedNode)
  {
    return;
  }

  d->ViewContextMenuEventData = eventData;
  d->ViewContextMenuEventData["NodeID"] = QVariant(associatedNode->GetID());

  d->InvertAction->setVisible(true);
  d->IdentityAction->setVisible(true);
  d->ResetCenterOfTransformLocalAction->setVisible(true);
  d->ResetCenterOfTransformAllTransformedNodesBoundsAction->setVisible(true);
  d->ResetCenterOfTransformMenuAction->setVisible(true);
  d->ToggleInteractionAction->setVisible(true);

  d->removeResetCenterOfTransformationForTransformedNodesActions();
  d->showResetCenterOfTransformationForTransformedNodesActions(associatedNode);
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

  return vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
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

  double center_World[3] = { (bounds_World[0] + bounds_World[1]) / 2.0,
                             (bounds_World[2] + bounds_World[3]) / 2.0,
                             (bounds_World[4] + bounds_World[5]) / 2.0 };

  vtkNew<vtkGeneralTransform> transformFromWorld;
  transformNode->GetTransformFromWorld(transformFromWorld);

  double center_Local[3] = { 0.0, 0.0, 0.0 };
  transformFromWorld->TransformPoint(center_World, center_Local);

  transformNode->SetCenterOfTransformation(center_Local);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::toggleInteractionBox(bool visible)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }

  vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(
    shNode->GetItemDataNode(currentItemID));
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(
    shNode->GetItemDataNode(currentItemID));
  if (!transformNode)
  {
    transformNode = transformableNode->GetParentTransformNode();
  }

  if (!transformNode)
  {
    std::stringstream transformNameSS;
    transformNameSS << "Interaction_" << transformableNode->GetName();
    transformNode = vtkMRMLTransformNode::SafeDownCast(
      transformableNode->GetScene()->AddNewNodeByClass("vtkMRMLTransformNode", transformNameSS.str()));
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(transformableNode);
    if (displayableNode)
    {
      double bounds[6] = { 0.0, -1.0, 0.0, -1.0, 0.0, -1.0 };
      displayableNode->GetBounds(bounds);
      transformNode->SetCenterOfTransformation(
        (bounds[1] + bounds[0]) / 2.0,
        (bounds[3] + bounds[2]) / 2.0,
        (bounds[5] + bounds[4]) / 2.0);
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

  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(
    transformNode->GetDisplayNode());
  if (!displayNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get transform display node";
    return;
  }
  displayNode->SetEditorVisibility(visible);
}
