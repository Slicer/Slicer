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
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkNew.h>
#include <vtkMatrix4x4.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>
#include <QMessageBox>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyTransformsPluginPrivate: public QObject
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

  QAction* ToggleInteractionBoxAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPluginPrivate::qSlicerSubjectHierarchyTransformsPluginPrivate(qSlicerSubjectHierarchyTransformsPlugin& object)
: q_ptr(&object)
{
  this->TransformIcon = QIcon(":Icons/Transform.png");
  this->ToggleInteractionBoxAction = nullptr;
  this->InvertAction = nullptr;
  this->IdentityAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyTransformsPlugin);

  this->InvertAction = new QAction("Invert transform",q);
  QObject::connect(this->InvertAction, SIGNAL(triggered()), q, SLOT(invert()));

  this->IdentityAction = new QAction("Reset transform to identity",q);
  QObject::connect(this->IdentityAction, SIGNAL(triggered()), q, SLOT(identity()));

  this->ToggleInteractionBoxAction = new QAction("Interaction in 3D view",q);
  QObject::connect(this->ToggleInteractionBoxAction, SIGNAL(toggled(bool)), q, SLOT(toggleInteractionBox(bool)));
  this->ToggleInteractionBoxAction->setCheckable(true);
  this->ToggleInteractionBoxAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPluginPrivate::~qSlicerSubjectHierarchyTransformsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyTransformsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyTransformsPlugin::qSlicerSubjectHierarchyTransformsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyTransformsPluginPrivate(*this) )
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
    QString transformInfo = QString("\nTransform to parent:\n%1\nTransform from parent:\n%2").arg(
      transformNode->GetTransformToParentInfo()).arg(transformNode->GetTransformFromParentInfo());
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
  actions << d->InvertAction << d->IdentityAction;
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

  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    d->InvertAction->setVisible(true);
    vtkMRMLTransformNode* tnode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (tnode && tnode->IsLinear())
      {
      d->IdentityAction->setVisible(true);
      }
    }
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyTransformsPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyTransformsPlugin);

  QList<QAction*> actions;
  actions << d->ToggleInteractionBoxAction;
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
    vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (transformNode)
      {
      vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
      if (!displayNode)
        {
        transformNode->CreateDefaultDisplayNodes();
        displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
        }
      if (displayNode)
        {
        d->ToggleInteractionBoxAction->setVisible(true);
        bool wasBlocked = d->ToggleInteractionBoxAction->blockSignals(true);
        d->ToggleInteractionBoxAction->setChecked(displayNode->GetEditorVisibility());
        d->ToggleInteractionBoxAction->blockSignals(wasBlocked);
        }
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyTransformsPlugin::invert()
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

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(
    shNode->GetItemDataNode(currentItemID) );
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

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(
    shNode->GetItemDataNode(currentItemID) );
  if (transformNode && transformNode->IsLinear())
    {
    vtkNew<vtkMatrix4x4> matrix; // initialized to identity by default
    transformNode->SetMatrixTransformToParent(matrix.GetPointer());
    }
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

  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast(
    shNode->GetItemDataNode(currentItemID) );
  if (!transformNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get transform node";
    return;
    }
  vtkMRMLTransformDisplayNode* displayNode = vtkMRMLTransformDisplayNode::SafeDownCast(
    transformNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get transform display node";
    return;
    }

  displayNode->SetEditorVisibility(visible);
}
