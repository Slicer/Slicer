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
#include "qMRMLSliceWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h" 
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
#include <QAction>
#include <QDebug>
#include <QSettings>
#include <QStandardItem>
#include <QTimer>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyVolumesPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVolumesPlugin);
protected:
  qSlicerSubjectHierarchyVolumesPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyVolumesPluginPrivate(qSlicerSubjectHierarchyVolumesPlugin& object);
  ~qSlicerSubjectHierarchyVolumesPluginPrivate() override;
  void init();

  bool resetFieldOfViewOnShow();

  qMRMLSliceWidget* sliceWidgetForSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode);

public:
  QIcon VolumeIcon;
  QIcon VolumeVisibilityOffIcon;
  QIcon VolumeVisibilityOnIcon;

  QAction* ShowVolumesInBranchAction;
  QAction* ShowVolumeInForegroundAction;
  QAction* ResetFieldOfViewOnShowAction;
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

  this->ShowVolumesInBranchAction = nullptr;
  this->ShowVolumeInForegroundAction = nullptr;
  this->ResetFieldOfViewOnShowAction = nullptr;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVolumesPlugin);

  // Connect layout changes to slot so that volume visibility icons are correctly updated
  if (qSlicerApplication::application()->layoutManager())
    {
    QObject::connect(qSlicerApplication::application()->layoutManager(), SIGNAL(layoutChanged(int)), q, SLOT(onLayoutChanged(int)));
    // Make sure initial connections are made even before a layout change
    QTimer::singleShot(0, q, SLOT(onLayoutChanged()));
    }

  this->ShowVolumesInBranchAction = new QAction("Show volumes in folder",q);
  QObject::connect(this->ShowVolumesInBranchAction, SIGNAL(triggered()), q, SLOT(showVolumesInBranch()));

  this->ShowVolumeInForegroundAction = new QAction("Show in slice views as foreground", q);
  QObject::connect(this->ShowVolumeInForegroundAction, SIGNAL(triggered()), q, SLOT(showVolumeInForeground()));

  this->ResetFieldOfViewOnShowAction = new QAction("Reset field of view on show",q);
  QObject::connect(this->ResetFieldOfViewOnShowAction, SIGNAL(toggled(bool)), q, SLOT(toggleResetFieldOfViewOnShowAction(bool)));
  this->ResetFieldOfViewOnShowAction->setCheckable(true);
  this->ResetFieldOfViewOnShowAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPluginPrivate::~qSlicerSubjectHierarchyVolumesPluginPrivate() = default;

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVolumesPluginPrivate::resetFieldOfViewOnShow()
{
  QSettings settings;
  return settings.value("SubjectHierarchy/ResetFieldOfViewOnShowVolume", true).toBool();
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qSlicerSubjectHierarchyVolumesPluginPrivate::sliceWidgetForSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode)
{
  qMRMLLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    return nullptr;
    }

  QStringList sliceViewNames = layoutManager->sliceViewNames();
  foreach (QString sliceName, sliceViewNames)
    {
    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceName);
    if (sliceWidget->mrmlSliceCompositeNode() == compositeNode)
      {
      return sliceWidget;
      }
    }

  return nullptr;
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVolumesPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPlugin::qSlicerSubjectHierarchyVolumesPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyVolumesPluginPrivate(*this) )
{
  this->m_Name = QString("Volumes");

  Q_D(qSlicerSubjectHierarchyVolumesPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPlugin::~qSlicerSubjectHierarchyVolumesPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyVolumesPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is nullptr";
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

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkImageData* imageData = (volumeNode ? volumeNode->GetImageData() : nullptr);
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
    tooltipString.append(" !Invalid volume");
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

  // Volume
  vtkMRMLScalarVolumeNode* associatedVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (associatedVolumeNode)
    {
    if (visible)
      {
      // If visibility is on, then show the volume in the background of all slice views
      this->showVolumeInAllViews(associatedVolumeNode, vtkMRMLApplicationLogic::BackgroundLayer);
      }
    else
      {
      // If visibility is off, then hide the volume from all layers of all slice views
      this->hideVolumeFromAllViews(associatedVolumeNode);
      }
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

  // Collect all volumes that are shown in any slice views in any layers
  QSet<vtkIdType> shownVolumeItemIDs;
  this->collectShownVolumes(shownVolumeItemIDs);

  if (shownVolumeItemIDs.contains(itemID))
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::showVolumeInAllViews(
  vtkMRMLScalarVolumeNode* node, int layer/*=vtkMRMLApplicationLogic::BackgroundLayer*/ )
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);

  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": nullptr node";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Collect currently shown volumes from the given layers so that their visibility icons can be updated after
  // showing the specified volume
  QSet<vtkIdType> subjectHierarchyItemsToUpdate;
  this->collectShownVolumes(subjectHierarchyItemsToUpdate, layer);
  subjectHierarchyItemsToUpdate.insert(shNode->GetItemByDataNode(node));

  vtkMRMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLSliceCompositeNode"));
    if (layer & vtkMRMLApplicationLogic::BackgroundLayer)
      {
      compositeNode->SetBackgroundVolumeID(node->GetID());
      }
    else if (layer & vtkMRMLApplicationLogic::ForegroundLayer)
      {
      compositeNode->SetForegroundVolumeID(node->GetID());
      }
    else if (layer & vtkMRMLApplicationLogic::LabelLayer)
      {
      compositeNode->SetLabelVolumeID(node->GetID());
      }

    if (d->resetFieldOfViewOnShow())
      {
      qMRMLSliceWidget* sliceWidget = d->sliceWidgetForSliceCompositeNode(compositeNode);
      if (sliceWidget)
        {
        sliceWidget->fitSliceToBackground();
        }
      }
    }

  // Update scene model for subject hierarchy nodes that were just shown
  for ( QSet<vtkIdType>::iterator volumeItemIt = subjectHierarchyItemsToUpdate.begin();
       volumeItemIt != subjectHierarchyItemsToUpdate.end(); ++volumeItemIt )
   {
   shNode->ItemModified(*volumeItemIt);
   }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::hideVolumeFromAllViews(vtkMRMLScalarVolumeNode* node)
{
  if (!node || !node->GetID())
    {
    qCritical() << Q_FUNC_INFO << ": nullptr node";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }

  char* volumeNodeID = node->GetID();
  vtkMRMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLSliceCompositeNode"));
    char* backgroundVolumeID = compositeNode->GetBackgroundVolumeID();
    char* foregroundVolumeID = compositeNode->GetForegroundVolumeID();
    char* labelVolumeID = compositeNode->GetLabelVolumeID();
    if (backgroundVolumeID && !strcmp(backgroundVolumeID, volumeNodeID))
      {
      compositeNode->SetBackgroundVolumeID(nullptr);
      }
    if (foregroundVolumeID && !strcmp(foregroundVolumeID, volumeNodeID))
      {
      compositeNode->SetForegroundVolumeID(nullptr);
      }
    if (labelVolumeID && !strcmp(labelVolumeID, volumeNodeID))
      {
      compositeNode->SetLabelVolumeID(nullptr);
      }
    }

  // Get subject hierarchy item for the volume node and have the scene model updated
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkIdType volumeItemID = shNode->GetItemByDataNode(node);
  if (volumeItemID)
    {
    shNode->ItemModified(volumeItemID);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::collectShownVolumes( QSet<vtkIdType>& shownVolumeItemIDs,
  int layer/*=vtkMRMLApplicationLogic::BackgroundLayer | vtkMRMLApplicationLogic::ForegroundLayer | vtkMRMLApplicationLogic::LabelLayer*/ )const
{
  shownVolumeItemIDs.clear();

  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }
  if (scene->IsBatchProcessing())
    {
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  vtkMRMLSliceCompositeNode* compositeNode = nullptr;
  const int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast( scene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if ( layer & vtkMRMLApplicationLogic::BackgroundLayer
      && compositeNode->GetBackgroundVolumeID() && strcmp(compositeNode->GetBackgroundVolumeID(),"") )
      {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode( scene->GetNodeByID(compositeNode->GetBackgroundVolumeID())) );
      }
    if ( layer & vtkMRMLApplicationLogic::ForegroundLayer
      && compositeNode->GetForegroundVolumeID() && strcmp(compositeNode->GetForegroundVolumeID(),"") )
      {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode( scene->GetNodeByID(compositeNode->GetForegroundVolumeID())) );
      }
    if ( layer & vtkMRMLApplicationLogic::LabelLayer
      && compositeNode->GetLabelVolumeID() && strcmp(compositeNode->GetLabelVolumeID(),"") )
      {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode( scene->GetNodeByID(compositeNode->GetLabelVolumeID())) );
      }
    }
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVolumesPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyVolumesPlugin);

  QList<QAction*> actions;
  actions << d->ShowVolumesInBranchAction << d->ShowVolumeInForegroundAction << d->ResetFieldOfViewOnShowAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);

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
    d->ShowVolumeInForegroundAction->setVisible(true);

    d->ResetFieldOfViewOnShowAction->setChecked(d->resetFieldOfViewOnShow());
    d->ResetFieldOfViewOnShowAction->setVisible(true);
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
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
    }

  // First, hide all volumes from all views, so that only the volumes from the selected branch are shown
  QSet<vtkIdType> subjectHierarchyItemsToUpdate;
  this->collectShownVolumes(subjectHierarchyItemsToUpdate, vtkMRMLApplicationLogic::BackgroundLayer);
  for ( QSet<vtkIdType>::iterator volumeItemIt = subjectHierarchyItemsToUpdate.begin();
       volumeItemIt != subjectHierarchyItemsToUpdate.end(); ++volumeItemIt )
   {
   this->hideVolumeFromAllViews(vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(*volumeItemIt)));
   }

  // Deselect all volumes before showing the ones from the branch
  selectionNode->SetActiveVolumeID(nullptr);
  selectionNode->SetSecondaryVolumeID(nullptr);

  // Show volumes in branch
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

      // Show first two volume in branch only
      if (!selectionNode->GetActiveVolumeID())
        {
        selectionNode->SetActiveVolumeID(volumeNode->GetID());
        this->showVolumeInAllViews(volumeNode, vtkMRMLApplicationLogic::BackgroundLayer);
        }
      else if (!selectionNode->GetSecondaryVolumeID())
        {
        selectionNode->SetSecondaryVolumeID(volumeNode->GetID());
        this->showVolumeInAllViews(volumeNode, vtkMRMLApplicationLogic::ForegroundLayer);

        // Make sure the secondary volume is shown in a semi-transparent way
        vtkMRMLSliceCompositeNode* compositeNode = nullptr;
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
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::showVolumeInForeground()
{
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
  vtkIdType currentItemID = qSlicerSubjectHierarchyPluginHandler::instance()->currentItem();
  if (currentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid current item";
    return;
  }

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!volumeNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to get current item as a volume node";
    return;
  }

  // Show volume in foreground
  this->showVolumeInAllViews(volumeNode, vtkMRMLApplicationLogic::ForegroundLayer);

  // Make sure the secondary volume is shown in a semi-transparent way
  vtkMRMLSliceCompositeNode* compositeNode = nullptr;
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

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::onLayoutChanged(int layout)
{
  Q_UNUSED(layout);
  this->onLayoutChanged();
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::onLayoutChanged()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }

  // Connect Modified event of each composite node to the plugin, so that visibility icons are
  // updated when volumes are shown/hidden from outside subject hierarchy
  vtkMRMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLSliceCompositeNode"));
    qvtkReconnect(compositeNode, vtkCommand::ModifiedEvent, this, SLOT(onSliceCompositeNodeModified()));
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::onSliceCompositeNodeModified()
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    if (!scene->IsClosing() && !scene->IsRestoring())
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
      }
    return;
    }

  vtkMRMLScalarVolumeNode* volumeNode = nullptr;
  int numberOfVolumeNodes = scene->GetNumberOfNodesByClass("vtkMRMLScalarVolumeNode");
  for (int i=0; i<numberOfVolumeNodes; i++)
    {
    volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLScalarVolumeNode"));
    vtkIdType volumeItemID = shNode->GetItemByDataNode(volumeNode);
    if (volumeItemID)
      {
      shNode->ItemModified(volumeItemID);
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::toggleResetFieldOfViewOnShowAction(bool on)
{
  QSettings settings;
  settings.setValue("SubjectHierarchy/ResetFieldOfViewOnShowVolume", on);
}
