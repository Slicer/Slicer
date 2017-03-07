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
#include "qSlicerSubjectHierarchySegmentsPlugin.h"

#include "qSlicerSubjectHierarchySegmentationsPlugin.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkSlicerSegmentationsModuleLogic.h"

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyConstants.h"
#include "vtkMRMLSubjectHierarchyNode.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Qt includes
#include <QDebug>
#include <QIcon>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

// SlicerQt includes
#include "qSlicerApplication.h"

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

//-----------------------------------------------------------------------------
/// \ingroup SlicerRt_QtModules_Segmentations
class qSlicerSubjectHierarchySegmentsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchySegmentsPlugin);
protected:
  qSlicerSubjectHierarchySegmentsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchySegmentsPluginPrivate(qSlicerSubjectHierarchySegmentsPlugin& object);
  ~qSlicerSubjectHierarchySegmentsPluginPrivate();
  void init();
public:
  QIcon SegmentIcon;

  QAction* ShowOnlyCurrentSegmentAction;
  QAction* ShowAllSegmentsAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchySegmentsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentsPluginPrivate::qSlicerSubjectHierarchySegmentsPluginPrivate(qSlicerSubjectHierarchySegmentsPlugin& object)
: q_ptr(&object)
, SegmentIcon(QIcon(":Icons/Segment.png"))
{
  this->ShowOnlyCurrentSegmentAction = NULL;
  this->ShowAllSegmentsAction = NULL;
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchySegmentsPlugin);

  // Show only current segment action
  this->ShowOnlyCurrentSegmentAction = new QAction("Show only this segment",q);
  QObject::connect(this->ShowOnlyCurrentSegmentAction, SIGNAL(triggered()), q, SLOT(showOnlyCurrentSegment()));

  // Show all segments action
  this->ShowAllSegmentsAction = new QAction("Show all segments",q);
  QObject::connect(this->ShowAllSegmentsAction, SIGNAL(triggered()), q, SLOT(showAllSegments()));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentsPluginPrivate::~qSlicerSubjectHierarchySegmentsPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentsPlugin::qSlicerSubjectHierarchySegmentsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchySegmentsPluginPrivate(*this) )
{
  this->m_Name = QString("Segments");

  Q_D(qSlicerSubjectHierarchySegmentsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchySegmentsPlugin::~qSlicerSubjectHierarchySegmentsPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchySegmentsPlugin::canReparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)const
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
  if ( parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID
    || shNode->GetItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName()).empty() )
    {
    // Cannot reparent if item is not a segment or there is no parent
    return 0.0;
    }

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(parentItemID));
  if (segmentationNode)
    {
    // If item is segment and parent is segmentation then can reparent
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
bool qSlicerSubjectHierarchySegmentsPlugin::reparentItemInsideSubjectHierarchy(vtkIdType itemID, vtkIdType parentItemID)
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
  if ( parentItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID
    || shNode->GetItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName()).empty()
    || shNode->GetItemParent(itemID) == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID )
    {
    // Cannot reparent if item is not a segment or there is no parent
    return false;
    }

  // Get source and target segmentation items
  vtkMRMLSegmentationNode* fromSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    shNode->GetItemDataNode( shNode->GetItemParent(itemID) ) );
  vtkMRMLSegmentationNode* toSegmentationNode = vtkMRMLSegmentationNode::SafeDownCast(
    shNode->GetItemDataNode(parentItemID) );
  if (!fromSegmentationNode || !toSegmentationNode)
    {
    return false;
    }

  // Get segment ID
  std::string segmentId(shNode->GetItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName()));

  // Perform reparenting
  // Note: No actual subject hierarchy reparenting is done, because the function call below triggers
  //   removal and addition of segment subject hierarchy nodes as the segment is removed from source
  //   segmentation and added to target segmentation
  bool success = toSegmentationNode->GetSegmentation()->CopySegmentFromSegmentation(
    fromSegmentationNode->GetSegmentation(), segmentId, true );

  // Notify user if failed to reparent
  if (!success)
    {
    // If the two master representations are the same, then probably the segment IDs were duplicate
    if (fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName() == toSegmentationNode->GetSegmentation()->GetMasterRepresentationName())
      {
      QString message = QString("Segment ID of the moved segment (%1) might exist in the target segmentation.\nPlease check the error window for details.").arg(segmentId.c_str());
      QMessageBox::information(NULL, tr("Failed to move segment between segmentations"), message);
      return false;
      }

    // Otherwise master representation has to be changed
    QString message = QString("Cannot convert source master representation '%1' into target master '%2',"
      "thus unable to move segment '%3' from segmentation '%4' to '%5'.\n\n"
      "Would you like to change the master representation of '%5' to '%1'?\n\n"
      "Note: This may result in unwanted data loss in %5.")
      .arg(fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str())
      .arg(toSegmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str())
      .arg(segmentId.c_str()).arg(fromSegmentationNode->GetName()).arg(toSegmentationNode->GetName());
    QMessageBox::StandardButton answer =
      QMessageBox::question(NULL, tr("Failed to move segment between segmentations"), message,
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer == QMessageBox::Yes)
      {
      // Convert target segmentation to master representation of source segmentation
      bool successfulConversion = toSegmentationNode->GetSegmentation()->CreateRepresentation(
        fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName() );
      if (!successfulConversion)
        {
        QString message = QString("Failed to convert %1 to %2!").arg(toSegmentationNode->GetName())
          .arg(fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str());
        QMessageBox::warning(NULL, tr("Conversion failed"), message);
        return false;
        }

      // Change master representation of target to that of source
      toSegmentationNode->GetSegmentation()->SetMasterRepresentationName(
        fromSegmentationNode->GetSegmentation()->GetMasterRepresentationName() );

      // Retry reparenting
      return this->reparentItemInsideSubjectHierarchy(itemID, parentItemID);
      }
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchySegmentsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Segment
  if (!shNode->GetItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName()).empty())
    {
    return 1.0;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchySegmentsPlugin::roleForPlugin()const
{
  return "Segment";
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchySegmentsPlugin::helpText()const
{
  //return QString("<p style=\" margin-top:4px; margin-bottom:1px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; font-size:9pt; font-weight:600; color:#000000;\">Create new Contour set from scratch</span></p>"
  //  "<p style=\" margin-top:0px; margin-bottom:11px; margin-left:26px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'sans-serif'; font-size:9pt; color:#000000;\">Right-click on an existing Study node and select 'Create child contour set'. This menu item is only available for Study level nodes</span></p>");
  //TODO:
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchySegmentsPlugin::tooltip(vtkIdType itemID)const
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
    return QString("Invalid!");
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return QString("Invalid!");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  if (scene->IsImporting())
    {
    // During import SH node may be created before the segmentation is read into the scene,
    // so don't attempt to access the segment yet
    return tooltipString;
    }

  vtkSegment* segment = vtkSlicerSegmentationsModuleLogic::GetSegmentForSegmentSubjectHierarchyItem(itemID, scene);
  if (!segment)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to get segment for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return tooltipString;
    }

  // Representations
  std::vector<std::string> containedRepresentationNames;
  segment->GetContainedRepresentationNames(containedRepresentationNames);
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

  // Color
  double color[3] = {0.0,0.0,0.0};
  segment->GetColor(color);
  tooltipString.append( QString(" (Color: %1,%2,%3)").arg(
    (int)(color[0]*255)).arg((int)(color[1]*255)).arg((int)(color[2]*255)) );

  // Tags
  std::map<std::string,std::string> tags;
  segment->GetTags(tags);
  tooltipString.append( QString(" (Tags: ") );
  if (tags.empty())
    {
    tooltipString.append( QString("None)") );
    }
  else
    {
    for (std::map<std::string,std::string>::iterator tagIt=tags.begin(); tagIt!=tags.end(); ++tagIt)
      {
      std::string tagString = tagIt->first + ": " + tagIt->second + ", ";
      tooltipString.append( tagString.c_str() );
      }
    tooltipString = tooltipString.left(tooltipString.length()-2).append(")");
    }

  return tooltipString;
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchySegmentsPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchySegmentsPlugin);

  // Contour set
  if (this->canOwnSubjectHierarchyItem(itemID))
    {
    return d->SegmentIcon;
    }

  // Item unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchySegmentsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentsPlugin::setDisplayVisibility(vtkIdType itemID, int visible)
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

  // Get segmentation node and display node
  vtkMRMLSegmentationNode* segmentationNode =
    vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(itemID, shNode->GetScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
    return;
    }

  // Get segment ID
  std::string segmentId = shNode->GetItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());

  // Set visibility
  displayNode->SetSegmentVisibility(segmentId, (bool)visible);

  // Trigger update of visibility icon
  shNode->ItemModified(itemID);
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchySegmentsPlugin::getDisplayVisibility(vtkIdType itemID)const
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
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene!";
    return -1;
    }

  if (scene->IsImporting())
    {
    // during import SH node may be created before the segmentation is read into the scene,
    // so don't attempt to access the segment yet
    return -1;
    }

  // Get segmentation node and display node
  vtkMRMLSegmentationNode* segmentationNode =
    vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(itemID, scene);
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return -1;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
    return -1;
    }

  // Get segment ID
  std::string segmentId = shNode->GetItemAttribute(itemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());

  // Get visibility
  return (displayNode->GetSegmentVisibility(segmentId) ? 1 : 0);
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchySegmentsPlugin::itemContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchySegmentsPlugin);

  QList<QAction*> actions;
  actions << d->ShowOnlyCurrentSegmentAction << d->ShowAllSegmentsAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentsPlugin::showContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(const qSlicerSubjectHierarchySegmentsPlugin);
  this->hideAllContextMenuActions();

  qSlicerSubjectHierarchySegmentationsPlugin* segmentationsPlugin = qobject_cast<qSlicerSubjectHierarchySegmentationsPlugin*>(
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Segmentations") );

  // Segments plugin shows all segmentations plugin functions in segment context menu
  segmentationsPlugin->showContextMenuActionsForItem(itemID);

  // Owned Segment
  if (this->canOwnSubjectHierarchyItem(itemID) && this->isThisPluginOwnerOfItem(itemID))
    {
    d->ShowOnlyCurrentSegmentAction->setVisible(true);
    d->ShowAllSegmentsAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentsPlugin::editProperties(vtkIdType itemID)
{
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  // Switch to segmentations module and select parent segmentation node
  vtkMRMLSegmentationNode* segmentationNode =
    vtkSlicerSegmentationsModuleLogic::GetSegmentationNodeForSegmentSubjectHierarchyItem(itemID, shNode->GetScene());
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
    }

  qSlicerApplication::application()->openNodeModule(segmentationNode);
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentsPlugin::showOnlyCurrentSegment()
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
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
    return;
    }

  // Get segmentation node and display node
  vtkIdType segmentationShItemID = shNode->GetItemParent(currentItemID);
  if (segmentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation subject hierarchy item for segment item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(segmentationShItemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
    return;
    }

  // Hide all segments except the current one
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    vtkIdType segmentItemID = (*segmentIt);
    bool visible = false;
    if (segmentItemID == currentItemID)
      {
      visible = true;
      }

    // Get segment ID
    std::string segmentId = shNode->GetItemAttribute(segmentItemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
    // Set visibility
    displayNode->SetSegmentVisibility(segmentId, visible);
    // Trigger update of visibility icon
    shNode->ItemModified(segmentItemID);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchySegmentsPlugin::showAllSegments()
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
    qCritical() << Q_FUNC_INFO << ": Invalid current item!";
    return;
    }

  // Get segmentation node and display node
  vtkIdType segmentationShItemID = shNode->GetItemParent(currentItemID);
  if (segmentationShItemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to find segmentation subject hierarchy item for segment item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(shNode->GetItemDataNode(segmentationShItemID));
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find segmentation node for segment subject hierarchy item " << shNode->GetItemName(currentItemID).c_str();
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode() );
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node for segmentation!";
    return;
    }

  // Show all segments
  std::vector<vtkIdType> segmentShItemIDs;
  shNode->GetItemChildren(segmentationShItemID, segmentShItemIDs);
  std::vector<vtkIdType>::iterator segmentIt;
  for (segmentIt = segmentShItemIDs.begin(); segmentIt != segmentShItemIDs.end(); ++segmentIt)
    {
    vtkIdType segmentItemID = (*segmentIt);
    // Get segment ID
    std::string segmentId = shNode->GetItemAttribute(segmentItemID, vtkMRMLSegmentationNode::GetSegmentIDAttributeName());
    // Set visibility
    displayNode->SetSegmentVisibility(segmentId, true);
    // Trigger update of visibility icon
    shNode->ItemModified(segmentItemID);
    }
}
