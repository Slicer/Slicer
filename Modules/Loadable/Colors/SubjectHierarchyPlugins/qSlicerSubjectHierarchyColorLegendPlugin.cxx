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
#include "qSlicerSubjectHierarchyColorLegendPlugin.h"

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"

// Colors includes
#include "vtkSlicerColorLogic.h"

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewLogic.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLVolumeDisplayNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QSettings>

// MRML widgets includes
#include "qMRMLNodeComboBox.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchyColorLegendPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyColorLegendPlugin);
protected:
  qSlicerSubjectHierarchyColorLegendPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyColorLegendPluginPrivate(qSlicerSubjectHierarchyColorLegendPlugin& object);
  ~qSlicerSubjectHierarchyColorLegendPluginPrivate() override;
  void init();
public:

  QAction* ShowColorLegendAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyColorLegendPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyColorLegendPluginPrivate::qSlicerSubjectHierarchyColorLegendPluginPrivate(qSlicerSubjectHierarchyColorLegendPlugin& object)
  : q_ptr(&object)
  , ShowColorLegendAction(nullptr)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyColorLegendPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyColorLegendPlugin);

  this->ShowColorLegendAction = new QAction("Show color legend",q);
  qSlicerSubjectHierarchyAbstractPlugin::setActionPosition(this->ShowColorLegendAction,
    qSlicerSubjectHierarchyAbstractPlugin::SectionDefault, 10);
  QObject::connect(this->ShowColorLegendAction, SIGNAL(toggled(bool)), q, SLOT(toggleVisibilityForCurrentItem(bool)));
  this->ShowColorLegendAction->setCheckable(true);
  this->ShowColorLegendAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyColorLegendPluginPrivate::~qSlicerSubjectHierarchyColorLegendPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyColorLegendPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyColorLegendPlugin::qSlicerSubjectHierarchyColorLegendPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyColorLegendPluginPrivate(*this) )
{
  this->m_Name = QString("ColorLegend");

  Q_D(qSlicerSubjectHierarchyColorLegendPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyColorLegendPlugin::~qSlicerSubjectHierarchyColorLegendPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyColorLegendPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyColorLegendPlugin);

  QList<QAction*> actions;
  actions << d->ShowColorLegendAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyColorLegendPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyColorLegendPlugin);

  if (!itemID)
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
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
    {
    // Not a displayable node color legend is not applicable
    return;
    }
  vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
  if (!displayNode || !displayNode->GetColorNode())
    {
    // No color node for this node, color legend is not applicable
    return;
    }

  vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
  QSignalBlocker blocker(d->ShowColorLegendAction);
  d->ShowColorLegendAction->setChecked(colorLegendDisplayNode && colorLegendDisplayNode->GetVisibility());
  d->ShowColorLegendAction->setVisible(true);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyColorLegendPlugin::toggleVisibilityForCurrentItem(bool on)
{
  Q_D(qSlicerSubjectHierarchyColorLegendPlugin);
  vtkIdType itemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
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
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
    {
    // Not a displayable node color legend is not applicable
    return;
    }
  vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
  if (!displayNode || !displayNode->GetColorNode())
    {
    // No color node for this node, color legend is not applicable
    return;
    }

  vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode = nullptr;
  if (on)
    {
    colorLegendDisplayNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(displayNode);
    }
  else
    {
    colorLegendDisplayNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(displayNode);
    }
  if (colorLegendDisplayNode)
    {
    colorLegendDisplayNode->SetVisibility(on);
    // If visibility is set to false then prevent making the node visible again on show.
    colorLegendDisplayNode->SetShowMode(on ? vtkMRMLDisplayNode::ShowDefault : vtkMRMLDisplayNode::ShowIgnore);
    }
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyColorLegendPlugin::showItemInView(vtkIdType itemID, vtkMRMLAbstractViewNode* viewNode, vtkIdList* allItemsToShow)
{
  Q_D(qSlicerSubjectHierarchyColorLegendPlugin);

  vtkMRMLViewNode* threeDViewNode = vtkMRMLViewNode::SafeDownCast(viewNode);
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(viewNode);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find scalar volume node associated to subject hierarchy item " << itemID;
    return false;
    }

  bool wasVisible = false;
  vtkMRMLColorLegendDisplayNode* displayNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // if there is no color legend node => create it, get first color legend node otherwise
    displayNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create color display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

  if (viewNode)
    {
    // Show in specific view
    MRMLNodeModifyBlocker blocker(displayNode);
    // show
    if (!wasVisible)
      {
      displayNode->SetVisibility(true);
      }
    displayNode->AddViewNodeID(viewNode->GetID());
    }
  else if (sliceNode)
    {
    // Show in specific view
    MRMLNodeModifyBlocker blocker(displayNode);
    // show
    if (!wasVisible)
      {
      displayNode->SetVisibility(true);
      }
    displayNode->AddViewNodeID(sliceNode->GetID());
    }
  else
    {
    // Show in all views
    MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->RemoveAllViewNodeIDs();
    displayNode->SetVisibility(true);
    }

  return true;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyColorLegendPlugin::showColorLegendInView( bool show, vtkIdType itemID, vtkMRMLViewNode* viewNode/*=nullptr*/)
{
  Q_D(qSlicerSubjectHierarchyColorLegendPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find scalar volume node associated to subject hierarchy item " << itemID;
    return false;
    }

  bool wasVisible = false;
  vtkMRMLColorLegendDisplayNode* displayNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // there is no color legend display node
    if (!show)
      {
      // not visible and should not be visible, so we are done
      return true;
      }
    // if there is no color legend node => create it, get first color legend node otherwise
    displayNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create color display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

  if (viewNode)
    {
    // Show/hide in specific view
    MRMLNodeModifyBlocker blocker(displayNode);
    if (show)
      {
      // show
      if (!wasVisible)
        {
        displayNode->SetVisibility(true);
        // This was hidden in all views, show it only in the currently selected view
        displayNode->RemoveAllViewNodeIDs();
        }
      displayNode->AddViewNodeID(viewNode->GetID());
      }
    else
      {
      // This hides the volume rendering in all views, which is a bit more than asked for,
      // but since drag-and-drop to view only requires selective showing (and not selective hiding),
      // this should be good enough. The behavior can be refined later if needed.
      displayNode->SetVisibility(false);
      }
    }
  else
    {
    // Show in all views
    MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->RemoveAllViewNodeIDs();
    displayNode->SetVisibility(show);
    }

  return true;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchyColorLegendPlugin::showColorLegendInSlice( bool show, vtkIdType itemID, vtkMRMLSliceNode* sliceNode/*=nullptr*/)
{
  Q_D(qSlicerSubjectHierarchyColorLegendPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
    }
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find scalar volume node associated to subject hierarchy item " << itemID;
    return false;
    }

  bool wasVisible = false;
  vtkMRMLColorLegendDisplayNode* displayNode = vtkSlicerColorLogic::GetColorLegendDisplayNode(volumeNode);
  if (displayNode)
    {
    wasVisible = displayNode->GetVisibility();
    }
  else
    {
    // there is no color legend display node
    if (!show)
      {
      // not visible and should not be visible, so we are done
      return true;
      }
    // if there is no color legend node => create it, get first color legend node otherwise
    displayNode = vtkSlicerColorLogic::AddDefaultColorLegendDisplayNode(volumeNode);
    }
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to create color display node for scalar volume node " << volumeNode->GetName();
    return false;
    }

  if (sliceNode)
    {
    // Show/hide in specific view
    MRMLNodeModifyBlocker blocker(displayNode);
    if (show)
      {
      // show
      if (!wasVisible)
        {
        displayNode->SetVisibility(true);
        // This was hidden in all views, show it only in the currently selected view
        displayNode->RemoveAllViewNodeIDs();
        }
      displayNode->AddViewNodeID(sliceNode->GetID());
      }
    else
      {
      // This hides the volume rendering in all views, which is a bit more than asked for,
      // but since drag-and-drop to view only requires selective showing (and not selective hiding),
      // this should be good enough. The behavior can be refined later if needed.
      displayNode->SetVisibility(false);
      }
    }
  else
    {
    // Show in all views
    MRMLNodeModifyBlocker blocker(displayNode);
    displayNode->RemoveAllViewNodeIDs();
    displayNode->SetVisibility(show);
    }

  return true;
}
