/*==============================================================================

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

// Segmentations includes
#include "qSlicerSubjectHierarchySegmentationsPlugin.h"

#include "qSlicerSubjectHierarchySegmentsPlugin.h"
#include "vtkSegmentation.h"

// SubjectHierarchy includes
#include "vtkSlicerSegmentationsModuleLogic.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QMessageBox>
#include <QApplication>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSegmentationNode.h>
#include <vtkMRMLSegmentationDisplayNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkNew.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Widgets
class qSlicerSubjectHierarchySegmentationsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchySegmentationsPlugin);
protected:
  qSlicerSubjectHierarchySegmentationsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchySegmentationsPluginPrivate(qSlicerSubjectHierarchySegmentationsPlugin& object);
  ~qSlicerSubjectHierarchySegmentationsPluginPrivate() override;
  void init();
public:
  QIcon SegmentationIcon;

  QAction* ExportBinaryLabelmapAction;
  QAction* ExportClosedSurfaceAction;
  QAction* ConvertLabelmapToSegmentationAction;
  QAction* ConvertModelToSegmentationAction;
  QAction* ConvertModelsToSegmentationAction;
  QAction* Toggle2DFillVisibilityAction;
  QAction* Toggle2DOutlineVisibilityAction;

  bool SegmentSubjectHierarchyItemRemovalInProgress;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchySegmentationsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentationsPluginPrivate::qSlicerSubjectHierarchySegmentationsPluginPrivate(qSlicerSubjectHierarchySegmentationsPlugin& object)
: q_ptr(&object)
, SegmentationIcon(QIcon(":Icons/Segmentation.png"))
, ExportBinaryLabelmapAction(nullptr)
, ExportClosedSurfaceAction(nullptr)
, ConvertLabelmapToSegmentationAction(nullptr)
, ConvertModelToSegmentationAction(nullptr)
, ConvertModelsToSegmentationAction(nullptr)
, Toggle2DFillVisibilityAction(nullptr)
, Toggle2DOutlineVisibilityAction(nullptr)
, SegmentSubjectHierarchyItemRemovalInProgress(false)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchySegmentationsPlugin);

  this->ExportBinaryLabelmapAction = new QAction("Export visible segments to binary labelmap",q);
  QObject::connect(this->ExportBinaryLabelmapAction, SIGNAL(triggered()), q, SLOT(exportToBinaryLabelmap()));

  this->ExportClosedSurfaceAction = new QAction("Export visible segments to models",q);
  QObject::connect(this->ExportClosedSurfaceAction, SIGNAL(triggered()), q, SLOT(exportToClosedSurface()));

  this->ConvertLabelmapToSegmentationAction = new QAction("Convert labelmap to segmentation node",q);
  QObject::connect(this->ConvertLabelmapToSegmentationAction, SIGNAL(triggered()), q, SLOT(convertLabelmapToSegmentation()));

  this->ConvertModelToSegmentationAction = new QAction("Convert model to segmentation node",q);
  QObject::connect(this->ConvertModelToSegmentationAction, SIGNAL(triggered()), q, SLOT(convertModelToSegmentation()));

  this->ConvertModelsToSegmentationAction = new QAction("Convert models to segmentation node",q);
  QObject::connect(this->ConvertModelsToSegmentationAction, SIGNAL(triggered()), q, SLOT(convertModelsToSegmentation()));

  this->Toggle2DFillVisibilityAction = new QAction("2D fill visibility",q);
  QObject::connect(this->Toggle2DFillVisibilityAction, SIGNAL(toggled(bool)), q, SLOT(toggle2DFillVisibility(bool)));
  this->Toggle2DFillVisibilityAction->setCheckable(true);
  this->Toggle2DFillVisibilityAction->setChecked(false);

  this->Toggle2DOutlineVisibilityAction = new QAction("2D outline visibility",q);
  QObject::connect(this->Toggle2DOutlineVisibilityAction, SIGNAL(toggled(bool)), q, SLOT(toggle2DOutlineVisibility(bool)));
  this->Toggle2DOutlineVisibilityAction->setCheckable(true);
  this->Toggle2DOutlineVisibilityAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentationsPluginPrivate::~qSlicerSubjectHierarchySegmentationsPluginPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchySegmentationsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentationsPlugin::qSlicerSubjectHierarchySegmentationsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchySegmentationsPluginPrivate(*this) )
{
  this->m_Name = QString("Segmentations");

  Q_D(qSlicerSubjectHierarchySegmentationsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentationsPlugin::~qSlicerSubjectHierarchySegmentationsPlugin() = default;

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchySegmentationsPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLSegmentationNode"))
    {
    // Node is a segmentation
    return 0.9;
    }
  return 0.0;
}

//----------------------------------------------------------------------------
bool qSlicerSubjectHierarchySegmentationsPlugin::addNodeToSubjectHierarchy(vtkMRMLNode* nodeToAdd, vtkIdType parentItemID)
{
  if (!qSlicerSubjectHierarchyAbstractPlugin::addNodeToSubjectHierarchy(nodeToAdd, parentItemID))
    {
    return false;
    }
  vtkMRMLSegmentationNode* addedSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(nodeToAdd);
  if (!addedSegmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": segmentation node was expected";
    return false;
    }
  this->updateAllSegmentsFromMRML(addedSegmentationNode);
  return true;
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchySegmentationsPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const
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
  if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Cannot reparent if there is no parent
    return 0.0;
    }

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  if (segmentationNode)
    {
    // If item is labelmap or model and parent is segmentation then can reparent
    vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (labelmapNode || modelNode)
      {
      return 1.0;
      }
    }

  return 0.0;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchySegmentationsPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
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
  if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Cannot reparent if there is no parent
    return false;
    }

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode || (!labelmapNode && !modelNode))
    {
    // Invalid inputs
    return false;
    }

  bool success = false;
  std::string importedRepresentationName("");
  if (labelmapNode)
    {
    importedRepresentationName = std::string(vtkSegmentationConverter::GetBinaryLabelmapRepresentationName());
    success = vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(labelmapNode, segmentationNode);
    }
  else
    {
    importedRepresentationName = std::string(vtkSegmentationConverter::GetClosedSurfaceRepresentationName());
    success = vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, segmentationNode);
    }

  // Notify user if failed to import
  if (!success)
    {
    // Probably master representation has to be changed
    QString message = QString("Cannot convert source master representation '%1' into target master '%2',"
      "thus unable to import node '%3' to segmentation '%4'.\n\n"
      "Would you like to change the master representation of '%4' to '%1'?\n\n"
      "Note: This may result in unwanted data loss in %4.")
      .arg(importedRepresentationName.c_str())
      .arg(segmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str())
      .arg(labelmapNode ? labelmapNode->GetName() : modelNode->GetName()).arg(segmentationNode->GetName());
    QMessageBox::StandardButton answer =
      QMessageBox::question(nullptr, tr("Failed to import data to segmentation"), message,
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes)
      {
      // Convert target segmentation to master representation of source segmentation
      bool successfulConversion = segmentationNode->GetSegmentation()->CreateRepresentation(importedRepresentationName);
      if (!successfulConversion)
        {
        QString message = QString("Failed to convert %1 to %2").arg(segmentationNode->GetName()).arg(importedRepresentationName.c_str());
        QMessageBox::warning(nullptr, tr("Conversion failed"), message);
        return false;
        }

      // Change master representation of target to that of source
      segmentationNode->GetSegmentation()->SetMasterRepresentationName(importedRepresentationName);

      // Retry reparenting
      return this->reparentItemInsideSubjectHierarchy(itemID, parentItemID);
      }
    }

  // Real reparenting does not happen, the dragged node will remain where it was
  return false;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchySegmentationsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Segmentation
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (associatedNode && associatedNode->IsA("vtkMRMLSegmentationNode"))
    {
    // Make sure the segmentation subject hierarchy item indicates its virtual branch
    shNode->SetItemAttribute(itemID,
      vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName().c_str(), "1");
    return 0.9;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchySegmentationsPlugin::roleForPlugin()const
{
  return "Segmentation";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchySegmentationsPlugin::tooltip(vtkIdType itemID)const
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
    return QString("Invalid");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item not associated to valid segmentation node";
    return tooltipString;
    }

  // Representations
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  std::vector<std::string> containedRepresentationNames;
  segmentation->GetContainedRepresentationNames(containedRepresentationNames);
  tooltipString.append( QString(" (Representations: ") );
  if (containedRepresentationNames.empty())
    {
    tooltipString.append( QString("None!)") );
    }
  else
    {
    for (std::vector<std::string>::iterator reprIt = containedRepresentationNames.begin();
      reprIt != containedRepresentationNames.end(); ++reprIt)
      {
      tooltipString.append( reprIt->c_str() );
      tooltipString.append( ", " );
      }
    tooltipString = tooltipString.left(tooltipString.length()-2).append(")");
    }

  // Master representation
  tooltipString.append(QString(" (Master representation: %1)").arg(segmentation->GetMasterRepresentationName().c_str()));

  // Number of segments
  tooltipString.append(QString(" (Number of segments: %1)").arg(segmentation->GetNumberOfSegments()));

  return tooltipString;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchySegmentationsPlugin::helpText()const
{
  //TODO:
  //return QString("<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">Create new Contour set from scratch</span></p>"
  //  "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">Right-click on an existing Study node and select 'Create child contour set'. This menu item is only available for Study level nodes</span></p>");
  return QString();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchySegmentationsPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchySegmentationsPlugin);

  // Segmentation
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->SegmentationIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchySegmentationsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
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

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item not associated to valid segmentation node";
    return;
    }

  segmentationNode->SetDisplayVisibility(visible);

  // Trigger updating subject hierarchy visibility icon by calling modified on the segmentation SH node and all its parents
  std::set<vtkIdType> parentItems;
  vtkIdType parentItem = shNode->GetItemByDataNode(segmentationNode);
  do
    {
    parentItems.insert(parentItem);
    }
  while ( (parentItem = shNode->GetItemParent(parentItem) ) != shNode->GetSceneItemID() ); // The double parentheses avoids a Linux build warning

  std::set<vtkIdType>::iterator parentIt;
  for (parentIt=parentItems.begin(); parentIt!=parentItems.end(); ++parentIt)
    {
    shNode->ItemModified(*parentIt);
    }
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchySegmentationsPlugin::getDisplayVisibility(vtkIdType itemID)const
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

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation node associated to subject hierarchy item " << itemID;
    return -1;
    }

  return segmentationNode->GetDisplayVisibility();
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchySegmentationsPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchySegmentationsPlugin);

  QList<QAction*> actions;
  actions << d->ExportBinaryLabelmapAction << d->ExportClosedSurfaceAction
    << d->ConvertLabelmapToSegmentationAction << d->ConvertModelToSegmentationAction << d->ConvertModelsToSegmentationAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchySegmentationsPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // There are no scene actions in this plugin
    return;
    }

  // Owned Segmentation or Segment (segments plugin shows all segmentations plugin functions in segment context menu)
  qSlicerSubjectHierarchySegmentsPlugin* segmentsPlugin = qobject_cast<qSlicerSubjectHierarchySegmentsPlugin*>(
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Segments") );
  if ( (this->canOwnSubjectHierarchyItem(itemID) && this->isThisPluginOwnerOfItem(itemID))
    || (segmentsPlugin->canOwnSubjectHierarchyItem(itemID) && segmentsPlugin->isThisPluginOwnerOfItem(itemID)) )
    {
    d->ExportBinaryLabelmapAction->setVisible(true);
    d->ExportClosedSurfaceAction->setVisible(true);
    }
  else if (!shNode->GetItemOwnerPluginName(itemID).compare("LabelMaps"))
    {
    d->ConvertLabelmapToSegmentationAction->setVisible(true);
    }
  else if (!shNode->GetItemOwnerPluginName(itemID).compare("Models"))
    {
    d->ConvertModelToSegmentationAction->setVisible(true);
    }
  else if (!shNode->GetItemOwnerPluginName(itemID).compare("Folder"))
    {
    d->ConvertModelsToSegmentationAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchySegmentationsPlugin::visibilityContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchySegmentationsPlugin);

  QList<QAction*> actions;
  actions << d->Toggle2DFillVisibilityAction << d->Toggle2DOutlineVisibilityAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchySegmentationsPlugin);

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

  // Segmentation
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(itemID));
    if (!segmentationNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find segmentation node associated to subject hierarchy item " << itemID;
      return;
      }
    vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
    if (!displayNode)
      {
      qCritical() << Q_FUNC_INFO << ": Failed to find display node for segmentation node " << segmentationNode->GetName();
      return;
      }

    d->Toggle2DFillVisibilityAction->blockSignals(true);
    d->Toggle2DFillVisibilityAction->setChecked(displayNode->GetVisibility2DFill());
    d->Toggle2DFillVisibilityAction->blockSignals(false);
    d->Toggle2DFillVisibilityAction->setVisible(true);

    d->Toggle2DOutlineVisibilityAction->blockSignals(true);
    d->Toggle2DOutlineVisibilityAction->setChecked(displayNode->GetVisibility2DOutline());
    d->Toggle2DOutlineVisibilityAction->blockSignals(false);
    d->Toggle2DOutlineVisibilityAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::onSegmentAdded(vtkObject* caller, void* callData)
{
  // Get segmentation node
  vtkMRMLSegmentationNode* segmentationNode = reinterpret_cast<vtkMRMLSegmentationNode*>(caller);
  if (!segmentationNode)
    {
    return;
    }
  if (segmentationNode->GetScene()->IsImporting())
    {
    // During scene import SH may not exist yet (if the scene was created without automatic SH creation)
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy node
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy item for segmentation node " << segmentationNode->GetName();
    return;
    }

  // Get segment ID and segment
  char* segmentId = reinterpret_cast<char*>(callData);
  if (!segmentId)
    {
    // Calling InvokePendingModifiedEvent loses event parameters, so in this case segment IDs are empty
    this->updateAllSegmentsFromMRML(segmentationNode);
    return;
    }
  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get added segment with ID '" << segmentId << "'";
    return;
    }

  // Find the current SegmentID if it already exists
  vtkIdType segmentShItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string currentSegmentId = shNode->GetItemAttribute(*segmentIt, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
    if (!currentSegmentId.compare(segmentId))
      {
      segmentShItemID = (*segmentIt);
      break;
      }
    }

  if (segmentShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Add the segment in subject hierarchy to allow individual handling (e.g. visibility)
    vtkIdType segmentShItemID = shNode->CreateHierarchyItem(
      segmentationShItemID, (segment->GetName() ? segment->GetName() : ""),
      vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyVirtualBranchAttributeName());
    shNode->SetItemAttribute(segmentShItemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName(), segmentId);
    // Set plugin for the new item (automatically selects the segment plugin based on confidence values)
    qSlicerSubjectHierarchyPluginHandler::instance()->findAndSetOwnerPluginForSubjectHierarchyItem(segmentShItemID);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::onSegmentRemoved(vtkObject* caller, void* callData)
{
  // Do nothing if subject hierarchy item removal is already in progress
  Q_D(qSlicerSubjectHierarchySegmentationsPlugin);
  if (d->SegmentSubjectHierarchyItemRemovalInProgress)
    {
    return;
    }

  // Get segmentation node
  vtkMRMLSegmentationNode* segmentationNode = reinterpret_cast<vtkMRMLSegmentationNode*>(caller);
  if (!segmentationNode)
    {
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy item
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Subject hierarchy item cannot be found for segmentation node "
      << segmentationNode->GetName() << " so per-segment subject hierarchy node cannot be removed.";
    return;
    }

  // Get segment ID
  char* segmentId = reinterpret_cast<char*>(callData);
  if (!segmentId)
    {
    // Calling InvokePendingModifiedEvent loses event parameters, so in this case segment IDs are empty
    this->updateAllSegmentsFromMRML(segmentationNode);
    return;
    }

  // Find subject hierarchy item for segment
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string currentSegmentId = shNode->GetItemAttribute(*segmentIt, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
    if (currentSegmentId.empty())
      {
      // this child item is not a segment - ignore it
      continue;
      }
    if (!currentSegmentId.compare(segmentId))
      {
      shNode->RemoveItem(*segmentIt);
      return;
      }
    }

  // Log message if segment subject hierarchy item was not found
  qDebug() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for segment" << segmentId << " in segmentation " << segmentationNode->GetName();
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::onSegmentModified(vtkObject* caller, void* callData)
{
  // Get segmentation node
  vtkMRMLSegmentationNode* segmentationNode = reinterpret_cast<vtkMRMLSegmentationNode*>(caller);
  if (!segmentationNode)
    {
    return;
    }
  if (segmentationNode->GetScene()->IsImporting())
    {
    // during scene import SH may not exist yet (if the scene was created without automatic SH creation)
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy item
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for segmentation node "
      << segmentationNode->GetName() << " so per-segment subject hierarchy node cannot be created";
    return;
    }

  // Get segment ID and segment
  char* segmentId = reinterpret_cast<char*>(callData);
  if (!segmentId)
    {
    // no segmentId is specified - it means that any and all may have been changed
    this->updateAllSegmentsFromMRML(segmentationNode);
    return;
    }

  vtkSegment* segment = segmentationNode->GetSegmentation()->GetSegment(segmentId);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get added segment with ID '" << segmentId << "'";
    return;
    }

  // Find subject hierarchy item for segment
  vtkIdType segmentShItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string currentSegmentId = shNode->GetItemAttribute(*segmentIt, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
    if (!currentSegmentId.compare(segmentId))
      {
      segmentShItemID = (*segmentIt);
      break;
      }
    }

  if (segmentShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    // Segment name and color is set for new segments before adding them to the segmentation.
    // In that case the subject hierarchy item does not exist yet for the segment
    return;
    }

  // Rename segment subject hierarchy item if segment name is different (i.e. has just been renamed)
  if (shNode->GetItemName(segmentShItemID).compare(segment->GetName() ? segment->GetName() : ""))
    {
    shNode->SetItemName(segmentShItemID, (segment->GetName() ? segment->GetName() : ""));
    // modified event is triggered by the name change, so there is no need for invoking modified event
    }
  else
    {
    shNode->InvokeCustomModifiedEvent(
      vtkMRMLSubjectHierarchyNode::SubjectHierarchyItemModifiedEvent, (void*)&segmentShItemID);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::onSubjectHierarchyItemModified(vtkObject* caller, void* callData)
{
  vtkMRMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(caller);
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get item ID
  vtkIdType itemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (callData)
    {
    vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
    if (itemIdPtr)
      {
      itemID = *itemIdPtr;
      }
    }
  if (!itemID)
    {
    return;
    }

  if (!shNode->HasItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName()))
    {
    return;
    }
  // If segment name is different than subject hierarchy item name then rename segment
  vtkSegment* segment = vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(itemID, shNode->GetScene());
  if (!segment)
    {
    return;
    }
  if (segment->GetName() && strcmp(segment->GetName(), shNode->GetItemName(itemID).c_str())==0)
    {
    // no change
    return;
    }
  segment->SetName(shNode->GetItemName(itemID).c_str());
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::onSubjectHierarchyItemAboutToBeRemoved(vtkObject* caller, void* callData)
{
  Q_D(qSlicerSubjectHierarchySegmentationsPlugin);

  vtkMRMLSubjectHierarchyNode* shNode = reinterpret_cast<vtkMRMLSubjectHierarchyNode*>(caller);
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  if (!shNode->GetScene() || shNode->GetScene()->IsClosing())
    {
    return;
    }

  // Get item ID
  vtkIdType itemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  if (callData)
    {
    vtkIdType* itemIdPtr = reinterpret_cast<vtkIdType*>(callData);
    if (itemIdPtr)
      {
      itemID = *itemIdPtr;
      }
    }

  // If a segment subject hierarchy item was removed then remove segment from its segmentation
  // Note: No need to handle removal of segmentation item, because the virtual branch is
  //       automatically removed in case the parent node is removed (in vtkMRMLSubjectHierarchyNode::RemoveItem)
  if (shNode->HasItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName()))
    {
    std::string segmentId = shNode->GetItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());

    // Rely only on ID because the removed node is not in the scene any more
    vtkIdType parentItemID = shNode->GetItemParent(itemID);
    if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      return;
      }
    vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    if (segmentationNode && segmentationNode->GetSegmentation()->GetSegment(segmentId))
      {
      d->SegmentSubjectHierarchyItemRemovalInProgress = true;
      segmentationNode->GetSegmentation()->RemoveSegment(segmentId);
      d->SegmentSubjectHierarchyItemRemovalInProgress = false;
      }
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::exportToBinaryLabelmap()
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
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));

  // Segmentations plugin provides the functionality for segments too, see if it is a segment
  if (!segmentationNode)
    {
    vtkIdType parentItemID = shNode->GetItemParent(currentItemID);
    if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      return;
      }
    segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    }

  // Create binary labelmap representation using default parameters
  bool success = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  if (!success)
    {
    QString message = QString( "Failed to create binary labelmap representation for segmentation %1 using default"
      "conversion parameters!\n\nPlease visit the Segmentation module and try the advanced create representation function.").
      arg(segmentationNode->GetName() );
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segmentation to labelmap node"), message);
    return;
    }

  // Get exported (visible) segment IDs
  std::vector<std::string> segmentIDs;
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  displayNode->GetVisibleSegmentIDs(segmentIDs);

  // Create new labelmap node
  vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(
    segmentationNode->GetScene()->CreateNodeByClass("vtkMRMLLabelMapVolumeNode"));
  vtkMRMLLabelMapVolumeNode* newLabelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(
    segmentationNode->GetScene()->AddNode(newNode));
  newLabelmapNode->CreateDefaultDisplayNodes();
  std::string exportedNodeName = std::string(segmentationNode->GetName());
  if (segmentIDs.size() == 1)
    {
    exportedNodeName += "-" + std::string(segmentationNode->GetSegmentation()->GetSegment(segmentIDs[0])->GetName());
    }
  exportedNodeName += "-label";
  exportedNodeName = segmentationNode->GetScene()->GetUniqueNameByString(exportedNodeName.c_str());
  newLabelmapNode->SetName(exportedNodeName.c_str());

  // Get reference volume
  vtkMRMLVolumeNode* referenceVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    segmentationNode->GetNodeReference(vtkMRMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str()) );

  // Export visible segments into a multi-label labelmap volume
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  success = vtkSlicerSegmentationsModuleLogic::ExportVisibleSegmentsToLabelmapNode(
    segmentationNode, newLabelmapNode, referenceVolumeNode, vtkSegmentation::EXTENT_REFERENCE_GEOMETRY);
  QApplication::restoreOverrideCursor();
  if (!success)
    {
    QString message = QString("Failed to export segments from segmentation %1 to labelmap node!\n\n"
      "Most probably the segment cannot be converted into binary labelmap representation").
      arg(segmentationNode->GetName());
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segments"), message);
    return;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::exportToClosedSurface()
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
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));

  // Segmentations plugin provides the functionality for segments too, see if it is a segment
  if (!segmentationNode)
    {
    vtkIdType parentItemID = shNode->GetItemParent(currentItemID);
    if (parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
      {
      return;
      }
    segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
    }

  // Create closed surface representation using default parameters
  bool success = segmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
  if (!success)
    {
    QString message = QString( "Failed to create closed surface representation for segmentation %1 using default"
      "conversion parameters!\n\nPlease visit the Segmentation module and try the advanced create representation function.").
      arg(segmentationNode->GetName() );
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segmentation to models"), message);
    return;
    }

  // Create new folder item
  std::string newFolderName = std::string(segmentationNode->GetName()) + "-models";
  vtkIdType folderItemID = shNode->CreateFolderItem(
    shNode->GetItemParent(currentItemID),
    shNode->GenerateUniqueItemName(newFolderName) );

  // Export visible segments into a models
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  success = vtkSlicerSegmentationsModuleLogic::ExportVisibleSegmentsToModels(
    segmentationNode, folderItemID );
  QApplication::restoreOverrideCursor();
  if (!success)
    {
    QString message = QString("Failed to export segments from segmentation %1 to models!\n\n"
      "Most probably the segment cannot be converted into closed surface representation.").
      arg(segmentationNode->GetName());
    qCritical() << Q_FUNC_INFO << ": " << message;
    QMessageBox::warning(nullptr, tr("Failed to export segments"), message);
    return;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::convertLabelmapToSegmentation()
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
  vtkMRMLLabelMapVolumeNode* labelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!labelmapNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access labelmap node";
    return;
    }

  // Create new segmentation node
  vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(
    labelmapNode->GetScene()->CreateNodeByClass("vtkMRMLSegmentationNode"));
  vtkMRMLSegmentationNode* newSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    labelmapNode->GetScene()->AddNode(newNode));
  std::string newSegmentationNodeName = std::string(labelmapNode->GetName()) + "-segmentation";
  newSegmentationNode->SetName(newSegmentationNodeName.c_str());

  if (!vtkSlicerSegmentationsModuleLogic::ImportLabelmapToSegmentationNode(labelmapNode, newSegmentationNode))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to import labelmap '" << labelmapNode->GetName() << "' to segmentation '" << newSegmentationNode->GetName() << "'";
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::convertModelToSegmentation()
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
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!modelNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access model node";
    return;
    }

  // Create new segmentation node
  vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(
    modelNode->GetScene()->CreateNodeByClass("vtkMRMLSegmentationNode"));
  vtkMRMLSegmentationNode* newSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    modelNode->GetScene()->AddNode(newNode));
  std::string newSegmentationNodeName = std::string(modelNode->GetName()) + "-segmentation";
  newSegmentationNode->SetName(newSegmentationNodeName.c_str());

  if (!vtkSlicerSegmentationsModuleLogic::ImportModelToSegmentationNode(modelNode, newSegmentationNode))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to import model '" << modelNode->GetName() << "' to segmentation '" << newSegmentationNode->GetName() << "'";
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::convertModelsToSegmentation()
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

  // Create new segmentation node
  vtkSmartPointer<vtkMRMLNode> newNode = vtkSmartPointer<vtkMRMLNode>::Take(
    shNode->GetScene()->CreateNodeByClass("vtkMRMLSegmentationNode"));
  vtkMRMLSegmentationNode* newSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    shNode->GetScene()->AddNode(newNode));
  std::string newSegmentationNodeName = shNode->GetItemName(currentItemID) + "-segmentation";
  newSegmentationNode->SetName(newSegmentationNodeName.c_str());
  newSegmentationNode->SetMasterRepresentationToClosedSurface();

  if (!vtkSlicerSegmentationsModuleLogic::ImportModelsToSegmentationNode(currentItemID, newSegmentationNode))
    {
    qCritical() << Q_FUNC_INFO << ": Failed to import models from folder '" << shNode->GetItemName(currentItemID).c_str()
      << "' to segmentation '" << newSegmentationNode->GetName() << "'";
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::updateAllSegmentsFromMRML(vtkMRMLSegmentationNode* segmentationNode)
{
  // Get segmentation node
  if (!segmentationNode)
    {
    qWarning() << Q_FUNC_INFO << ": invalid segmentation node";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Get associated subject hierarchy item
  vtkIdType segmentationShItemID = shNode->GetItemByDataNode(segmentationNode);
  if (segmentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find subject hierarchy item for segmentation node "
      << segmentationNode->GetName() << " so per-segment subject hierarchy node cannot be created";
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    qWarning() << Q_FUNC_INFO << ": invalid segmentation";
    return;
    }

  // List of segment IDs that have to be added to the segment list
  std::vector<std::string> segmentIDsToBeAddedToSh;
  segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDsToBeAddedToSh);

  // Segment modify/remove
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    std::string segmentId = shNode->GetItemAttribute(*segmentIt, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
    vtkSegment* segment = segmentation->GetSegment(segmentId);
    if (!segment)
      {
      // Segment has been removed
      this->onSegmentRemoved(segmentationNode, (void*)(segmentId.c_str()));
      continue;
      }
    this->onSegmentModified(segmentationNode, (void*)(segmentId.c_str()));

    // Remove segment ID from the list of segments to be added (it's already added)
    segmentIDsToBeAddedToSh.erase(std::remove(segmentIDsToBeAddedToSh.begin(), segmentIDsToBeAddedToSh.end(), segmentId), segmentIDsToBeAddedToSh.end());
    }

  // Segment add
  for (std::vector<std::string>::iterator segmentIdIt = segmentIDsToBeAddedToSh.begin(); segmentIdIt != segmentIDsToBeAddedToSh.end(); ++segmentIdIt)
    {
    this->onSegmentAdded(segmentationNode, (void*)(segmentIdIt->c_str()));
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::toggle2DFillVisibility(bool on)
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
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation node associated to subject hierarchy item " << currentItemID;
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find display node for segmentation node " << segmentationNode->GetName();
    return;
    }

  // Set 2D fill visibility
  displayNode->SetVisibility2DFill(on);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentationsPlugin::toggle2DOutlineVisibility(bool on)
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
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation node associated to subject hierarchy item " << currentItemID;
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find display node for segmentation node " << segmentationNode->GetName();
    return;
    }

  // Set 2D outline visibility
  displayNode->SetVisibility2DOutline(on);
}
