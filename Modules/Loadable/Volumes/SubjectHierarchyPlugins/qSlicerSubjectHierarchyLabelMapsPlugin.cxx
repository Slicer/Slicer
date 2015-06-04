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
#include "qSlicerSubjectHierarchyLabelMapsPlugin.h"
#include "qSlicerSubjectHierarchyDefaultPlugin.h"

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLScene.h>
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
class qSlicerSubjectHierarchyLabelMapsPluginPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyLabelMapsPlugin);
protected:
  qSlicerSubjectHierarchyLabelMapsPlugin* const q_ptr;
public:
  qSlicerSubjectHierarchyLabelMapsPluginPrivate(qSlicerSubjectHierarchyLabelMapsPlugin& object);
  ~qSlicerSubjectHierarchyLabelMapsPluginPrivate();
  void init();
public:
  QIcon LabelmapIcon;
  QIcon VolumeVisibilityOffIcon;
  QIcon VolumeVisibilityOnIcon;

  QAction* ToggleLabelmapOutlineDisplayAction;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyLabelMapsPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPluginPrivate::qSlicerSubjectHierarchyLabelMapsPluginPrivate(qSlicerSubjectHierarchyLabelMapsPlugin& object)
: q_ptr(&object)
{
  this->LabelmapIcon = QIcon(":Icons/Labelmap.png");
  this->VolumeVisibilityOffIcon = QIcon(":Icons/VolumeVisibilityOff.png");
  this->VolumeVisibilityOnIcon = QIcon(":Icons/VolumeVisibilityOn.png");

  this->ToggleLabelmapOutlineDisplayAction = NULL;
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPluginPrivate::~qSlicerSubjectHierarchyLabelMapsPluginPrivate()
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPlugin::qSlicerSubjectHierarchyLabelMapsPlugin(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSubjectHierarchyLabelMapsPluginPrivate(*this) )
{
  this->m_Name = QString("LabelMaps");

  Q_D(qSlicerSubjectHierarchyLabelMapsPlugin);
  d->init();
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyLabelMapsPlugin);

  this->ToggleLabelmapOutlineDisplayAction = new QAction("Toggle labelmap outline display",q);
  QObject::connect(this->ToggleLabelmapOutlineDisplayAction, SIGNAL(toggled(bool)), q, SLOT(toggleLabelmapOutlineDisplay(bool)));
  this->ToggleLabelmapOutlineDisplayAction->setCheckable(true);
  this->ToggleLabelmapOutlineDisplayAction->setChecked(false);
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyLabelMapsPlugin::~qSlicerSubjectHierarchyLabelMapsPlugin()
{
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyLabelMapsPlugin::canAddNodeToSubjectHierarchy(vtkMRMLNode* node, vtkMRMLSubjectHierarchyNode* parent/*=NULL*/)const
{
  Q_UNUSED(parent);
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::canAddNodeToSubjectHierarchy: Input node is NULL!";
    return 0.0;
    }
  else if (node->IsA("vtkMRMLLabelMapVolumeNode"))
    {
    // Node is a labelmap
    return 0.7;
    }
  return 0.0;
}

//---------------------------------------------------------------------------
double qSlicerSubjectHierarchyLabelMapsPlugin::canOwnSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::canOwnSubjectHierarchyNode: Input node is NULL!";
    return 0.0;
    }

  // Volume
  vtkMRMLNode* associatedNode = node->GetAssociatedNode();
  if (associatedNode && associatedNode->IsA("vtkMRMLLabelMapVolumeNode"))
    {
    return 0.7;
    }

  return 0.0;
}

//---------------------------------------------------------------------------
const QString qSlicerSubjectHierarchyLabelMapsPlugin::roleForPlugin()const
{
  return "Label map volume";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyLabelMapsPlugin::tooltip(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::tooltip: Subject hierarchy node is NULL!";
    return QString("Invalid!");
    }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(node);

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node->GetAssociatedNode());
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
QIcon qSlicerSubjectHierarchyLabelMapsPlugin::icon(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyLabelMapsPlugin);

  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::icon: NULL node given!";
    return QIcon();
    }

  // Volume
  if (this->canOwnSubjectHierarchyNode(node))
    {
    return d->LabelmapIcon;
    }

  // Node unknown by plugin
  return QIcon();
}

//---------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyLabelMapsPlugin::visibilityIcon(int visible)
{
  Q_D(qSlicerSubjectHierarchyLabelMapsPlugin);

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
void qSlicerSubjectHierarchyLabelMapsPlugin::setDisplayVisibility(vtkMRMLSubjectHierarchyNode* node, int visible)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::setDisplayVisibility: NULL node!";
    return;
    }

  vtkMRMLLabelMapVolumeNode* associatedLabelMapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(node->GetAssociatedNode());
  // Volume
  if (associatedLabelMapNode)
    {
    this->showLabelMap(associatedLabelMapNode, visible);
    }
  // Default
  else
    {
    qSlicerSubjectHierarchyPluginHandler::instance()->defaultPlugin()->setDisplayVisibility(node, visible);
    }
}

//-----------------------------------------------------------------------------
int qSlicerSubjectHierarchyLabelMapsPlugin::getDisplayVisibility(vtkMRMLSubjectHierarchyNode* node)const
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::getDisplayVisibility: NULL node!";
    return -1;
    }

  // Sanity checks for volume
  vtkMRMLLabelMapVolumeNode* labelMapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(node->GetAssociatedNode());
  if (!labelMapNode)
    {
    return -1;
    }
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::getDisplayVisibility: Unable to get selection node to show volume node " << node->GetName();
    return -1;
    }

  /// Update selection node based on current volumes visibility (if the selection is different in the slice viewers, then the first one is set)
  /// TODO: This is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)
  this->updateSelectionNodeBasedOnCurrentLabelMapVisibility();

  if ( selectionNode->GetActiveLabelVolumeID() && !strcmp(selectionNode->GetActiveLabelVolumeID(), labelMapNode->GetID()) )
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::showLabelMap(vtkMRMLScalarVolumeNode* node, int visible/*=1*/)
{
  if (!node)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::showLabelMap: NULL node!";
    return;
    }
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  if (!scene)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::showLabelMap: Invalid MRML scene!";
    return;
    }
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::showLabelMap: Unable to get selection node to show volume node " << node->GetName();
    return;
    }

  // Get volume node
  vtkMRMLLabelMapVolumeNode* labelMapNode = NULL;
  if ((labelMapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(node)) == NULL)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::showLabelMap: Node to show node is not a label map node: " << node->GetName();
    return;
    }

  /// Update selection node based on current volumes visibility (if the selection is different in the slice viewers, then the first one is set)
  /// TODO: This is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)
  this->updateSelectionNodeBasedOnCurrentLabelMapVisibility();

  // Show
  if (visible)
    {
    if (selectionNode->GetActiveLabelVolumeID() && strlen(selectionNode->GetActiveLabelVolumeID()))
      {
      // Needed so that visibility icon is updated (could be done in a faster way, but there is no noticeable overhead)
      vtkMRMLLabelMapVolumeNode* originalLabelmapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(
        scene->GetNodeByID(selectionNode->GetActiveLabelVolumeID()) );
      this->showLabelMap(originalLabelmapNode, 0);
      }
    selectionNode->SetActiveLabelVolumeID(labelMapNode->GetID());
    }
  // Hide
  else
    {
    selectionNode->SetActiveLabelVolumeID(NULL);
    }
  qSlicerCoreApplication::application()->applicationLogic()->PropagateVolumeSelection();

  // Get subject hierarchy node for the volume node and have the scene model updated
  vtkMRMLSubjectHierarchyNode* subjectHierarchyNode = vtkMRMLSubjectHierarchyNode::GetAssociatedSubjectHierarchyNode(labelMapNode);
  if (subjectHierarchyNode)
    {
    subjectHierarchyNode->Modified();
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::updateSelectionNodeBasedOnCurrentLabelMapVisibility()const
{
  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::updateSelectionNodeBasedOnCurrentVolumesVisibility: Unable to get selection node";
    return;
    }

  // TODO: This is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)

  // Determine labelmap selection (if the selection is different in the slice viewers, then the first one is set)
  std::string selectedLabelmapID = this->getSelectedLabelmapVolumeNodeID();
  selectionNode->SetActiveLabelVolumeID(selectedLabelmapID.c_str());
}

//---------------------------------------------------------------------------
std::string qSlicerSubjectHierarchyLabelMapsPlugin::getSelectedLabelmapVolumeNodeID()const
{
  // TODO: This method is a workaround (http://www.na-mic.org/Bug/view.php?id=3551)
  std::string selectedLabelmapID("");

  vtkMRMLSelectionNode* selectionNode = qSlicerCoreApplication::application()->applicationLogic()->GetSelectionNode();
  if (!selectionNode)
    {
    qCritical() << "qSlicerSubjectHierarchyLabelMapsPlugin::getSelectedLabelmapVolumeNodeID: Unable to get selection node";
    return selectedLabelmapID;
    }

  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  vtkMRMLSliceCompositeNode* compositeNode = NULL;
  const int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");

  for (int i=0; i<numberOfCompositeNodes; i++)
    {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceCompositeNode" ) );
    if (compositeNode && compositeNode->GetLabelVolumeID() && selectedLabelmapID.empty())
      {
      selectedLabelmapID = std::string(compositeNode->GetLabelVolumeID());
      break;
      }
    }

  return selectedLabelmapID;
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyLabelMapsPlugin::nodeContextMenuActions()const
{
  Q_D(const qSlicerSubjectHierarchyLabelMapsPlugin);

  QList<QAction*> actions;
  actions << d->ToggleLabelmapOutlineDisplayAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::showContextMenuActionsForNode(vtkMRMLSubjectHierarchyNode* node)
{
  Q_D(qSlicerSubjectHierarchyLabelMapsPlugin);
  this->hideAllContextMenuActions();

  if (!node)
    {
    // There are no scene actions in this plugin
    return;
    }

  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();

  // Volume
  if (this->canOwnSubjectHierarchyNode(node))
    {
    // Determine current state of the toggle labelmap outline checkbox (from the first slice view)
    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast ( scene->GetNthNodeByClass( 0, "vtkMRMLSliceNode" ) );
    int useLabelOutline = sliceNode->GetUseLabelOutline();
    d->ToggleLabelmapOutlineDisplayAction->blockSignals(true);
    d->ToggleLabelmapOutlineDisplayAction->setChecked(useLabelOutline);
    d->ToggleLabelmapOutlineDisplayAction->blockSignals(false);

    d->ToggleLabelmapOutlineDisplayAction->setVisible(true);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::toggleLabelmapOutlineDisplay(bool checked)
{
  vtkMRMLScene* scene = qSlicerSubjectHierarchyPluginHandler::instance()->scene();
  vtkMRMLSliceNode* sliceNode = NULL;
  const int numberOfSliceNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceNode");

  for (int i=0; i<numberOfSliceNodes; i++)
    {
    sliceNode = vtkMRMLSliceNode::SafeDownCast ( scene->GetNthNodeByClass( i, "vtkMRMLSliceNode" ) );
    sliceNode->SetUseLabelOutline(checked);
    }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyLabelMapsPlugin::editProperties(vtkMRMLSubjectHierarchyNode* node)
{
  // Switch to volumes module and volume already selected
  qSlicerAbstractModuleWidget* moduleWidget = qSlicerSubjectHierarchyAbstractPlugin::switchToModule("Volumes");
  if (moduleWidget)
    {
    // Get node selector combobox
    qMRMLNodeComboBox* nodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("ActiveVolumeNodeSelector");

    // Choose current data node
    if (nodeSelector)
      {
      nodeSelector->setCurrentNode(node->GetAssociatedNode());
      }
    }
}
