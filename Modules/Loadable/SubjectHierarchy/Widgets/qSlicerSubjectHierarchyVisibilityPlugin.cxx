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
#include "qSlicerSubjectHierarchyVisibilityPlugin.h"

// SubjectHierarchy logic includes
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyVisibilityPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVisibilityPlugin);
protected:
  qSlicerSubjectHierarchyVisibilityPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyVisibilityPluginPrivate(qSlicerSubjectHierarchyVisibilityPlugin& object);
  ~qSlicerSubjectHierarchyVisibilityPluginPrivate() override;
  void init();
public:
  QAction* ToggleVisibility2DAction;
  QAction* ToggleVisibility3DAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVisibilityPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVisibilityPluginPrivate::qSlicerSubjectHierarchyVisibilityPluginPrivate(qSlicerSubjectHierarchyVisibilityPlugin& object)
: q_ptr(&object)
{
  this->ToggleVisibility2DAction = nullptr;
  this->ToggleVisibility3DAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVisibilityPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVisibilityPlugin);

  this->ToggleVisibility2DAction = new QAction("2D visibility",q);
  QObject::connect(this->ToggleVisibility2DAction, SIGNAL(toggled(bool)), q, SLOT(toggleCurrentItemVisibility2D(bool)));
  this->ToggleVisibility2DAction->setCheckable(true);
  this->ToggleVisibility2DAction->setChecked(false);

  this->ToggleVisibility3DAction = new QAction("3D visibility",q);
  QObject::connect(this->ToggleVisibility3DAction, SIGNAL(toggled(bool)), q, SLOT(toggleCurrentItemVisibility3D(bool)));
  this->ToggleVisibility3DAction->setCheckable(true);
  this->ToggleVisibility3DAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVisibilityPluginPrivate::~qSlicerSubjectHierarchyVisibilityPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVisibilityPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVisibilityPlugin::qSlicerSubjectHierarchyVisibilityPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyVisibilityPluginPrivate(*this) )
{
  this->m_Name = QString("Visibility");

  Q_D(qSlicerSubjectHierarchyVisibilityPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVisibilityPlugin::~qSlicerSubjectHierarchyVisibilityPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVisibilityPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyVisibilityPlugin);

  QList<QAction*> actions;
  actions << d->ToggleVisibility2DAction << d->ToggleVisibility3DAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVisibilityPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVisibilityPlugin);

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

  // Determine 2D and 3D visibility of branch. Visible only if visibility is on for all items in branch
  bool visible2D = true;
  bool visible2DVisible = false;
  bool visible3D = true;
  bool visible3DVisible = false;
  vtkSmartPointer<vtkCollection> childDisplayableNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(itemID, childDisplayableNodes, "vtkMRMLDisplayableNode");
  childDisplayableNodes->InitTraversal();
  for (int i=0; i<childDisplayableNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    if (!displayableNode || volumeNode)
      {
      // Disable it for volume nodes
      // (instead, it has a different visibility icon and a show in foreground action in the Volumes plugin)
      continue;
      }
    vtkMRMLDisplayNode* displayNode = vtkMRMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode());
    if (!displayNode)
      {
      displayableNode->CreateDefaultDisplayNodes();
      displayNode = displayableNode->GetDisplayNode();
      }
    if (!displayNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find display node for displayable node " << displayableNode->GetName();
      continue;
      }
    visible2D = visible2D && (displayNode->GetVisibility2D() > 0);
    visible2DVisible = true;
    visible3D = visible3D && (displayNode->GetVisibility3D() > 0);
    visible3DVisible = true;
    }

  bool wasBlocked = d->ToggleVisibility2DAction->blockSignals(true);
  d->ToggleVisibility2DAction->setChecked(visible2D);
  d->ToggleVisibility2DAction->blockSignals(wasBlocked);
  d->ToggleVisibility2DAction->setVisible(visible2DVisible);

  wasBlocked = d->ToggleVisibility3DAction->blockSignals(true);
  d->ToggleVisibility3DAction->setChecked(visible3D);
  d->ToggleVisibility3DAction->blockSignals(wasBlocked);
  d->ToggleVisibility3DAction->setVisible(visible3DVisible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVisibilityPlugin::toggleCurrentItemVisibility2D(bool on)
{
  // Get currently selected node and scene
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current subject hierarchy item";
    return;
    }

  vtkSmartPointer<vtkCollection> childDisplayableNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childDisplayableNodes, "vtkMRMLDisplayableNode");
  childDisplayableNodes->InitTraversal();
  for (int i=0; i<childDisplayableNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    vtkMRMLDisplayNode* displayNode = displayableNode ? vtkMRMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode()) : nullptr;
    if (displayNode)
      {
      displayNode->SetVisibility2D(on);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVisibilityPlugin::toggleCurrentItemVisibility3D(bool on)
{
  // Get currently selected node and scene
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (!currentItemID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current subject hierarchy item";
    return;
    }

  vtkSmartPointer<vtkCollection> childDisplayableNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childDisplayableNodes, "vtkMRMLDisplayableNode");
  childDisplayableNodes->InitTraversal();
  for (int i=0; i<childDisplayableNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(childDisplayableNodes->GetItemAsObject(i));
    vtkMRMLDisplayNode* displayNode = displayableNode ? vtkMRMLDisplayNode::SafeDownCast(displayableNode->GetDisplayNode()) : nullptr;
    if (displayNode)
      {
      displayNode->SetVisibility3D(on);
      }
    }
}
