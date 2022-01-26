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

// SubjectHierarchy MRML includes
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLSubjectHierarchyConstants.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyMarkupsPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Terminologies includes
#include "qSlicerTerminologyItemDelegate.h"
#include "qSlicerTerminologyNavigatorWidget.h"
#include "qSlicerTerminologySelectorDialog.h"
#include "vtkSlicerTerminologiesModuleLogic.h"
#include "vtkSlicerTerminologyEntry.h"

// MRML markups includes
#include "vtkMRMLMarkupsFiducialNode.h"

// MRML widgets includes
#include "qMRMLNodeComboBox.h"

// MRML includes
#include <vtkMRMLAbstractViewNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLMarkupsDisplayNode.h>
#include <vtkMRMLMarkupsPlaneDisplayNode.h>
#include <vtkMRMLMarkupsROIDisplayNode.h>
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLScene.h>

//Logic includes
#include <vtkSlicerMarkupsLogic.h>

// vtkSegmentationCore includes
#include <vtkSegment.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QAction>
#include <QDebug>
#include <QInputDialog>
#include <QMenu>
#include <QStandardItem>
#include <QTimer>

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerApplication.h"

//-----------------------------------------------------------------------------
const char* INTERACTION_HANDLE_TYPE_PROPERTY = "InteractionHandleType";

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SubjectHierarchy_Plugins
class qSlicerSubjectHierarchyMarkupsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyMarkupsPlugin);
protected:
  qSlicerSubjectHierarchyMarkupsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyMarkupsPluginPrivate(qSlicerSubjectHierarchyMarkupsPlugin& object);
  ~qSlicerSubjectHierarchyMarkupsPluginPrivate() override;
  void init();

  vtkMRMLMarkupsNode* markupsNodeFromViewContextMenuEventData();
  void jumpToPoint(int controlPointIndex);

public:
  QAction* RenamePointAction{nullptr};
  QAction* RefocusCameraAction{nullptr};
  QAction* DeletePointAction{nullptr};
  QAction* DeleteNodeAction{nullptr};
  QAction* ToggleSelectPointAction{nullptr};
  QAction* JumpToPreviousPointAction{nullptr};
  QAction* JumpToNextPointAction{nullptr};
  QAction* JumpToClosestPointAction{nullptr};
  QAction* EditNodeTerminologyAction{nullptr};
  QAction* ToggleCurrentItemHandleInteractive{nullptr};
  QAction* ToggleHandleInteractive{nullptr};

  QMenu* CurrentItemHandleVisibilityMenu{nullptr};
  QAction* CurrentItemHandleVisibilityAction{nullptr};
  QAction* ToggleCurrentItemTranslateHandleVisible{nullptr};
  QAction* ToggleCurrentItemRotateHandleVisible{nullptr};
  QAction* ToggleCurrentItemScaleHandleVisible{nullptr};

  QMenu* HandleVisibilityMenu{nullptr};
  QAction* HandleVisibilityAction{nullptr};
  QAction* ToggleTranslateHandleVisible{nullptr};
  QAction* ToggleRotateHandleVisible{nullptr};
  QAction* ToggleScaleHandleVisible{nullptr};

  QList< vtkWeakPointer<vtkMRMLMarkupsNode> > NodesToDelete;

  QVariantMap ViewContextMenuEventData;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyMarkupsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyMarkupsPluginPrivate::qSlicerSubjectHierarchyMarkupsPluginPrivate(qSlicerSubjectHierarchyMarkupsPlugin& object)
: q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyMarkupsPlugin);

  // View context menu

  this->RenamePointAction = new QAction("Rename control point...", q);
  this->RenamePointAction->setObjectName("RenamePointAction");
  q->setActionPosition(this->RenamePointAction, qSlicerSubjectHierarchyAbstractPlugin::SectionComponent);
  QObject::connect(this->RenamePointAction, SIGNAL(triggered()), q, SLOT(renamePoint()));

  this->RefocusCameraAction = new QAction("Refocus camera on this point", q);
  this->RefocusCameraAction->setObjectName("RefocusCameraAction");
  q->setActionPosition(this->RefocusCameraAction, qSlicerSubjectHierarchyAbstractPlugin::SectionComponent);
  QObject::connect(this->RefocusCameraAction, SIGNAL(triggered()), q, SLOT(refocusCamera()));

  this->ToggleSelectPointAction = new QAction("Toggle select control point", q);
  this->ToggleSelectPointAction->setObjectName("ToggleSelectPointAction");
  q->setActionPosition(this->ToggleSelectPointAction, qSlicerSubjectHierarchyAbstractPlugin::SectionComponent);
  QObject::connect(this->ToggleSelectPointAction, SIGNAL(triggered()), q, SLOT(toggleSelectPoint()));

  this->JumpToPreviousPointAction = new QAction("Jump to previous control point", q);
  this->JumpToPreviousPointAction->setToolTip("Jump slice views to the previous control point");
  this->JumpToPreviousPointAction->setObjectName("JumpToPreviousPointAction");
  q->setActionPosition(this->JumpToPreviousPointAction, qSlicerSubjectHierarchyAbstractPlugin::SectionComponent);
  QObject::connect(this->JumpToPreviousPointAction, SIGNAL(triggered()), q, SLOT(jumpToPreviousPoint()));

  this->JumpToNextPointAction = new QAction("Jump to next control point", q);
  this->JumpToNextPointAction->setToolTip("Jump slice views to the next control point");
  this->JumpToNextPointAction->setObjectName("JumpToNextPointAction");
  q->setActionPosition(this->JumpToNextPointAction, qSlicerSubjectHierarchyAbstractPlugin::SectionComponent);
  QObject::connect(this->JumpToNextPointAction, SIGNAL(triggered()), q, SLOT(jumpToNextPoint()));

  this->JumpToClosestPointAction = new QAction("Jump to closest control point", q);
  this->JumpToClosestPointAction->setToolTip("Jump slice views to the closest control point");
  this->JumpToClosestPointAction->setObjectName("JumpToClosestPointAction");
  q->setActionPosition(this->JumpToClosestPointAction, qSlicerSubjectHierarchyAbstractPlugin::SectionComponent);
  QObject::connect(this->JumpToClosestPointAction, SIGNAL(triggered()), q, SLOT(jumpToClosestPoint()));

  this->DeletePointAction = new QAction("Delete control point", q);
  this->DeletePointAction->setObjectName("DeletePointAction");
  q->setActionPosition(this->DeletePointAction, qSlicerSubjectHierarchyAbstractPlugin::SectionComponent);
  QObject::connect(this->DeletePointAction, SIGNAL(triggered()), q, SLOT(deletePoint()));

  this->DeleteNodeAction = new QAction("Delete markup", q);
  this->DeleteNodeAction->setObjectName("DeleteNodeAction");
  q->setActionPosition(this->DeleteNodeAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode);
  QObject::connect(this->DeleteNodeAction, SIGNAL(triggered()), q, SLOT(requestDeleteNode()));

  this->EditNodeTerminologyAction = new QAction("Edit markup terminology...", q);
  this->EditNodeTerminologyAction->setObjectName("editNodeTerminologyAction");
  q->setActionPosition(this->EditNodeTerminologyAction, qSlicerSubjectHierarchyAbstractPlugin::SectionNode);
  QObject::connect(this->EditNodeTerminologyAction, SIGNAL(triggered()), q, SLOT(editNodeTerminology()));

  int interactionHandlesSection = qSlicerSubjectHierarchyAbstractPlugin::SectionNode + 20;


  this->ToggleHandleInteractive = new QAction("Interaction");
  this->ToggleHandleInteractive->setObjectName("ToggleHandleInteractive");
  this->ToggleHandleInteractive->setCheckable(true);
  q->setActionPosition(this->ToggleHandleInteractive, interactionHandlesSection);
  QObject::connect(this->ToggleHandleInteractive, SIGNAL(triggered()), q, SLOT(toggleHandleInteractive()));

  this->ToggleTranslateHandleVisible = new QAction("Translate");
  this->ToggleTranslateHandleVisible->setCheckable(true);
  this->ToggleTranslateHandleVisible->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle);
  QObject::connect(this->ToggleTranslateHandleVisible, SIGNAL(triggered()), q, SLOT(toggleHandleTypeVisibility()));

  this->ToggleRotateHandleVisible = new QAction("Rotate");
  this->ToggleRotateHandleVisible->setCheckable(true);
  this->ToggleRotateHandleVisible->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, vtkMRMLMarkupsDisplayNode::ComponentRotationHandle);
  QObject::connect(this->ToggleRotateHandleVisible, SIGNAL(triggered()), q, SLOT(toggleHandleTypeVisibility()));

  this->ToggleScaleHandleVisible = new QAction("Scale");
  this->ToggleScaleHandleVisible->setCheckable(true);
  this->ToggleScaleHandleVisible->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, vtkMRMLMarkupsDisplayNode::ComponentScaleHandle);
  QObject::connect(this->ToggleScaleHandleVisible, SIGNAL(triggered()), q, SLOT(toggleHandleTypeVisibility()));

  this->HandleVisibilityMenu = new QMenu();
  this->HandleVisibilityMenu->addAction(this->ToggleTranslateHandleVisible);
  this->HandleVisibilityMenu->addAction(this->ToggleRotateHandleVisible);
  this->HandleVisibilityMenu->addAction(this->ToggleScaleHandleVisible);

  this->HandleVisibilityAction = new QAction("Interaction options");
  this->HandleVisibilityAction->setObjectName("HandleInteractionOptions");
  q->setActionPosition(this->HandleVisibilityAction, interactionHandlesSection);
  this->HandleVisibilityAction->setMenu(this->HandleVisibilityMenu);

  // Visibility menu

  this->ToggleCurrentItemHandleInteractive = new QAction("Interaction");
  this->ToggleCurrentItemHandleInteractive->setObjectName("ToggleCurrentItemHandleInteractive");
  this->ToggleCurrentItemHandleInteractive->setCheckable(true);
  QObject::connect(this->ToggleCurrentItemHandleInteractive, SIGNAL(triggered()), q, SLOT(toggleCurrentItemHandleInteractive()));

  this->ToggleCurrentItemTranslateHandleVisible = new QAction("Translate");
  this->ToggleCurrentItemTranslateHandleVisible->setCheckable(true);
  this->ToggleCurrentItemTranslateHandleVisible->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle);
  QObject::connect(this->ToggleCurrentItemTranslateHandleVisible, SIGNAL(triggered()), q, SLOT(toggleCurrentItemHandleTypeVisibility()));

  this->ToggleCurrentItemRotateHandleVisible = new QAction("Rotate");
  this->ToggleCurrentItemRotateHandleVisible->setCheckable(true);
  this->ToggleCurrentItemRotateHandleVisible->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, vtkMRMLMarkupsDisplayNode::ComponentRotationHandle);
  QObject::connect(this->ToggleCurrentItemRotateHandleVisible, SIGNAL(triggered()), q, SLOT(toggleCurrentItemHandleTypeVisibility()));

  this->ToggleCurrentItemScaleHandleVisible = new QAction("Scale");
  this->ToggleCurrentItemScaleHandleVisible->setCheckable(true);
  this->ToggleCurrentItemScaleHandleVisible->setProperty(INTERACTION_HANDLE_TYPE_PROPERTY, vtkMRMLMarkupsDisplayNode::ComponentScaleHandle);
  QObject::connect(this->ToggleCurrentItemScaleHandleVisible, SIGNAL(triggered()), q, SLOT(toggleCurrentItemHandleTypeVisibility()));

  this->CurrentItemHandleVisibilityMenu = new QMenu();
  this->CurrentItemHandleVisibilityMenu->addAction(this->ToggleCurrentItemTranslateHandleVisible);
  this->CurrentItemHandleVisibilityMenu->addAction(this->ToggleCurrentItemRotateHandleVisible);
  this->CurrentItemHandleVisibilityMenu->addAction(this->ToggleCurrentItemScaleHandleVisible);

  this->CurrentItemHandleVisibilityAction = new QAction("Interaction options");
  this->CurrentItemHandleVisibilityAction->setMenu(this->CurrentItemHandleVisibilityMenu);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyMarkupsPluginPrivate::~qSlicerSubjectHierarchyMarkupsPluginPrivate() = default;

//-----------------------------------------------------------------------------
vtkMRMLMarkupsNode* qSlicerSubjectHierarchyMarkupsPluginPrivate::markupsNodeFromViewContextMenuEventData()
{
  Q_Q(qSlicerSubjectHierarchyMarkupsPlugin);
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
  vtkMRMLMarkupsNode* markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);

  return markupsNode;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPluginPrivate::jumpToPoint(int controlPointIndex)
{
  Q_Q(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = this->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
    {
    qCritical() << Q_FUNC_INFO << ": cannot get application logic";
    return;
    }

  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qCritical() << Q_FUNC_INFO << ": could not get the Markups module logic.";
    return;
    }

  int viewGroup = -1;
  if (this->ViewContextMenuEventData.contains("ViewNodeID"))
    {
    vtkMRMLAbstractViewNode* viewNode = vtkMRMLAbstractViewNode::SafeDownCast(
      markupsNode->GetScene()->GetNodeByID(ViewContextMenuEventData["ViewNodeID"].toString().toStdString()));
    if (viewNode)
      {
      viewGroup = viewNode->GetViewGroup();
      }
    }
  markupsLogic->JumpSlicesToNthPointInMarkup(markupsNode->GetID(), controlPointIndex, true /*centered*/, viewGroup);
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyMarkupsPlugin methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyMarkupsPlugin::qSlicerSubjectHierarchyMarkupsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyMarkupsPluginPrivate(*this) )
{
  this->m_Name = QString("Markups");

  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyMarkupsPlugin::~qSlicerSubjectHierarchyMarkupsPlugin() = default;

//-----------------------------------------------------------------------------
double qSlicerSubjectHierarchyMarkupsPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node, vtkIdType parentItemID/*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/)const
{
  Q_UNUSED(parentItemID);
  if (!node)
    {
    qCritical() << Q_FUNC_INFO << ": Input node is NULL";
    return 0.0;
    }

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
    {
    qCritical() << Q_FUNC_INFO << ": cannot get application logic";
    return 0.0;
    }

  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qCritical() << Q_FUNC_INFO << ": could not get the Markups module logic.";
    return 0.0;
    }

  vtkMRMLMarkupsNode* markupsNode= vtkMRMLMarkupsNode::SafeDownCast(node);
  if (!markupsNode)
    {
    return 0.0;
    }

  bool registered = markupsLogic->GetWidgetByMarkupsType(markupsNode->GetMarkupType()) ? true : false;
  if (registered)
    {
    // Item is a registered markup
    return 0.5;
    }

  // Item is not a registered markup
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyMarkupsPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID)const
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

  // Markup
  vtkMRMLNode* associatedNode = shNode->GetItemDataNode(itemID);
  if (!associatedNode)
    {
    //NOTE: should there be a warning here?
    return 0.0;
    }

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
    {
    qCritical() << Q_FUNC_INFO << ": cannot get application logic";
    return 0.0;
    }

  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qCritical() << Q_FUNC_INFO << ": could not get the Markups module logic.";
    return 0.0;
    }

  vtkMRMLMarkupsNode* associatedMarkupsNode = vtkMRMLMarkupsNode::SafeDownCast(associatedNode);
  if (!associatedMarkupsNode)
    {
    return 0.0;
    }
  bool registered = markupsLogic->GetWidgetByMarkupsType(associatedMarkupsNode->GetMarkupType()) ? true : false;
  if (registered)
    {
    // Item is a registered markup
    return 0.5;
    }
  // Item is not a registered markup
  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyMarkupsPlugin::roleForPlugin()const
{
  return "Markup";
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyMarkupsPlugin::icon(vtkIdType itemID)
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QIcon();
    }

  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  if (!this->canOwnSubjectHierarchyItem(itemID))
    {
    // Item unknown by plugin
    return QIcon();
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    return QIcon();
    }
  vtkMRMLNode* node = shNode->GetItemDataNode(itemID);
  if (!node)
    {
    return QIcon();
    }

  vtkMRMLMarkupsNode *markupsNode = vtkMRMLMarkupsNode::SafeDownCast(node);
  if (markupsNode == nullptr)
    {
    return QIcon();
    }

  return QIcon(markupsNode->GetIcon());
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyMarkupsPlugin::visibilityIcon(int visible)
{
  // Have the default plugin (which is not registered) take care of this
  return qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->visibilityIcon(visible);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::setDisplayColor(vtkIdType itemID, QColor color, QMap<int, QVariant> terminologyMetaData)
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

  // Get display node
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find node for subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return;
    }
  vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": No display node";
    return;
    }

  // Set terminology metadata
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::TerminologyRole))
    {
    displayableNode->SetAttribute(vtkSegment::GetTerminologyEntryTagName(),
      terminologyMetaData[qSlicerTerminologyItemDelegate::TerminologyRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::NameRole))
    {
    displayableNode->SetName(
      terminologyMetaData[qSlicerTerminologyItemDelegate::NameRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::NameAutoGeneratedRole))
    {
    displayableNode->SetAttribute( vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName(),
      terminologyMetaData[qSlicerTerminologyItemDelegate::NameAutoGeneratedRole].toString().toUtf8().constData() );
    }
  if (terminologyMetaData.contains(qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole))
    {
    displayableNode->SetAttribute( vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName(),
      terminologyMetaData[qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole].toString().toUtf8().constData() );
    }

  // Set color
  double* oldColorArray = displayNode->GetColor();
  QColor oldColor = QColor::fromRgbF(oldColorArray[0], oldColorArray[1], oldColorArray[2]);
  if (oldColor != color)
    {
    displayNode->SetSelectedColor(color.redF(), color.greenF(), color.blueF());

    // Trigger update of color swatch
    shNode->ItemModified(itemID);
    }
}

//-----------------------------------------------------------------------------
QColor qSlicerSubjectHierarchyMarkupsPlugin::getDisplayColor(vtkIdType itemID, QMap<int, QVariant> &terminologyMetaData)const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return QColor(0,0,0,0);
    }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return QColor(0,0,0,0);
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid MRML scene";
    return QColor(0,0,0,0);
    }

  if (scene->IsImporting())
    {
    // During import SH node may be created before the segmentation is read into the scene,
    // so don't attempt to access the segment yet
    return QColor(0,0,0,0);
    }

  // Get display node
  vtkMRMLDisplayableNode* displayableNode = vtkMRMLDisplayableNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!displayableNode)
    {
    qCritical() << Q_FUNC_INFO << ": Unable to find node for subject hierarchy item " << shNode->GetItemName(itemID).c_str();
    return QColor(0,0,0,0);
    }
  vtkMRMLDisplayNode* displayNode = displayableNode->GetDisplayNode();
  if (!displayNode)
    {
    // this is normal when the markups node is being created
    return QColor(0,0,0,0);
    }

  // Get terminology metadata
  terminologyMetaData.clear();
  terminologyMetaData[qSlicerTerminologyItemDelegate::TerminologyRole] =
    displayableNode->GetAttribute(vtkSegment::GetTerminologyEntryTagName());
  terminologyMetaData[qSlicerTerminologyItemDelegate::NameRole] = displayableNode->GetName();
  // If auto generated flags are not initialized, then set them to the default
  // (color: on, name: off - this way color will be set from the selector but name will not)
  bool nameAutoGenerated = false;
  if (displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName()))
    {
    nameAutoGenerated = QVariant(displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName())).toBool();
    }
  terminologyMetaData[qSlicerTerminologyItemDelegate::NameAutoGeneratedRole] = nameAutoGenerated;
  bool colorAutoGenerated = true;
  if (displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName()))
    {
    colorAutoGenerated = QVariant(displayableNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName())).toBool();
    }
  terminologyMetaData[qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole] = colorAutoGenerated;

  // Get and return color
  double* colorArray = displayNode->GetSelectedColor();
  return QColor::fromRgbF(colorArray[0], colorArray[1], colorArray[2]);
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyMarkupsPlugin::viewContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyMarkupsPlugin);

  QList<QAction*> actions;
  actions << d->RenamePointAction << d->RefocusCameraAction << d->ToggleSelectPointAction
    << d->JumpToPreviousPointAction << d->JumpToNextPointAction << d->JumpToClosestPointAction
    << d->DeletePointAction << d->DeleteNodeAction << d->EditNodeTerminologyAction
    << d->ToggleHandleInteractive << d->HandleVisibilityAction;
  return actions;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData)
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
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

  // Markup
  vtkMRMLMarkupsNode* associatedNode = vtkMRMLMarkupsNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!associatedNode)
    {
    return;
    }

  d->ViewContextMenuEventData = eventData;
  d->ViewContextMenuEventData["NodeID"] = QVariant(associatedNode->GetID());

  int componentType = d->ViewContextMenuEventData["ComponentType"].toInt();
  bool pointActionsDisabled =
    componentType == vtkMRMLMarkupsDisplayNode::ComponentTranslationHandle ||
    componentType == vtkMRMLMarkupsDisplayNode::ComponentRotationHandle ||
    componentType == vtkMRMLMarkupsDisplayNode::ComponentScaleHandle ||
    componentType == vtkMRMLMarkupsDisplayNode::ComponentPlane ||
    componentType == vtkMRMLMarkupsROIDisplayNode::ComponentROI;

  d->RenamePointAction->setVisible(!pointActionsDisabled);
  d->DeletePointAction->setVisible(!pointActionsDisabled);
  if (!pointActionsDisabled)
    {
    if (associatedNode->GetFixedNumberOfControlPoints())
      {
      d->DeletePointAction->setText("Clear control point position");
      }
    else
      {
      d->DeletePointAction->setText("Delete control point");
      }
    }
  d->DeleteNodeAction->setVisible(true);
  d->ToggleSelectPointAction->setVisible(!pointActionsDisabled);

  d->JumpToClosestPointAction->setVisible(componentType == vtkMRMLMarkupsDisplayNode::ComponentLine
    && d->ViewContextMenuEventData.find("WorldPosition") != d->ViewContextMenuEventData.end());

  bool isControlPoint = componentType == vtkMRMLMarkupsDisplayNode::ComponentControlPoint;
  d->RenamePointAction->setVisible(isControlPoint);
  d->RefocusCameraAction->setVisible(isControlPoint);
  d->ToggleSelectPointAction->setVisible(isControlPoint);
  d->DeletePointAction->setVisible(isControlPoint);
  d->JumpToPreviousPointAction->setVisible(isControlPoint);
  d->JumpToNextPointAction->setVisible(isControlPoint);
  if (isControlPoint)
    {
    d->JumpToPreviousPointAction->setEnabled(false);
    d->JumpToNextPointAction->setEnabled(false);
    int currentControlPointIndex = d->ViewContextMenuEventData["ComponentIndex"].toInt();
    for (int controlPointIndex = currentControlPointIndex-1; controlPointIndex >= 0; controlPointIndex--)
      {
      if (associatedNode->GetNthControlPointPositionStatus(controlPointIndex) == vtkMRMLMarkupsNode::PositionDefined
        && associatedNode->GetNthControlPointVisibility(controlPointIndex))
        {
        // found previous control point
        d->JumpToPreviousPointAction->setEnabled(true);
        d->ViewContextMenuEventData["PreviousControlPointIndex"] = QVariant(controlPointIndex);
        break;
        }
      }
    for (int controlPointIndex = currentControlPointIndex+1; controlPointIndex < associatedNode->GetNumberOfControlPoints(); controlPointIndex++)
      {
      if (associatedNode->GetNthControlPointPositionStatus(controlPointIndex) == vtkMRMLMarkupsNode::PositionDefined
        && associatedNode->GetNthControlPointVisibility(controlPointIndex))
        {
        // found next control point
        d->JumpToNextPointAction->setEnabled(true);
        d->ViewContextMenuEventData["NextControlPointIndex"] = QVariant(controlPointIndex);
        break;
        }
      }
    }

  d->EditNodeTerminologyAction->setVisible(!pointActionsDisabled);

  // Update action text with relevant markup type
  QString markup_type = associatedNode->GetTypeDisplayName();
  d->DeleteNodeAction->setText("Delete " + markup_type);
  d->EditNodeTerminologyAction->setText("Edit " + markup_type + " terminology...");

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(associatedNode->GetDisplayNode());
  d->ToggleHandleInteractive->setVisible(displayNode != nullptr);
  d->HandleVisibilityAction->setVisible(displayNode != nullptr);
  vtkMRMLMarkupsROIDisplayNode* roiDisplayNode = vtkMRMLMarkupsROIDisplayNode::SafeDownCast(displayNode);
  vtkMRMLMarkupsPlaneDisplayNode* planeDisplayNode = vtkMRMLMarkupsPlaneDisplayNode::SafeDownCast(displayNode);
  d->ToggleScaleHandleVisible->setVisible(roiDisplayNode != nullptr || planeDisplayNode != nullptr);
  if (displayNode)
    {
    d->ToggleHandleInteractive->setChecked(displayNode->GetHandlesInteractive());
    d->ToggleTranslateHandleVisible->setChecked(displayNode->GetTranslationHandleVisibility());
    d->ToggleRotateHandleVisible->setChecked(displayNode->GetRotationHandleVisibility());
    d->ToggleScaleHandleVisible->setChecked(displayNode->GetScaleHandleVisibility());
    }
}

//-----------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyMarkupsPlugin::visibilityContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyMarkupsPlugin);

  QList<QAction*> actions;
  actions << d->ToggleCurrentItemHandleInteractive << d->CurrentItemHandleVisibilityAction;
  return actions;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::showVisibilityContextMenuActionsForItem(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
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

  // Markup
  vtkMRMLMarkupsNode* associatedNode = vtkMRMLMarkupsNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (associatedNode)
    {
    vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(associatedNode->GetDisplayNode());
    d->ToggleCurrentItemHandleInteractive->setVisible(displayNode != nullptr);
    d->CurrentItemHandleVisibilityAction->setVisible(displayNode != nullptr);
    d->ToggleCurrentItemScaleHandleVisible->setVisible(vtkMRMLMarkupsROIDisplayNode::SafeDownCast(displayNode) != nullptr);
    if (displayNode)
      {
      d->ToggleCurrentItemHandleInteractive->setChecked(displayNode->GetHandlesInteractive());
      d->ToggleCurrentItemTranslateHandleVisible->setChecked(displayNode->GetTranslationHandleVisibility());
      d->ToggleCurrentItemRotateHandleVisible->setChecked(displayNode->GetRotationHandleVisibility());
      d->ToggleCurrentItemScaleHandleVisible->setChecked(displayNode->GetScaleHandleVisibility());
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::showContextMenuActionsForItem(vtkIdType)
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
  // make sure we don't use metadata from some previous view context menu calls
  d->ViewContextMenuEventData.clear();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::renamePoint()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  // Get point index
  int componentIndex = d->ViewContextMenuEventData["ComponentIndex"].toInt();

  // Pop up an entry box for the new name, with the old name as default
  QString oldName(markupsNode->GetNthControlPointLabel(componentIndex).c_str());

  bool ok = false;
  QString newName = QInputDialog::getText(nullptr, QString("Rename ") + oldName, "New name:", QLineEdit::Normal, oldName, &ok);
  if (!ok)
    {
    return;
    }

  markupsNode->SetNthControlPointLabel(componentIndex, newName.toUtf8().constData());
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::refocusCamera()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (!appLogic)
    {
    qCritical() << Q_FUNC_INFO << ": cannot get application logic";
    return;
    }

  // Get point index
  int componentIndex = d->ViewContextMenuEventData["ComponentIndex"].toInt();

  vtkSlicerMarkupsLogic* markupsLogic = vtkSlicerMarkupsLogic::SafeDownCast(appLogic->GetModuleLogic("Markups"));
  if (!markupsLogic)
    {
    qCritical() << Q_FUNC_INFO << ": could not get the Markups module logic.";
    return;
    }

  markupsLogic->FocusCamerasOnNthPointInMarkup(markupsNode->GetID(), componentIndex);
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::deletePoint()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  // Get point index
  int componentIndex = d->ViewContextMenuEventData["ComponentIndex"].toInt();

  if (markupsNode->GetFixedNumberOfControlPoints())
    {
    markupsNode->UnsetNthControlPointPosition(componentIndex);
    }
  else
    {
    markupsNode->RemoveNthControlPoint(componentIndex);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::requestDeleteNode()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  d->NodesToDelete.push_back(markupsNode);
  QTimer::singleShot(0, this, SLOT(removeNodesToBeDeleted()));
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::removeNodesToBeDeleted()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access MRML scene";
    return;
    }
  if (scene->IsClosing())
    {
    return;
    }

  foreach(vtkWeakPointer<vtkMRMLMarkupsNode> markupsNode, d->NodesToDelete)
    {
    if (!markupsNode)
      {
      continue;
      }
    scene->RemoveNode(markupsNode);
    }
  d->NodesToDelete.clear();
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::toggleSelectPoint()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  // Get point index
  int componentIndex = d->ViewContextMenuEventData["ComponentIndex"].toInt();

  markupsNode->SetNthControlPointSelected(componentIndex, !markupsNode->GetNthControlPointSelected(componentIndex));
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::jumpToPreviousPoint()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
  d->jumpToPoint(d->ViewContextMenuEventData["PreviousControlPointIndex"].toInt());
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::jumpToNextPoint()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
  d->jumpToPoint(d->ViewContextMenuEventData["NextControlPointIndex"].toInt());
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::jumpToClosestPoint()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }
  if (d->ViewContextMenuEventData.find("WorldPosition") == d->ViewContextMenuEventData.end())
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get world position";
    return;
    }
  QVariantList worldPosVector = d->ViewContextMenuEventData["WorldPosition"].toList();
  if (worldPosVector.size() != 3)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid world position";
    return;
    }
  double worldPos[3] = { worldPosVector[0].toDouble(), worldPosVector[1].toDouble(), worldPosVector[2].toDouble() };
  int controlPointIndex = markupsNode->GetClosestControlPointIndexToPositionWorld(worldPos, true);
  if (controlPointIndex >= 0)
    {
    d->jumpToPoint(controlPointIndex);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::editNodeTerminology()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->mrmlScene();
  if (!scene)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access MRML scene";
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = scene->GetSubjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  vtkSlicerTerminologiesModuleLogic* terminologiesLogic = vtkSlicerTerminologiesModuleLogic::SafeDownCast(
    qSlicerCoreApplication::application()->moduleLogic("Terminologies"));
  if (!terminologiesLogic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get Terminologies module logic";
    return;
    }

  // Make sure display node exists to be able to access and set color based on terminology
  markupsNode->CreateDefaultDisplayNodes();
  vtkMRMLDisplayNode* displayNode = markupsNode->GetDisplayNode();
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid display node";
    return;
    }

  // Convert string list to VTK terminology entry. Do not check success, as an empty terminology is also a valid starting point
  vtkNew<vtkSlicerTerminologyEntry> terminologyEntry;
  const char* oldTerminologyAttribute = markupsNode->GetAttribute(vtkSegment::GetTerminologyEntryTagName());
  std::string oldTerminologyString(oldTerminologyAttribute ? oldTerminologyAttribute : "");
  terminologiesLogic->DeserializeTerminologyEntry(oldTerminologyString, terminologyEntry);

  // If auto generated flags are not initialized, then set them to the default
  // (color: on, name: off - this way color will be set from the selector but name will not)
  bool nameAutoGenerated = false;
  if (markupsNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName()))
    {
    nameAutoGenerated = QVariant(markupsNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetNameAutoGeneratedAttributeName())).toBool();
    }
  bool colorAutoGenerated = true;
  if (markupsNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName()))
    {
    colorAutoGenerated = QVariant(markupsNode->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetColorAutoGeneratedAttributeName())).toBool();
    }

  // Create terminology info bundle to pass to the selector dialog
  QColor generatedColor(vtkSlicerTerminologyType::INVALID_COLOR[0], vtkSlicerTerminologyType::INVALID_COLOR[1], vtkSlicerTerminologyType::INVALID_COLOR[2]);
  double* oldColorArray = displayNode->GetSelectedColor();
  QColor oldColor = QColor::fromRgbF(oldColorArray[0], oldColorArray[1], oldColorArray[2]);
  qSlicerTerminologyNavigatorWidget::TerminologyInfoBundle terminologyInfo(
    terminologyEntry, markupsNode->GetName(), nameAutoGenerated, oldColor, colorAutoGenerated, generatedColor);

  if (!qSlicerTerminologySelectorDialog::getTerminology(terminologyInfo, nullptr))
    {
    // User canceled
    return;
    }

  QString newTerminologyString(vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry(terminologyInfo.GetTerminologyEntry()).c_str());
  if (oldColor != terminologyInfo.Color || newTerminologyString.compare(oldTerminologyString.c_str()))
    {
    QMap<int, QVariant> terminologyMetaData;
    terminologyMetaData[qSlicerTerminologyItemDelegate::TerminologyRole] = newTerminologyString;
    terminologyMetaData[qSlicerTerminologyItemDelegate::NameRole] = terminologyInfo.Name;
    terminologyMetaData[qSlicerTerminologyItemDelegate::NameAutoGeneratedRole] = terminologyInfo.NameAutoGenerated;
    terminologyMetaData[qSlicerTerminologyItemDelegate::ColorAutoGeneratedRole] = terminologyInfo.ColorAutoGenerated;

    // Have the plugin set the color
    vtkIdType shItemID = shNode->GetItemByDataNode(markupsNode);
    this->setDisplayColor(shItemID, terminologyInfo.Color, terminologyMetaData);
    }
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::toggleHandleInteractive()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get display node";
    return;
    }
  displayNode->SetHandlesInteractive(!displayNode->GetHandlesInteractive());
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::toggleCurrentItemHandleInteractive()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

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

  vtkMRMLMarkupsNode* markupNode = vtkMRMLMarkupsNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (!markupNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid markup node";
    return;
    }

  markupNode->CreateDefaultDisplayNodes();
  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid display node";
    return;
    }

  displayNode->SetHandlesInteractive(!displayNode->GetHandlesInteractive());
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::toggleCurrentItemHandleTypeVisibility()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get display node";
    return;
    }

  QObject* sender = QObject::sender();
  if (!sender)
    {
    return;
    }

  int componentType = sender->property(INTERACTION_HANDLE_TYPE_PROPERTY).toInt();
  displayNode->SetHandleVisibility(componentType, !displayNode->GetHandleVisibility(componentType));
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::toggleHandleTypeVisibility()
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);

  vtkMRMLMarkupsNode* markupsNode = d->markupsNodeFromViewContextMenuEventData();
  if (!markupsNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get markups node";
    return;
    }

  vtkMRMLMarkupsDisplayNode* displayNode = vtkMRMLMarkupsDisplayNode::SafeDownCast(markupsNode->GetDisplayNode());
  if (!displayNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get display node";
    return;
    }

  QObject* sender = QObject::sender();
  if (!sender)
    {
    return;
    }

  int componentType = sender->property(INTERACTION_HANDLE_TYPE_PROPERTY).toInt();
  displayNode->SetHandleVisibility(componentType, !displayNode->GetHandleVisibility(componentType));
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyMarkupsPlugin::editProperties(vtkIdType itemID)
{
  Q_D(qSlicerSubjectHierarchyMarkupsPlugin);
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }
  if (d->ViewContextMenuEventData.contains("ComponentIndex"))
    {
    int componentIndex = d->ViewContextMenuEventData["ComponentIndex"].toInt();
    qSlicerApplication::application()->openNodeModule(shNode->GetItemDataNode(itemID), "ControlPointIndex", QString::number(componentIndex));
    }
  else
    {
    qSlicerApplication::application()->openNodeModule(shNode->GetItemDataNode(itemID));
    }
}
