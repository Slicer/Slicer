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

// Segmentations includes
#include "qMRMLSegmentEditorWidget.h"
#include "qMRMLSortFilterSegmentsProxyModel.h"
#include "ui_qMRMLSegmentEditorWidget.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentEditorNode.h"
#include "qMRMLSegmentationGeometryDialog.h"

// vtkSegmentationCore Includes
#include "vtkSegmentation.h"
#include "vtkSegmentationHistory.h"
#include "vtkSegment.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSlicerSegmentationsModuleLogic.h"
#include "vtkBinaryLabelmapToClosedSurfaceConversionRule.h"

// Segment editor effects includes
#include "qSlicerSegmentEditorAbstractEffect.h"
#include "qSlicerSegmentEditorAbstractLabelEffect.h"
#include "qSlicerSegmentEditorEffectFactory.h"

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkAlgorithmOutput.h>
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkDataArray.h>
#include <vtkGeneralTransform.h>
#include <vtkImageThreshold.h>
#include <vtkImageExtractComponents.h>
#include <vtkInteractorObserver.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

// Slicer includes
#include <vtkMRMLSliceLogic.h>
#include <vtkSlicerApplicationLogic.h>

// MRML includes
#include <vtkMRMLCameraDisplayableManager.h>
#include <vtkMRMLCameraWidget.h>
#include <vtkMRMLCrosshairDisplayableManager.h>
#include <vtkMRMLLabelMapVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceIntersectionWidget.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLViewNode.h>
#include <vtkMRMLInteractionNode.h>
#include <vtkMRMLSliceCompositeNode.h>

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>
#include "qSlicerModuleManager.h"
#include <qSlicerAbstractModule.h>
#include <qSlicerAbstractModuleWidget.h>
#include <qMRMLSegmentationFileExportWidget.h>
#include <qMRMLSliceWidget.h>
#include <qMRMLSliceView.h>
#include <qMRMLThreeDWidget.h>
#include <qMRMLThreeDView.h>

// Qt includes
#include <QAbstractItemView>
#include <QAction>
#include <QButtonGroup>
#include <QDebug>
#include <QGridLayout>
#include <QInputDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QPointer>
#include <QScrollArea>
#include <QSettings>
#include <QShortcut>
#include <QTableView>
#include <QToolButton>
#include <QVBoxLayout>

// CTK includes
#include <ctkCollapsibleButton.h>

static const int BINARY_LABELMAP_SCALAR_TYPE = VTK_UNSIGNED_CHAR;
// static const unsigned char BINARY_LABELMAP_VOXEL_FULL = 1; // unused
static const unsigned char BINARY_LABELMAP_VOXEL_EMPTY = 0;

static const char NULL_EFFECT_NAME[] = "NULL";

//---------------------------------------------------------------------------
class vtkSegmentEditorEventCallbackCommand : public vtkCallbackCommand
{
public:
  static vtkSegmentEditorEventCallbackCommand *New()
    {
    return new vtkSegmentEditorEventCallbackCommand;
    }
  /// Segment editor widget observing the event
  QPointer<qMRMLSegmentEditorWidget> EditorWidget;
  /// Slice widget or 3D widget
  QPointer<qMRMLWidget> ViewWidget;
};

//-----------------------------------------------------------------------------
struct SegmentEditorEventObservation
{
  vtkSmartPointer<vtkSegmentEditorEventCallbackCommand> CallbackCommand;
  vtkWeakPointer<vtkObject> ObservedObject;
  QVector<int> ObservationTags;
};

//-----------------------------------------------------------------------------
// qMRMLSegmentEditorWidgetPrivate methods

//-----------------------------------------------------------------------------
class qMRMLSegmentEditorWidgetPrivate: public Ui_qMRMLSegmentEditorWidget
{
  Q_DECLARE_PUBLIC(qMRMLSegmentEditorWidget);

protected:
  qMRMLSegmentEditorWidget* const q_ptr;
public:
  qMRMLSegmentEditorWidgetPrivate(qMRMLSegmentEditorWidget& object);
  ~qMRMLSegmentEditorWidgetPrivate();
  void init();

  /// Simple mechanism to let the effects know that default modifier labelmap has changed
  void notifyEffectsOfReferenceGeometryChange(const std::string& geometry);
  /// Simple mechanism to let the effects know that source volume has changed
  void notifyEffectsOfSourceVolumeNodeChange();
  /// Simple mechanism to let the effects know that layout has changed
  void notifyEffectsOfLayoutChange();

  /// Select first segment in table view
  void selectFirstSegment();

  /// Enable or disable effects and their options based on input selection
  void updateEffectsEnabledFromMRML();

  /// Set cursor for effect. If effect is nullptr then the cursor is reset to default.
  void setEffectCursor(qSlicerSegmentEditorAbstractEffect* effect);

  /// Updates default modifier labelmap based on reference geometry (to set origin, spacing, and directions)
  /// and existing segments (to set extents). If reference geometry conversion parameter is empty
  /// then existing segments are used for determining origin, spacing, and directions and the resulting
  /// geometry is written to reference geometry conversion parameter.
  bool resetModifierLabelmapToDefault();

  /// Updates selected segment labelmap in a geometry aligned with default modifierLabelmap.
  bool updateSelectedSegmentLabelmap();

  /// Updates a resampled source volume in a geometry aligned with default modifierLabelmap.
  bool updateAlignedSourceVolume();

  /// Updates mask labelmap.
  /// Geometry of mask will be the same as current modifierLabelmap.
  /// This mask only considers segment-based regions (and ignores masking based on
  /// source volume intensity).
  bool updateMaskLabelmap();

  bool updateReferenceGeometryImage();

  static std::string getReferenceImageGeometryFromSegmentation(vtkSegmentation* segmentation);
  std::string referenceImageGeometry();

  bool segmentationDisplayableInView(vtkMRMLAbstractViewNode* viewNode);

  QToolButton* toolButton(qSlicerSegmentEditorAbstractEffect* effect);

  /// Return segmentation node's internal labelmap IJK to renderer world coordinate transform.
  /// If cannot be retrieved (segmentation is not defined, non-linearly transformed, etc.)
  /// then false is returned;
  bool segmentationIJKToRAS(vtkMatrix4x4* ijkToRas);

public:
  /// Segment editor parameter set node containing all selections and working images
  vtkWeakPointer<vtkMRMLSegmentEditorNode> ParameterSetNode;

  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;
  vtkSmartPointer<vtkSegmentationHistory> SegmentationHistory;

  vtkWeakPointer<vtkMRMLScalarVolumeNode> SourceVolumeNode;

  // Observe InteractionNode to detect when mouse mode is changed
  vtkWeakPointer<vtkMRMLInteractionNode> InteractionNode;

  /// Lock widget to make segmentation read-only.
  // In the future locked state may be read from the Segmentation node.
  bool Locked;

  /// Ordering of effects
  QStringList EffectNameOrder;
  bool UnorderedEffectsVisible;
  int EffectColumnCount;

  /// List of registered effect instances
  QList<qSlicerSegmentEditorAbstractEffect*> RegisteredEffects;

  /// Active effect
  qSlicerSegmentEditorAbstractEffect* ActiveEffect;
  /// Last active effect
  /// Stored to allow quick toggling between no effect/last active effect.
  qSlicerSegmentEditorAbstractEffect* LastActiveEffect;

  /// Structure containing necessary objects for each slice and 3D view handling interactions
  QVector<SegmentEditorEventObservation> EventObservations;

  /// List of view node IDs where custom cursor is set
  QSet<QString> CustomCursorInViewNodeIDs;

  /// Indicates if views and layouts are observed
  /// (essentially, the widget is active).
  bool ViewsObserved;

  /// List of view node IDs in display nodes, which were specified when views observation was set up.
  /// If node IDs change (segmentation node is shown/hidden in a specific view) then view observations has to be refreshed.
  QMap<QString, std::vector<std::string> > ObservedViewNodeIDs; // <SegmentationDisplayNodeID, ViewNodeIDs>

  bool AutoShowSourceVolumeNode;

  /// Button group for the effects
  QButtonGroup EffectButtonGroup;

  /// Button group for the UndoRedoGroupBox
  QButtonGroup UndoRedoButtonGroup;

  /// These volumes are owned by this widget and a pointer is given to each effect
  /// so that they can access and modify it
  vtkOrientedImageData* AlignedSourceVolume;
  /// Modifier labelmap that is kept in memory to avoid memory reallocations on each editing operation.
  /// When update of this labelmap is requested its geometry is reset and its content is cleared.
  vtkOrientedImageData* ModifierLabelmap;
  vtkOrientedImageData* SelectedSegmentLabelmap;
  vtkOrientedImageData* MaskLabelmap;
  /// Image that contains reference geometry. Scalars are not allocated.
  vtkOrientedImageData* ReferenceGeometryImage;

  /// Input data that is used for computing AlignedSourceVolume.
  /// It is stored so that it can be determined that the source volume has to be updated
  vtkMRMLVolumeNode* AlignedSourceVolumeUpdateSourceVolumeNode;
  vtkMRMLTransformNode* AlignedSourceVolumeUpdateSourceVolumeNodeTransform;
  vtkMRMLTransformNode* AlignedSourceVolumeUpdateSegmentationNodeTransform;

  int MaskModeComboBoxFixedItemsCount;

  /// If reference geometry changes compared to this value then we notify effects and
  /// set this value to the current value. This allows notifying effects when there is a change.
  std::string LastNotifiedReferenceImageGeometry;

  QList< QShortcut* > KeyboardShortcuts;

  Qt::ToolButtonStyle EffectButtonStyle;

  // When segmentation node selector is visible then rotate warning is displayed in that row.
  // However, if the node selector is hidden then the rotation warning button would take
  // and entire row.
  // To prevent this, rotation warning button is displayed in the add/remove/etc. segment
  // button row when segmentation node selector is hidden.
  // Qt does not have an API to check if a widget is in a layout, therefore we store this
  // information in this flag.
  bool RotateWarningInNodeSelectorLayout;

  QString DefaultTerminologyEntrySettingsKey;
  QString DefaultTerminologyEntry;
};

//-----------------------------------------------------------------------------
qMRMLSegmentEditorWidgetPrivate::qMRMLSegmentEditorWidgetPrivate(qMRMLSegmentEditorWidget& object)
  : q_ptr(&object)
  , Locked(false)
  , ActiveEffect(nullptr)
  , LastActiveEffect(nullptr)
  , ViewsObserved(false)
  , AutoShowSourceVolumeNode(true)
  , AlignedSourceVolume(nullptr)
  , ModifierLabelmap(nullptr)
  , SelectedSegmentLabelmap(nullptr)
  , MaskLabelmap(nullptr)
  , ReferenceGeometryImage(nullptr)
  , AlignedSourceVolumeUpdateSourceVolumeNode(nullptr)
  , AlignedSourceVolumeUpdateSourceVolumeNodeTransform(nullptr)
  , AlignedSourceVolumeUpdateSegmentationNodeTransform(nullptr)
  , MaskModeComboBoxFixedItemsCount(0)
  , EffectButtonStyle(Qt::ToolButtonIconOnly)
  , RotateWarningInNodeSelectorLayout(true)
{
  this->AlignedSourceVolume = vtkOrientedImageData::New();
  this->ModifierLabelmap = vtkOrientedImageData::New();
  this->MaskLabelmap = vtkOrientedImageData::New();
  this->SelectedSegmentLabelmap = vtkOrientedImageData::New();
  this->ReferenceGeometryImage = vtkOrientedImageData::New();
  this->SegmentationHistory = vtkSmartPointer<vtkSegmentationHistory>::New();

  // Define default effect order
  this->EffectNameOrder
    // Thresholding is the the starting point for most segmentations
    // (it can often create usable segmentation by itself, or used to define intensity range for painting)
    << "Threshold"
    // Local painting
    << "Paint" << "Draw" << "Erase" << "Level tracing" << "Grow from seeds" << "Fill between slices"
    // Global processing
    << "Margin" << "Hollow" << "Smoothing"
    // Global splitting, merging
    << "Scissors" << "Islands" << "Logical operators"
    // Operating on volumes
    << "Mask volume";
  this->UnorderedEffectsVisible = true;
  this->DefaultTerminologyEntrySettingsKey = "Segmentations/DefaultTerminologyEntry";
  this->EffectColumnCount = 2;
}

//-----------------------------------------------------------------------------
qMRMLSegmentEditorWidgetPrivate::~qMRMLSegmentEditorWidgetPrivate()
{
  Q_Q(qMRMLSegmentEditorWidget);
  q->removeViewObservations();

  foreach(qSlicerSegmentEditorAbstractEffect* effect, this->RegisteredEffects)
    {
    delete effect;
    }
  this->RegisteredEffects.clear();
  if (this->AlignedSourceVolume)
    {
    this->AlignedSourceVolume->Delete();
    this->AlignedSourceVolume = nullptr;
    }
  if (this->ModifierLabelmap)
    {
    this->ModifierLabelmap->Delete();
    this->ModifierLabelmap = nullptr;
    }
  if (this->MaskLabelmap)
    {
    this->MaskLabelmap->Delete();
    this->MaskLabelmap = nullptr;
    }
  if (this->SelectedSegmentLabelmap)
    {
    this->SelectedSegmentLabelmap->Delete();
    this->SelectedSegmentLabelmap = nullptr;
    }
  if (this->ReferenceGeometryImage)
    {
    this->ReferenceGeometryImage->Delete();
    this->ReferenceGeometryImage = nullptr;
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidgetPrivate::init()
{
  Q_Q(qMRMLSegmentEditorWidget);
  this->setupUi(q);

  // Set combobox size adjust policy to prevent long node leading to large minimum width.
  // (AdjustToContentsOnFirstShow is used by default, which may make the minimum width quite large.)
  this->SegmentationNodeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  this->SourceVolumeNodeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

  this->SliceRotateWarningButton->setMaximumHeight(this->SegmentationNodeComboBox->sizeHint().height());
  this->SliceRotateWarningButton->setMaximumWidth(this->SegmentationNodeComboBox->sizeHint().height());
  this->SpecifyGeometryButton->setMaximumHeight(this->SourceVolumeNodeComboBox->sizeHint().height());
  this->SpecifyGeometryButton->setMaximumWidth(this->SourceVolumeNodeComboBox->sizeHint().height());

  this->MaskModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Everywhere"), vtkMRMLSegmentationNode::EditAllowedEverywhere);
  this->MaskModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Inside all segments"), vtkMRMLSegmentationNode::EditAllowedInsideAllSegments);
  this->MaskModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Inside all visible segments"), vtkMRMLSegmentationNode::EditAllowedInsideVisibleSegments);
  this->MaskModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Outside all segments"), vtkMRMLSegmentationNode::EditAllowedOutsideAllSegments);
  this->MaskModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Outside all visible segments"), vtkMRMLSegmentationNode::EditAllowedOutsideVisibleSegments);
  this->MaskModeComboBox->insertSeparator(this->MaskModeComboBox->count());
  this->MaskModeComboBoxFixedItemsCount = this->MaskModeComboBox->count();

  this->OverwriteModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Overwrite all"), vtkMRMLSegmentEditorNode::OverwriteAllSegments);
  this->OverwriteModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Overwrite visible"), vtkMRMLSegmentEditorNode::OverwriteVisibleSegments);
  this->OverwriteModeComboBox->addItem(qMRMLSegmentEditorWidget::tr("Allow overlap"), vtkMRMLSegmentEditorNode::OverwriteNone);

  this->SwitchToSegmentationsButton->setIcon(q->style()->standardIcon(QStyle::SP_ArrowRight));

  QMenu* segmentationsButtonMenu = new QMenu(qMRMLSegmentEditorWidget::tr("Segmentations"), this->SwitchToSegmentationsButton);

  QAction* importExportAction = new QAction("Import/export nodes...", segmentationsButtonMenu);
  segmentationsButtonMenu->addAction(importExportAction);
  QObject::connect(importExportAction, SIGNAL(triggered()), q, SLOT(onImportExportActionClicked()));
  QAction* exportToFileAction = new QAction("Export to files...", segmentationsButtonMenu);
  segmentationsButtonMenu->addAction(exportToFileAction);
  QObject::connect(exportToFileAction, SIGNAL(triggered()), q, SLOT(onExportToFilesActionClicked()));

  this->SwitchToSegmentationsButton->setMenu(segmentationsButtonMenu);

  // Make connections
  QObject::connect( this->SegmentationNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(onSegmentationNodeChanged(vtkMRMLNode*)) );
  QObject::connect(this->SliceRotateWarningButton, SIGNAL(clicked()),
    q, SLOT(rotateSliceViewsToSegmentation()));
  QObject::connect( this->SourceVolumeNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(onSourceVolumeNodeChanged(vtkMRMLNode*)) );
  QObject::connect( this->SpecifyGeometryButton, SIGNAL(clicked()),
    q, SLOT(showSegmentationGeometryDialog()));
  QObject::connect( this->SegmentsTableView, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
    q, SLOT(onSegmentSelectionChanged(QItemSelection,QItemSelection)) );
  QObject::connect( this->SegmentsTableView, SIGNAL(segmentAboutToBeModified(QString)),
    q, SLOT(saveStateForUndo()) );
  QObject::connect( this->AddSegmentButton, SIGNAL(clicked()), q, SLOT(onAddSegment()) );
  QObject::connect( this->RemoveSegmentButton, SIGNAL(clicked()), q, SLOT(onRemoveSegment()) );
  QObject::connect( this->SwitchToSegmentationsButton, SIGNAL(clicked()), q, SLOT(onSwitchToSegmentations()) );

  QObject::connect( this->MaskModeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(onMaskModeChanged(int)));
  QObject::connect( this->SourceVolumeIntensityMaskCheckBox, SIGNAL(toggled(bool)), q, SLOT(onSourceVolumeIntensityMaskChecked(bool)));
  QObject::connect( this->SourceVolumeIntensityMaskRangeWidget, SIGNAL(valuesChanged(double,double)),
    q, SLOT(onSourceVolumeIntensityMaskRangeChanged(double,double)));
  QObject::connect( this->OverwriteModeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(onOverwriteModeChanged(int)));

  QObject::connect( this->UndoButton, SIGNAL(clicked()), q, SLOT(undo()) );
  QObject::connect( this->RedoButton, SIGNAL(clicked()), q, SLOT(redo()) );

  q->qvtkConnect(this->SegmentationHistory, vtkCommand::ModifiedEvent,
    q, SLOT(onSegmentationHistoryChanged()));

  // Widget properties
  this->SegmentsTableView->setSelectionMode(QAbstractItemView::SingleSelection);
  this->SegmentsTableView->setHeaderVisible(true);
  this->SegmentsTableView->setVisibilityColumnVisible(true);
  this->SegmentsTableView->setColorColumnVisible(true);
  this->SegmentsTableView->setOpacityColumnVisible(false);
  this->AddSegmentButton->setEnabled(false);
  this->RemoveSegmentButton->setEnabled(false);
  this->SwitchToSegmentationsButton->setEnabled(false);
  this->EffectsGroupBox->setEnabled(false);
  this->OptionsGroupBox->setEnabled(false);

  this->EffectsGroupBox->setLayout(new QGridLayout(this->EffectsGroupBox));

  this->EffectButtonGroup.setExclusive(true);
  QObject::connect(&this->EffectButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), q, SLOT(onEffectButtonClicked(QAbstractButton*) ) );

  this->UndoRedoButtonGroup.addButton(this->UndoButton);
  this->UndoRedoButtonGroup.addButton(this->RedoButton);

  // Create layout for effect options
  QVBoxLayout* layout = new QVBoxLayout(this->EffectsOptionsFrame);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  // Update effect buttons
  q->updateEffectList();

  this->OptionsGroupBox->hide();
  this->OptionsGroupBox->setTitle("");
  this->EffectHelpBrowser->setText("");
  this->MaskingGroupBox->hide();

  q->updateSliceRotateWarningButtonVisibility();
}

//-----------------------------------------------------------------------------
QToolButton* qMRMLSegmentEditorWidgetPrivate::toolButton(qSlicerSegmentEditorAbstractEffect* effect)
{
  QList<QAbstractButton*> effectButtons = this->EffectButtonGroup.buttons();
  foreach(QAbstractButton* effectButton, effectButtons)
    {
    qSlicerSegmentEditorAbstractEffect* foundEffect = qobject_cast<qSlicerSegmentEditorAbstractEffect*>(
      effectButton->property("Effect").value<QObject*>());
    if (effect == foundEffect)
      {
      return dynamic_cast<QToolButton*>(effectButton);
      }
    }
  // not found
  return nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidgetPrivate::notifyEffectsOfReferenceGeometryChange(const std::string& geometry)
{
  if (geometry.compare(this->LastNotifiedReferenceImageGeometry) == 0)
    {
    // no change
    return;
    }
  this->LastNotifiedReferenceImageGeometry = geometry;

  foreach(qSlicerSegmentEditorAbstractEffect* effect, this->RegisteredEffects)
    {
    effect->referenceGeometryChanged();
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidgetPrivate::notifyEffectsOfSourceVolumeNodeChange()
{
  foreach(qSlicerSegmentEditorAbstractEffect* effect, this->RegisteredEffects)
    {
    effect->sourceVolumeNodeChanged();
    effect->masterVolumeNodeChanged(); // for backward compatibility
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidgetPrivate::notifyEffectsOfLayoutChange()
{
  foreach(qSlicerSegmentEditorAbstractEffect* effect, this->RegisteredEffects)
    {
    effect->layoutChanged();
    }
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidgetPrivate::resetModifierLabelmapToDefault()
{
  std::string referenceImageGeometry = this->referenceImageGeometry();
  if (referenceImageGeometry.empty())
    {
    qCritical() << Q_FUNC_INFO << ": Cannot determine default modifierLabelmap geometry";
    return false;
    }

  std::string modifierLabelmapReferenceImageGeometryBaseline = vtkSegmentationConverter::SerializeImageGeometry(this->ModifierLabelmap);

  // Set reference geometry to labelmap (origin, spacing, directions, extents) and allocate scalars
  vtkNew<vtkMatrix4x4> referenceGeometryMatrix;
  int referenceExtent[6] = {0,-1,0,-1,0,-1};
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceGeometryMatrix.GetPointer(), referenceExtent);
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, this->ModifierLabelmap, true, BINARY_LABELMAP_SCALAR_TYPE, 1);

  vtkOrientedImageDataResample::FillImage(this->ModifierLabelmap, BINARY_LABELMAP_VOXEL_EMPTY);

  return true;
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidgetPrivate::updateSelectedSegmentLabelmap()
{
  if (!this->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return false;
    }

  vtkMRMLSegmentationNode* segmentationNode = this->ParameterSetNode->GetSegmentationNode();
  std::string referenceImageGeometry = this->referenceImageGeometry();
  if (!segmentationNode || referenceImageGeometry.empty())
    {
    return false;
    }
  const char* selectedSegmentID = this->ParameterSetNode->GetSelectedSegmentID();
  if (!selectedSegmentID)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment selection";
    return false;
    }

  vtkSegment* selectedSegment = segmentationNode->GetSegmentation()->GetSegment(selectedSegmentID);
  if (selectedSegment == nullptr)
    {
    qWarning() << Q_FUNC_INFO << " failed: Segment " << selectedSegmentID << " not found in segmentation";
    return false;
    }
  vtkOrientedImageData* segmentLabelmap = vtkOrientedImageData::SafeDownCast(
    selectedSegment->GetRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()));
  if (!segmentLabelmap)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to get binary labelmap representation in segmentation " << segmentationNode->GetName();
    return false;
    }
  int* extent = segmentLabelmap->GetExtent();
  if (extent[0] > extent[1] || extent[2] > extent[3] || extent[4] > extent[5])
    {
    vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, this->SelectedSegmentLabelmap, false);
    this->SelectedSegmentLabelmap->SetExtent(0, -1, 0, -1, 0, -1);
    this->SelectedSegmentLabelmap->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    return true;
    }

  vtkNew<vtkImageThreshold> threshold;
  threshold->SetInputData(segmentLabelmap);
  threshold->ThresholdBetween(selectedSegment->GetLabelValue(), selectedSegment->GetLabelValue());
  threshold->SetInValue(1);
  threshold->SetOutValue(0);
  threshold->Update();

  vtkSmartPointer<vtkOrientedImageData> thresholdedSegmentLabelmap = vtkSmartPointer<vtkOrientedImageData>::New();
  thresholdedSegmentLabelmap->ShallowCopy(threshold->GetOutput());
  thresholdedSegmentLabelmap->CopyDirections(segmentLabelmap);

  vtkNew<vtkOrientedImageData> referenceImage;
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceImage.GetPointer(), false);
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(
    thresholdedSegmentLabelmap, referenceImage.GetPointer(), this->SelectedSegmentLabelmap, /*linearInterpolation=*/false);

  return true;
}


//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidgetPrivate::updateAlignedSourceVolume()
{
  if (!this->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return false;
    }

  vtkMRMLSegmentationNode* segmentationNode = this->ParameterSetNode->GetSegmentationNode();
  vtkMRMLScalarVolumeNode* sourceVolumeNode = this->ParameterSetNode->GetSourceVolumeNode();
  std::string referenceImageGeometry = this->referenceImageGeometry();
  if (!segmentationNode || !sourceVolumeNode || !sourceVolumeNode->GetImageData()
    || !sourceVolumeNode->GetImageData()->GetPointData() || referenceImageGeometry.empty())
    {
    return false;
    }

  vtkNew<vtkOrientedImageData> referenceImage;
  vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceImage.GetPointer(), false);

  int* referenceImageExtent = referenceImage->GetExtent();
  int* alignedSourceVolumeExtent = this->AlignedSourceVolume->GetExtent();
  // If source volume node and transform nodes did not change and the aligned source volume covers the entire reference geometry
  // then we don't need to update the aligned source volume.
  if (vtkOrientedImageDataResample::DoGeometriesMatch(referenceImage.GetPointer(), this->AlignedSourceVolume)
    && alignedSourceVolumeExtent[0] <= referenceImageExtent[0] && alignedSourceVolumeExtent[1] >= referenceImageExtent[1]
    && alignedSourceVolumeExtent[2] <= referenceImageExtent[2] && alignedSourceVolumeExtent[3] >= referenceImageExtent[3]
    && alignedSourceVolumeExtent[4] <= referenceImageExtent[4] && alignedSourceVolumeExtent[5] >= referenceImageExtent[5]
    && vtkOrientedImageDataResample::DoExtentsMatch(referenceImage.GetPointer(), this->AlignedSourceVolume)
    && this->AlignedSourceVolumeUpdateSourceVolumeNode == sourceVolumeNode
    && this->AlignedSourceVolumeUpdateSourceVolumeNodeTransform == sourceVolumeNode->GetParentTransformNode()
    && this->AlignedSourceVolumeUpdateSegmentationNodeTransform == segmentationNode->GetParentTransformNode() )
    {
    // Extents and nodes are matching, check if they have not been modified since the aligned master
    // volume generation.
    bool updateAlignedSourceVolumeRequired = false;
    if (sourceVolumeNode->GetMTime() > this->AlignedSourceVolume->GetMTime())
      {
      updateAlignedSourceVolumeRequired = true;
      }
    else if (sourceVolumeNode->GetParentTransformNode() && sourceVolumeNode->GetParentTransformNode()->GetMTime() > this->AlignedSourceVolume->GetMTime())
      {
      updateAlignedSourceVolumeRequired = true;
      }
    else if (segmentationNode->GetParentTransformNode() && segmentationNode->GetParentTransformNode()->GetMTime() > this->AlignedSourceVolume->GetMTime())
      {
      updateAlignedSourceVolumeRequired = true;
      }
    if (!updateAlignedSourceVolumeRequired)
      {
      return true;
      }
    }

  // Get a read-only version of sourceVolume as a vtkOrientedImageData
  vtkNew<vtkOrientedImageData> sourceVolume;

  if (sourceVolumeNode->GetImageData()->GetNumberOfScalarComponents() == 1)
    {
    sourceVolume->vtkImageData::ShallowCopy(sourceVolumeNode->GetImageData());
    }
  else
    {
    vtkNew<vtkImageExtractComponents> extract;
    extract->SetInputData(sourceVolumeNode->GetImageData());
    extract->Update();
    extract->SetComponents(0); // TODO: allow user to configure this
    sourceVolume->vtkImageData::ShallowCopy(extract->GetOutput());
    }
  vtkSmartPointer<vtkMatrix4x4> ijkToRasMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  sourceVolumeNode->GetIJKToRASMatrix(ijkToRasMatrix);
  sourceVolume->SetGeometryFromImageToWorldMatrix(ijkToRasMatrix);

  vtkNew<vtkGeneralTransform> sourceVolumeToSegmentationTransform;
  vtkMRMLTransformNode::GetTransformBetweenNodes(sourceVolumeNode->GetParentTransformNode(),
    segmentationNode->GetParentTransformNode(), sourceVolumeToSegmentationTransform.GetPointer());

  double backgroundValue = sourceVolumeNode->GetImageBackgroundScalarComponentAsDouble(0);
  vtkOrientedImageDataResample::ResampleOrientedImageToReferenceOrientedImage(sourceVolume, referenceImage, this->AlignedSourceVolume,
    /*linearInterpolation=*/true, /*padImage=*/false, sourceVolumeToSegmentationTransform, backgroundValue);

  this->AlignedSourceVolumeUpdateSourceVolumeNode = sourceVolumeNode;
  this->AlignedSourceVolumeUpdateSourceVolumeNodeTransform = sourceVolumeNode->GetParentTransformNode();
  this->AlignedSourceVolumeUpdateSegmentationNodeTransform = segmentationNode->GetParentTransformNode();

  return true;
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidgetPrivate::updateMaskLabelmap()
{
  if (!this->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return false;
    }
  vtkMRMLSegmentationNode* segmentationNode = this->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation node";
    return false;
    }

  std::string referenceGeometryStr = this->referenceImageGeometry();
  if (referenceGeometryStr.empty())
    {
    qCritical() << Q_FUNC_INFO << ": Cannot determine mask labelmap geometry";
    return false;
    }
  vtkNew<vtkOrientedImageData> referenceGeometry;
  if (!vtkSegmentationConverter::DeserializeImageGeometry(referenceGeometryStr, referenceGeometry, false))
    {
    qCritical() << Q_FUNC_INFO << ": Cannot determine mask labelmap geometry";
    return false;
    }

  // GenerateEditMask can add intensity range based mask, too. We do not use it here, as currently
  // editable intensity range is taken into account in qSlicerSegmentEditorAbstractEffect::modifySelectedSegmentByLabelmap.
  // It would simplify implementation if we passed source volume and intensity range to GenerateEditMask here
  // and removed intensity range based masking from modifySelectedSegmentByLabelmap.
  if (!segmentationNode->GenerateEditMask(this->MaskLabelmap, this->ParameterSetNode->GetMaskMode(), referenceGeometry,
    this->ParameterSetNode->GetSelectedSegmentID() ? this->ParameterSetNode->GetSelectedSegmentID() : "",
    this->ParameterSetNode->GetMaskSegmentID() ? this->ParameterSetNode->GetMaskSegmentID() : ""))
    {
    qCritical() << Q_FUNC_INFO << ": Mask generation failed";
    return false;
    }
  return true;
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidgetPrivate::updateReferenceGeometryImage()
{
  std::string geometry = this->referenceImageGeometry();
  if (geometry.empty())
    {
    return false;
    }
  return vtkSegmentationConverter::DeserializeImageGeometry(geometry, this->ReferenceGeometryImage, false /* don't allocate scalars */);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidgetPrivate::selectFirstSegment()
{
  if (!this->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  vtkMRMLSegmentationNode* segmentationNode = this->ParameterSetNode->GetSegmentationNode();
  if ( segmentationNode
    && segmentationNode->GetSegmentation()->GetNumberOfSegments() > 0 )
    {
    std::vector<std::string> segmentIDs;
    segmentationNode->GetSegmentation()->GetSegmentIDs(segmentIDs);

    QStringList firstSegmentID;
    firstSegmentID << QString(segmentIDs[0].c_str());
    this->SegmentsTableView->setSelectedSegmentIDs(firstSegmentID);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidgetPrivate::setEffectCursor(qSlicerSegmentEditorAbstractEffect* effect)
{
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return;
    }

  // We update the default cursor as well so that if the user hovers the mouse over
  // a markup, the cursor shape is then restored to the effect cursor.

  foreach(QString sliceViewName, layoutManager->sliceViewNames())
    {
    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    QString viewNodeID = QString::fromStdString(sliceWidget->mrmlSliceNode()->GetID());
    if (!this->segmentationDisplayableInView(sliceWidget->mrmlSliceNode()))
      {
      // segmentation is not displayable in this view anymore
      if (!this->CustomCursorInViewNodeIDs.contains(viewNodeID))
        {
        // we did not use this view previously either, so don't change the cursor there
        continue;
        }
      }
    if (effect && effect->showEffectCursorInSliceView())
      {
      sliceWidget->sliceView()->setViewCursor(effect->createCursor(sliceWidget));
      sliceWidget->sliceView()->setDefaultViewCursor(effect->createCursor(sliceWidget));
      this->CustomCursorInViewNodeIDs.insert(viewNodeID);
      }
    else
      {
      sliceWidget->sliceView()->setViewCursor(QCursor());
      sliceWidget->sliceView()->setDefaultViewCursor(QCursor());
      this->CustomCursorInViewNodeIDs.remove(viewNodeID);
      }
    }
  for (int threeDViewId = 0; threeDViewId < layoutManager->threeDViewCount(); ++threeDViewId)
    {
    qMRMLThreeDWidget* threeDWidget = layoutManager->threeDWidget(threeDViewId);
    QString viewNodeID = QString::fromStdString(threeDWidget->mrmlViewNode()->GetID());
    if (!this->segmentationDisplayableInView(threeDWidget->mrmlViewNode()))
      {
      // segmentation is not displayable in this view anymore
      if (!this->CustomCursorInViewNodeIDs.contains(viewNodeID))
        {
        // we did not use this view previously either, so don't change the cursor there
        continue;
        }
      }
    if (effect && effect->showEffectCursorInThreeDView())
      {
      threeDWidget->threeDView()->setViewCursor(effect->createCursor(threeDWidget));
      threeDWidget->threeDView()->setDefaultViewCursor(effect->createCursor(threeDWidget));
      this->CustomCursorInViewNodeIDs.insert(viewNodeID);
      }
    else
      {
      threeDWidget->threeDView()->setViewCursor(QCursor());
      threeDWidget->threeDView()->setDefaultViewCursor(QCursor());
      this->CustomCursorInViewNodeIDs.remove(viewNodeID);
      }
    }
}

//-----------------------------------------------------------------------------
std::string qMRMLSegmentEditorWidgetPrivate::getReferenceImageGeometryFromSegmentation(vtkSegmentation* segmentation)
{
  if (!segmentation)
    {
    return "";
    }

  // If "reference image geometry" conversion parameter is set then use that
  std::string referenceImageGeometry = segmentation->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
  if (!referenceImageGeometry.empty())
    {
    // Extend reference image geometry to contain all segments (needed for example for properly handling imported segments
    // that do not fit into the reference image geometry)
    vtkSmartPointer<vtkOrientedImageData> commonGeometryImage = vtkSmartPointer<vtkOrientedImageData>::New();
    vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, commonGeometryImage, false);
    // Determine extent that contains all segments
    int commonSegmentExtent[6] = { 0, -1, 0, -1, 0, -1 };
    segmentation->DetermineCommonLabelmapExtent(commonSegmentExtent, commonGeometryImage);
    if (commonSegmentExtent[0] <= commonSegmentExtent[1]
      && commonSegmentExtent[2] <= commonSegmentExtent[3]
      && commonSegmentExtent[4] <= commonSegmentExtent[5])
      {
      // Expand commonGeometryExtent as needed to contain commonSegmentExtent
      int commonGeometryExtent[6] = { 0, -1, 0, -1, 0, -1 };
      commonGeometryImage->GetExtent(commonGeometryExtent);
      for (int i = 0; i < 3; i++)
        {
        commonGeometryExtent[i * 2] = std::min(commonSegmentExtent[i * 2], commonGeometryExtent[i * 2]);
        commonGeometryExtent[i * 2 + 1] = std::max(commonSegmentExtent[i * 2 + 1], commonGeometryExtent[i * 2 + 1]);
        }
      commonGeometryImage->SetExtent(commonGeometryExtent);
      referenceImageGeometry = vtkSegmentationConverter::SerializeImageGeometry(commonGeometryImage);
      }

    // TODO: Use oversampling (if it's 'A' then ignore and changed to 1)
    return referenceImageGeometry;
    }
  if (segmentation->ContainsRepresentation(vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    // If no reference image geometry is specified but there are labels already then determine geometry from that
    referenceImageGeometry = segmentation->DetermineCommonLabelmapGeometry();
    return referenceImageGeometry;
    }
  return "";
}

//-----------------------------------------------------------------------------
std::string qMRMLSegmentEditorWidgetPrivate::referenceImageGeometry()
{
  if (!this->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    notifyEffectsOfReferenceGeometryChange("");
    return "";
    }

  vtkMRMLSegmentationNode* segmentationNode = this->ParameterSetNode->GetSegmentationNode();
  vtkSegmentation* segmentation = segmentationNode ? segmentationNode->GetSegmentation() : nullptr;
  if (!segmentationNode || !segmentation)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation";
    notifyEffectsOfReferenceGeometryChange("");
    return "";
    }

  std::string referenceImageGeometry;
  referenceImageGeometry = this->getReferenceImageGeometryFromSegmentation(segmentation);
  if (referenceImageGeometry.empty())
    {
    // If no reference image geometry could be determined then use the source volume's geometry
    vtkMRMLScalarVolumeNode* sourceVolumeNode = this->ParameterSetNode->GetSourceVolumeNode();
    if (!sourceVolumeNode)
      {
      // cannot determine reference geometry
      return "";
      }
    // Update the referenceImageGeometry parameter for next time
    segmentationNode->SetReferenceImageGeometryParameterFromVolumeNode(sourceVolumeNode);
    // Update extents to include all existing segments
    referenceImageGeometry = this->getReferenceImageGeometryFromSegmentation(segmentation);
    }
  notifyEffectsOfReferenceGeometryChange(referenceImageGeometry);
  return referenceImageGeometry;
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidgetPrivate::segmentationDisplayableInView(vtkMRMLAbstractViewNode* viewNode)
{
  if (!viewNode)
    {
    qWarning() << Q_FUNC_INFO << ": failed. Invalid viewNode.";
    return false;
    }
  if (!this->ParameterSetNode)
    {
    return false;
    }
  vtkMRMLSegmentationNode* segmentationNode = this->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return false;
    }
  const char* viewNodeID = viewNode->GetID();
  int numberOfDisplayNodes = segmentationNode->GetNumberOfDisplayNodes();
  for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
    {
    vtkMRMLDisplayNode* segmentationDisplayNode = segmentationNode->GetNthDisplayNode(displayNodeIndex);
    if (segmentationDisplayNode && segmentationDisplayNode->IsDisplayableInView(viewNodeID))
      {
      return true;
      }
    }
  return false;
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidgetPrivate::segmentationIJKToRAS(vtkMatrix4x4* ijkToRas)
{
  if (!this->ParameterSetNode)
    {
    return false;
    }
  if (!this->updateReferenceGeometryImage())
    {
    return false;
    }

  vtkMRMLSegmentationNode* segmentationNode = this->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode || !segmentationNode->GetSegmentation())
    {
    return false;
    }
  if (!segmentationNode->GetSegmentation()->ContainsRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName()))
    {
    return false;
    }
  this->ReferenceGeometryImage->GetImageToWorldMatrix(ijkToRas);
  vtkMRMLTransformNode* transformNode = segmentationNode->GetParentTransformNode();
  if (transformNode)
    {
    if (!transformNode->IsTransformToWorldLinear())
      {
      return false;
      }
    vtkSmartPointer<vtkMatrix4x4> volumeRasToWorldRas = vtkSmartPointer<vtkMatrix4x4>::New();
    transformNode->GetMatrixTransformToWorld(volumeRasToWorldRas);
    vtkMatrix4x4::Multiply4x4(volumeRasToWorldRas, ijkToRas, ijkToRas);
    }
  return true;
}


//-----------------------------------------------------------------------------
// qMRMLSegmentEditorWidget methods

//-----------------------------------------------------------------------------
qMRMLSegmentEditorWidget::qMRMLSegmentEditorWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLSegmentEditorWidgetPrivate(*this))
{
  Q_D(qMRMLSegmentEditorWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLSegmentEditorWidget::~qMRMLSegmentEditorWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateEffectList()
{
  Q_D(qMRMLSegmentEditorWidget);

  if (d->toolButton(nullptr) == nullptr)
    {
    // Add nullptr effect (arrow button to deactivate all effects)
    QToolButton* effectButton = new QToolButton(d->EffectsGroupBox);
    effectButton->setObjectName(NULL_EFFECT_NAME);
    effectButton->setCheckable(true);
    effectButton->setIcon(QIcon(":Icons/NullEffect.png"));
    effectButton->setText("None");
    effectButton->setToolTip("No editing");
    effectButton->setToolButtonStyle(d->EffectButtonStyle);
    effectButton->setProperty("Effect", QVariant::fromValue<QObject*>(nullptr));
    effectButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred); // make all effect buttons the same width
    d->EffectButtonGroup.addButton(effectButton);;
    }


  // Create local copy of factory effects, so that
  // - Effects can have different parameters
  // - Segment editors can have different active effects
  QList<qSlicerSegmentEditorAbstractEffect*> addedEffects = qSlicerSegmentEditorEffectFactory::instance()->copyEffects(d->RegisteredEffects);

  // Set up effect connections and options frame for all newly added effects
  foreach(qSlicerSegmentEditorAbstractEffect* effect, addedEffects)
    {
    // Connect callbacks that allow effects to send requests to the editor widget without
    // introducing a direct dependency of the effect on the widget.
    effect->setCallbackSlots(this,
      SLOT(setActiveEffectByName(QString)),
      SLOT(updateVolume(void*, bool&)),
      SLOT(saveStateForUndo()));

    // Set parameter set node (if it has been already set in the widget)
    if (d->ParameterSetNode)
      {
      effect->setParameterSetNode(d->ParameterSetNode);
      effect->setMRMLDefaults();
      // Connect parameter modified event to update effect options widget
      qvtkReconnect(d->ParameterSetNode, vtkMRMLSegmentEditorNode::EffectParameterModified, effect, SLOT(updateGUIFromMRML()));
      }

    effect->setVolumes(d->AlignedSourceVolume, d->ModifierLabelmap, d->MaskLabelmap, d->SelectedSegmentLabelmap, d->ReferenceGeometryImage);

    QToolButton* effectButton = new QToolButton(d->EffectsGroupBox);
    effectButton->setObjectName(effect->name());
    effectButton->setCheckable(true);
    effectButton->setToolButtonStyle(d->EffectButtonStyle);
    effectButton->setIcon(effect->icon());
    effectButton->setText(effect->name());
    effectButton->setToolTip(effect->name());
    effectButton->setProperty("Effect", QVariant::fromValue<QObject*>(effect));
    effectButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);  // make all effect buttons the same width
    d->EffectButtonGroup.addButton(effectButton);

    // Add effect options frame to the options widget and hide them
    effect->setupOptionsFrame();
    QFrame* effectOptionsFrame = effect->optionsFrame();
    effectOptionsFrame->setVisible(false);
    d->EffectsOptionsFrame->layout()->addWidget(effectOptionsFrame);
    }

  // Remove all buttons from the layout (so that it can be re-populated with
  // a different button order) and hide all buttons (to only show buttons
  // that are requested to be displayed).
  QList<QAbstractButton*> effectButtons = d->EffectButtonGroup.buttons();
  foreach (QAbstractButton* button, effectButtons)
    {
    button->hide();
    QLayoutItem *child;
    while ((child = d->EffectsGroupBox->layout()->takeAt(0)) != 0);
    }

  QList<qSlicerSegmentEditorAbstractEffect*> displayedEffects; // list of effect buttons to be displayed
  QList<qSlicerSegmentEditorAbstractEffect*> unorderedEffects = d->RegisteredEffects;

  // Add null effect first
  displayedEffects << nullptr;

  // Add effects in the requested order
  foreach(QString effectName, d->EffectNameOrder)
    {
    qSlicerSegmentEditorAbstractEffect* effect = this->effectByName(effectName);
    if (effect)
      {
      displayedEffects << effect;
      unorderedEffects.removeOne(effect);
      }
    }
  // Add unordered effects
  if (d->UnorderedEffectsVisible)
    {
    displayedEffects << unorderedEffects;
    }

  // Add buttons of displayed effect to layout
  int rowIndex = 0;
  int columnIndex = 0;
  foreach(qSlicerSegmentEditorAbstractEffect* effect, displayedEffects)
    {
    QToolButton* effectButton = d->toolButton(effect);
    if (!effectButton)
      {
      qWarning() << Q_FUNC_INFO << " failed: error showing effect button";
      continue;
      }
    effectButton->show();
    auto gridLayout = dynamic_cast<QGridLayout*>(d->EffectsGroupBox->layout());
    gridLayout->addWidget(effectButton, rowIndex, columnIndex);
    if(columnIndex == d->EffectColumnCount - 1)
      {
      columnIndex = 0;
      ++rowIndex;
      }
    else
      {
      ++columnIndex;
      }
    }

  // Set UndoRedoGroupBox buttons with same column count as EffectsGroupBox
  rowIndex = 0;
  columnIndex = 0;
  QList<QAbstractButton*> undoRedoButtons = d->UndoRedoButtonGroup.buttons();
  foreach(QAbstractButton* button, undoRedoButtons)
    {
    auto undoRedoGridLayout = dynamic_cast<QGridLayout*>(d->UndoRedoGroupBox->layout());
    undoRedoGridLayout->addWidget(button, rowIndex, columnIndex);
    QToolButton* toolButton = qobject_cast<QToolButton*>(button);
    if (toolButton)
      {
      toolButton->setToolButtonStyle(d->EffectButtonStyle);
      }

    if(columnIndex == d->EffectColumnCount - 1)
      {
      columnIndex = 0;
      ++rowIndex;
      }
    else
      {
      ++columnIndex;
      }
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsClosing() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }

  if (!d->ParameterSetNode)
    {
    d->SegmentationNodeComboBox->setEnabled(false);
    d->EffectsGroupBox->setEnabled(false);
    d->MaskingGroupBox->setEnabled(false);
    d->EffectsOptionsFrame->setEnabled(false);
    d->SourceVolumeNodeComboBox->setEnabled(false);
    return;
    }
  d->SegmentationNodeComboBox->setEnabled(true);

  MRMLNodeModifyBlocker blocker(d->ParameterSetNode);

  this->updateWidgetFromSegmentationNode();
  this->updateWidgetFromSourceVolumeNode();
  this->updateSliceRotateWarningButtonVisibility();

  d->EffectsGroupBox->setEnabled(d->SegmentationNode != nullptr);
  d->MaskingGroupBox->setEnabled(d->SegmentationNode != nullptr);
  d->EffectsOptionsFrame->setEnabled(d->SegmentationNode != nullptr);
  d->SourceVolumeNodeComboBox->setEnabled(d->SegmentationNode != nullptr);

  QString selectedSegmentID;
  if (d->ParameterSetNode->GetSelectedSegmentID() && strcmp(d->ParameterSetNode->GetSelectedSegmentID(), "") != 0)
    {
    selectedSegmentID = QString(d->ParameterSetNode->GetSelectedSegmentID());

    // Check if selected segment ID is invalid.
    if (!d->SegmentationNode
      || !d->SegmentationNode->GetSegmentation()
      || d->SegmentationNode->GetSegmentation()->GetSegmentIndex(d->ParameterSetNode->GetSelectedSegmentID()) < 0)
      {
      selectedSegmentID.clear();
      }
    }

  // Disable adding new segments until source volume is set (or reference geometry is specified for the segmentation).
  // This forces the user to select a source volume before start adding segments.
  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  bool enableAddSegments = (segmentationNode != nullptr) && (!d->Locked) && ((d->SourceVolumeNode != nullptr) || (!d->referenceImageGeometry().empty()));
  d->AddSegmentButton->setEnabled(enableAddSegments);

  // Only enable remove button if a segment is selected
  d->RemoveSegmentButton->setEnabled(!selectedSegmentID.isEmpty() && (!d->Locked));

  d->Show3DButton->setLocked(d->Locked);
  d->SwitchToSegmentationsButton->setEnabled(true);

  // Segments list section
  if (!selectedSegmentID.isEmpty())
    {
    if (segmentationNode)
      {
      QStringList segmentID;
      segmentID << QString(selectedSegmentID);
      d->SegmentsTableView->setSelectedSegmentIDs(segmentID);
      }
    }
  else
    {
    d->SegmentsTableView->clearSelection();
    }
  d->SegmentsTableView->setReadOnly(d->Locked);

  // Effects section (list and options)
  this->updateEffectsSectionFromMRML();

  // Undo/redo section
  this->updateUndoRedoButtonsState();

  // Masking section
  this->updateMaskingSection();

  // Segmentation object might have been replaced, update selected segment
  this->onSegmentAddedRemoved();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateMaskingSection()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsClosing())
    {
    return;
    }

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  bool wasBlocked = d->MaskModeComboBox->blockSignals(true);
  int maskModeIndex = -1;
  if (d->ParameterSetNode->GetMaskMode() == vtkMRMLSegmentationNode::EditAllowedInsideSingleSegment)
    {
    // segment item
    maskModeIndex = d->MaskModeComboBox->findData(d->ParameterSetNode->GetMaskSegmentID());
    }
  else
    {
    // fixed item, identified by overwrite mode id
    maskModeIndex = d->MaskModeComboBox->findData(d->ParameterSetNode->GetMaskMode());
    }
  d->MaskModeComboBox->setCurrentIndex(maskModeIndex);
  d->MaskModeComboBox->blockSignals(wasBlocked);

  wasBlocked = d->SourceVolumeIntensityMaskCheckBox->blockSignals(true);
  d->SourceVolumeIntensityMaskCheckBox->setChecked(d->ParameterSetNode->GetSourceVolumeIntensityMask());
  d->SourceVolumeIntensityMaskCheckBox->blockSignals(wasBlocked);

  // Update segment names
  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  if (segmentationNode)
    {
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    std::vector< std::string > segmentIDs;
    segmentation->GetSegmentIDs(segmentIDs);
    for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      int currentSegmentItemIndex = d->MaskModeComboBox->findData(QString::fromLocal8Bit(segmentIdIt->c_str()));
      if (currentSegmentItemIndex >= d->MaskModeComboBoxFixedItemsCount)
        {
        QString segmentName = segmentation->GetSegment(*segmentIdIt)->GetName();
        d->MaskModeComboBox->setItemText(currentSegmentItemIndex, segmentName);
        }
      }
    }

  // Initialize mask range if it has never set and intensity masking es enabled
  if (d->ParameterSetNode->GetSourceVolumeIntensityMask()
    && d->ParameterSetNode->GetSourceVolumeIntensityMaskRange()[0] == d->ParameterSetNode->GetSourceVolumeIntensityMaskRange()[1])
    {
    // threshold was uninitialized, set some default
    double range[2] = { 0.0 };
    if (d->SourceVolumeNode && d->SourceVolumeNode->GetImageData() && d->SourceVolumeNode->GetImageData()->GetPointData())
      {
      d->SourceVolumeNode->GetImageData()->GetScalarRange(range);
      }
    d->ParameterSetNode->SetSourceVolumeIntensityMaskRange(range[0] + 0.25*(range[1] - range[0]), range[0] + 0.75*(range[1] - range[0]));
    }

  wasBlocked = d->SourceVolumeIntensityMaskRangeWidget->blockSignals(true);
  d->SourceVolumeIntensityMaskRangeWidget->setVisible(d->ParameterSetNode->GetSourceVolumeIntensityMask());
  d->SourceVolumeIntensityMaskRangeWidget->setMinimumValue(d->ParameterSetNode->GetSourceVolumeIntensityMaskRange()[0]);
  d->SourceVolumeIntensityMaskRangeWidget->setMaximumValue(d->ParameterSetNode->GetSourceVolumeIntensityMaskRange()[1]);
  d->SourceVolumeIntensityMaskRangeWidget->blockSignals(wasBlocked);

  wasBlocked = d->OverwriteModeComboBox->blockSignals(true);
  int overwriteModeIndex = d->OverwriteModeComboBox->findData(d->ParameterSetNode->GetOverwriteMode());
  d->OverwriteModeComboBox->setCurrentIndex(overwriteModeIndex);
  d->OverwriteModeComboBox->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::setMasterRepresentationToBinaryLabelmap()
{
  Q_D(qMRMLSegmentEditorWidget);

  if (d->SegmentationNode == nullptr)
    {
    qDebug() << Q_FUNC_INFO << " failed: segmentation node is invalid.";
    return false;
    }

  if (d->SegmentationNode->GetSegmentation()->GetNumberOfSegments() < 1)
    {
    // If segmentation contains no segments, then set binary labelmap as master by default
    d->SegmentationNode->GetSegmentation()->SetMasterRepresentationName(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    return true;
    }

  if (d->SegmentationNode->GetSegmentation()->GetMasterRepresentationName() == vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName())
    {
    // Current master representation is already binary labelmap
    return true;
    }

  MRMLNodeModifyBlocker blocker(d->SegmentationNode);

  // Editing is only possible if binary labelmap is the master representation
  // If master is not binary labelmap, then ask the user if they wants to make it master
  QString message = QString("Editing requires binary labelmap master representation, but currently the master representation is %1. "
    "Changing the master representation requires conversion. Some details may be lost during conversion process.\n\n"
    "Change master representation to binary labelmap?").
    arg(d->SegmentationNode->GetSegmentation()->GetMasterRepresentationName().c_str());
  QMessageBox::StandardButton answer =
    QMessageBox::question(nullptr, tr("Change master representation to binary labelmap?"), message,
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
  if (answer != QMessageBox::Yes)
    {
    // User rejected the conversion
    qDebug() << Q_FUNC_INFO << " failed: user rejected changing of master representation.";
    return false;
    }

  // All other representations are invalidated when changing to binary labelmap.
  // Re-creating closed surface if it was present before, so that changes can be seen.
  bool closedSurfacePresent = d->SegmentationNode->GetSegmentation()->ContainsRepresentation(
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());

  // Make sure binary labelmap representation exists
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  bool createBinaryLabelmapRepresentationSuccess = d->SegmentationNode->GetSegmentation()->CreateRepresentation(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
  QApplication::restoreOverrideCursor();
  if (!createBinaryLabelmapRepresentationSuccess)
    {
    QString message = QString("Failed to create binary labelmap representation in segmentation %1 for editing!\nPlease see Segmentations module for details.").
      arg(d->SegmentationNode->GetName());
    QMessageBox::critical(nullptr, tr("Failed to create binary labelmap for editing"), message);
    qCritical() << Q_FUNC_INFO << ": " << message;
    return false;
    }

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

  d->SegmentationNode->GetSegmentation()->SetMasterRepresentationName(
    vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());

  if (closedSurfacePresent)
    {
    d->SegmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
    }

  // Show binary labelmap in 2D
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(d->SegmentationNode->GetDisplayNode());
  if (displayNode)
    {
    displayNode->SetPreferredDisplayRepresentationName2D(
      vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
    }

  QApplication::restoreOverrideCursor();

  this->updateSliceRotateWarningButtonVisibility();
  return true;
}


//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateWidgetFromSegmentationNode()
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  // Save segmentation node selection
  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();

  // This block is outside (segmentationNode != d->SegmentationNode) block because
  // first valid node may be selected in the combobox automatically, therefore
  // d->SegmentationNodeComboBox->currentNode() may initially differ from d->SegmentationNode.
  if (segmentationNode != d->SegmentationNodeComboBox->currentNode())
    {
    bool wasBlocked = d->SegmentationNodeComboBox->blockSignals(true);
    d->SegmentationNodeComboBox->setCurrentNode(segmentationNode);
    d->SegmentationNodeComboBox->blockSignals(wasBlocked);
    }

  if (segmentationNode != d->SegmentationNode)
    {
    // Connect events needed to update closed surface button
    qvtkReconnect(d->SegmentationNode, segmentationNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
    qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::ContainedRepresentationNamesModified, this, SLOT(onSegmentAddedRemoved()));
    qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentAdded, this, SLOT(onSegmentAddedRemoved()));
    qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentRemoved, this, SLOT(onSegmentAddedRemoved()));
    qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::SegmentModified, this, SLOT(updateMaskingSection()));
    qvtkReconnect(d->SegmentationNode, segmentationNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(onSegmentationDisplayModified()));
    qvtkReconnect(d->SegmentationNode, segmentationNode, vtkSegmentation::MasterRepresentationModified, this, SLOT(updateSliceRotateWarningButtonVisibility()));
    d->SegmentationNode = segmentationNode;

    bool wasBlocked = d->SegmentsTableView->blockSignals(true);
    d->SegmentsTableView->setSegmentationNode(d->SegmentationNode);
    d->SegmentsTableView->blockSignals(wasBlocked);

    d->Show3DButton->setSegmentationNode(d->SegmentationNode);

    if (segmentationNode)
      {
      // If a geometry reference volume was defined for this segmentation then select it as source volumeSelect source volume node
      vtkMRMLNode* referenceVolumeNode = segmentationNode->GetNodeReference(
        vtkMRMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str());
      // Make sure the source volume selection is performed fully before proceeding
      d->SourceVolumeNodeComboBox->setCurrentNode(referenceVolumeNode);

      // Make sure there is a display node and get it
      segmentationNode->CreateDefaultDisplayNodes();
      vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());

      // Remember whether closed surface is present so that it can be re-converted later if necessary
      bool closedSurfacePresent = segmentationNode->GetSegmentation()->ContainsRepresentation(
        vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
      bool binaryLabelmapPresent = d->SegmentationNode->GetSegmentation()->ContainsRepresentation(
        vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
      // Show closed surface in 3D if present
      if (displayNode && closedSurfacePresent)
        {
        displayNode->SetPreferredDisplayRepresentationName3D(
          vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
        }

      // Show binary labelmap in 2D
      if (displayNode && binaryLabelmapPresent)
        {
        displayNode->SetPreferredDisplayRepresentationName2D(
          vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
        }

      if (segmentationNode->GetSegmentation()->GetNumberOfSegments() > 0)
        {
        // Select first segment to enable all effects (including per-segment ones)
        d->selectFirstSegment();
        }

      // Set label layer to empty, because edit actor will be shown in the slice views during editing
      this->hideLabelLayer();
      }

    emit segmentationNodeChanged(d->SegmentationNode);
    }

  d->SegmentationHistory->SetSegmentation(d->SegmentationNode ? d->SegmentationNode->GetSegmentation() : nullptr);

  // Update closed surface button with new segmentation
  this->onSegmentAddedRemoved();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateWidgetFromSourceVolumeNode()
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return;
    }
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  if (!segmentation)
    {
    return;
    }
  vtkMRMLScalarVolumeNode* sourceVolumeNode = d->ParameterSetNode->GetSourceVolumeNode();
  if (sourceVolumeNode == d->SourceVolumeNode)
    {
    // no change
    return;
    }

  qvtkReconnect(d->SourceVolumeNode, sourceVolumeNode, vtkMRMLVolumeNode::ImageDataModifiedEvent, this, SLOT(onSourceVolumeImageDataModified()));
  d->SourceVolumeNode = sourceVolumeNode;

  bool wasBlocked = d->SourceVolumeNodeComboBox->blockSignals(true);
  d->SourceVolumeNodeComboBox->setCurrentNode(d->SourceVolumeNode);
  d->SourceVolumeNodeComboBox->blockSignals(wasBlocked);

  if (d->SourceVolumeNode && d->AutoShowSourceVolumeNode)
    {
    this->showSourceVolumeInSliceViewers();
    }

  this->onSourceVolumeImageDataModified();

  emit sourceVolumeNodeChanged(d->SourceVolumeNode);
  emit masterVolumeNodeChanged(d->SourceVolumeNode);

  // Notify effects about change
  d->notifyEffectsOfSourceVolumeNodeChange();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSourceVolumeImageDataModified()
{
  Q_D(qMRMLSegmentEditorWidget);

  // Update intensity range slider widget
  if (d->SourceVolumeNode != nullptr && d->SourceVolumeNode->GetImageData() != nullptr
    && d->SourceVolumeNode->GetImageData()->GetPointData() != nullptr)
    {
    double range[2] = { 0.0, 0.0 };
    d->SourceVolumeNode->GetImageData()->GetScalarRange(range);
    d->SourceVolumeIntensityMaskRangeWidget->setRange(range[0], range[1]);
    d->SourceVolumeIntensityMaskRangeWidget->setEnabled(true);
    }
  else
    {
    d->SourceVolumeIntensityMaskRangeWidget->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qMRMLSegmentEditorWidget::activeEffect()const
{
  Q_D(const qMRMLSegmentEditorWidget);

  return d->ActiveEffect;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setActiveEffect(qSlicerSegmentEditorAbstractEffect* effect)
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    if (effect != nullptr)
      {
      qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
      }
    return;
    }

  d->ParameterSetNode->SetActiveEffectName(effect ? effect->name().toUtf8() : "");
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateEffectsSectionFromMRML()
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  // Disable editing if no source volume node is set:
  // source volume determines the extent of editing, so even though the segmentation is valid
  // without a source volume, editing is not possible until it is selected.

  // Disable effect selection and options altogether if no source volume is selected
  bool effectsOverallEnabled = (d->ParameterSetNode->GetSourceVolumeNode() != nullptr) && (!d->Locked);
  d->EffectsGroupBox->setEnabled(effectsOverallEnabled);
  d->OptionsGroupBox->setEnabled(effectsOverallEnabled);

  // Enable only non-per-segment effects if no segment is selected, otherwise enable all effects
  if (effectsOverallEnabled)
    {
    vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
    bool segmentAvailable = segmentationNode && (segmentationNode->GetSegmentation()->GetNumberOfSegments() > 0);
    QString selectedSegmentID(d->ParameterSetNode->GetSelectedSegmentID());
    bool segmentSelected = !selectedSegmentID.isEmpty();
    QList<QAbstractButton*> effectButtons = d->EffectButtonGroup.buttons();
    foreach(QAbstractButton* effectButton, effectButtons)
      {
      qSlicerSegmentEditorAbstractEffect* effect = qobject_cast<qSlicerSegmentEditorAbstractEffect*>(
        effectButton->property("Effect").value<QObject*>());
      if (!effect)
        {
        // nullptr effect
        continue;
        }
      effectButton->setEnabled(!effect->requireSegments() || (segmentAvailable && (segmentSelected || !effect->perSegment())));
      }
    }

  // Update effect options
  const char* activeEffectName = d->ParameterSetNode->GetActiveEffectName();
  qSlicerSegmentEditorAbstractEffect* activeEffect = this->effectByName(activeEffectName); // newly selected effect
  if (activeEffect == d->ActiveEffect)
    {
    return;
    }

  // Deactivate previously selected effect
  if (d->ActiveEffect)
    {
    d->ActiveEffect->deactivate();
    }

  if (activeEffect && !this->setMasterRepresentationToBinaryLabelmap())
    {
    // effect cannot be activated because master representation has to be binary labelmap
    qDebug() << Q_FUNC_INFO << ": Cannot activate effect, failed to set binary labelmap as master representation.";
    activeEffect = nullptr;
    }

  if (activeEffect)
    {
    // Create observations between view interactors and the editor widget.
    // The captured events are propagated to the active effect if any.
    this->setupViewObservations();

    // Deactivate markup/ruler/ROI placement
    if (d->InteractionNode && d->InteractionNode->GetCurrentInteractionMode() != vtkMRMLInteractionNode::ViewTransform)
      {
      d->InteractionNode->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
      }

    // Activate newly selected effect
    activeEffect->activate();
    d->OptionsGroupBox->show();
    d->OptionsGroupBox->setTitle(activeEffect->name());
    d->EffectHelpBrowser->setCollapsibleText(activeEffect->helpText());
    d->MaskingGroupBox->show();
    }
  else
    {
    d->OptionsGroupBox->hide();
    d->OptionsGroupBox->setTitle("");
    d->EffectHelpBrowser->setText("");
    d->MaskingGroupBox->hide();

    this->removeViewObservations();
    }

  // Update button checked states
  QString effectName(NULL_EFFECT_NAME);
  if (activeEffect)
    {
    effectName = activeEffect->name();
    }
  QList<QAbstractButton*> effectButtons = d->EffectButtonGroup.buttons();
  foreach(QAbstractButton* effectButton, effectButtons)
    {
    bool checked = effectButton->isChecked();
    bool needToBeChecked = (effectButton->objectName().compare(effectName) == 0);
    if (checked != needToBeChecked)
      {
      bool wasBlocked = effectButton->blockSignals(true);
      effectButton->setChecked(needToBeChecked);
      effectButton->blockSignals(wasBlocked);
      }
    }

  // Set cursor for active effect, but only when in view/transform node.
  // In other mouse modes, the application sets a custom cursor and the Segment Editor must not override that.
  if (!d->InteractionNode
    || d->InteractionNode->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewTransform)
    {
    d->setEffectCursor(activeEffect);
    }

  // Set active effect
  d->LastActiveEffect = d->ActiveEffect;
  d->ActiveEffect = activeEffect;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (newScene == this->mrmlScene())
    {
    return;
    }

  // Setting the scene would trigger MRML node update from GUI
  // (selection of first node in combo box)
  // but GUI has not been fully update yet from MRML, so
  // we must prevent these MRML node updates.
  // A full update of widgets from MRML is performed after
  // scene update in the widgets is completed.
  {
    const QSignalBlocker blocker1(d->SegmentationNodeComboBox);
    const QSignalBlocker blocker2(d->SourceVolumeNodeComboBox);
    Superclass::setMRMLScene(newScene);
  }

  // Make connections that depend on the Slicer application

  // Observe layout manager (may not be available if running without main window)
  if (qSlicerApplication::application()->layoutManager())
    {
    QObject::connect(qSlicerApplication::application()->layoutManager(), SIGNAL(layoutChanged(int)),
      this, SLOT(onLayoutChanged(int)));
    }

  vtkMRMLInteractionNode *interactionNode = nullptr;
  if (newScene)
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(newScene->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  this->qvtkReconnect(d->InteractionNode, interactionNode, vtkCommand::ModifiedEvent, this, SLOT(onInteractionNodeModified()));
  d->InteractionNode = interactionNode;

  // Update UI
  this->updateWidgetFromMRML();

  // observe close event so can re-add a parameters node if necessary
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent, this, SLOT(onMRMLSceneEndCloseEvent()));
  this->qvtkConnect(this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onMRMLSceneEndBatchProcessEvent()));
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onMRMLSceneEndCloseEvent()
{
  this->initializeParameterSetNode();
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onMRMLSceneEndBatchProcessEvent()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!this->mrmlScene())
    {
    return;
    }
  if (d->ParameterSetNode->GetSegmentationNode() != d->SegmentationNodeComboBox->currentNode())
    {
    this->setSegmentationNode(d->SegmentationNodeComboBox->currentNode());
    }

  if (d->ParameterSetNode->GetSourceVolumeNode() != d->SourceVolumeNodeComboBox->currentNode())
    {
    this->setSourceVolumeNode(d->SourceVolumeNodeComboBox->currentNode());
    }

  // force update (clear GUI if no node is selected anymore)
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onInteractionNodeModified()
{
  Q_D(const qMRMLSegmentEditorWidget);
  if (!d->InteractionNode || !d->ActiveEffect)
    {
    return;
    }
  // Only notify the active effect about interaction node changes
  // (inactive effects should not interact with the user)
  d->ActiveEffect->interactionNodeModified(d->InteractionNode);
}

//------------------------------------------------------------------------------
vtkMRMLSegmentEditorNode* qMRMLSegmentEditorWidget::mrmlSegmentEditorNode()const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->ParameterSetNode;
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setMRMLSegmentEditorNode(vtkMRMLSegmentEditorNode* newSegmentEditorNode)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (d->ParameterSetNode == newSegmentEditorNode)
    {
    return;
    }

  // Connect modified event on ParameterSetNode to updating the widget
  qvtkReconnect(d->ParameterSetNode, newSegmentEditorNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  // Set parameter set node
  d->ParameterSetNode = newSegmentEditorNode;

  if (!d->ParameterSetNode)
    {
    return;
    }

  this->initializeParameterSetNode();

  // Update UI
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
vtkMRMLInteractionNode* qMRMLSegmentEditorWidget::interactionNode()const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->InteractionNode;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setInteractionNode(vtkMRMLInteractionNode* interactionNode)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (d->InteractionNode == interactionNode)
    {
    return;
    }
  this->qvtkReconnect(d->InteractionNode, interactionNode, vtkCommand::ModifiedEvent, this, SLOT(onInteractionNodeModified()));
  d->InteractionNode = interactionNode;
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::initializeParameterSetNode()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode)
    {
    return;
    }

  // Block EffectParameterModified events while we are still initializing the parameter node.
  // Not doing so can result in a number of error messages when the scene is closed and the
  // parameter node is reinitialized.
  MRMLNodeModifyBlocker blocker(d->ParameterSetNode);

  // Set parameter set node to all effects
  foreach(qSlicerSegmentEditorAbstractEffect* effect, d->RegisteredEffects)
    {
    effect->setParameterSetNode(d->ParameterSetNode);
    effect->setMRMLDefaults();

    // Connect parameter modified event to update effect options widget
    qvtkReconnect(d->ParameterSetNode, vtkMRMLSegmentEditorNode::EffectParameterModified, effect, SLOT(updateGUIFromMRML()));
    }
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setSegmentationNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    if (node)
      {
      qCritical() << Q_FUNC_INFO << " failed: need to set segment editor node first";
      }
    return;
    }
  if (d->ParameterSetNode->GetSegmentationNode() == node)
    {
    // no change
    return;
    }

  this->setActiveEffect(nullptr); // deactivate current effect when we switch to a different segmentation
  d->ParameterSetNode->SetAndObserveSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentEditorWidget::segmentationNode()const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->SegmentationNodeComboBox->currentNode();
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setSegmentationNodeID(const QString& nodeID)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!this->mrmlScene())
    {
    qCritical() << Q_FUNC_INFO << " failed: MRML scene is not set";
    return;
    }
  this->setSegmentationNode(vtkMRMLSegmentationNode::SafeDownCast(
    this->mrmlScene()->GetNodeByID(nodeID.toUtf8())));
}

//------------------------------------------------------------------------------
QString qMRMLSegmentEditorWidget::segmentationNodeID()const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->SegmentationNodeComboBox->currentNodeID();
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentEditorWidget::currentSegmentID()const
{
  Q_D(const qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return QString();
    }

  const char* selectedSegmentID = d->ParameterSetNode->GetSelectedSegmentID();
  return QString(selectedSegmentID);
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setSourceVolumeNode(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode || !this->segmentationNode())
    {
    if (node)
      {
      qCritical() << Q_FUNC_INFO << " failed: need to set segment editor and segmentation nodes first";
      }
    return;
    }

  // Set source volume to parameter set node
  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
  if (d->ParameterSetNode->GetSourceVolumeNode() == volumeNode)
    {
    return;
    }

  d->ParameterSetNode->SetAndObserveSourceVolumeNode(volumeNode);
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentEditorWidget::sourceVolumeNode()const
{
  Q_D(const qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode)
    {
    return nullptr;
    }
  return d->ParameterSetNode->GetSourceVolumeNode();
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setSourceVolumeNodeID(const QString& nodeID)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!this->mrmlScene())
    {
    qCritical() << Q_FUNC_INFO << " failed: MRML scene is not set";
    return;
    }
  this->setSourceVolumeNode(this->mrmlScene()->GetNodeByID(nodeID.toUtf8()));
}

//------------------------------------------------------------------------------
QString qMRMLSegmentEditorWidget::sourceVolumeNodeID()const
{
  Q_D(const qMRMLSegmentEditorWidget);
  vtkMRMLNode* sourceVolumeNode = this->sourceVolumeNode();
  if (!sourceVolumeNode || !sourceVolumeNode->GetID())
    {
    return "";
    }
  return sourceVolumeNode->GetID();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSegmentationNodeChanged(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->setSegmentationNode(node);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSegmentSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
  Q_UNUSED(selected);
  Q_UNUSED(deselected);
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  QStringList selectedSegmentIDs = d->SegmentsTableView->selectedSegmentIDs();

  // If selection did not change, then return
  QString currentSegmentID(d->ParameterSetNode->GetSelectedSegmentID());
  // Only the first selected segment is used (if multiple segments are selected then the others
  // are ignored; multi-select may be added in the future).
  QString selectedSegmentID(selectedSegmentIDs.isEmpty() ? QString() : selectedSegmentIDs[0]);
  if (!currentSegmentID.compare(selectedSegmentID))
    {
    return;
    }

  // Set segment ID if changed
  if (selectedSegmentIDs.isEmpty())
    {
    d->ParameterSetNode->SetSelectedSegmentID(nullptr);
    // Also de-select current effect if per-segment
    if (d->ActiveEffect && d->ActiveEffect->perSegment())
      {
      this->setActiveEffect(nullptr);
      }
    }
  else
    {
    d->ParameterSetNode->SetSelectedSegmentID(selectedSegmentID.toUtf8().constData());
    }

  // Disable editing if no segment is selected
  this->updateWidgetFromMRML();

  emit currentSegmentIDChanged(selectedSegmentID);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setCurrentSegmentID(const QString segmentID)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (d->ParameterSetNode)
    {
    d->ParameterSetNode->SetSelectedSegmentID(segmentID.toUtf8().constData());
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSourceVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->setSourceVolumeNode(node);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onEffectButtonClicked(QAbstractButton* button)
{
  // Get effect that was just clicked
  qSlicerSegmentEditorAbstractEffect* clickedEffect = qobject_cast<qSlicerSegmentEditorAbstractEffect*>(
    button->property("Effect").value<QObject*>() );

  this->setActiveEffect(clickedEffect);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onAddSegment()
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return;
    }

  d->SegmentationHistory->SaveState();

  // Create empty segment in current segmentation
  std::string addedSegmentID = segmentationNode->GetSegmentation()->AddEmptySegment(d->SegmentsTableView->textFilter().toStdString());

  // Set default terminology entry from application settings
  vtkSegment* addedSegment = segmentationNode->GetSegmentation()->GetSegment(addedSegmentID);
  if (addedSegment)
    {
    QString defaultTerminologyEntryStr = this->defaultTerminologyEntry();
    if (!defaultTerminologyEntryStr.isEmpty())
      {
      addedSegment->SetTag(vtkSegment::GetTerminologyEntryTagName(), defaultTerminologyEntryStr.toUtf8().constData());
      }
    }

  // Set segment status to one that is visible by current filtering criteria
  int status = 0;
  for (int i = 0; i < vtkSlicerSegmentationsModuleLogic::LastStatus; ++i)
    {
    if (d->SegmentsTableView->sortFilterProxyModel()->showStatus(i))
      {
      status = i;
      break;
      }
    }
  vtkSlicerSegmentationsModuleLogic::SetSegmentStatus(addedSegment, status);

  // Select the new segment
  if (!addedSegmentID.empty())
    {
    QStringList segmentIDList;
    segmentIDList << QString(addedSegmentID.c_str());
    d->SegmentsTableView->setSelectedSegmentIDs(segmentIDList);
    // Make sure update of the effect buttons happens if the selected segment IDs do not change
    this->updateEffectsSectionFromMRML();
    }

  // Assign the new segment the terminology of the (now second) last segment
  if (segmentationNode->GetSegmentation()->GetNumberOfSegments() > 1)
    {
    vtkSegment* secondLastSegment = segmentationNode->GetSegmentation()->GetNthSegment(
      segmentationNode->GetSegmentation()->GetNumberOfSegments() - 2 );
    std::string repeatedTerminologyEntry("");
    secondLastSegment->GetTag(secondLastSegment->GetTerminologyEntryTagName(), repeatedTerminologyEntry);
    segmentationNode->GetSegmentation()->GetSegment(addedSegmentID)->SetTag(
      secondLastSegment->GetTerminologyEntryTagName(), repeatedTerminologyEntry );
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onRemoveSegment()
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  std::string selectedSegmentID = (d->ParameterSetNode->GetSelectedSegmentID() ? d->ParameterSetNode->GetSelectedSegmentID() : "");
  if (!segmentationNode || selectedSegmentID.empty())
    {
    return;
    }

  d->SegmentationHistory->SaveState();

  // Switch to a new valid segment now (to avoid transient state when no segments are selected
  // as it could deactivate current effect).
  vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
  std::vector< std::string > segmentIDs;
  segmentation->GetSegmentIDs(segmentIDs);
  if (segmentIDs.size() > 1)
    {
    std::string newSelectedSegmentID;
    std::string previousSegmentID = segmentIDs.front();
    for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      if (*segmentIdIt == selectedSegmentID)
        {
        // found the currently selected segment
        // use the next segment (if at the end, use the previous)
        ++segmentIdIt;
        if (segmentIdIt != segmentIDs.end())
          {
          newSelectedSegmentID = *segmentIdIt;
          }
        else
          {
          newSelectedSegmentID = previousSegmentID;
          }
        break;
        }
      previousSegmentID = *segmentIdIt;
      }
    QStringList newSelectedSegmentIdList;
    newSelectedSegmentIdList << QString(newSelectedSegmentID.c_str());
    d->SegmentsTableView->setSelectedSegmentIDs(newSelectedSegmentIdList);
    }

  // Remove segment
  segmentationNode->GetSegmentation()->RemoveSegment(selectedSegmentID);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qMRMLSegmentEditorWidget::switchToSegmentationsModule()
{
  // Switch to Segmentations module
  qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("Segmentations");
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (!moduleWithAction)
    {
    qCritical() << Q_FUNC_INFO << ": Segmentations module not found";
    return nullptr;
    }
  moduleWithAction->widgetRepresentation(); // Make sure it's created before showing
  moduleWithAction->action()->trigger();

  // Get module widget
  qSlicerAbstractModuleWidget* moduleWidget = dynamic_cast<qSlicerAbstractModuleWidget*>(moduleWithAction->widgetRepresentation());;
  if (!moduleWidget)
    {
    qCritical() << Q_FUNC_INFO << ": Segmentations module is not available";
    return nullptr;
    }

  return moduleWidget;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSwitchToSegmentations()
{
  Q_D(qMRMLSegmentEditorWidget);

  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return;
    }

  qSlicerAbstractModuleWidget* moduleWidget = this->switchToSegmentationsModule();
  if (!moduleWidget)
    {
    qCritical() << Q_FUNC_INFO << ": Segmentations module is not found";
    return;
    }

  // Get segmentation selector combobox and set segmentation
  qMRMLNodeComboBox* nodeSelector = moduleWidget->findChild<qMRMLNodeComboBox*>("MRMLNodeComboBox_Segmentation");
  if (!nodeSelector)
    {
    qCritical() << Q_FUNC_INFO << ": MRMLNodeComboBox_Segmentation is not found in Segmentations module";
    return;
    }
  nodeSelector->setCurrentNode(segmentationNode);

  // Get segments table and select segment
  qMRMLSegmentsTableView* segmentsTable = moduleWidget->findChild<qMRMLSegmentsTableView*>("SegmentsTableView");
  if (!segmentsTable)
    {
    qCritical() << Q_FUNC_INFO << ": SegmentsTableView is not found in Segmentations module";
    return;
    }
  segmentsTable->setSelectedSegmentIDs(d->SegmentsTableView->selectedSegmentIDs());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onCreateSurfaceToggled(bool on)
{
  Q_D(qMRMLSegmentEditorWidget);

  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(
    segmentationNode->GetDisplayNode());
  if (!displayNode)
    {
    return;
    }

  MRMLNodeModifyBlocker segmentationNodeBlocker(segmentationNode);
  MRMLNodeModifyBlocker displayNodeBlocker(displayNode);

  // If just have been checked, then create closed surface representation and show it
  if (on)
    {
    // Make sure closed surface representation exists
    if (segmentationNode->GetSegmentation()->CreateRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName() ))
      {
      // Set closed surface as displayed poly data representation
      displayNode->SetPreferredDisplayRepresentationName3D(
        vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName() );
      // But keep binary labelmap for 2D
      bool binaryLabelmapPresent = segmentationNode->GetSegmentation()->ContainsRepresentation(
        vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName());
      if (binaryLabelmapPresent)
        {
        displayNode->SetPreferredDisplayRepresentationName2D(
          vtkSegmentationConverter::GetSegmentationBinaryLabelmapRepresentationName() );
        }
      }
    }
  // If unchecked, then remove representation (but only if it's not the master representation)
  else if (segmentationNode->GetSegmentation()->GetMasterRepresentationName() !=
    vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName())
    {
    segmentationNode->GetSegmentation()->RemoveRepresentation(
      vtkSegmentationConverter::GetSegmentationClosedSurfaceRepresentationName());
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSegmentAddedRemoved()
{
  Q_D(qMRMLSegmentEditorWidget);

  vtkMRMLSegmentationNode* segmentationNode = nullptr;
  if (d->ParameterSetNode)
    {
    segmentationNode = d->ParameterSetNode->GetSegmentationNode();
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    }

  d->SwitchToSegmentationsButton->setEnabled(segmentationNode!= nullptr);

  // Update mask mode combo box with current segment names

  bool wasBlocked = d->MaskModeComboBox->blockSignals(true);

  // save selection (if it's a non-fixed item)
  QString selectedSegmentId;
  if (d->MaskModeComboBox->currentIndex() >= d->MaskModeComboBoxFixedItemsCount)
    {
    selectedSegmentId = d->MaskModeComboBox->itemData(d->MaskModeComboBox->currentIndex()).toString();
    }

  // Remove segment names, keep only fixed items
  while (d->MaskModeComboBox->count() > d->MaskModeComboBoxFixedItemsCount)
    {
    d->MaskModeComboBox->removeItem(d->MaskModeComboBox->count()-1);
    }

  if (segmentationNode)
    {
    vtkSegmentation* segmentation = segmentationNode->GetSegmentation();
    std::vector< std::string > segmentIDs;
    segmentation->GetSegmentIDs(segmentIDs);
    for (std::vector< std::string >::const_iterator segmentIdIt = segmentIDs.begin(); segmentIdIt != segmentIDs.end(); ++segmentIdIt)
      {
      QString segmentName = segmentation->GetSegment(*segmentIdIt)->GetName();
      d->MaskModeComboBox->addItem(tr("Inside ") + segmentName, QString::fromLocal8Bit(segmentIdIt->c_str()));
      }

    // restore selection
    if (!selectedSegmentId.isEmpty())
      {
      int maskModeIndex = d->MaskModeComboBox->findData(selectedSegmentId);
      d->MaskModeComboBox->setCurrentIndex(maskModeIndex);
      }
    }
  d->MaskModeComboBox->blockSignals(wasBlocked);

  if (segmentationNode && d->MaskModeComboBox->currentIndex()<0)
    {
    // probably the currently selected mask segment was deleted,
    // switch to the first masking option (no mask).
    d->MaskModeComboBox->setCurrentIndex(0);
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::showSourceVolumeInSliceViewers(bool forceShowInBackground /*=false*/, bool fitSlice /*=false*/)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode->GetSourceVolumeNode())
    {
    return;
    }
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return;
    }
  vtkCollection* sliceLogics = layoutManager->mrmlSliceLogics();
  if (!sliceLogics)
    {
    return;
    }
  vtkObject* object = nullptr;
  vtkCollectionSimpleIterator it;
  for (sliceLogics->InitTraversal(it); (object = sliceLogics->GetNextItemAsObject(it));)
    {
    vtkMRMLSliceLogic* sliceLogic = vtkMRMLSliceLogic::SafeDownCast(object);
    if (!sliceLogic)
      {
      continue;
      }
    vtkMRMLSliceNode* sliceNode = sliceLogic->GetSliceNode();
    if (!d->segmentationDisplayableInView(sliceNode))
      {
      continue;
      }
    vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
    if (!sliceCompositeNode)
      {
      continue;
      }
    std::string backgroundVolumeID = (sliceCompositeNode->GetBackgroundVolumeID() ? sliceCompositeNode->GetBackgroundVolumeID() : "");
    std::string foregroundVolumeID = (sliceCompositeNode->GetForegroundVolumeID() ? sliceCompositeNode->GetForegroundVolumeID() : "");
    std::string sourceVolumeID = (d->ParameterSetNode->GetSourceVolumeNode()->GetID() ? d->ParameterSetNode->GetSourceVolumeNode()->GetID() : "");
    bool sourceVolumeAlreadyShown = (backgroundVolumeID == sourceVolumeID || foregroundVolumeID == sourceVolumeID);
    if (!sourceVolumeAlreadyShown || forceShowInBackground)
      {
      sliceCompositeNode->SetBackgroundVolumeID(d->ParameterSetNode->GetSourceVolumeNode()->GetID());
      sliceCompositeNode->SetForegroundVolumeID(nullptr);
      sliceCompositeNode->SetLabelVolumeID(nullptr);
      }
    if (fitSlice)
      {
      sliceLogic->FitSliceToAll(true);
      }
    }
}


//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onLayoutChanged(int layoutIndex)
{
  Q_D(qMRMLSegmentEditorWidget);
  Q_UNUSED(layoutIndex);

  if (d->ViewsObserved)
    {
    // Refresh view observations with the new layout
    this->setupViewObservations();

    if (d->AutoShowSourceVolumeNode)
      {
      // Set volume selection to all slice viewers in new layout
      this->showSourceVolumeInSliceViewers();
      }

    // Let effects know about the updated layout
    d->notifyEffectsOfLayoutChange();
    }

}

//---------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qMRMLSegmentEditorWidget::effectByName(QString name)
{
  Q_D(qMRMLSegmentEditorWidget);

  if (name.isEmpty())
    {
    return nullptr;
    }

  // Find effect with name
  qSlicerSegmentEditorAbstractEffect* currentEffect = nullptr;
  foreach(currentEffect, d->RegisteredEffects)
    {
    if (currentEffect->name().compare(name) == 0)
      {
      return currentEffect;
      }
    }

  return nullptr;
}

//------------------------------------------------------------------------------
QStringList qMRMLSegmentEditorWidget::effectNameOrder() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->EffectNameOrder;
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setEffectNameOrder(const QStringList& effectNames)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (effectNames == d->EffectNameOrder)
    {
    // no change
    return;
    }
  d->EffectNameOrder = effectNames;
  this->updateEffectList();
}

//------------------------------------------------------------------------------
int qMRMLSegmentEditorWidget::effectColumnCount() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->EffectColumnCount;
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setEffectColumnCount(int columnCount)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->EffectColumnCount = columnCount;
  this->updateEffectList();
}

//------------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::unorderedEffectsVisible() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->UnorderedEffectsVisible;
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setUnorderedEffectsVisible(bool visible)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (visible == d->UnorderedEffectsVisible)
    {
    // no change
    return;
    }
  d->UnorderedEffectsVisible = visible;
  this->updateEffectList();
}

//---------------------------------------------------------------------------
QStringList qMRMLSegmentEditorWidget::availableEffectNames()
{
  Q_D(qMRMLSegmentEditorWidget);
  QStringList availableEffectNames;
  foreach(qSlicerSegmentEditorAbstractEffect* effect, d->RegisteredEffects)
    {
    availableEffectNames << effect->name();
    }
  return availableEffectNames;
}

//---------------------------------------------------------------------------
int qMRMLSegmentEditorWidget::effectCount()
{
  Q_D(qMRMLSegmentEditorWidget);
  return d->EffectsGroupBox->layout()->count();
}

//---------------------------------------------------------------------------
qSlicerSegmentEditorAbstractEffect* qMRMLSegmentEditorWidget::effectByIndex(int index)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (index < 0 || index >= d->EffectsGroupBox->layout()->count())
    {
    return nullptr;
    }
  QLayoutItem* item = d->EffectsGroupBox->layout()->itemAt(index);
  if (!item || !item->widget())
    {
    return nullptr;
    }
  qSlicerSegmentEditorAbstractEffect* foundEffect = qobject_cast<qSlicerSegmentEditorAbstractEffect*>(
    item->widget()->property("Effect").value<QObject*>());

  return foundEffect;
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setupViewObservations()
{
  Q_D(qMRMLSegmentEditorWidget);

  // Make sure previous observations are cleared before setting up the new ones
  this->removeViewObservations();

  d->ObservedViewNodeIDs.clear();

  // Set up interactor observations
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return;
    }

  // Slice views
  foreach (QString sliceViewName, layoutManager->sliceViewNames())
    {
    // Create command for slice view
    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    if (!d->segmentationDisplayableInView(sliceWidget->mrmlSliceNode()))
      {
      continue;
      }
    qMRMLSliceView* sliceView = sliceWidget->sliceView();
    vtkNew<vtkSegmentEditorEventCallbackCommand> interactionCallbackCommand;
    interactionCallbackCommand->EditorWidget = this;
    interactionCallbackCommand->ViewWidget = sliceWidget;
    interactionCallbackCommand->SetClientData( reinterpret_cast<void*>(interactionCallbackCommand.GetPointer()) );
    interactionCallbackCommand->SetCallback( qMRMLSegmentEditorWidget::processEvents );

    // Connect interactor events
    vtkRenderWindowInteractor* interactor = sliceView->interactorStyle()->GetInteractor();
    SegmentEditorEventObservation interactorObservation;
    interactorObservation.CallbackCommand = interactionCallbackCommand.GetPointer();
    interactorObservation.ObservedObject = interactor;
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::LeftButtonPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::RightButtonPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::RightButtonReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::RightButtonDoubleClickEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MiddleButtonPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MiddleButtonReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MiddleButtonDoubleClickEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseMoveEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelForwardEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelBackwardEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelLeftEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelRightEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::KeyPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::KeyReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::EnterEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::LeaveEvent, interactorObservation.CallbackCommand, 1.0);
    d->EventObservations << interactorObservation;

    // Slice node observation
    vtkMRMLSliceNode* sliceNode = sliceWidget->sliceLogic()->GetSliceNode();
    SegmentEditorEventObservation sliceNodeObservation;
    sliceNodeObservation.CallbackCommand = interactionCallbackCommand.GetPointer();
    sliceNodeObservation.ObservedObject = sliceNode;
    sliceNodeObservation.ObservationTags << sliceNode->AddObserver(vtkCommand::ModifiedEvent, sliceNodeObservation.CallbackCommand, 1.0);
    d->EventObservations << sliceNodeObservation;

    // Slice pose observation
    SegmentEditorEventObservation slicePoseObservation;
    slicePoseObservation.CallbackCommand = interactionCallbackCommand.GetPointer();
    slicePoseObservation.ObservedObject = sliceNode->GetSliceToRAS();
    slicePoseObservation.ObservationTags << sliceNode->GetSliceToRAS()->AddObserver(vtkCommand::ModifiedEvent, slicePoseObservation.CallbackCommand, 1.0);
    d->EventObservations << slicePoseObservation;
    }

  // 3D views
  for (int threeDViewId=0; threeDViewId<layoutManager->threeDViewCount(); ++threeDViewId)
    {
    // Create command for 3D view
    qMRMLThreeDWidget* threeDWidget = layoutManager->threeDWidget(threeDViewId);
    if (!d->segmentationDisplayableInView(threeDWidget->mrmlViewNode()))
      {
      continue;
      }
    qMRMLThreeDView* threeDView = threeDWidget->threeDView();
    vtkNew<vtkSegmentEditorEventCallbackCommand> interactionCallbackCommand;
    interactionCallbackCommand->EditorWidget = this;
    interactionCallbackCommand->ViewWidget = threeDWidget;
    interactionCallbackCommand->SetClientData( reinterpret_cast<void*>(interactionCallbackCommand.GetPointer()) );
    interactionCallbackCommand->SetCallback( qMRMLSegmentEditorWidget::processEvents );

    // Connect interactor events
    vtkRenderWindowInteractor* interactor = threeDView->interactor();
    SegmentEditorEventObservation interactorObservation;
    interactorObservation.CallbackCommand = interactionCallbackCommand.GetPointer();
    interactorObservation.ObservedObject = interactor;
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::LeftButtonPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::LeftButtonReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::RightButtonPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::RightButtonReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MiddleButtonPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MiddleButtonReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseMoveEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelForwardEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelBackwardEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelLeftEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::MouseWheelRightEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::KeyPressEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::KeyReleaseEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::EnterEvent, interactorObservation.CallbackCommand, 1.0);
    interactorObservation.ObservationTags << interactor->AddObserver(vtkCommand::LeaveEvent, interactorObservation.CallbackCommand, 1.0);
    d->EventObservations << interactorObservation;

    // 3D view node observations
    vtkMRMLViewNode* viewNode = threeDWidget->mrmlViewNode();
    SegmentEditorEventObservation viewNodeObservation;
    viewNodeObservation.CallbackCommand = interactionCallbackCommand.GetPointer();
    viewNodeObservation.ObservedObject = viewNode;
    viewNodeObservation.ObservationTags << viewNode->AddObserver(vtkCommand::ModifiedEvent, viewNodeObservation.CallbackCommand, 1.0);
    d->EventObservations << viewNodeObservation;
    }

  if (d->SegmentationNode)
    {
    int numberOfDisplayNodes = d->SegmentationNode->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
      vtkMRMLDisplayNode* segmentationDisplayNode = d->SegmentationNode->GetNthDisplayNode(displayNodeIndex);
      if (!segmentationDisplayNode)
        {
        // this may occur during scene close
        continue;
        }
      d->ObservedViewNodeIDs[segmentationDisplayNode->GetID()] = segmentationDisplayNode->GetViewNodeIDs();
      }
    }

  this->updateSliceRotateWarningButtonVisibility();

  d->ViewsObserved = true;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::removeViewObservations()
{
  Q_D(qMRMLSegmentEditorWidget);
  foreach (SegmentEditorEventObservation eventObservation, d->EventObservations)
    {
    if (eventObservation.ObservedObject)
      {
      foreach (int observationTag, eventObservation.ObservationTags)
        {
        eventObservation.ObservedObject->RemoveObserver(observationTag);
        }
      }
    }
  d->EventObservations.clear();
  d->ViewsObserved = false;
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setActiveEffectByName(QString effectName)
{
  qSlicerSegmentEditorAbstractEffect* effect = this->effectByName(effectName);
  this->setActiveEffect(effect);
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::saveStateForUndo()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (d->SegmentationHistory->GetMaximumNumberOfStates() > 0)
    {
    d->SegmentationHistory->SaveState();
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateVolume(void* volumeToUpdate, bool& success)
{
  Q_D(qMRMLSegmentEditorWidget);

  if (volumeToUpdate == d->AlignedSourceVolume)
    {
    success = d->updateAlignedSourceVolume();
    }
  else if (volumeToUpdate == d->ModifierLabelmap)
    {
    success = d->resetModifierLabelmapToDefault();
    }
  else if (volumeToUpdate == d->MaskLabelmap)
    {
    success = d->updateMaskLabelmap();
    }
  else if (volumeToUpdate == d->SelectedSegmentLabelmap)
    {
    success = d->updateSelectedSegmentLabelmap();
    }
  else if (volumeToUpdate == d->ReferenceGeometryImage)
    {
    success = d->updateReferenceGeometryImage();
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Failed to update volume";
    success = false;
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::processEvents(vtkObject* caller,
                                        unsigned long eid,
                                        void* clientData,
                                        void* vtkNotUsed(callData))
{
  // Get and parse client data
  vtkSegmentEditorEventCallbackCommand* callbackCommand = reinterpret_cast<vtkSegmentEditorEventCallbackCommand*>(clientData);
  qMRMLSegmentEditorWidget* self = callbackCommand->EditorWidget.data();
  qMRMLWidget* viewWidget = callbackCommand->ViewWidget.data();
  if (!self || !viewWidget)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid event data";
    return;
    }
  // Do nothing if scene is closing
  if (!self->mrmlScene() || self->mrmlScene()->IsClosing())
    {
    return;
    }
  // If the segment editor node is no longer valid then ignore all events
  if (!self->mrmlSegmentEditorNode())
    {
    return;
    }

  vtkMatrix4x4* sliceToRAS = vtkMatrix4x4::SafeDownCast(caller);
  if (sliceToRAS)
    {
    self->updateSliceRotateWarningButtonVisibility();
    return;
    }

  // Get active effect
  qSlicerSegmentEditorAbstractEffect* activeEffect = self->activeEffect();
  if (!activeEffect)
    {
    return;
    }

  // Call processing function of active effect. Handle both interactor and view node events
  vtkRenderWindowInteractor* callerInteractor = vtkRenderWindowInteractor::SafeDownCast(caller);
  vtkMRMLAbstractViewNode* callerViewNode = vtkMRMLAbstractViewNode::SafeDownCast(caller);
  if (callerInteractor)
    {

    // Do not process events while a touch gesture is in progress (e.g., do not paint in the view
    // while doing multi-touch pinch/rotate).
    qMRMLSliceWidget* sliceWidget = qobject_cast<qMRMLSliceWidget*>(viewWidget);
    if (sliceWidget)
      {
      vtkMRMLCrosshairDisplayableManager* crosshairDisplayableManager = vtkMRMLCrosshairDisplayableManager::SafeDownCast(
        sliceWidget->sliceView()->displayableManagerByClassName("vtkMRMLCrosshairDisplayableManager"));
      if (crosshairDisplayableManager)
        {
        int widgetState = crosshairDisplayableManager->GetSliceIntersectionWidget()->GetWidgetState();
        if (widgetState == vtkMRMLSliceIntersectionWidget::WidgetStateTouchGesture)
          {
          return;
          }
        }
      }
    qMRMLThreeDWidget* threeDWidget = qobject_cast<qMRMLThreeDWidget*>(viewWidget);
    if (threeDWidget)
      {
      vtkMRMLCameraDisplayableManager* cameraDisplayableManager = vtkMRMLCameraDisplayableManager::SafeDownCast(
        threeDWidget->threeDView()->displayableManagerByClassName("vtkMRMLCameraDisplayableManager"));
      if (cameraDisplayableManager)
        {
        int widgetState = cameraDisplayableManager->GetCameraWidget()->GetWidgetState();
        if (widgetState == vtkMRMLCameraWidget::WidgetStateTouchGesture)
          {
          return;
          }
        }
      }

    bool abortEvent = activeEffect->processInteractionEvents(callerInteractor, eid, viewWidget);
    if (abortEvent)
      {
      /// Set the AbortFlag on the vtkCommand associated with the event.
      /// It causes other observers of the interactor not to receive the events.
      callbackCommand->SetAbortFlag(1);
      }
    }
  else if (callerViewNode)
    {
    activeEffect->processViewNodeEvents(callerViewNode, eid, viewWidget);
    }
  else
    {
    qCritical() << Q_FUNC_INFO << ": Unsupported caller object";
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSourceVolumeIntensityMaskChecked(bool checked)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }
  d->ParameterSetNode->SetSourceVolumeIntensityMask(checked);
  /*
  this->ThresholdRangeWidget->blockSignals(true);
  this->ThresholdRangeWidget->setVisible(checked);
  this->ThresholdRangeWidget->blockSignals(false);
  */
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSourceVolumeIntensityMaskRangeChanged(double min, double max)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }
  d->ParameterSetNode->SetSourceVolumeIntensityMaskRange(min, max);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onMaskModeChanged(int index)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }

  QString selectedSegmentId;
  if (index >= d->MaskModeComboBoxFixedItemsCount)
    {
    // specific index is selected
    d->ParameterSetNode->SetMaskSegmentID(d->MaskModeComboBox->itemData(index).toString().toUtf8());
    d->ParameterSetNode->SetMaskMode(vtkMRMLSegmentationNode::EditAllowedInsideSingleSegment);
    }
  else
    {
    // inside/outside all/visible segments
    d->ParameterSetNode->SetMaskMode(d->MaskModeComboBox->itemData(index).toInt());
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onOverwriteModeChanged(int index)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }
  d->ParameterSetNode->SetOverwriteMode(d->OverwriteModeComboBox->itemData(index).toInt());
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::segmentationNodeSelectorVisible() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->SegmentationNodeComboBox->isVisible();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setSegmentationNodeSelectorVisible(bool visible)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SegmentationNodeComboBox->setVisible(visible);
  d->SegmentationNodeLabel->setVisible(visible);
  if (visible != d->RotateWarningInNodeSelectorLayout)
    {
    d->RotateWarningInNodeSelectorLayout = visible;
    if (d->RotateWarningInNodeSelectorLayout)
      {
      d->SegmentActionsLayout->removeWidget(d->SliceRotateWarningButton);
      d->SegmentationNodeSelectorLayout->addWidget(d->SliceRotateWarningButton);
      }
    else
      {
      d->SegmentationNodeSelectorLayout->removeWidget(d->SliceRotateWarningButton);
      d->SegmentActionsLayout->addWidget(d->SliceRotateWarningButton);
      }
    }
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::sourceVolumeNodeSelectorVisible() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->SourceVolumeNodeComboBox->isVisible();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setSourceVolumeNodeSelectorVisible(bool visible)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SourceVolumeNodeComboBox->setVisible(visible);
  d->SourceVolumeNodeLabel->setVisible(visible);
  d->SpecifyGeometryButton->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::switchToSegmentationsButtonVisible() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->SwitchToSegmentationsButton->isVisible();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setSwitchToSegmentationsButtonVisible(bool visible)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SwitchToSegmentationsButton->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::undoEnabled() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->UndoRedoGroupBox->isVisible();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setUndoEnabled(bool enabled)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (enabled)
    {
    d->SegmentationHistory->RemoveAllStates();
    }
  d->UndoRedoGroupBox->setVisible(enabled);
}

//-----------------------------------------------------------------------------
int qMRMLSegmentEditorWidget::maximumNumberOfUndoStates() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->SegmentationHistory->GetMaximumNumberOfStates();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setMaximumNumberOfUndoStates(int maxNumberOfStates)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SegmentationHistory->SetMaximumNumberOfStates(maxNumberOfStates);
}

//------------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::readOnly() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->Locked;
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setReadOnly(bool aReadOnly)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->Locked = aReadOnly;
  if (aReadOnly)
    {
    setActiveEffect(nullptr);
    }
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::toggleSourceVolumeIntensityMaskEnabled()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segment editor parameter set node";
    return;
    }
  d->ParameterSetNode->SetSourceVolumeIntensityMask(
    !d->ParameterSetNode->GetSourceVolumeIntensityMask());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::undo()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->SegmentationNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(d->SegmentationNode);
  d->SegmentationHistory->RestorePreviousState();
  d->SegmentationNode->InvokeCustomModifiedEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, d->SegmentationNode->GetDisplayNode());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::redo()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->SegmentationNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(d->SegmentationNode);
  d->SegmentationHistory->RestoreNextState();
  d->SegmentationNode->InvokeCustomModifiedEvent(vtkMRMLDisplayableNode::DisplayModifiedEvent, d->SegmentationNode->GetDisplayNode());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateUndoRedoButtonsState()
{
  Q_D(qMRMLSegmentEditorWidget);
  d->UndoButton->setEnabled(!d->Locked && d->SegmentationHistory->IsRestorePreviousStateAvailable());
  d->RedoButton->setEnabled(!d->Locked && d->SegmentationHistory->IsRestoreNextStateAvailable());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSegmentationHistoryChanged()
{
  this->updateUndoRedoButtonsState();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::installKeyboardShortcuts(QWidget* parent /*=nullptr*/)
{
  Q_D(qMRMLSegmentEditorWidget);
  this->uninstallKeyboardShortcuts();

  if (parent == nullptr)
    {
    parent = qSlicerApplication::application()->mainWindow();
    }

  // Keys 1, 2, ..., 9, 0 => toggle activation of effect 1..10
  for (int effectIndex = 1; effectIndex <= 10; effectIndex++)
    {
    QShortcut* s = new QShortcut(QKeySequence(QString::number(effectIndex % 10)), parent);
    d->KeyboardShortcuts.push_back(s);
    s->setProperty("effectIndex", effectIndex);
    QObject::connect(s, SIGNAL(activated()), this, SLOT(onSelectEffectShortcut()));
    }

  // Keys Shift + 1, 2, ..., 9, 0 => toggle activation of effect 11..20
  for (int effectIndex = 1; effectIndex <= 10; effectIndex++)
    {
    QShortcut* s = new QShortcut(QKeySequence("Shift+"+QString::number(effectIndex % 10)), parent);
    d->KeyboardShortcuts.push_back(s);
    s->setProperty("effectIndex", effectIndex+10);
    QObject::connect(s, SIGNAL(activated()), this, SLOT(onSelectEffectShortcut()));
    }

  // Escape => deactivate active effect
  QShortcut* deactivateEffectShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), parent);
  d->KeyboardShortcuts.push_back(deactivateEffectShortcut);
  deactivateEffectShortcut->setProperty("effectIndex", 0);
  QObject::connect(deactivateEffectShortcut, SIGNAL(activated()), this, SLOT(onSelectEffectShortcut()));

  // Space => activate/deactivate last effect
  QShortcut* toggleActiveEffectShortcut = new QShortcut(QKeySequence(Qt::Key_Space), parent);
  d->KeyboardShortcuts.push_back(toggleActiveEffectShortcut);
  toggleActiveEffectShortcut->setProperty("effectIndex", -1);
  QObject::connect(toggleActiveEffectShortcut, SIGNAL(activated()), this, SLOT(onSelectEffectShortcut()));

  // z, y => undo, redo
  QShortcut* undoShortcut = new QShortcut(QKeySequence(Qt::Key_Z), parent);
  d->KeyboardShortcuts.push_back(undoShortcut);
  QObject::connect(undoShortcut, SIGNAL(activated()), this, SLOT(undo()));
  QShortcut* undoShortcut2 = new QShortcut(QKeySequence::Undo, parent);
  d->KeyboardShortcuts.push_back(undoShortcut2);
  QObject::connect(undoShortcut2, SIGNAL(activated()), this, SLOT(undo()));

  QShortcut* redoShortcut = new QShortcut(QKeySequence(Qt::Key_Y), parent);
  d->KeyboardShortcuts.push_back(redoShortcut);
  QObject::connect(redoShortcut, SIGNAL(activated()), this, SLOT(redo()));
  QShortcut* redoShortcut2 = new QShortcut(QKeySequence::Redo, parent);
  d->KeyboardShortcuts.push_back(redoShortcut2);
  QObject::connect(redoShortcut2, SIGNAL(activated()), this, SLOT(redo()));

  // Keys qw/*,.<> => select previous, next segment
  Qt::Key prevNexSegmentKeys[] =
    {
    Qt::Key_Q, Qt::Key_W, // near effect selector numbers on a regular keyboard
    Qt::Key_Slash, Qt::Key_Asterisk, // available on the numpad
    Qt::Key_Comma, Qt::Key_Period, // commonly used in other applications
    Qt::Key_Greater, Qt::Key_Less, // commonly used in other applications
    Qt::Key_unknown // add shortcuts above, this must be the last line
    };
  for (int keyIndex = 0; prevNexSegmentKeys[keyIndex] != Qt::Key_unknown; keyIndex++)
    {
    QShortcut* prevShortcut = new QShortcut(QKeySequence(prevNexSegmentKeys[keyIndex]), parent);
    d->KeyboardShortcuts.push_back(prevShortcut);
    prevShortcut->setProperty("segmentIndexOffset", -1);
    QObject::connect(prevShortcut, SIGNAL(activated()), this, SLOT(onSelectSegmentShortcut()));
    keyIndex++;
    QShortcut* nextShortcut = new QShortcut(QKeySequence(prevNexSegmentKeys[keyIndex]), parent);
    d->KeyboardShortcuts.push_back(nextShortcut);
    nextShortcut->setProperty("segmentIndexOffset", +1);
    QObject::connect(nextShortcut, SIGNAL(activated()), this, SLOT(onSelectSegmentShortcut()));
    }

  QShortcut* toggleSourceVolumeIntensityMaskShortcut = new QShortcut(QKeySequence(Qt::Key_I), parent);
  d->KeyboardShortcuts.push_back(toggleSourceVolumeIntensityMaskShortcut);
  QObject::connect(toggleSourceVolumeIntensityMaskShortcut, SIGNAL(activated()), this, SLOT(toggleSourceVolumeIntensityMaskEnabled()));
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::uninstallKeyboardShortcuts()
{
  Q_D(qMRMLSegmentEditorWidget);
  foreach(QShortcut* shortcut, d->KeyboardShortcuts)
    {
    shortcut->disconnect(SIGNAL(activated()));
    shortcut->setParent(nullptr);
    delete shortcut;
    }
  d->KeyboardShortcuts.clear();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSelectEffectShortcut()
{
  Q_D(qMRMLSegmentEditorWidget);
  QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
  if (shortcut == nullptr || d->Locked)
    {
    return;
    }
  qSlicerSegmentEditorAbstractEffect* activeEffect = this->activeEffect();
  int selectedEffectIndex = shortcut->property("effectIndex").toInt();
  if (selectedEffectIndex >= 0)
    {
    qSlicerSegmentEditorAbstractEffect* selectedEffect = this->effectByIndex(selectedEffectIndex);
    if (selectedEffect == activeEffect)
      {
      // effect is already active => deactivate it
      selectedEffect = nullptr;
      }
    this->setActiveEffect(selectedEffect);
    }
  else
    {
    this->setActiveEffect(d->LastActiveEffect);
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSelectSegmentShortcut()
{
  Q_D(qMRMLSegmentEditorWidget);
  QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
  if (shortcut == nullptr)
    {
    return;
    }

  // Direction that the segment selection should be moved (-1/+1 is previous/next)
  int segmentIndexOffset = shortcut->property("segmentIndexOffset").toInt();
  this->selectSegmentAtOffset (segmentIndexOffset);
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::hideLabelLayer()
{
  Q_D(qMRMLSegmentEditorWidget);
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return;
    }
  vtkCollection* sliceLogics = layoutManager->mrmlSliceLogics();
  if (!sliceLogics)
    {
    return;
    }
  vtkObject* object = nullptr;
  vtkCollectionSimpleIterator it;
  for (sliceLogics->InitTraversal(it); (object = sliceLogics->GetNextItemAsObject(it));)
    {
    vtkMRMLSliceLogic* sliceLogic = vtkMRMLSliceLogic::SafeDownCast(object);
    if (!sliceLogic)
      {
      continue;
      }
    if (!d->segmentationDisplayableInView(sliceLogic->GetSliceNode()))
      {
      continue;
      }
    vtkMRMLSliceCompositeNode* sliceCompositeNode = sliceLogic->GetSliceCompositeNode();
    if (!sliceCompositeNode)
      {
      continue;
      }
    sliceCompositeNode->SetLabelVolumeID(nullptr);
    }
}

//---------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::turnOffLightboxes()
{
  Q_D(qMRMLSegmentEditorWidget);
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    return false;
    }
  vtkCollection* sliceLogics = layoutManager->mrmlSliceLogics();
  if (!sliceLogics)
    {
    return false;
    }

  bool lightboxFound = false;
  vtkObject* object = nullptr;
  vtkCollectionSimpleIterator it;
  for (sliceLogics->InitTraversal(it); (object = sliceLogics->GetNextItemAsObject(it));)
    {
    vtkMRMLSliceLogic* sliceLogic = vtkMRMLSliceLogic::SafeDownCast(object);
    if (!sliceLogic)
      {
      continue;
      }
    vtkMRMLSliceNode* sliceNode = sliceLogic->GetSliceNode();
    if (!sliceNode)
      {
      continue;
      }
    if (!d->segmentationDisplayableInView(sliceNode))
      {
      continue;
      }
    if (sliceNode->GetLayoutGridRows() != 1 || sliceNode->GetLayoutGridColumns() != 1)
      {
      lightboxFound = true;
      sliceNode->SetLayoutGrid(1, 1);
      }
    }

  return lightboxFound;
}

//---------------------------------------------------------------------------
Qt::ToolButtonStyle qMRMLSegmentEditorWidget::effectButtonStyle() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->EffectButtonStyle;
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setEffectButtonStyle(Qt::ToolButtonStyle toolButtonStyle)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (d->EffectButtonStyle == toolButtonStyle)
    {
    return;
    }
  d->EffectButtonStyle = toolButtonStyle;
  QList<QAbstractButton*> effectButtons = d->EffectButtonGroup.buttons();
  foreach(QAbstractButton* button, effectButtons)
    {
    QToolButton* toolButton = dynamic_cast<QToolButton*>(button);
    toolButton->setToolButtonStyle(d->EffectButtonStyle);
    }
  QList<QAbstractButton*> undoRedoButtons = d->UndoRedoButtonGroup.buttons();
  foreach(QAbstractButton* button, undoRedoButtons)
    {
    QToolButton* toolButton = qobject_cast<QToolButton*>(button);
    if (toolButton)
      {
      toolButton->setToolButtonStyle(d->EffectButtonStyle);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::segmentationNodeSelectorAddAttribute(const QString& nodeType,
  const QString& attributeName, const QVariant& attributeValue/*=QVariant()*/)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SegmentationNodeComboBox->addAttribute(nodeType, attributeName, attributeValue);
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::segmentationNodeSelectorRemoveAttribute(const QString& nodeType,
  const QString& attributeName)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SegmentationNodeComboBox->removeAttribute(nodeType, attributeName);
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::sourceVolumeNodeSelectorAddAttribute(const QString& nodeType,
  const QString& attributeName, const QVariant& attributeValue/*=QVariant()*/)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SourceVolumeNodeComboBox->addAttribute(nodeType, attributeName, attributeValue);
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::sourceVolumeNodeSelectorRemoveAttribute(const QString& nodeType,
  const QString& attributeName)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SourceVolumeNodeComboBox->removeAttribute(nodeType, attributeName);
}



//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onImportExportActionClicked()
{
  Q_D(qMRMLSegmentEditorWidget);

  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return;
    }

  qSlicerAbstractModuleWidget* moduleWidget = this->switchToSegmentationsModule();
  if (!moduleWidget)
    {
    qCritical() << Q_FUNC_INFO << ": Segmentations module is not found";
    return;
    }

  // Get import/export collapsible button and uncollapse it
  ctkCollapsibleButton* collapsibleButton = moduleWidget->findChild<ctkCollapsibleButton*>("CollapsibleButton_ImportExportSegment");
  if (!collapsibleButton)
    {
    qCritical() << Q_FUNC_INFO << ": CollapsibleButton_ImportExportSegment is not found in Segmentations module";
    return;
    }
  collapsibleButton->setCollapsed(false);

  // Make sure import/export is visible
  if (moduleWidget->parent() && moduleWidget->parent()->parent() && moduleWidget->parent()->parent()->parent())
    {
    QScrollArea* segmentationsScrollArea = qobject_cast<QScrollArea*>(moduleWidget->parent()->parent()->parent());
    if (segmentationsScrollArea)
      {
      segmentationsScrollArea->ensureWidgetVisible(collapsibleButton);
      }
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onExportToFilesActionClicked()
{
  Q_D(qMRMLSegmentEditorWidget);

  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode->GetSegmentationNode();
  if (!segmentationNode)
    {
    return;
    }

  // Create dialog to show the parameters widget in a popup window
  QDialog* exportDialog = new QDialog(this, Qt::Dialog);
  exportDialog->setObjectName("SegmentationExportToFileWindow");
  exportDialog->setWindowTitle("Export segments to files");

  QVBoxLayout* layout = new QVBoxLayout(exportDialog);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  // Create file export widget to allow user editing conversion details

  qMRMLSegmentationFileExportWidget* exportToFileWidget = new qMRMLSegmentationFileExportWidget(exportDialog);
  exportToFileWidget->setMRMLScene(this->mrmlScene());
  exportToFileWidget->setSegmentationNode(d->SegmentationNode);
  exportToFileWidget->setSettingsKey("ExportSegmentsToFiles");
  layout->addWidget(exportToFileWidget);

  // Connect conversion done event to dialog close
  QObject::connect(exportToFileWidget, SIGNAL(exportToFilesDone()),
    exportDialog, SLOT(accept()));

  // Show dialog
  exportDialog->exec();

  // Delete dialog when done
  exportDialog->deleteLater();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::onSegmentationDisplayModified()
{
  Q_D(qMRMLSegmentEditorWidget);
  // Update observed views

  if (!d->ViewsObserved)
    {
    // no views are observed, nothing to do
    return;
    }

  if (!d->SegmentationNode)
    {
    return;
    }

  bool observedNodeIDsChanged = false;
  if (d->ObservedViewNodeIDs.size() == d->SegmentationNode->GetNumberOfDisplayNodes())
    {
    int numberOfDisplayNodes = d->SegmentationNode->GetNumberOfDisplayNodes();
    for (int displayNodeIndex = 0; displayNodeIndex < numberOfDisplayNodes; displayNodeIndex++)
      {
      vtkMRMLDisplayNode* segmentationDisplayNode = d->SegmentationNode->GetNthDisplayNode(displayNodeIndex);
      if (!segmentationDisplayNode)
        {
        continue;
        }
      if (!d->ObservedViewNodeIDs.contains(segmentationDisplayNode->GetID()))
        {
        observedNodeIDsChanged = true;
        break;
        }
      if (d->ObservedViewNodeIDs[segmentationDisplayNode->GetID()] != segmentationDisplayNode->GetViewNodeIDs())
        {
        observedNodeIDsChanged = true;
        break;
        }
      }
    }
  else
    {
    observedNodeIDsChanged = true;
    }
  if (!observedNodeIDsChanged)
    {
    return;
    }

  // Refresh view observations
  this->onLayoutChanged(-1);
}

//---------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::autoShowSourceVolumeNode() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->AutoShowSourceVolumeNode;
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setAutoShowSourceVolumeNode(bool autoShow)
{
  Q_D(qMRMLSegmentEditorWidget);
  if (d->AutoShowSourceVolumeNode == autoShow)
    {
    return;
    }
  d->AutoShowSourceVolumeNode = autoShow;
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::updateSliceRotateWarningButtonVisibility()
{
  Q_D(qMRMLSegmentEditorWidget);
  vtkMRMLSegmentationNode* segmentationNode = d->ParameterSetNode ? d->ParameterSetNode->GetSegmentationNode() : nullptr;
  vtkNew<vtkMatrix4x4> segmentationIJKToRAS;
  if (!segmentationNode || !segmentationNode->GetSegmentation()
    || !d->segmentationIJKToRAS(segmentationIJKToRAS.GetPointer()))
    {
    // segmentation orientation cannot be determined
    d->SliceRotateWarningButton->hide();
    return;
    }

  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    d->SliceRotateWarningButton->hide();
    return;
    }

  // Check if any of the slices are rotated
  foreach(QString sliceViewName, layoutManager->sliceViewNames())
    {
    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    if (!d->segmentationDisplayableInView(sliceWidget->mrmlSliceNode()))
      {
      continue;
      }
    vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
    if (!sliceNode)
      {
      continue;
      }
    vtkMatrix4x4* sliceToRAS = sliceNode->GetSliceToRAS();
    // Only need to check alignment of X and Y axes, if they are aligned then Z axis will be aligned, too
    for (int sliceAxisIndex = 0; sliceAxisIndex < 2; ++sliceAxisIndex)
      {
      double sliceAxisDirection[3] = {0.0};
      vtkAddonMathUtilities::GetOrientationMatrixColumn(sliceToRAS, sliceAxisIndex, sliceAxisDirection);
      bool foundParallelSegmentationAxis = false; // found a segmentation axis that is parallel to this slice axis
      for (int segmentationAxisIndex = 0; segmentationAxisIndex < 3; ++segmentationAxisIndex)
        {
        double segmentationAxisDirection[3] = {0.0};
        vtkAddonMathUtilities::GetOrientationMatrixColumn(segmentationIJKToRAS.GetPointer(), segmentationAxisIndex, segmentationAxisDirection);
        double angleDiffRad = vtkMath::AngleBetweenVectors(sliceAxisDirection, segmentationAxisDirection);
        const double maxAngleDifferenceRad = 1e-3; // we consider angles to be parallel if difference is less than about 0.1 deg
        if (angleDiffRad < maxAngleDifferenceRad || angleDiffRad > vtkMath::Pi() - maxAngleDifferenceRad)
          {
          // found a volume axis that this slice axis is parallel to
          foundParallelSegmentationAxis = true;
          break;
          }
        }
      if (!foundParallelSegmentationAxis)
        {
        d->SliceRotateWarningButton->show();
        return;
        }
      }
    }
  d->SliceRotateWarningButton->hide();
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::rotateSliceViewsToSegmentation()
{
  Q_D(qMRMLSegmentEditorWidget);
  vtkNew<vtkMatrix4x4> segmentationIJKToRAS;
  if (!d->segmentationIJKToRAS(segmentationIJKToRAS.GetPointer()))
    {
    // segmentation orientation cannot be determined
    d->SliceRotateWarningButton->hide();
    return;
    }

  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
    {
    // application is closing
    d->SliceRotateWarningButton->hide();
    return;
    }
  foreach(QString sliceViewName, layoutManager->sliceViewNames())
    {
    qMRMLSliceWidget* sliceWidget = layoutManager->sliceWidget(sliceViewName);
    if (!d->segmentationDisplayableInView(sliceWidget->mrmlSliceNode()))
      {
      continue;
      }
    vtkMRMLSliceNode* sliceNode = sliceWidget->mrmlSliceNode();
    if (!sliceNode)
      {
      continue;
      }
    sliceNode->RotateToAxes(segmentationIJKToRAS.GetPointer());
    }
  this->updateSliceRotateWarningButtonVisibility();
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::showSegmentationGeometryDialog()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (!d->SegmentationNode || !d->ParameterSetNode)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid segmentation or parameter set node";
    return;
    }

  qMRMLSegmentationGeometryDialog* geometryDialog = new qMRMLSegmentationGeometryDialog(d->SegmentationNode, this);
  geometryDialog->setEditEnabled(true);
  geometryDialog->setResampleLabelmaps(true);
  int success = geometryDialog->exec();
  geometryDialog->deleteLater();
  if (!success)
    {
    // cancel clicked
    return;
    }

  // If no source volume is selected but a valid geometry is specified then create a blank source volume
  if (!d->ParameterSetNode->GetSourceVolumeNode())
    {
    std::string referenceImageGeometry = d->getReferenceImageGeometryFromSegmentation(d->SegmentationNode->GetSegmentation());
    vtkNew<vtkMatrix4x4> referenceGeometryMatrix;
    int referenceExtent[6] = { 0,-1,0,-1,0,-1 };
    vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, referenceGeometryMatrix.GetPointer(), referenceExtent);
    if (referenceExtent[0] <= referenceExtent[1]
      && referenceExtent[2] <= referenceExtent[3]
      && referenceExtent[4] <= referenceExtent[5])
      {
      // Create new image, allocate memory
      vtkNew<vtkOrientedImageData> blankImage;
      vtkSegmentationConverter::DeserializeImageGeometry(referenceImageGeometry, blankImage.GetPointer());
      vtkOrientedImageDataResample::FillImage(blankImage.GetPointer(), 0.0);

      // Create volume node from blank image
      std::string sourceVolumeNodeName = (d->SegmentationNode->GetName() ? d->SegmentationNode->GetName() : "Volume") + std::string(" source volume");
      vtkMRMLScalarVolumeNode* sourceVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->mrmlScene()->AddNewNodeByClass("vtkMRMLScalarVolumeNode", sourceVolumeNodeName.c_str()));
      sourceVolumeNode->SetAndObserveTransformNodeID(d->SegmentationNode->GetTransformNodeID());
      vtkSlicerSegmentationsModuleLogic::CopyOrientedImageDataToVolumeNode(blankImage.GetPointer(), sourceVolumeNode);

      // Use blank volume as master
      this->setSourceVolumeNode(sourceVolumeNode);
      }
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::selectPreviousSegment()
{
  this->selectSegmentAtOffset(-1);
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::selectNextSegment()
{
  this->selectSegmentAtOffset(1);
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::selectSegmentAtOffset (int offset)
{
  Q_D(qMRMLSegmentEditorWidget);

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(this->segmentationNode());
  if (segmentationNode == nullptr || segmentationNode->GetDisplayNode() == nullptr)
    {
    return;
    }
  vtkMRMLSegmentationDisplayNode* displayNode = vtkMRMLSegmentationDisplayNode::SafeDownCast(segmentationNode->GetDisplayNode());
  if (displayNode == nullptr)
    {
    return;
    }
  std::vector<std::string> visibleSegmentIDs;
  displayNode->GetVisibleSegmentIDs(visibleSegmentIDs);
  QString currentSegmentID = this->currentSegmentID();

  QModelIndexList selectedRows = d->SegmentsTableView->tableWidget()->selectionModel()->selectedRows();
  if (selectedRows.empty())
    {
    return;
    }

  int selectedRow = selectedRows[0].row();
  int rowCount = d->SegmentsTableView->tableWidget()->model()->rowCount();
  QString selectedSegmentID = currentSegmentID;
  for (int i = 0; i < rowCount; ++i)
    {
    // 'offset' represents the direction that the segment selection should be moved (-1/+1 is previous/next)
    selectedRow = std::max(0, (selectedRow + offset) % rowCount);
    std::string segmentID = d->SegmentsTableView->segmentIDForRow(selectedRow).toStdString();
    if (std::find(visibleSegmentIDs.begin(), visibleSegmentIDs.end(), segmentID) != visibleSegmentIDs.end())
      {
      selectedSegmentID = QString::fromStdString(segmentID);
      break;
      }
    }
  this->setCurrentSegmentID(selectedSegmentID.toStdString().c_str());
}

//---------------------------------------------------------------------------
QString qMRMLSegmentEditorWidget::defaultTerminologyEntry()
{
  Q_D(qMRMLSegmentEditorWidget);
  if (d->DefaultTerminologyEntrySettingsKey.isEmpty())
    {
    return d->DefaultTerminologyEntry;
    }
  else
    {
    QSettings settings;
    return d->DefaultTerminologyEntry = settings.value(d->DefaultTerminologyEntrySettingsKey, d->DefaultTerminologyEntry).toString();
    }
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setDefaultTerminologyEntry(const QString& terminologyEntry)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->DefaultTerminologyEntry = terminologyEntry;
  if (!d->DefaultTerminologyEntrySettingsKey.isEmpty())
    {
    QSettings settings;
    settings.setValue(d->DefaultTerminologyEntrySettingsKey, d->DefaultTerminologyEntry);
    }
}

//---------------------------------------------------------------------------
QString qMRMLSegmentEditorWidget::defaultTerminologyEntrySettingsKey() const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->DefaultTerminologyEntrySettingsKey;
}

//---------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setDefaultTerminologyEntrySettingsKey(const QString& terminologyEntrySettingsKey)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->DefaultTerminologyEntrySettingsKey = terminologyEntrySettingsKey;
}

// --------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::jumpSlices()
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SegmentsTableView->jumpSlices();
}

// --------------------------------------------------------------------------
void qMRMLSegmentEditorWidget::setJumpToSelectedSegmentEnabled(bool enable)
{
  Q_D(qMRMLSegmentEditorWidget);
  d->SegmentsTableView->setJumpToSelectedSegmentEnabled(enable);
}

// --------------------------------------------------------------------------
bool qMRMLSegmentEditorWidget::jumpToSelectedSegmentEnabled()const
{
  Q_D(const qMRMLSegmentEditorWidget);
  return d->SegmentsTableView->jumpToSelectedSegmentEnabled();
}
