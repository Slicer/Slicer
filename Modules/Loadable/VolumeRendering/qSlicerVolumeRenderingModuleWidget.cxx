// Qt includes
#include <QSettings>

// CTK includes
#include <ctkUtils.h>

// qMRMLWidgets include
#include "qMRMLSceneModel.h"

// qSlicerVolumeRendering includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModule.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkSlicerVolumeRenderingLogic.h"

// MRML includes
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVector.h>
#include <vtkVolumeProperty.h>

// STD includes
#include <cassert>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingModuleWidgetPrivate
  : public Ui_qSlicerVolumeRenderingModule
{
  Q_DECLARE_PUBLIC(qSlicerVolumeRenderingModuleWidget);
protected:
  qSlicerVolumeRenderingModuleWidget* const q_ptr;

public:
  qSlicerVolumeRenderingModuleWidgetPrivate(qSlicerVolumeRenderingModuleWidget& object);
  virtual void setupUi(qSlicerVolumeRenderingModuleWidget*);
  vtkMRMLVolumeRenderingDisplayNode* createVolumeRenderingDisplayNode(
    vtkMRMLVolumeNode* volumeNode);
  void populateRenderingTechniqueComboBox();
  void populatePresetsIcons(qMRMLNodeComboBox* presetsNodeComboBox);

  vtkMRMLVolumeRenderingDisplayNode* DisplayNode;
  QMap<int, int>                     LastTechniques;
  double                             OldPresetPosition;
};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidgetPrivate
::qSlicerVolumeRenderingModuleWidgetPrivate(
  qSlicerVolumeRenderingModuleWidget& object)
  : q_ptr(&object)
{
  this->DisplayNode = 0;
  this->OldPresetPosition = 0.;
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidgetPrivate::setupUi(qSlicerVolumeRenderingModuleWidget* q)
{
  this->Ui_qSlicerVolumeRenderingModule::setupUi(q);

  QObject::connect(this->VolumeNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLVolumeNodeChanged(vtkMRMLNode*)));
  // Inputs
  QObject::connect(this->VisibilityCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onVisibilityChanged(bool)));
  QObject::connect(this->DisplayNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLDisplayNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->ROINodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLROINodeChanged(vtkMRMLNode*)));
  QObject::connect(this->VolumePropertyNodeComboBox,
                   SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->ViewCheckableNodeComboBox,
                   SIGNAL(checkedNodesChanged()),
                   q, SLOT(onCheckedViewNodesChanged()));
                   
  // Rendering
  QObject::connect(this->ROICropCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onCropToggled(bool)));
  QObject::connect(this->ROIFitPushButton,
                   SIGNAL(clicked()),
                   q, SLOT(fitROIToVolume()));

  // Techniques
  QObject::connect(this->RenderingMethodComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentRenderingMethodChanged(int)));

  QSettings settings;
  int defaultGPUMemorySize = settings.value("VolumeRendering/GPUMemorySize").toInt();
  this->MemorySizeComboBox->addItem(
    QString("Default (%1 Mo)").arg(defaultGPUMemorySize), 0);
  this->MemorySizeComboBox->insertSeparator(1);
  this->MemorySizeComboBox->addItem("128 Mo", 128);
  this->MemorySizeComboBox->addItem("256 Mo", 256);
  this->MemorySizeComboBox->addItem("512 Mo", 512);
  this->MemorySizeComboBox->addItem("1024 Mo", 1024);
  this->MemorySizeComboBox->addItem("1.5 Go", 1536);
  this->MemorySizeComboBox->addItem("2 Go", 2048);
  this->MemorySizeComboBox->addItem("3 Go", 3072);
  this->MemorySizeComboBox->addItem("4 Go", 4096);

  QObject::connect(this->MemorySizeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentMemorySizeChanged(int)));

  QObject::connect(this->QualityControlComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentQualityControlChanged(int)));
  QObject::connect(this->FramerateSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentFramerateChanged(double)));
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentRenderingTechniqueChanged(int)));
  // NCI Raycast mapper
  QObject::connect(this->DistColorBlendingSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentDistanceColorBlendingChanged(double)));
  QObject::connect(this->ICPEScaleSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentICPEScaleChanged(double)));
  QObject::connect(this->ICPESmoothnessSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentICPESmoothnessChanged(double)));
  QObject::connect(this->DepthPeelingSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentDepthPeelingThreshold(double)));
  QObject::connect(this->RenderingTechniqueFgComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentRenderingTechniqueFgChanged(int)));
  QObject::connect(this->FusionComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentFusionChanged(int)));
  QObject::connect(this->BgFgRatioSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentBgFgRatioChanged(double)));
  this->RenderingTechniqueFgComboBox->addItem(
    "Composite With Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  this->RenderingTechniqueFgComboBox->addItem(
      "Composite Pseudo Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  this->RenderingTechniqueFgComboBox->addItem(
    "Maximum Intensity Projection",
      vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection);
  this->RenderingTechniqueFgComboBox->addItem(
      "Minimum Intensity Projection",
      vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection);
  this->RenderingTechniqueFgComboBox->addItem(
    "Gradient Magnitude Opacity Modulation",
    vtkMRMLVolumeRenderingDisplayNode::GradiantMagnitudeOpacityModulation);
  this->RenderingTechniqueFgComboBox->addItem(
    "Illustrative Context Preserving Exploration",
    vtkMRMLVolumeRenderingDisplayNode::IllustrativeContextPreservingExploration);

  // Volume Properties
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());
  this->PresetsNodeComboBox->setMRMLScene(volumeRenderingLogic->GetPresetsScene());
  this->PresetsNodeComboBox->setCurrentNode(0);

  QObject::connect(this->PresetsNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(applyPreset(vtkMRMLNode*)));

  this->VolumePropertyNodeWidget->setThreshold(
    !volumeRenderingLogic->GetUseLinearRamp());
  QObject::connect(this->VolumePropertyNodeWidget,  SIGNAL(thresholdChanged(bool)),
                   q, SLOT(onThresholdChanged(bool)));
  QObject::connect(this->VolumePropertyNodeWidget,  SIGNAL(chartsExtentChanged()),
                   q, SLOT(updatePresetSliderRange()));

  QObject::connect(this->ROICropDisplayCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onROICropDisplayCheckBoxToggled(bool)));

  QObject::connect(this->PresetOffsetSlider, SIGNAL(valueChanged(double)),
                   q, SLOT(offsetPreset(double)));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(sliderPressed()),
                   q, SLOT(startInteraction()));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(valueChanged(double)),
                   q, SLOT(interaction()));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(sliderReleased()),
                   q, SLOT(endInteraction()));

  QObject::connect(this->PresetsNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(resetOffset()));
  QObject::connect(this->VolumePropertyNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(resetOffset()));

  QObject::connect(this->SynchronizeScalarDisplayNodeButton, SIGNAL(clicked()),
                   q, SLOT(synchronizeScalarDisplayNode()));
  QObject::connect(this->SynchronizeScalarDisplayNodeButton, SIGNAL(toggled(bool)),
                   q, SLOT(setFollowVolumeDisplayNode(bool)));
  QObject::connect(this->IgnoreVolumesThresholdCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setIgnoreVolumesThreshold(bool)));

  // Default values
  this->InputsCollapsibleButton->setCollapsed(true);
  this->InputsCollapsibleButton->setEnabled(false);;
  this->AdvancedCollapsibleButton->setCollapsed(true);
  this->AdvancedCollapsibleButton->setEnabled(false);

  this->ExpandSynchronizeWithVolumesButton->setChecked(false);

  this->AdvancedTabWidget->setCurrentWidget(this->VolumePropertyTab);
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidgetPrivate
::createVolumeRenderingDisplayNode(vtkMRMLVolumeNode* volumeNode)
{
  Q_Q(qSlicerVolumeRenderingModuleWidget);
  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());

  vtkMRMLVolumeRenderingDisplayNode *displayNode =
    logic->CreateVolumeRenderingDisplayNode();

  vtkMRMLVolumePropertyNode *propNode = NULL;
  vtkMRMLAnnotationROINode  *roiNode = NULL;

  int wasModifying = displayNode->StartModify();
  // Init the volume rendering without the threshold info
  // of the Volumes module...
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(1);
  logic->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode,
                                         &propNode, &roiNode);
  // ... but then apply the user settings.
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(
    this->IgnoreVolumesThresholdCheckBox->isChecked());
  bool wasLastVolumeVisible = this->VisibilityCheckBox->isChecked();
  displayNode->SetVisibility(wasLastVolumeVisible);
  foreach (vtkMRMLViewNode* viewNode, q->mrmlViewNodes())
    {
    displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
    }
  // Have at list 1 view node
  if (q->mrmlViewNodes().size() == 0)
    {
    // find the fist view node in the scene
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(
      q->mrmlScene()->GetNthNodeByClass(0, "vtkMRMLViewNode"));
    displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
    }
  displayNode->EndModify(wasModifying);
  if (volumeNode)
    {
    volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }
  return displayNode;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidgetPrivate
::populateRenderingTechniqueComboBox()
{
  Q_Q(qSlicerVolumeRenderingModuleWidget);
  this->RenderingTechniqueComboBox->clear();
  vtkMRMLVolumeRenderingDisplayNode* displayNode = q->mrmlDisplayNode();
  int volumeMapper = displayNode ?
    displayNode->GetCurrentVolumeMapper() : vtkMRMLVolumeRenderingDisplayNode::None;
  if (volumeMapper == vtkMRMLVolumeRenderingDisplayNode::None)
    {
    return;
    }
  this->RenderingTechniqueComboBox->addItem(
    "Composite With Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  if (volumeMapper == vtkMRMLVolumeRenderingDisplayNode::NCIGPURayCast ||
      volumeMapper == vtkMRMLVolumeRenderingDisplayNode::NCIGPURayCastMultiVolume)
    {
    this->RenderingTechniqueComboBox->addItem(
      "Composite Pseudo Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
    }
  if (volumeMapper != vtkMRMLVolumeRenderingDisplayNode::VTKGPUTextureMapping)
    {
    this->RenderingTechniqueComboBox->addItem(
      "Maximum Intensity Projection",
      vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection);
    this->RenderingTechniqueComboBox->addItem(
      "Minimum Intensity Projection",
      vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection);
    }
  if (volumeMapper == vtkMRMLVolumeRenderingDisplayNode::NCIGPURayCast ||
      volumeMapper == vtkMRMLVolumeRenderingDisplayNode::NCIGPURayCastMultiVolume)
    {
    this->RenderingTechniqueComboBox->addItem(
      "Gradient Magnitude Opacity Modulation",
      vtkMRMLVolumeRenderingDisplayNode::GradiantMagnitudeOpacityModulation);
    this->RenderingTechniqueComboBox->addItem(
      "Illustrative Context Preserving Exploration",
      vtkMRMLVolumeRenderingDisplayNode::IllustrativeContextPreservingExploration);
    }
  this->RenderingTechniqueComboBox->setEnabled(
    volumeMapper != vtkMRMLVolumeRenderingDisplayNode::VTKGPUTextureMapping);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidgetPrivate
::populatePresetsIcons(qMRMLNodeComboBox* presetsNodeComboBox)
{
  // This is a hack and doesn't work yet
  for (int i = 0; i < presetsNodeComboBox->nodeCount(); ++i)
    {
    vtkMRMLNode* presetNode = presetsNodeComboBox->nodeFromIndex(i);
    QIcon presetIcon(QString(":/presets/") + presetNode->GetName());
    //QIcon presetIcon(":/Icons/VisibleOff.png");
    if (!presetIcon.isNull())
      {
      qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(presetsNodeComboBox->sortFilterProxyModel()->sourceModel());
      sceneModel->setData(sceneModel->indexFromNode(presetNode), presetIcon, Qt::DecorationRole);
      }
    }
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget
::qSlicerVolumeRenderingModuleWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
    , d_ptr( new qSlicerVolumeRenderingModuleWidgetPrivate(*this) )
{
  // setup the UI only in setup where the logic is available
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::~qSlicerVolumeRenderingModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setup()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qSlicerVolumeRenderingModuleWidget
::mrmlVolumeNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLScalarVolumeNode::SafeDownCast(
    d->VolumeNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumeNode(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->VolumeNodeComboBox->setCurrentNode(volumeNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMRMLVolumeNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLScalarVolumeNode* volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);

  if (!volumeNode)
    {
    this->setMRMLDisplayNode(0);
    return;
    }

  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());

  // see if the volume has any display node for a current viewer
  vtkMRMLVolumeRenderingDisplayNode *dnode =
    logic->GetFirstVolumeRenderingDisplayNode(volumeNode);
  if (!this->mrmlScene()->IsClosing())
    {
    if (!dnode)
      {
      dnode = d->createVolumeRenderingDisplayNode(volumeNode);
      }
    else
      {
      // Because the displayable manager can only display 1 volume at
      // a time, here the displayable manager is told that the display node
      // is the new "current" display node and it should be displayed 
      // instead of whichever current one.
      dnode->Modified();
      }
    }

  // DepthPeelingThresholdSliderWidget depends on the scalar range of the volume
  // Set the range here before the display node is set and the
  // DepthPeelingThreshold slider value updated.
  vtkImageData* imageData = volumeNode ? volumeNode->GetImageData() : 0;
  if (imageData)
    {
    double range[2];
    imageData->GetScalarRange(range);
    bool oldBlockSignals =
      d->DepthPeelingSliderWidget->blockSignals(true);
    d->DepthPeelingSliderWidget->setRange(range[0], range[1]);
    d->DepthPeelingSliderWidget->blockSignals(oldBlockSignals);
    }

  this->setMRMLDisplayNode(dnode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onVisibilityChanged(bool visible)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  d->DisplayNode->SetVisibility(visible);
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidget
::mrmlDisplayNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
    d->DisplayNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLDisplayNode(vtkMRMLNode* displayNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->DisplayNodeComboBox->setCurrentNode(displayNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::onCurrentMRMLDisplayNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLVolumeRenderingDisplayNode* displayNode =
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);

  // update view node references
  vtkMRMLScalarVolumeNode* volumeNode = this->mrmlVolumeNode();

  // if display node is not referenced by current volume, add the refrence
  if (volumeNode && displayNode)
    {
    vtkSlicerVolumeRenderingLogic *logic =
      vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
    vtkMRMLVolumeRenderingDisplayNode* dnode =
      logic->GetVolumeRenderingDisplayNodeByID(volumeNode,
                                               displayNode->GetID());
    if (dnode != displayNode)
      {
      volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
      }
    }

  this->qvtkReconnect(d->DisplayNode, displayNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateFromMRMLDisplayNode()));

  d->DisplayNode = displayNode;

  this->updateFromMRMLDisplayNode();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::updateFromMRMLDisplayNode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  // We don't want to update ViewCheckableNodeComboBox if it's checked nodes
  // are good. Otherwise it would lead to an inconsistent state.
  if (d->DisplayNode &&
      !((d->DisplayNode->GetNumberOfViewNodeIDs() == 0 ||
         d->DisplayNode->GetNumberOfViewNodeIDs() == d->ViewCheckableNodeComboBox->nodeCount())&&
        (d->ViewCheckableNodeComboBox->allChecked() ||
         d->ViewCheckableNodeComboBox->noneChecked())))
    {
    for (int i = 0; i < d->ViewCheckableNodeComboBox->nodeCount(); ++i)
      {
      vtkMRMLNode* view = d->ViewCheckableNodeComboBox->nodeFromIndex(i);
      Q_ASSERT(view);
      d->ViewCheckableNodeComboBox->setCheckState(
        view,
        d->DisplayNode && d->DisplayNode->IsViewNodeIDPresent(view->GetID()) ? Qt::Checked : Qt::Unchecked);
      }
    }

  // set display node from current GUI state
  this->setMRMLVolumePropertyNode(
    d->DisplayNode ? d->DisplayNode->GetVolumePropertyNode() : 0);
  this->setMRMLROINode(d->DisplayNode ? d->DisplayNode->GetROINode() : 0);
  d->VisibilityCheckBox->setChecked(
    d->DisplayNode ? d->DisplayNode->GetVisibility() : false);
  d->ROICropCheckBox->setChecked(
    d->DisplayNode ? d->DisplayNode->GetCroppingEnabled() : false);

  // Techniques tab
  QSettings settings;
  int defaultRenderingMethod =
    settings.value("VolumeRendering/RenderingMethod",
                   vtkMRMLVolumeRenderingDisplayNode::VTKCPURayCast).toInt();
  int currentVolumeMapper = d->DisplayNode ?
    d->DisplayNode->GetCurrentVolumeMapper() : defaultRenderingMethod;
  d->RenderingMethodComboBox->setCurrentIndex( currentVolumeMapper );
  int index = d->DisplayNode ?
    d->MemorySizeComboBox->findData(QVariant(d->DisplayNode->GetGPUMemorySize())) : -1;
  d->MemorySizeComboBox->setCurrentIndex(index);
  d->QualityControlComboBox->setCurrentIndex(
    d->DisplayNode ? d->DisplayNode->GetPerformanceControl() : -1);
  if (d->DisplayNode)
    {
    d->FramerateSliderWidget->setValue(d->DisplayNode->GetExpectedFPS());
    }
  // Rendering technique
  d->RenderingTechniqueComboBox->blockSignals(true);
  d->populateRenderingTechniqueComboBox();
  int technique = d->DisplayNode ? d->DisplayNode->GetRaycastTechnique() : -1;
  index = d->RenderingTechniqueComboBox->findData(QVariant(technique));
  if (index == -1 && d->LastTechniques.contains(currentVolumeMapper))
    {
    index = d->RenderingTechniqueComboBox->findData(
      d->LastTechniques[currentVolumeMapper]);
    }
  if (index == -1)
    {
    index = 0;
    }
  d->RenderingTechniqueComboBox->setCurrentIndex(-1);
  d->RenderingTechniqueComboBox->blockSignals(false);
  d->RenderingTechniqueComboBox->setCurrentIndex(index);

  // NCI Raycast mapper
  d->DistColorBlendingSliderWidget->setValue(
    d->DisplayNode ? d->DisplayNode->GetDistanceColorBlending() : 0.);
  d->ICPEScaleSliderWidget->setValue(
    d->DisplayNode ? d->DisplayNode->GetICPEScale() : 0.);
  d->ICPESmoothnessSliderWidget->setValue(
    d->DisplayNode ? d->DisplayNode->GetICPESmoothness() : 0.);
  d->DepthPeelingSliderWidget->setValue(
    d->DisplayNode ? d->DisplayNode->GetDepthPeelingThreshold() : 0.);
  technique = d->DisplayNode ? d->DisplayNode->GetRaycastTechniqueFg() : -1;
  index = d->RenderingTechniqueFgComboBox->findData(QVariant(technique));
  d->RenderingTechniqueFgComboBox->setCurrentIndex(index);
  d->FusionComboBox->setCurrentIndex(
    d->DisplayNode ? d->DisplayNode->GetMultiVolumeFusionMethod() :
    vtkMRMLVolumeRenderingDisplayNode::AlphaBlendingOR);
  d->BgFgRatioSliderWidget->setValue(
    d->DisplayNode ? d->DisplayNode->GetBgFgRatio() : 0.);

  // Opacity/color
  bool follow = d->DisplayNode ? d->DisplayNode->GetFollowVolumeDisplayNode() != 0 : false;
  if (follow)
    {
    d->SynchronizeScalarDisplayNodeButton->setCheckState(Qt::Checked);
    }
  d->SynchronizeScalarDisplayNodeButton->setChecked(follow);
  d->IgnoreVolumesThresholdCheckBox->setChecked( d->DisplayNode ?
    d->DisplayNode->GetIgnoreVolumeDisplayNodeThreshold() != 0 : false);
}

// --------------------------------------------------------------------------
QList<vtkMRMLViewNode*> qSlicerVolumeRenderingModuleWidget::mrmlViewNodes()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  QList<vtkMRMLViewNode*> res;
  foreach(vtkMRMLNode* checkedNode, d->ViewCheckableNodeComboBox->checkedNodes())
    {
    res << vtkMRMLViewNode::SafeDownCast(checkedNode);
    }
  return res;
}
// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::addVolumeIntoView(vtkMRMLNode* viewNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->ViewCheckableNodeComboBox->check(viewNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCheckedViewNodesChanged()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  // set view in the currently selected display node
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  int wasModifying = displayNode->StartModify();

  displayNode->RemoveAllViewNodeIDs();
  if (!d->ViewCheckableNodeComboBox->allChecked() &&
      !d->ViewCheckableNodeComboBox->noneChecked())
    {
    foreach (vtkMRMLViewNode* viewNode, this->mrmlViewNodes())
      {
      displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : 0);
      }
    }

  displayNode->EndModify(wasModifying);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCropToggled(bool crop)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  d->DisplayNode->SetCroppingEnabled(crop);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::fitROIToVolume()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic())
    ->FitROIToVolume(d->DisplayNode);

  Q_ASSERT(d->ROIWidget->mrmlROINode() == this->mrmlROINode());
  Q_ASSERT(d->ROIWidget->mrmlROINode() == d->DisplayNode->GetROINode());

  if (d->ROIWidget->mrmlROINode())
    {
    double xyz[3];
    double rxyz[3];

    d->ROIWidget->mrmlROINode()->GetXYZ(xyz);
    d->ROIWidget->mrmlROINode()->GetRadiusXYZ(rxyz);

    double bounds[6];
    for (int i=0; i < 3; ++i)
      {
      bounds[i]   = xyz[i]-rxyz[i];
      bounds[3+i] = xyz[i]+rxyz[i];
      }
    d->ROIWidget->setExtent(bounds[0], bounds[3],
                            bounds[1], bounds[4],
                            bounds[2], bounds[5]);
    }
}

// --------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* qSlicerVolumeRenderingModuleWidget
::mrmlVolumePropertyNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLVolumePropertyNode::SafeDownCast(
    d->VolumePropertyNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::setMRMLVolumePropertyNode(vtkMRMLNode* volumePropertyNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  // Set if not already set
  d->VolumePropertyNodeComboBox->setCurrentNode(volumePropertyNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode* volumePropertyNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  d->DisplayNode->SetAndObserveVolumePropertyNodeID(
    volumePropertyNode ? volumePropertyNode->GetID() : 0);
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationROINode* qSlicerVolumeRenderingModuleWidget
::mrmlROINode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLAnnotationROINode::SafeDownCast(
    d->ROINodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLROINode(vtkMRMLNode* roiNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->ROINodeComboBox->setCurrentNode(roiNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMRMLROINodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  vtkMRMLAnnotationROINode *roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);
  d->DisplayNode->SetAndObserveROINodeID(roiNode ? roiNode->GetID() : 0);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentRenderingMethodChanged(int index)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  d->DisplayNode->SetCurrentVolumeMapper(index);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMemorySizeChanged(int index)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  int gpuMemorySize = d->MemorySizeComboBox->itemData(index).toInt();
  Q_ASSERT(gpuMemorySize >= 0 && gpuMemorySize < 10000);
  d->DisplayNode->SetGPUMemorySize(gpuMemorySize);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentQualityControlChanged(int index)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  displayNode->SetPerformanceControl(index);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentFramerateChanged(double fps)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  d->DisplayNode->SetExpectedFPS(fps);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentRenderingTechniqueChanged(int index)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  int technique = d->RenderingTechniqueComboBox->itemData(index).toInt();
  d->LastTechniques[d->DisplayNode->GetCurrentVolumeMapper()] = technique;
  d->DisplayNode->SetRaycastTechnique(technique);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentDistanceColorBlendingChanged(double value)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  d->DisplayNode->SetDistanceColorBlending(value);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentICPEScaleChanged(double value)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  d->DisplayNode->SetICPEScale(value);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentICPESmoothnessChanged(double value)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  d->DisplayNode->SetICPESmoothness(value);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentDepthPeelingThreshold(double value)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  d->DisplayNode->SetDepthPeelingThreshold(value);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentRenderingTechniqueFgChanged(int index)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  int technique = d->RenderingTechniqueFgComboBox->itemData(index).toInt();
  d->DisplayNode->SetRaycastTechniqueFg(technique);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentFusionChanged(int index)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  d->DisplayNode->SetMultiVolumeFusionMethod(index);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentBgFgRatioChanged(double value)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }

  d->DisplayNode->SetBgFgRatio(value);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::applyPreset(vtkMRMLNode* node)
{
  vtkMRMLVolumePropertyNode* presetNode =
    vtkMRMLVolumePropertyNode::SafeDownCast(node);
  vtkMRMLVolumePropertyNode* volumePropertyNode =
    this->mrmlVolumePropertyNode();
  if (!presetNode || !volumePropertyNode)
    {
    return;
    }
  assert(presetNode->GetVolumeProperty());
  assert(presetNode->GetVolumeProperty()->GetRGBTransferFunction());
  assert(presetNode->GetVolumeProperty()->GetRGBTransferFunction()->GetRange()[1] >
         presetNode->GetVolumeProperty()->GetRGBTransferFunction()->GetRange()[0]);
  volumePropertyNode->Copy(presetNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::startInteraction()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkVolumeProperty* volumeProperty =
    d->VolumePropertyNodeWidget->volumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::StartInteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::endInteraction()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkVolumeProperty* volumeProperty =
    d->VolumePropertyNodeWidget->volumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::EndInteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::interaction()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkVolumeProperty* volumeProperty =
    d->VolumePropertyNodeWidget->volumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::InteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::offsetPreset(double newPosition)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->VolumePropertyNodeWidget->moveAllPoints(
    newPosition - d->OldPresetPosition, 0., true);
  d->OldPresetPosition = newPosition;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::resetOffset()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  // Reset the slider position to the center.
  d->OldPresetPosition = 0.;
  d->PresetOffsetSlider->setValue(0.);
  this->updatePresetSliderRange();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::updatePresetSliderRange()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->VolumePropertyNodeWidget->volumeProperty())
    {
    return;
    }
  double extent[4];
  d->VolumePropertyNodeWidget->chartsExtent(extent);
  double width = extent[1] - extent[0];
  bool wasBlocking = d->PresetOffsetSlider->blockSignals(true);
  d->PresetOffsetSlider->setSingleStep(
    width ? ctk::closestPowerOfTen(width) / 100. : 0.1);
  d->PresetOffsetSlider->setPageStep(d->PresetOffsetSlider->singleStep());
  d->PresetOffsetSlider->setRange(-width, width);
  d->PresetOffsetSlider->blockSignals(wasBlocking);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::synchronizeScalarDisplayNode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->CopyScalarDisplayToVolumeRenderingDisplayNode(
    d->DisplayNode);
}


// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setFollowVolumeDisplayNode(bool follow)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->DisplayNode->SetFollowVolumeDisplayNode(follow ? 1 : 0);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setIgnoreVolumesThreshold(bool ignore)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->DisplayNode->SetIgnoreVolumeDisplayNodeThreshold(ignore ? 1 : 0);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onThresholdChanged(bool threshold)
{
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->SetUseLinearRamp(!threshold);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget
::onROICropDisplayCheckBoxToggled(bool toggle)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  // When the display box is visible, it should probably activate the
  // cropping (to follow the "what you see is what you get" pattern).
  if (toggle)
    {
    d->ROICropCheckBox->setChecked(true);
    }
}
