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
#include "qSlicerSubjectHierarchyVolumesPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkCollection.h>
#include <vtkImageData.h>

// Qt includes
#include <QDebug>
#include <QStandardItem>
#include <QAction>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

// STD includes
#include <set>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyVolumesPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVolumesPlugin);
protected:
  qSlicerSubjectHierarchyVolumesPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyVolumesPluginPrivate(qSlicerSubjectHierarchyVolumesPlugin& object);
  ~qSlicerSubjectHierarchyVolumesPluginPrivate();
  void init();
public:
  QIcon VolumeIcon;
  QIcon VolumeVisibilityOffIcon;
  QIcon VolumeVisibilityOnIcon;

  QAction* ShowVolumesInBranchAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVolumesPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPluginPrivate::qSlicerSubjectHierarchyVolumesPluginPrivate(qSlicerSubjectHierarchyVolumesPlugin& object)
: q_ptr(&object)
{
  this->VolumeIcon = QIcon(":Icons/Volume.png");
  this->VolumeVisibilityOffIcon = QIcon(":Icons/VolumeVisibilityOff.png");
  this->VolumeVisibilityOnIcon = QIcon(":Icons/VolumeVisibilityOn.png");

  this->ShowVolumesInBranchAction = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPluginPrivate::~qSlicerSubjectHierarchyVolumesPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPlugin::qSlicerSubjectHierarchyVolumesPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyVolumesPluginPrivate(*this) )
{
  this->m_Name = QString("Volumes");

  Q_D(qSlicerSubjectHierarchyVolumesPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVolumesPlugin);

  this->ShowVolumesInBranchAction = new QAction("Show volumes in branch",q);
  QObject::connect(this->ShowVolumesInBranchAction, SIGNAL(triggered()), q, SLOT(showVolumesInBranch()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPlugin::~qSlicerSubjectHierarchyVolumesPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyVolumesPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLScalarVolumeNode"))
    {
    // Node is a volume
    return 0.5;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyVolumesPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Volume
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLScalarVolumeNode"))
    {
    return 0.5; // There are other plugins that can handle special volume nodes better, thus the relatively low value
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyVolumesPlugin::roleForPlugin()const
{
  return "Scalar volume";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyVolumesPlugin::tooltip(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QString("Invalid!");
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QString("Error!");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkImageData* imageData = (volumeNode ? volumeNode->GetImageData() : NULL);
  if (volumeNode && imageData)
    {
    int dimensions[3] = {0,0,0};
    imageData->GetDimensions(dimensions);
    double spacing[3] = {0.0,0.0,0.0};
    volumeNode->GetSpacing(spacing);
    tooltipString.append( QString(" (Dimensions: %1x%2x%3  Spacing: %4mm x %5mm x %6mm)")
      .arg(dimensions[0]).arg(dimensions[1]).arg(dimensions[2])
      .arg(spacing[0],0,'g',3).arg(spacing[1],0,'g',3).arg(spacing[2],0,'g',3) );
    }
  else
    {
    tooltipString.append(" !Invalid volume!");
    }

  return tooltipString;
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyVolumesPlugin::icon(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  // Volume
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->VolumeIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyVolumesPlugin::visibilityIcon(int visible)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);

  if (visible == 1)
    {
    return d->VolumeVisibilityOnIcon;
    }
  else
    {
    return d->VolumeVisibilityOffIcon;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
{
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

  vtkMRMLScalarVolumeNode* associatedVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  // Volume
  if (associatedVolumeNode)
    {
    this->showVolume(associatedVolumeNode, visible);
    }
  // Default
  else
    {
    qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->setDisplayVisibility(itemID, visible);
    }
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyVolumesPlugin::getDisplayVisibility(vtkIdType itemID)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return -1;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return -1;
    }

  // Sanity checks for volume
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
    {
    return -1;
    }
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get selection node to show volume node " << volumeNode->GetName();
    return -1;
    }

  /// Update selection node based on current volumes visibility (if the selection is different in the slice viewers, then the first one is set)
  /// TODO: This is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)
  this->updateSelectionNodeBasedOnCurrentVolumesVisibility();

  if ( ( selectionNode->GetActiveVolumeID() && !strcmp(volumeNode->GetID(), selectionNode->GetActiveVolumeID()) )
    || ( selectionNode->GetSecondaryVolumeID() && !strcmp(volumeNode->GetID(), selectionNode->GetSecondaryVolumeID()) ) )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::showVolume(vtkMRMLScalarVolumeNode* node, int visible/*=1*/)
{
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": NULL node!";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return;
    }
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get selection node to show volume node " << node->GetName();
    return;
    }

  // Get volume node
  vtkMRMLScalarVolumeNode* volumeNode = NULL;
  if ((volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node)) == NULL)
    {
    qCritical() << Q_FUNC_INFO << ": Node to show node is not a volume node: " << node->GetName();
    return;
    }

  /// Update selection node based on current volumes visibility (if the selection is different in the slice viewers, then the first one is set)
  /// TODO: This is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)
  this->updateSelectionNodeBasedOnCurrentVolumesVisibility();

  // Show
  if (visible)
    {
    // If background is empty, show volume there
    if (!selectionNode->GetActiveVolumeID() || !strlen(selectionNode->GetActiveVolumeID()))
      {
      selectionNode->SetActiveVolumeID(volumeNode->GetID());
      }
    // If foreground is empty, show volume there
    else if (!selectionNode->GetSecondaryVolumeID() || !strlen(selectionNode->GetSecondaryVolumeID()))
      {
      selectionNode->SetSecondaryVolumeID(volumeNode->GetID());
      }
    // If both foreground and background are occupied, then bump background to foreground and show new volume in background
    else
      {
      // Hide volume that has been foreground
      // Needed so that visibility icon is updated (could be done in a faster way, but there is no noticeable overhead)
      vtkMRMLScalarVolumeNode* originalSecondaryVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        scene->GetNodeByID(selectionNode->GetSecondaryVolumeID()) );
      this->showVolume(originalSecondaryVolumeNode, 0);

      // Show volume that has been background in foreground
      vtkMRMLScalarVolumeNode* originalActiveVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        scene->GetNodeByID(selectionNode->GetActiveVolumeID()) );
      if (originalActiveVolumeNode)
        {
        selectionNode->SetSecondaryVolumeID(originalActiveVolumeNode->GetID());
        }

      // Show new volume as background
      selectionNode->SetActiveVolumeID(volumeNode->GetID());
      }

    // Set selection in slice views
    qSlicerCoreApplication::application()->applicationLogic()->PropagateVolumeSelection();

    // Make sure the secondary volume is shown in a semi-transparent way
    if (selectionNode->GetSecondaryVolumeID() && strlen(selectionNode->GetSecondaryVolumeID()))
      {
      vtkMRMLSliceCompositeNode* compositeNode = NULL;
      int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
      for (int i=0; i<numberOfCompositeNodes; i++)
        {
        compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
        if (compositeNode && compositeNode->GetForegroundOpacity() == 0.0)
          {
          compositeNode->SetForegroundOpacity(0.5);
          }
        }
      }
    }
  // Hide
  else
    {
    if ( selectionNode->GetActiveVolumeID() && !strcmp(volumeNode->GetID(), selectionNode->GetActiveVolumeID()) )
      {
      selectionNode->SetActiveVolumeID(NULL);
      }
    else if ( selectionNode->GetSecondaryVolumeID() && !strcmp(volumeNode->GetID(), selectionNode->GetSecondaryVolumeID()) )
      {
      selectionNode->SetSecondaryVolumeID(NULL);
      }
    // Set selection in slice views
    qSlicerCoreApplication::application()->applicationLogic()->PropagateVolumeSelection();
    }

  // Get subject hierarchy item for the volume node and have the scene model updated
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType volumeItemID = shNode->GetItemByDataNode(volumeNode);
  if (volumeItemID != vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    shNode->ItemModified(volumeItemID);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::updateSelectionNodeBasedOnCurrentVolumesVisibility()const
{
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get selection node";
    return;
    }

  // TODO: This is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)

  // Determine background volume selection (if the selection is different in the slice viewers, then the first one is set)
  std::string selectedBackgroundVolumeID = this->getSelectedBackgroundVolumeNodeID();
  selectionNode->SetActiveVolumeID(selectedBackgroundVolumeID.c_str());

  // Determine foreground volume selection (if the selection is different in the slice viewers, then the first one is set)
  std::string selectedForegroundVolumeID = this->getSelectedForegroundVolumeNodeID();
  selectionNode->SetSecondaryVolumeID(selectedForegroundVolumeID.c_str());
}

//---------------------------------------------------------------------------
std::string qSlicerSubjectHierarchyVolumesPlugin::getSelectedBackgroundVolumeNodeID()const
{
  // TODO: This method is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)
  std::string selectedBackgroundVolumeID("");

  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get selection node";
    return selectedBackgroundVolumeID;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return selectedBackgroundVolumeID;
    }

  vtkMRMLSliceCompositeNode* compositeNode = NULL;
  const int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if (compositeNode && compositeNode->GetBackgroundVolumeID() && selectedBackgroundVolumeID.empty())
      {
      selectedBackgroundVolumeID = std::string(compositeNode->GetBackgroundVolumeID());
      break;
      }
    }

  return selectedBackgroundVolumeID;
}

//---------------------------------------------------------------------------
std::string qSlicerSubjectHierarchyVolumesPlugin::getSelectedForegroundVolumeNodeID()const
{
  // TODO: This method is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)
  std::string selectedForegroundVolumeID("");

  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get selection node";
    return selectedForegroundVolumeID;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return selectedForegroundVolumeID;
    }

  vtkMRMLSliceCompositeNode* compositeNode = NULL;
  const int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if (compositeNode && compositeNode->GetForegroundVolumeID() && selectedForegroundVolumeID.empty())
      {
      selectedForegroundVolumeID = std::string(compositeNode->GetForegroundVolumeID());
      break;
      }
    }

  return selectedForegroundVolumeID;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVolumesPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyVolumesPlugin);

  QList<QAction*> actions;
  actions << d->ShowVolumesInBranchAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);
  this->hideAllContextMenuActions();

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

  // Volume
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    // No scalar volume context menu
    }

  // Folders (Patient, Study, Folder)
  if ( shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient())
    || shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy())
    || shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()) )
    {
    d->ShowVolumesInBranchAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::showVolumesInBranch()
{
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get selection node";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
    return;
    }

  std::set<vtkIdType> subjectHierarchyItemsToUpdate;

  // Collect subject hierarchy items of currently shown volumes to be able to update them in the scene model
  vtkIdType activeVolumeShItemID =
    shNode->GetItemByDataNode( scene->GetNodeByID(selectionNode->GetActiveVolumeID()) );
  subjectHierarchyItemsToUpdate.insert(activeVolumeShItemID);
  vtkIdType secondaryVolumeShItemID =
    shNode->GetItemByDataNode( scene->GetNodeByID(selectionNode->GetSecondaryVolumeID()) );
  subjectHierarchyItemsToUpdate.insert(secondaryVolumeShItemID);

  // Hide all volumes before showing the ones from the study
  selectionNode->SetActiveVolumeID(NULL);
  selectionNode->SetSecondaryVolumeID(NULL);

  // Show volumes in study
  vtkSmartPointer<vtkCollection> childVolumeNodes = vtkSmartPointer<vtkCollection>::New();
  shNode->GetDataNodesInBranch(currentItemID, childVolumeNodes, "vtkMRMLScalarVolumeNode");
  childVolumeNodes->InitTraversal();
  for (int i=0; i<childVolumeNodes->GetNumberOfItems(); ++i)
    {
    vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(childVolumeNodes->GetItemAsObject(i));
    if (volumeNode)
      {
      // Get subject hierarchy item for the volume node
      vtkIdType volumeShItemID = shNode->GetItemByDataNode(volumeNode);
      if (volumeShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
        {
        qCritical() << Q_FUNC_INFO << ": Unable to get subject hierarchy item";
        continue;
        }

      // Show first two volume in study only
      if (!selectionNode->GetActiveVolumeID())
        {
        selectionNode->SetActiveVolumeID(volumeNode->GetID());
        subjectHierarchyItemsToUpdate.insert(volumeShItemID);
        }
      else if (!selectionNode->GetSecondaryVolumeID())
        {
        selectionNode->SetSecondaryVolumeID(volumeNode->GetID());
        subjectHierarchyItemsToUpdate.insert(volumeShItemID);

        // Make sure the secondary volume is shown in a semi-transparent way
        vtkMRMLSliceCompositeNode* compositeNode = NULL;
        int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
        for (int i=0; i<numberOfCompositeNodes; i++)
          {
          compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
          if (compositeNode && compositeNode->GetForegroundOpacity() == 0.0)
            {
            compositeNode->SetForegroundOpacity(0.5);
            }
          }
        }
      }
    }

  qSlicerCoreApplication::application()->applicationLogic()->PropagateVolumeSelection();

  // Update scene model for subject hierarchy nodes that were just made hidden
  for ( std::set<vtkIdType>::iterator volumeItemIt = subjectHierarchyItemsToUpdate.begin();
       volumeItemIt != subjectHierarchyItemsToUpdate.end(); ++volumeItemIt )
   {
   shNode->ItemModified(*volumeItemIt);
   }
}
