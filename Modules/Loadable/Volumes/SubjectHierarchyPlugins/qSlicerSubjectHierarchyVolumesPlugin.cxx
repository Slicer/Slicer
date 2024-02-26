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
#include "qSlicerModuleManager.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerColorLogic.h"
#include "qSlicerAbstractCoreModule.h"

// MRML includes
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLColorLegendDisplayNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLViewNode.h>

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
#include <QMenu>

// CTK includes
#include "ctkSignalMapper.h"

namespace
{
const std::string PRESET_AUTO = "_AUTO";
const std::string DISPLAY_NODE_PRESET_PREFIX = "_DISPLAY_NODE_";
const int MAX_NUMBER_OF_DISPLAY_NODE_PRESETS = 4;
} // namespace

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyVolumesPluginPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchyVolumesPlugin);

protected:
  qSlicerSubjectHierarchyVolumesPlugin* const q_ptr;

public:
  qSlicerSubjectHierarchyVolumesPluginPrivate(qSlicerSubjectHierarchyVolumesPlugin& object);
  ~qSlicerSubjectHierarchyVolumesPluginPrivate() override;
  void init();

  bool resetFieldOfViewOnShow();
  bool resetViewOrientationOnShow();

  qMRMLSliceWidget* sliceWidgetForSliceCompositeNode(vtkMRMLSliceCompositeNode* compositeNode);

public:
  QIcon VolumeIcon;
  QIcon VolumeVisibilityOffIcon;
  QIcon VolumeVisibilityOnIcon;

  QAction* ShowVolumesInBranchAction{ nullptr };
  QAction* ShowVolumeInForegroundAction{ nullptr };
  QAction* ResetFieldOfViewOnShowAction{ nullptr };
  QAction* ResetViewOrientationOnShowAction{ nullptr };

  QAction* VolumeDisplayPresetAction{ nullptr };
  QMenu* PresetSubmenu{ nullptr };
  ctkSignalMapper* PresetModeMapper{ nullptr };

  QAction* ShowColorLegendAction{ nullptr };

  vtkWeakPointer<vtkMRMLVolumeNode> SelectedVolumeNode;
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyVolumesPluginPrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPluginPrivate::qSlicerSubjectHierarchyVolumesPluginPrivate(
  qSlicerSubjectHierarchyVolumesPlugin& object)
  : q_ptr(&object)
{
  this->VolumeIcon = QIcon(":Icons/Volume.png");
  this->VolumeVisibilityOffIcon = QIcon(":Icons/VolumeVisibilityOff.png");
  this->VolumeVisibilityOnIcon = QIcon(":Icons/VolumeVisibilityOn.png");
}

//------------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPluginPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchyVolumesPlugin);

  // Connect layout changes to slot so that volume visibility icons are correctly updated
  if (qSlicerApplication::application()->layoutManager())
  {
    QObject::connect(
      qSlicerApplication::application()->layoutManager(), SIGNAL(layoutChanged(int)), q, SLOT(onLayoutChanged(int)));
    // Make sure initial connections are made even before a layout change
    QTimer::singleShot(0, q, SLOT(onLayoutChanged()));
  }

  this->ShowVolumesInBranchAction = new QAction(qSlicerSubjectHierarchyVolumesPlugin::tr("Show volumes in folder"), q);
  QObject::connect(this->ShowVolumesInBranchAction, SIGNAL(triggered()), q, SLOT(showVolumesInBranch()));

  this->ShowVolumeInForegroundAction =
    new QAction(qSlicerSubjectHierarchyVolumesPlugin::tr("Show in slice views as foreground"), q);
  QObject::connect(this->ShowVolumeInForegroundAction, SIGNAL(triggered()), q, SLOT(showVolumeInForeground()));

  this->ResetFieldOfViewOnShowAction =
    new QAction(qSlicerSubjectHierarchyVolumesPlugin::tr("Reset field of view on show"), q);
  QObject::connect(
    this->ResetFieldOfViewOnShowAction, SIGNAL(toggled(bool)), q, SLOT(toggleResetFieldOfViewOnShowAction(bool)));
  this->ResetFieldOfViewOnShowAction->setCheckable(true);
  this->ResetFieldOfViewOnShowAction->setChecked(false);

  this->ResetViewOrientationOnShowAction =
    new QAction(qSlicerSubjectHierarchyVolumesPlugin::tr("Reset view orientation on show"), q);
  QObject::connect(this->ResetViewOrientationOnShowAction,
                   SIGNAL(toggled(bool)),
                   q,
                   SLOT(toggleResetViewOrientationOnShowAction(bool)));
  this->ResetViewOrientationOnShowAction->setCheckable(true);
  this->ResetViewOrientationOnShowAction->setChecked(false);

  // Add color legend action

  this->ShowColorLegendAction = new QAction(qSlicerSubjectHierarchyVolumesPlugin::tr("Show color legend"), q);
  this->ShowColorLegendAction->setObjectName("ShowColorLegendAction");
  q->setActionPosition(this->ShowColorLegendAction, qSlicerSubjectHierarchyAbstractPlugin::SectionBottom);
  QObject::connect(this->ShowColorLegendAction, SIGNAL(toggled(bool)), q, SLOT(toggleVisibilityForCurrentItem(bool)));
  this->ShowColorLegendAction->setCheckable(true);
  this->ShowColorLegendAction->setChecked(false);

  // Add volume preset actions

  this->VolumeDisplayPresetAction = new QAction(qSlicerSubjectHierarchyVolumesPlugin::tr("Window/level presets"));
  this->VolumeDisplayPresetAction->setObjectName("VolumeDisplayPresetAction");
  q->setActionPosition(this->VolumeDisplayPresetAction, qSlicerSubjectHierarchyAbstractPlugin::SectionBottom);

  // read volume preset names from volumes logic
  vtkSlicerVolumesLogic* volumesModuleLogic =
    (qSlicerCoreApplication::application()
       ? vtkSlicerVolumesLogic::SafeDownCast(qSlicerCoreApplication::application()->moduleLogic("Volumes"))
       : nullptr);
  if (!volumesModuleLogic)
  {
    qWarning() << Q_FUNC_INFO << " failed: Module logic 'Volumes' not found.";
    return;
  }

  this->PresetSubmenu = new QMenu();
  this->PresetSubmenu->setToolTipsVisible(true);
  QActionGroup* presetModeActions = new QActionGroup(this);
  presetModeActions->setExclusive(true);
  this->PresetModeMapper = new ctkSignalMapper(this);

  for (int displayNodePresetIndex = 0; displayNodePresetIndex < MAX_NUMBER_OF_DISPLAY_NODE_PRESETS;
       displayNodePresetIndex++)
  {
    QString presetIdStr =
      QString("%1%2").arg(QString::fromStdString(DISPLAY_NODE_PRESET_PREFIX)).arg(displayNodePresetIndex);
    QAction* presetAction = new QAction();
    presetAction->setObjectName(presetIdStr);
    presetAction->setToolTip(qSlicerSubjectHierarchyVolumesPlugin::tr("Default preset for the selected volume"));
    presetAction->setCheckable(true);
    this->PresetSubmenu->addAction(presetAction);
    presetModeActions->addAction(presetAction);
    this->PresetModeMapper->setMapping(presetAction, presetIdStr);
  }

  // Add Automatic preset
  QAction* autoAction = new QAction(qSlicerSubjectHierarchyVolumesPlugin::tr("Automatic"));
  autoAction->setObjectName(QString::fromStdString(PRESET_AUTO));
  autoAction->setToolTip(qSlicerSubjectHierarchyVolumesPlugin::tr("Display the full intensity range of the volume."));
  autoAction->setCheckable(true);
  this->PresetSubmenu->addAction(autoAction);
  presetModeActions->addAction(autoAction);
  this->PresetModeMapper->setMapping(autoAction, QString::fromStdString(PRESET_AUTO));

  std::vector<std::string> presetIds = volumesModuleLogic->GetVolumeDisplayPresetIDs();
  for (const auto& presetId : presetIds)
  {
    vtkSlicerVolumesLogic::VolumeDisplayPreset preset = volumesModuleLogic->GetVolumeDisplayPreset(presetId);
    QString presetName = qSlicerSubjectHierarchyVolumesPlugin::tr(preset.name.c_str());
    QString presetIdStr = QString::fromStdString(presetId);
    QAction* presetAction = new QAction(presetName);
    presetAction->setObjectName(presetIdStr);
    presetAction->setToolTip(qSlicerSubjectHierarchyVolumesPlugin::tr(preset.description.c_str()));
    if (!preset.icon.empty())
    {
      presetAction->setIcon(QIcon(QString::fromStdString(preset.icon)));
    }
    presetAction->setCheckable(true);
    this->PresetSubmenu->addAction(presetAction);
    presetModeActions->addAction(presetAction);
    this->PresetModeMapper->setMapping(presetAction, presetIdStr);
  }

  this->VolumeDisplayPresetAction->setMenu(this->PresetSubmenu);
  QObject::connect(presetModeActions, SIGNAL(triggered(QAction*)), this->PresetModeMapper, SLOT(map(QAction*)));
  QObject::connect(this->PresetModeMapper, SIGNAL(mapped(QString)), q, SLOT(setVolumePreset(QString)));
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPluginPrivate::~qSlicerSubjectHierarchyVolumesPluginPrivate() = default;

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVolumesPluginPrivate::resetFieldOfViewOnShow()
{
  QSettings settings;
  return settings.value("SubjectHierarchy/ResetFieldOfViewOnShowVolume", true).toBool();
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVolumesPluginPrivate::resetViewOrientationOnShow()
{
  QSettings settings;
  return settings.value("SubjectHierarchy/ResetViewOrientationOnShowVolume", true).toBool();
}

//------------------------------------------------------------------------------
qMRMLSliceWidget* qSlicerSubjectHierarchyVolumesPluginPrivate::sliceWidgetForSliceCompositeNode(
  vtkMRMLSliceCompositeNode* compositeNode)
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
  , d_ptr(new qSlicerSubjectHierarchyVolumesPluginPrivate(*this))
{
  this->m_Name = QString("Volumes");

  Q_D(qSlicerSubjectHierarchyVolumesPlugin);
  d->init();
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyVolumesPlugin::~qSlicerSubjectHierarchyVolumesPlugin() = default;

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVolumesPlugin::viewContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyVolumesPlugin);
  QList<QAction*> actions;
  actions << d->VolumeDisplayPresetAction << d->ShowColorLegendAction;
  return actions;
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::showViewContextMenuActionsForItem(vtkIdType itemID, QVariantMap eventData)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);

  d->VolumeDisplayPresetAction->setVisible(false);

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode || !shNode->GetScene())
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
  }
  if (itemID != shNode->GetSceneItemID())
  {
    return;
  }
  if (!eventData.contains("ViewNodeID"))
  {
    return;
  }
  vtkMRMLSliceNode* sliceNode =
    vtkMRMLSliceNode::SafeDownCast(shNode->GetScene()->GetNodeByID(eventData["ViewNodeID"].toString().toStdString()));
  if (!sliceNode)
  {
    return;
  }
  vtkMRMLSliceLogic* sliceLogic = qSlicerApplication::application()->applicationLogic()->GetSliceLogic(sliceNode);
  if (!sliceLogic)
  {
    return;
  }

  QVariantList worldPosVector = eventData["WorldPosition"].toList();
  if (worldPosVector.size() != 3)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid world position";
    return;
  }
  double worldPos[3] = { worldPosVector[0].toDouble(), worldPosVector[1].toDouble(), worldPosVector[2].toDouble() };
  int volumeLayer = sliceLogic->GetEditableLayerAtWorldPosition(worldPos);

  // Cache nodes to have them available for the menu action execution.
  d->SelectedVolumeNode = sliceLogic->GetLayerVolumeNode(volumeLayer);

  bool hasPrimaryDisplayNode = false;
  bool colorLegendIsVisible = false;
  if (d->SelectedVolumeNode)
  {
    // Check the checkbox of the current display preset
    vtkSlicerVolumesLogic* volumesModuleLogic =
      vtkSlicerVolumesLogic::SafeDownCast(qSlicerApplication::application()->moduleLogic("Volumes"));
    if (volumesModuleLogic)
    {
      // For presets in display node
      vtkMRMLScalarVolumeDisplayNode* scalarVolumeDisplayNode =
        vtkMRMLScalarVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
      double currentWindowWidth = scalarVolumeDisplayNode ? scalarVolumeDisplayNode->GetWindow() : 0;
      double currentWindowLevel = scalarVolumeDisplayNode ? scalarVolumeDisplayNode->GetLevel() : 0;
      int numberOfDisplayNodePresets = scalarVolumeDisplayNode->GetNumberOfWindowLevelPresets();
      // For presets in volumes module logic
      QString appliedPresetId = QString::fromStdString(
        volumesModuleLogic->GetAppliedVolumeDisplayPresetId(d->SelectedVolumeNode->GetVolumeDisplayNode()));
      for (QAction* presetAction : d->PresetSubmenu->actions())
      {
        QString presetId = presetAction->objectName();
        if (presetId.startsWith(QString::fromStdString(DISPLAY_NODE_PRESET_PREFIX)))
        {
          // Preset stored in display node
          int displayNodePresetIndex = presetId.right(presetId.length() - DISPLAY_NODE_PRESET_PREFIX.length()).toInt();
          if (scalarVolumeDisplayNode && displayNodePresetIndex < numberOfDisplayNodePresets)
          {
            // existing display node preset
            double presetWindowWidth = scalarVolumeDisplayNode->GetWindowPreset(displayNodePresetIndex);
            double presetWindowLevel = scalarVolumeDisplayNode->GetLevelPreset(displayNodePresetIndex);
            presetAction->setText(tr("Default (WW=%1, WL=%2)").arg(presetWindowWidth).arg(presetWindowLevel));
            presetAction->setChecked(!scalarVolumeDisplayNode->GetAutoWindowLevel()
                                     && currentWindowWidth == presetWindowWidth
                                     && currentWindowLevel == presetWindowLevel);
            presetAction->setVisible(true);
          }
          else
          {
            // don't display this action, no corresponding display node preset
            presetAction->setVisible(false);
          }
        }
        else if (presetId == QString::fromStdString(PRESET_AUTO))
        {
          presetAction->setChecked(scalarVolumeDisplayNode->GetAutoWindowLevel());
        }
        else
        {
          // Preset storedin volume logic
          presetAction->setChecked(presetAction->objectName() == appliedPresetId);
        }
      }
    }

    // Parameters for color legend checkbox
    vtkSlicerColorLogic* colorsModuleLogic =
      vtkSlicerColorLogic::SafeDownCast(qSlicerApplication::application()->moduleLogic("Colors"));
    if (colorsModuleLogic)
    {
      vtkMRMLVolumeDisplayNode* volumeDisplayNode =
        vtkMRMLVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
      if (volumeDisplayNode)
      {
        hasPrimaryDisplayNode = true;
      }
      vtkMRMLColorLegendDisplayNode* colorLegendDisplayNode =
        vtkSlicerColorLogic::GetColorLegendDisplayNode(d->SelectedVolumeNode);
      if (colorLegendDisplayNode)
      {
        colorLegendIsVisible = colorLegendDisplayNode->GetVisibility(sliceNode->GetID());
      }
    }
  }
  d->ShowColorLegendAction->setVisible(hasPrimaryDisplayNode);
  QSignalBlocker Block(d->ShowColorLegendAction);
  d->ShowColorLegendAction->setChecked(colorLegendIsVisible);

  d->VolumeDisplayPresetAction->setVisible(d->SelectedVolumeNode.GetPointer() != nullptr);
}

//----------------------------------------------------------------------------
double qSlicerSubjectHierarchyVolumesPlugin::canAddNodeToSubjectHierarchy(
  vtkMRMLNode* node,
  vtkIdType parentItemID /*=vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID*/) const
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
double qSlicerSubjectHierarchyVolumesPlugin::canOwnSubjectHierarchyItem(vtkIdType itemID) const
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
const QString qSlicerSubjectHierarchyVolumesPlugin::roleForPlugin() const
{
  return "Scalar volume";
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyVolumesPlugin::tooltip(vtkIdType itemID) const
{
  if (itemID == vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID)
  {
    qCritical() << Q_FUNC_INFO << ": Invalid input item";
    return tr("Invalid");
  }
  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return tr("Error");
  }

  // Get basic tooltip from abstract plugin
  QString tooltipString = Superclass::tooltip(itemID);

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  vtkImageData* imageData = (volumeNode ? volumeNode->GetImageData() : nullptr);
  if (volumeNode && imageData)
  {
    int dimensions[3] = { 0, 0, 0 };
    imageData->GetDimensions(dimensions);
    double spacing[3] = { 0.0, 0.0, 0.0 };
    volumeNode->GetSpacing(spacing);
    tooltipString.append(QString(" (%1 %2x%3x%4  %5 %6mm x %7mm x %8mm)")
                           .arg(tr("Dimensions:"))
                           .arg(dimensions[0])
                           .arg(dimensions[1])
                           .arg(dimensions[2])
                           .arg(tr("Spacing:"))
                           .arg(spacing[0], 0, 'g', 3)
                           .arg(spacing[1], 0, 'g', 3)
                           .arg(spacing[2], 0, 'g', 3));
  }
  else
  {
    tooltipString.append(tr(" Empty volume"));
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
  vtkMRMLScalarVolumeNode* associatedVolumeNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
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
int qSlicerSubjectHierarchyVolumesPlugin::getDisplayVisibility(vtkIdType itemID) const
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
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeNode)
  {
    return -1;
  }

  // Return with 1 if volume is shown using volume rendering
  int numberOfDisplayNodes = volumeNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
  {
    vtkMRMLDisplayNode* displayNode = volumeNode->GetNthDisplayNode(displayNodeIndex);
    if (!displayNode || !displayNode->IsShowModeDefault())
    {
      continue;
    }
    if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode))
    {
      // scalar volume display node does not control visibility, visibility in those
      // views will be collected from slice views below
      continue;
    }
    if (displayNode->GetVisibility())
    {
      return 1;
    }
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
void qSlicerSubjectHierarchyVolumesPlugin::showVolumeInAllViews(vtkMRMLScalarVolumeNode* node,
                                                                int layer /*=vtkMRMLApplicationLogic::BackgroundLayer*/)
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
  vtkIdType shItemId = shNode->GetItemByDataNode(node);
  subjectHierarchyItemsToUpdate.insert(shItemId);

  bool resetOrientation = d->resetViewOrientationOnShow();
  bool resetFov = d->resetFieldOfViewOnShow();

  vtkMRMLSliceCompositeNode* compositeNode = nullptr;
  int numberOfCompositeNodes = scene->GetNumberOfNodesByClass("vtkMRMLSliceCompositeNode");
  for (int i = 0; i < numberOfCompositeNodes; i++)
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
    if (resetOrientation || resetFov)
    {
      qMRMLSliceWidget* sliceWidget = d->sliceWidgetForSliceCompositeNode(compositeNode);
      vtkMRMLSliceLogic* sliceLogic = sliceWidget ? sliceWidget->sliceLogic() : nullptr;
      vtkMRMLSliceNode* sliceNode = sliceWidget ? sliceWidget->mrmlSliceNode() : nullptr;
      if (sliceLogic && sliceNode)
      {
        if (resetOrientation)
        {
          // Set to default orientation before rotation so that the view is snapped
          // closest to the default orientation of this slice view.
          sliceNode->SetOrientationToDefault();
          // If the volume is shown in only one view and the volume is a single slice then
          // make sure the view is aligned with that, otherwise just snap to closest view.
          bool forceSlicePlaneToSingleSlice = (numberOfCompositeNodes == 1);
          sliceWidget->sliceLogic()->RotateSliceToLowestVolumeAxes(forceSlicePlaneToSingleSlice);
        }
        if (resetFov)
        {
          sliceLogic->FitSliceToAll();
        }
      }
    }
  }

  // Volume rendering display
  qSlicerSubjectHierarchyAbstractPlugin* volumeRenderingPlugin =
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("VolumeRendering");
  vtkNew<vtkIdList> allShItemIds;
  allShItemIds->InsertNextId(shItemId);
  if (volumeRenderingPlugin)
  {
    std::vector<vtkMRMLNode*> allViewNodes;
    scene->GetNodesByClass("vtkMRMLViewNode", allViewNodes);
    int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
    {
      vtkMRMLDisplayNode* displayNode = node->GetNthDisplayNode(displayNodeIndex);
      // By default we do not show volume rendering, only if it is explicitly enabled.
      if (!displayNode || !displayNode->IsShowModeDefault())
      {
        continue;
      }
      if (!displayNode->IsA("vtkMRMLVolumeRenderingDisplayNode")) // ignore everything except VolumeRendering
      {
        // we only manage existing volume rendering display nodes here
        // (we don't want to show volume rendering until volume rendering has been explicitly enabled)
        continue;
      }
      for (vtkMRMLNode* node : allViewNodes)
      {
        vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(node);
        if (!viewNode)
        {
          continue;
        }
        if (!displayNode->IsDisplayableInView(viewNode->GetID()))
        {
          continue;
        }
        volumeRenderingPlugin->showItemInView(shItemId, viewNode, allShItemIds);
      }
    }
  }

  // Show color legend display node
  int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
  {
    vtkMRMLDisplayNode* displayNode = node->GetNthDisplayNode(displayNodeIndex);
    // ignore everything except vtkMRMLColorLegendDisplayNOde
    // we only manage existing color legend display nodes here
    // (we don't want to show color legend until color legend has been explicitly enabled)
    if (displayNode && displayNode->IsShowModeDefault() && displayNode->IsA("vtkMRMLColorLegendDisplayNode"))
    {
      displayNode->VisibilityOn();
    }
  }

  // Update scene model for subject hierarchy nodes that were just shown
  for (QSet<vtkIdType>::iterator volumeItemIt = subjectHierarchyItemsToUpdate.begin();
       volumeItemIt != subjectHierarchyItemsToUpdate.end();
       ++volumeItemIt)
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
  for (int i = 0; i < numberOfCompositeNodes; i++)
  {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLSliceCompositeNode"));
    const char* backgroundVolumeID = compositeNode->GetBackgroundVolumeID();
    const char* foregroundVolumeID = compositeNode->GetForegroundVolumeID();
    const char* labelVolumeID = compositeNode->GetLabelVolumeID();
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

  // Color legend display, volume rendering display node
  int numberOfDisplayNodes = node->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
  {
    vtkMRMLDisplayNode* displayNode = node->GetNthDisplayNode(displayNodeIndex);
    if (!displayNode || !displayNode->IsShowModeDefault())
    {
      continue;
    }
    if (vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode))
    {
      // visibility in slice views is managed separately
      continue;
    }
    displayNode->SetVisibility(false);
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
  for (int i = 0; i < numberOfCompositeNodes; i++)
  {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLSliceCompositeNode"));
    if (layer & vtkMRMLApplicationLogic::BackgroundLayer && compositeNode->GetBackgroundVolumeID()
        && strcmp(compositeNode->GetBackgroundVolumeID(), ""))
    {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode(scene->GetNodeByID(compositeNode->GetBackgroundVolumeID())));
    }
    if (layer & vtkMRMLApplicationLogic::ForegroundLayer && compositeNode->GetForegroundVolumeID()
        && strcmp(compositeNode->GetForegroundVolumeID(), ""))
    {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode(scene->GetNodeByID(compositeNode->GetForegroundVolumeID())));
    }
    if (layer & vtkMRMLApplicationLogic::LabelLayer && compositeNode->GetLabelVolumeID()
        && strcmp(compositeNode->GetLabelVolumeID(), ""))
    {
      shownVolumeItemIDs.insert(shNode->GetItemByDataNode(scene->GetNodeByID(compositeNode->GetLabelVolumeID())));
    }
  }
}

//---------------------------------------------------------------------------
QList<QAction*> qSlicerSubjectHierarchyVolumesPlugin::visibilityContextMenuActions() const
{
  Q_D(const qSlicerSubjectHierarchyVolumesPlugin);

  QList<QAction*> actions;
  actions << d->ShowVolumesInBranchAction << d->ShowVolumeInForegroundAction << d->ResetFieldOfViewOnShowAction
          << d->ResetViewOrientationOnShowAction << d->ShowColorLegendAction;
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

    d->ResetViewOrientationOnShowAction->setChecked(d->resetViewOrientationOnShow());
    d->ResetViewOrientationOnShowAction->setVisible(true);
  }

  // Folders (Patient, Study, Folder)
  if (shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelPatient())
      || shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetDICOMLevelStudy())
      || shNode->IsItemLevel(itemID, vtkMRMLSubjectHierarchyConstants::GetSubjectHierarchyLevelFolder()))
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
  for (QSet<vtkIdType>::iterator volumeItemIt = subjectHierarchyItemsToUpdate.begin();
       volumeItemIt != subjectHierarchyItemsToUpdate.end();
       ++volumeItemIt)
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
  for (int i = 0; i < childVolumeNodes->GetNumberOfItems(); ++i)
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
        for (int i = 0; i < numberOfCompositeNodes; i++)
        {
          compositeNode =
            vtkMRMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLSliceCompositeNode"));
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
  for (int i = 0; i < numberOfCompositeNodes; i++)
  {
    compositeNode = vtkMRMLSliceCompositeNode::SafeDownCast(scene->GetNthNodeByClass(i, "vtkMRMLSliceCompositeNode"));
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
  for (int i = 0; i < numberOfCompositeNodes; i++)
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
  for (int i = 0; i < numberOfVolumeNodes; i++)
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

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::toggleResetViewOrientationOnShowAction(bool on)
{
  QSettings settings;
  settings.setValue("SubjectHierarchy/ResetViewOrientationOnShowVolume", on);
}

//-----------------------------------------------------------------------------
bool qSlicerSubjectHierarchyVolumesPlugin::showItemInView(vtkIdType itemID,
                                                          vtkMRMLAbstractViewNode* viewNode,
                                                          vtkIdList* allItemsToShow)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);
  vtkMRMLViewNode* threeDViewNode = vtkMRMLViewNode::SafeDownCast(viewNode);
  if (threeDViewNode)
  {
    qSlicerSubjectHierarchyAbstractPlugin* volumeRenderingPlugin =
      qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("VolumeRendering");
    if (!volumeRenderingPlugin)
    {
      qCritical() << Q_FUNC_INFO << ": Failed to access Volume rendering subject hierarchy plugin";
      return false;
    }
    return volumeRenderingPlugin->showItemInView(itemID, viewNode, allItemsToShow);
  }

  vtkMRMLSubjectHierarchyNode* shNode = qSlicerSubjectHierarchyPluginHandler::instance()->subjectHierarchyNode();
  if (!shNode)
  {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return false;
  }

  vtkMRMLVolumeNode* volumeToShow = vtkMRMLVolumeNode::SafeDownCast(shNode->GetItemDataNode(itemID));
  if (!volumeToShow)
  {
    // This method can only handle volume nodes
    return false;
  }

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(viewNode);
  if (!sliceNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: can only show items if a valid slice view is specified";
    return false;
  }

  // Get foreground, background, and label nodes
  vtkMRMLVolumeNode* backgroundNode = nullptr;
  vtkMRMLVolumeNode* foregroundNode = nullptr;
  vtkMRMLLabelMapVolumeNode* labelNode = nullptr;
  for (vtkIdType itemIndex = 0; itemIndex < allItemsToShow->GetNumberOfIds(); itemIndex++)
  {
    vtkMRMLVolumeNode* volumeNode =
      vtkMRMLVolumeNode::SafeDownCast(shNode->GetItemDataNode(allItemsToShow->GetId(itemIndex)));
    if (!volumeNode)
    {
      continue;
    }
    if (vtkMRMLLabelMapVolumeNode::SafeDownCast(volumeNode))
    {
      if (labelNode == nullptr)
      {
        labelNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(volumeNode);
      }
    }
    else
    {
      if (!backgroundNode)
      {
        backgroundNode = volumeNode;
      }
      else if (!foregroundNode)
      {
        foregroundNode = volumeNode;
      }
    }
    if (backgroundNode && foregroundNode && labelNode)
    {
      // all volume layers are filled - we can ignore the rest of the selected volumes
      break;
    }
  }

  // Show the volume in slice view
  vtkMRMLSliceLogic* sliceLogic = nullptr;
  vtkSlicerApplicationLogic* appLogic = qSlicerApplication::application()->applicationLogic();
  if (appLogic)
  {
    sliceLogic = appLogic->GetSliceLogic(sliceNode);
  }
  if (!sliceLogic)
  {
    qCritical() << Q_FUNC_INFO << " failed: cannot get slice logic";
    return false;
  }
  vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
  if (!sliceCompositeNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: cannot get slice composite node";
    return false;
  }
  if (backgroundNode || foregroundNode)
  {
    sliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::BackgroundVolumeFlag
                                                   | vtkMRMLSliceCompositeNode::ForegroundVolumeFlag
                                                   | vtkMRMLSliceCompositeNode::ForegroundOpacityFlag);
    if (volumeToShow == backgroundNode)
    {
      sliceCompositeNode->SetBackgroundVolumeID(backgroundNode->GetID());
      if (!foregroundNode)
      {
        sliceCompositeNode->SetForegroundVolumeID(nullptr);
      }
      sliceLogic->EndSliceCompositeNodeInteraction();
    }
    if (volumeToShow == foregroundNode)
    {
      sliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::BackgroundVolumeFlag
                                                     | vtkMRMLSliceCompositeNode::ForegroundVolumeFlag);
      if (!backgroundNode)
      {
        sliceCompositeNode->SetBackgroundVolumeID(nullptr);
      }
      sliceCompositeNode->SetForegroundVolumeID(foregroundNode->GetID());
      sliceCompositeNode->SetForegroundOpacity(0.5);
      sliceLogic->EndSliceCompositeNodeInteraction();
    }
  }
  if (volumeToShow == labelNode)
  {
    sliceLogic->StartSliceCompositeNodeInteraction(vtkMRMLSliceCompositeNode::LabelVolumeFlag);
    sliceCompositeNode->SetLabelVolumeID(labelNode ? labelNode->GetID() : nullptr);
    sliceLogic->EndSliceCompositeNodeInteraction();
  }

  // Align view orientation and field of view to background volume
  if (d->resetViewOrientationOnShow() || d->resetFieldOfViewOnShow())
  {
    int interactionFlags = 0;
    if (d->resetViewOrientationOnShow())
    {
      interactionFlags |= vtkMRMLSliceNode::ResetOrientationFlag | vtkMRMLSliceNode::RotateToBackgroundVolumePlaneFlag;
    }
    if (d->resetFieldOfViewOnShow())
    {
      interactionFlags |= vtkMRMLSliceNode::ResetFieldOfViewFlag;
    }
    sliceLogic->StartSliceNodeInteraction(interactionFlags);
    if (d->resetViewOrientationOnShow())
    {
      // reset orientation before snapping slice to closest volume axis
      sliceNode->SetOrientationToDefault();
      sliceLogic->RotateSliceToLowestVolumeAxes();
    }
    if (d->resetFieldOfViewOnShow())
    {
      sliceLogic->FitSliceToAll();
    }
    sliceLogic->EndSliceNodeInteraction();
  }

  shNode->ItemModified(itemID);

  return true;
}

// --------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::setVolumePreset(const QString& presetId)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);

  if (d->SelectedVolumeNode == nullptr)
  {
    qCritical() << Q_FUNC_INFO << " failed: invalid volume";
    return;
  }

  if (presetId.startsWith(QString::fromStdString(DISPLAY_NODE_PRESET_PREFIX)))
  {
    // Preset stored in display node
    int displayNodePresetIndex = presetId.right(presetId.length() - DISPLAY_NODE_PRESET_PREFIX.length()).toInt();
    vtkMRMLScalarVolumeDisplayNode* scalarVolumeDisplayNode =
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
    if (!scalarVolumeDisplayNode)
    {
      qCritical() << Q_FUNC_INFO << " failed: volume display node is invalid";
      return;
    }
    scalarVolumeDisplayNode->SetWindowLevelFromPreset(displayNodePresetIndex);
  }
  else if (presetId == QString::fromStdString(PRESET_AUTO))
  {
    vtkMRMLScalarVolumeDisplayNode* scalarVolumeDisplayNode =
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast(d->SelectedVolumeNode->GetVolumeDisplayNode());
    if (!scalarVolumeDisplayNode)
    {
      qCritical() << Q_FUNC_INFO << " failed: volume display node is invalid";
      return;
    }
    scalarVolumeDisplayNode->SetAutoWindowLevel(true);
  }
  else
  {
    // Preset stored in volumes logic
    vtkSlicerVolumesLogic* volumesModuleLogic =
      vtkSlicerVolumesLogic::SafeDownCast(qSlicerApplication::application()->moduleLogic("Volumes"));
    if (!volumesModuleLogic)
    {
      qCritical() << Q_FUNC_INFO << " failed: volumes module logic is not available";
      return;
    }
    volumesModuleLogic->ApplyVolumeDisplayPreset(d->SelectedVolumeNode->GetVolumeDisplayNode(), presetId.toStdString());
  }
}

//---------------------------------------------------------------------------
void qSlicerSubjectHierarchyVolumesPlugin::toggleVisibilityForCurrentItem(bool on)
{
  Q_D(qSlicerSubjectHierarchyVolumesPlugin);
  if (d->SelectedVolumeNode)
  {
    // Show color legend display node
    vtkMRMLDisplayNode* displayNode = d->SelectedVolumeNode->GetVolumeDisplayNode();
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
}
