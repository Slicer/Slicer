/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Yarmakovich, Isomics Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// qSlicerVolumeRendering includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModuleWidget.h"

#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkSlicerVolumeRenderingLogic.h"

#include "qSlicerCPURayCastVolumeRenderingPropertiesWidget.h"
#include "qSlicerGPURayCastVolumeRenderingPropertiesWidget.h"
#include "qSlicerMultiVolumeRenderingPropertiesWidget.h"
#include "qSlicerVolumeRenderingPresetComboBox.h"
#include "qSlicerGPUMemoryComboBox.h"

// MRML includes
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLMarkupsROINode.h"

// VTK includes
#include <vtkVolumeProperty.h>

// STD includes
#include <vector>

// Qt includes
#include <QDebug>
#include <QSettings>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingModuleWidgetPrivate
  : public Ui_qSlicerVolumeRenderingModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerVolumeRenderingModuleWidget);
protected:
  qSlicerVolumeRenderingModuleWidget* const q_ptr;

public:
  qSlicerVolumeRenderingModuleWidgetPrivate(qSlicerVolumeRenderingModuleWidget& object);
  virtual ~qSlicerVolumeRenderingModuleWidgetPrivate();

  virtual void setupUi(qSlicerVolumeRenderingModuleWidget*);
  vtkMRMLVolumeRenderingDisplayNode* displayNodeForVolumeNode(vtkMRMLVolumeNode* volumeNode)const;
  vtkMRMLVolumeRenderingDisplayNode* createVolumeRenderingDisplayNode(vtkMRMLVolumeNode* volumeNode);

  QMap<int, int>                     LastTechniques;
  double                             OldPresetPosition;
  QMap<QString, QWidget*>            RenderingMethodWidgets;
  vtkWeakPointer<vtkMRMLDisplayableNode> CropROINode;
  vtkWeakPointer<vtkMRMLVolumeRenderingDisplayNode> VolumeRenderingDisplayNode;
  vtkWeakPointer<vtkMRMLVolumePropertyNode> VolumePropertyNode;
};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidgetPrivate::qSlicerVolumeRenderingModuleWidgetPrivate(qSlicerVolumeRenderingModuleWidget& object)
  : q_ptr(&object)
  , OldPresetPosition(0.0)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidgetPrivate::~qSlicerVolumeRenderingModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidgetPrivate::setupUi(qSlicerVolumeRenderingModuleWidget* q)
{
  this->Ui_qSlicerVolumeRenderingModuleWidget::setupUi(q);

  QObject::connect(this->VolumeNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLVolumeNodeChanged(vtkMRMLNode*)));
  // Inputs
  QObject::connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onVisibilityChanged(bool)));
  QObject::connect(this->ROINodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLROINodeChanged(vtkMRMLNode*)));
  QObject::connect(this->VolumePropertyNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode*)));

  // Rendering
  QObject::connect(this->ROICropCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onCropToggled(bool)));
  QObject::connect(this->ROICropDisplayCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onROICropDisplayCheckBoxToggled(bool)));
  QObject::connect(this->ROIFitPushButton, SIGNAL(clicked()),
                   q, SLOT(fitROIToVolume()));

  // Techniques
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic = vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());
  std::map<std::string, std::string> methods = volumeRenderingLogic->GetRenderingMethods();
  std::map<std::string, std::string>::const_iterator it;
  for (it = methods.begin(); it != methods.end(); ++it)
    {
    this->RenderingMethodComboBox->addItem(QString::fromStdString(it->first), QString::fromStdString(it->second));
    }
  QObject::connect(this->RenderingMethodComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentRenderingMethodChanged(int)));
  // Add empty widget at index 0 for the volume rendering methods with no widget.
  this->RenderingMethodStackedWidget->addWidget(new QWidget());
  q->addRenderingMethodWidget("vtkMRMLCPURayCastVolumeRenderingDisplayNode",
                              new qSlicerCPURayCastVolumeRenderingPropertiesWidget);
  q->addRenderingMethodWidget("vtkMRMLGPURayCastVolumeRenderingDisplayNode",
                              new qSlicerGPURayCastVolumeRenderingPropertiesWidget);
  q->addRenderingMethodWidget("vtkMRMLMultiVolumeRenderingDisplayNode",
                              new qSlicerMultiVolumeRenderingPropertiesWidget);

  // Currently, VTK ignores GPU memory size request - hide it on the GUI to not confuse users
  this->MemorySizeLabel->hide();
  this->MemorySizeComboBox->hide();

  QObject::connect(this->MemorySizeComboBox, SIGNAL(editTextChanged(QString)),
                   q, SLOT(onCurrentMemorySizeChanged()));
  QObject::connect(this->MemorySizeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentMemorySizeChanged()));

  for (int qualityIndex=0; qualityIndex<vtkMRMLViewNode::VolumeRenderingQuality_Last; qualityIndex++)
    {
    this->QualityControlComboBox->addItem(vtkMRMLViewNode::GetVolumeRenderingQualityAsString(qualityIndex));
    }
  QObject::connect(this->QualityControlComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentQualityControlChanged(int)));

  QObject::connect(this->FramerateSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onCurrentFramerateChanged(double)));

  QObject::connect(this->AutoReleaseGraphicsResourcesCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(onAutoReleaseGraphicsResourcesCheckBoxToggled(bool)));

  void onAutoReleaseGraphicsResourcesChanged(bool autoRelease);

  // Volume Properties
  this->PresetComboBox->setMRMLScene(volumeRenderingLogic->GetPresetsScene());
  this->PresetComboBox->setCurrentNode(nullptr);

  QObject::connect(this->PresetComboBox, SIGNAL(presetOffsetChanged(double, double, bool)),
                   this->VolumePropertyNodeWidget, SLOT(moveAllPoints(double, double, bool)));

  this->VolumePropertyNodeWidget->setThreshold(!volumeRenderingLogic->GetUseLinearRamp());
  QObject::connect(this->VolumePropertyNodeWidget, SIGNAL(thresholdChanged(bool)),
                   q, SLOT(onThresholdChanged(bool)));
  QObject::connect(this->VolumePropertyNodeWidget, SIGNAL(chartsExtentChanged()),
                   q, SLOT(onChartsExtentChanged()));

  QObject::connect(this->VolumePropertyNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this->PresetComboBox, SLOT(setMRMLVolumePropertyNode(vtkMRMLNode*)));

  QObject::connect(this->SynchronizeScalarDisplayNodeButton, SIGNAL(clicked()),
                   q, SLOT(synchronizeScalarDisplayNode()));
  QObject::connect(this->SynchronizeScalarDisplayNodeButton, SIGNAL(toggled(bool)),
                   q, SLOT(setFollowVolumeDisplayNode(bool)));
  QObject::connect(this->IgnoreVolumesThresholdCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setIgnoreVolumesThreshold(bool)));

  // Disable markups ROI widget by default
  this->MarkupsROIWidget->setVisible(false);

  // Default values
  this->InputsCollapsibleButton->setCollapsed(true);
  this->InputsCollapsibleButton->setEnabled(false);;
  this->AdvancedCollapsibleButton->setCollapsed(true);
  this->AdvancedCollapsibleButton->setEnabled(false);

  this->ExpandSynchronizeWithVolumesButton->setChecked(false);

  this->AdvancedTabWidget->setCurrentWidget(this->VolumePropertyTab);

  // Ensure that the view node combo box only shows view nodes, not slice nodes or chart nodes
  this->ViewCheckableNodeComboBox->setNodeTypes(QStringList(QString("vtkMRMLViewNode")));
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidgetPrivate::displayNodeForVolumeNode(vtkMRMLVolumeNode* volumeNode)const
{
  if (!volumeNode)
    {
    return nullptr;
    }

  Q_Q(const qSlicerVolumeRenderingModuleWidget);
  vtkSlicerVolumeRenderingLogic *logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
    return nullptr;
    }

  // Get volume rendering display node for volume
  return logic->GetFirstVolumeRenderingDisplayNode(volumeNode);
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidgetPrivate::createVolumeRenderingDisplayNode(
  vtkMRMLVolumeNode* volumeNode)
{
  Q_Q(qSlicerVolumeRenderingModuleWidget);

  vtkSlicerVolumeRenderingLogic *logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
    return nullptr;
    }

  vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode> displayNode =
    vtkSmartPointer<vtkMRMLVolumeRenderingDisplayNode>::Take(logic->CreateVolumeRenderingDisplayNode());
  displayNode->SetVisibility(0);
  q->mrmlScene()->AddNode(displayNode);

  if (volumeNode)
    {
    volumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
    }

  int wasModifying = displayNode->StartModify();
  // Initialize volume rendering without the threshold info of the Volumes module
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(1);
  logic->UpdateDisplayNodeFromVolumeNode(displayNode, volumeNode, nullptr, nullptr, false /*do not create ROI*/);
  // Apply previous selection to the newly selected volume
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(this->IgnoreVolumesThresholdCheckBox->isChecked());
  // Do not show newly selected volume (because it would be triggered by simply selecting it in the combobox,
  // and it would not adhere to the customary Slicer behavior)
  // Set selected views to the display node
  foreach (vtkMRMLAbstractViewNode* viewNode, this->ViewCheckableNodeComboBox->checkedViewNodes())
    {
    displayNode->AddViewNodeID(viewNode ? viewNode->GetID() : nullptr);
    }
  displayNode->EndModify(wasModifying);
  return displayNode;
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::qSlicerVolumeRenderingModuleWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerVolumeRenderingModuleWidgetPrivate(*this) )
{
  // setup the UI only in setup where the logic is available
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModuleWidget::~qSlicerVolumeRenderingModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setup()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
vtkMRMLVolumeNode* qSlicerVolumeRenderingModuleWidget::mrmlVolumeNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLVolumeNode::SafeDownCast(d->VolumeNodeComboBox->currentNode());
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

  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(node);

  vtkSlicerVolumeRenderingLogic* logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  if (!logic)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
    return;
    }

  vtkMRMLVolumeRenderingDisplayNode* displayNode = d->displayNodeForVolumeNode(volumeNode);
  if (!displayNode && volumeNode)
    {
    displayNode = d->createVolumeRenderingDisplayNode(volumeNode);
    }

  qvtkReconnect(d->VolumeRenderingDisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->VolumeRenderingDisplayNode = displayNode;

  d->ViewCheckableNodeComboBox->setMRMLDisplayNode(displayNode);

  // Select preset node that was previously selected for this volume
  vtkMRMLVolumePropertyNode* volumePropertyNode = this->mrmlVolumePropertyNode();
  if (volumePropertyNode)
    {
    vtkMRMLVolumePropertyNode* presetNode = logic->GetPresetByName(volumePropertyNode->GetName());
    bool wasBlocking = d->PresetComboBox->blockSignals(true);
    d->PresetComboBox->setCurrentNode(presetNode);
    d->PresetComboBox->blockSignals(wasBlocking);
    }

  // Update widget from display node of the volume node
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onVisibilityChanged(bool visible)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  // Get volume rendering display node for volume. Create if absent.
  vtkMRMLVolumeNode* volumeNode = this->mrmlVolumeNode();
  vtkMRMLVolumeRenderingDisplayNode* displayNode = d->displayNodeForVolumeNode(volumeNode);
  if (!displayNode)
    {
    if (volumeNode)
      {
      qCritical() << Q_FUNC_INFO << ": No volume rendering display node for volume " << volumeNode->GetName();
      }
    return;
    }

  displayNode->SetVisibility(visible);

  // Update widget from display node of the volume node
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::addRenderingMethodWidget(
  const QString& methodClassName, qSlicerVolumeRenderingPropertiesWidget* widget)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->RenderingMethodStackedWidget->addWidget(widget);
  d->RenderingMethodWidgets[methodClassName] = widget;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  // Get display node
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();

  // Get first view node
  vtkMRMLViewNode* firstViewNode = nullptr;
  if (displayNode && displayNode->GetScene())
    {
    firstViewNode = displayNode->GetFirstViewNode();
    }

  // Visibility checkbox
  d->VisibilityCheckBox->setChecked(displayNode ? displayNode->GetVisibility() : false);

  // Input section

  // Volume property selector
  // Update shift slider range and set transfer function extents when volume property node is modified
  vtkMRMLVolumePropertyNode* volumePropertyNode = (displayNode ? displayNode->GetVolumePropertyNode() : nullptr);
  this->qvtkReconnect(d->VolumePropertyNode, volumePropertyNode, vtkMRMLVolumePropertyNode::EffectiveRangeModified, this, SLOT(onEffectiveRangeModified()));
  bool volumePropertyNodeChanged = (d->VolumePropertyNode != volumePropertyNode);
  d->VolumePropertyNode = volumePropertyNode;
  if (volumePropertyNodeChanged)
    {
    // Perform widget updates
    this->onEffectiveRangeModified();
    }
  d->VolumePropertyNodeComboBox->setCurrentNode(volumePropertyNode);

  // ROI selector
  vtkMRMLAnnotationROINode* annotationROINode = (displayNode ? displayNode->GetAnnotationROINode() : nullptr);
  vtkMRMLMarkupsROINode* markupsROINode = (displayNode ? displayNode->GetMarkupsROINode() : nullptr);
  vtkMRMLDisplayableNode* roiNode = markupsROINode;
  if (!roiNode)
    {
    roiNode = annotationROINode;
    }
  this->qvtkReconnect(d->CropROINode, roiNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateWidgetFromROINode()));
  d->CropROINode = roiNode;
  this->updateWidgetFromROINode();

  bool wasBlocking = d->ROINodeComboBox->blockSignals(true);
  d->ROINodeComboBox->setCurrentNode(roiNode);
  d->ROINodeComboBox->blockSignals(wasBlocking);

  // Disable UI if there is no display node yet (need to show it first to have a display node)
  d->DisplayCollapsibleButton->setEnabled(displayNode != nullptr);
  d->AdvancedCollapsibleButton->setEnabled(displayNode != nullptr);

  // Display section
  d->PresetComboBox->setEnabled(volumePropertyNode != nullptr);
  wasBlocking = d->PresetComboBox->blockSignals(true);
  d->PresetComboBox->setCurrentNode(
    volumePropertyNode ? vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic())->GetPresetByName(volumePropertyNode->GetName()) : nullptr );
  d->PresetComboBox->blockSignals(wasBlocking);
  d->ROICropCheckBox->setChecked(roiNode && displayNode ? displayNode->GetCroppingEnabled() : false);
  d->ROICropCheckBox->setEnabled(displayNode != nullptr); // ROI can be created on request if display node is set
  d->ROICropDisplayCheckBox->setEnabled(displayNode != nullptr); // ROI can be created on request if display node is set
  d->ROIFitPushButton->setEnabled(roiNode != nullptr);
  d->RenderingMethodComboBox->setEnabled(displayNode != nullptr);

  // Advanced section

  // Volume properties tab
  d->VolumePropertyNodeWidget->setEnabled(volumePropertyNode != nullptr);

  // ROI tab
  d->MarkupsROIWidget->setMRMLMarkupsNode(markupsROINode);
  d->AnnotationROIWidget->setMRMLAnnotationROINode(annotationROINode);
  if (markupsROINode && !annotationROINode)
    {
    d->MarkupsROIWidget->setVisible(true);
    d->AnnotationROIWidget->setVisible(false);
    }
  else if (annotationROINode && !markupsROINode)
    {
    d->AnnotationROIWidget->setVisible(true);
    d->MarkupsROIWidget->setVisible(false);
    }

  // Techniques tab
  QSettings settings;
  QString defaultRenderingMethod =
    settings.value("VolumeRendering/RenderingMethod", QString("vtkMRMLGPURayCastVolumeRenderingDisplayNode")).toString();
  QString currentRenderingMethod = displayNode ? QString(displayNode->GetClassName()) : defaultRenderingMethod;
  d->RenderingMethodComboBox->setCurrentIndex(d->RenderingMethodComboBox->findData(currentRenderingMethod) );
  d->MemorySizeComboBox->setCurrentGPUMemory(firstViewNode ? firstViewNode->GetGPUMemorySize() : 0);
  d->QualityControlComboBox->setCurrentIndex(firstViewNode ? firstViewNode->GetVolumeRenderingQuality() : -1);
  d->AutoReleaseGraphicsResourcesCheckBox->setChecked(firstViewNode ? firstViewNode->GetAutoReleaseGraphicsResources() : false);

  if (firstViewNode)
    {
    d->FramerateSliderWidget->setValue(firstViewNode->GetExpectedFPS());
    }
  d->FramerateSliderWidget->setEnabled(
    firstViewNode && firstViewNode->GetVolumeRenderingQuality() == vtkMRMLViewNode::Adaptive );
  // Advanced rendering properties
  if (d->RenderingMethodWidgets[currentRenderingMethod])
    {
    qSlicerVolumeRenderingPropertiesWidget* renderingMethodWidget =
      qobject_cast<qSlicerVolumeRenderingPropertiesWidget*>(d->RenderingMethodWidgets[currentRenderingMethod]);
    renderingMethodWidget->setMRMLNode(displayNode);
    d->RenderingMethodStackedWidget->setCurrentWidget(renderingMethodWidget);
    }
  else
    {
    qWarning() << Q_FUNC_INFO << ": Failed to find rendering properties widget for rendering method " << currentRenderingMethod;
    // Index 0 is an empty widget
    d->RenderingMethodStackedWidget->setCurrentIndex(0);
    }

  // Volume properties tab
  d->SynchronizeScalarDisplayNodeButton->setEnabled(displayNode != nullptr);
  bool follow = displayNode ? displayNode->GetFollowVolumeDisplayNode() != 0 : false;
  if (follow)
    {
    d->SynchronizeScalarDisplayNodeButton->setCheckState(Qt::Checked);
    }
  d->SynchronizeScalarDisplayNodeButton->setChecked(follow);
  d->IgnoreVolumesThresholdCheckBox->setChecked(
    displayNode ? displayNode->GetIgnoreVolumeDisplayNodeThreshold() != 0 : false );
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::updateWidgetFromROINode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  bool roiVisible = false;
  if (d->CropROINode)
    {
    roiVisible = d->CropROINode->GetDisplayVisibility();
    }
  QSignalBlocker blocker(d->ROICropDisplayCheckBox);
  d->ROICropDisplayCheckBox->setChecked(roiVisible);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCropToggled(bool crop)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  if (crop)
    {
    // Create ROI node
    vtkSlicerVolumeRenderingLogic* logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
    if (logic)
      {
      logic->CreateROINode(displayNode);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
      }
    }

  displayNode->SetCroppingEnabled(crop);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::fitROIToVolume()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  // Fit ROI to volume
  vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic())->FitROIToVolume(displayNode);

  vtkMRMLMarkupsROINode* markupsROINode = displayNode->GetMarkupsROINode();
  vtkMRMLAnnotationROINode* annotationROINode = displayNode->GetAnnotationROINode();
  if (annotationROINode &&
     (d->AnnotationROIWidget->mrmlROINode() != this->mrmlROINode()
     || d->AnnotationROIWidget->mrmlROINode() != annotationROINode))
    {
    qCritical() << Q_FUNC_INFO << ": ROI node mismatch";
    return;
    }
  if (markupsROINode &&
     (d->MarkupsROIWidget->mrmlROINode() != this->mrmlMarkupsROINode()
     || d->MarkupsROIWidget->mrmlROINode() != markupsROINode))
    {
    qCritical() << Q_FUNC_INFO << ": ROI node mismatch";
    return;
    }

  // Update ROI widget extent
  if (markupsROINode && d->MarkupsROIWidget->mrmlROINode())
    {
    double xyz[3] = { 0.0 };
    double rxyz[3] = { 0.0 };

    d->MarkupsROIWidget->mrmlROINode()->GetXYZ(xyz);
    d->MarkupsROIWidget->mrmlROINode()->GetRadiusXYZ(rxyz);

    double bounds[6] = { 0.0 };
    for (int i = 0; i < 3; ++i)
      {
      bounds[i] = xyz[i] - rxyz[i];
      bounds[3 + i] = xyz[i] + rxyz[i];
      }
    d->MarkupsROIWidget->setExtent(bounds[0], bounds[3],
      bounds[1], bounds[4],
      bounds[2], bounds[5]);
    }
  else if (annotationROINode && d->AnnotationROIWidget->mrmlROINode())
    {
    double xyz[3] = {0.0};
    double rxyz[3] = {0.0};

    d->AnnotationROIWidget->mrmlROINode()->GetXYZ(xyz);
    d->AnnotationROIWidget->mrmlROINode()->GetRadiusXYZ(rxyz);

    double bounds[6] = {0.0};
    for (int i=0; i < 3; ++i)
      {
      bounds[i]   = xyz[i]-rxyz[i];
      bounds[3+i] = xyz[i]+rxyz[i];
      }
    d->AnnotationROIWidget->setExtent(bounds[0], bounds[3],
                            bounds[1], bounds[4],
                            bounds[2], bounds[5]);
    }
}

// --------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* qSlicerVolumeRenderingModuleWidget::mrmlVolumePropertyNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLVolumePropertyNode::SafeDownCast(d->VolumePropertyNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setMRMLVolumePropertyNode(vtkMRMLNode* volumePropertyNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  // Set if not already set
  d->VolumePropertyNodeComboBox->setCurrentNode(volumePropertyNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkMRMLVolumePropertyNode* volumePropertyNode = vtkMRMLVolumePropertyNode::SafeDownCast(node);

  // Set volume property node to display node
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveVolumePropertyNodeID(volumePropertyNode ? volumePropertyNode->GetID() : nullptr);
    }
}

// --------------------------------------------------------------------------
vtkMRMLDisplayableNode* qSlicerVolumeRenderingModuleWidget::mrmlROINode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLDisplayableNode::SafeDownCast(d->ROINodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
vtkMRMLAnnotationROINode* qSlicerVolumeRenderingModuleWidget::mrmlAnnotationROINode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLAnnotationROINode::SafeDownCast(d->ROINodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
vtkMRMLMarkupsROINode* qSlicerVolumeRenderingModuleWidget::mrmlMarkupsROINode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  return vtkMRMLMarkupsROINode::SafeDownCast(d->ROINodeComboBox->currentNode());
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
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (displayNode)
    {
    displayNode->SetAndObserveROINodeID(node ? node->GetID() : nullptr);
    }
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidget::mrmlDisplayNode()const
{
  Q_D(const qSlicerVolumeRenderingModuleWidget);
  vtkMRMLVolumeNode* volumeNode = this->mrmlVolumeNode();
  vtkMRMLVolumeRenderingDisplayNode* displayNode = d->displayNodeForVolumeNode(volumeNode);
  return displayNode;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentRenderingMethodChanged(int index)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  QString renderingClassName = d->RenderingMethodComboBox->itemData(index).toString();
  // Display node is already the right type, don't change anything
  if ( !displayNode || renderingClassName.isEmpty()
    || renderingClassName == displayNode->GetClassName())
    {
    return;
    }

  // Replace display nodes with new display nodes of the type corresponding to the requested method
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->ChangeVolumeRenderingMethod(renderingClassName.toUtf8());

  // Perform necessary setup steps for the new display node for the current volume
  this->onCurrentMRMLVolumeNodeChanged(d->VolumeNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentMemorySizeChanged()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  int gpuMemorySize = d->MemorySizeComboBox->currentGPUMemoryInMB();

  std::vector<vtkMRMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetGPUMemorySize(gpuMemorySize);
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentQualityControlChanged(int index)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkMRMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetVolumeRenderingQuality(index);
      }
    }

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onAutoReleaseGraphicsResourcesCheckBoxToggled(bool autoRelease)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkMRMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetAutoReleaseGraphicsResources(autoRelease);
      }
    }

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentFramerateChanged(double fps)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkMRMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetExpectedFPS(fps);
      }
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::synchronizeScalarDisplayNode()
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->CopyDisplayToVolumeRenderingDisplayNode(displayNode);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setFollowVolumeDisplayNode(bool follow)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetFollowVolumeDisplayNode(follow ? 1 : 0);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setIgnoreVolumesThreshold(bool ignore)
{
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetIgnoreVolumeDisplayNodeThreshold(ignore ? 1 : 0);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onThresholdChanged(bool threshold)
{
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  volumeRenderingLogic->SetUseLinearRamp(!threshold);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onROICropDisplayCheckBoxToggled(bool toggle)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  vtkMRMLVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  if (toggle)
    {
    // Create ROI node
    vtkSlicerVolumeRenderingLogic* logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
    if (logic)
      {
      logic->CreateROINode(displayNode);
      }
    else
      {
      qCritical() << Q_FUNC_INFO << ": Failed to access volume rendering logic";
      }
    }


  vtkMRMLDisplayableNode* roiNode = d->MarkupsROIWidget->mrmlROINode();
  if (!roiNode)
    {
    roiNode = d->AnnotationROIWidget->mrmlROINode();
    }
  if (!roiNode)
    {
    return;
    }

  int numberOfDisplayNodes = roiNode->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkMRMLNode* node = roiNode->GetNthDisplayNode(index);
    if (!node)
      {
      continue;
      }
    wasModifying[index] = node->StartModify();
    }

  roiNode->SetDisplayVisibility(toggle);

  // When the display box is visible, it activated the cropping
  // (to follow the "what you see is what you get" pattern).
  if (toggle)
    {
    displayNode->SetCroppingEnabled(toggle);
    }

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    vtkMRMLNode* node = roiNode->GetNthDisplayNode(index);
    if (!node)
      {
      continue;
      }
    node->EndModify(wasModifying[index]);
    }
}

//-----------------------------------------------------------
bool qSlicerVolumeRenderingModuleWidget::setEditedNode(vtkMRMLNode* node,
                                                       QString role /* = QString()*/,
                                                       QString context /* = QString()*/)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node))
    {
    vtkMRMLVolumeRenderingDisplayNode* displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node);

    vtkMRMLVolumeNode* displayableNode = vtkMRMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->VolumeNodeComboBox->setCurrentNode(displayableNode);
    return true;
    }

  if (vtkMRMLVolumePropertyNode::SafeDownCast(node))
    {
    // Find first volume rendering display node corresponding to this property node
    vtkMRMLScene* scene = this->mrmlScene();
    if (!scene)
      {
      return false;
      }
    vtkMRMLVolumeRenderingDisplayNode* displayNode = nullptr;
    vtkObject* itNode = nullptr;
    vtkCollectionSimpleIterator it;
    for (scene->GetNodes()->InitTraversal(it); (itNode = scene->GetNodes()->GetNextItemAsObject(it));)
      {
      displayNode = vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(itNode);
      if (!displayNode)
        {
        continue;
        }
      if (displayNode->GetVolumePropertyNode() != node)
        {
        continue;
        }
      vtkMRMLVolumeNode* displayableNode = vtkMRMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
      if (!displayableNode)
        {
        return false;
        }
      d->VolumeNodeComboBox->setCurrentNode(displayableNode);
      return true;
      }
    }

  if (vtkMRMLMarkupsROINode::SafeDownCast(node))
    {
    vtkSlicerVolumeRenderingLogic* volumeRenderingLogic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
    if (!volumeRenderingLogic)
      {
      qWarning() << Q_FUNC_INFO << "failed: invalid logic";
      return false;
      }
    vtkMRMLVolumeRenderingDisplayNode* displayNode = volumeRenderingLogic->GetFirstVolumeRenderingDisplayNodeByROINode(
      vtkMRMLMarkupsROINode::SafeDownCast(node));
    if (!displayNode)
      {
      return false;
      }
    vtkMRMLVolumeNode* displayableNode = vtkMRMLVolumeNode::SafeDownCast(displayNode->GetDisplayableNode());
    if (!displayableNode)
      {
      return false;
      }
    d->VolumeNodeComboBox->setCurrentNode(displayableNode);
    return true;
    }

  return false;
}

//-----------------------------------------------------------
double qSlicerVolumeRenderingModuleWidget::nodeEditable(vtkMRMLNode* node)
{
  if (vtkMRMLVolumePropertyNode::SafeDownCast(node)
    || vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node))
    {
    return 0.5;
    }
  else if (vtkMRMLMarkupsROINode::SafeDownCast(node))
    {
    vtkSlicerVolumeRenderingLogic* volumeRenderingLogic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
    if (!volumeRenderingLogic)
      {
      qWarning() << Q_FUNC_INFO << " failed: Invalid logic";
      return 0.0;
      }
    if (volumeRenderingLogic->GetFirstVolumeRenderingDisplayNodeByROINode(vtkMRMLMarkupsROINode::SafeDownCast(node)))
      {
      // This ROI node is a clipping ROI for volume rendering - claim it with higher confidence than the generic 0.5
      return 0.6;
      }
    return 0.0;
    }
  else
    {
    return 0.0;
    }
}

//-----------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onChartsExtentChanged()
{
  vtkMRMLVolumePropertyNode* volumePropertyNode = this->mrmlVolumePropertyNode();
  if (!volumePropertyNode)
    {
    return;
    }

  Q_D(qSlicerVolumeRenderingModuleWidget);
  double effectiveRange[4] = { 0.0 };
  d->VolumePropertyNodeWidget->chartsExtent(effectiveRange);

  int wasDisabled = volumePropertyNode->GetDisableModifiedEvent();
  volumePropertyNode->DisableModifiedEventOn();
  volumePropertyNode->SetEffectiveRange(effectiveRange[0], effectiveRange[1]);
  volumePropertyNode->SetDisableModifiedEvent(wasDisabled);

  // Update presets slider range
  d->PresetComboBox->updatePresetSliderRange();
}

//-----------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onEffectiveRangeModified()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

  vtkMRMLVolumePropertyNode* volumePropertyNode = this->mrmlVolumePropertyNode();
  if (!volumePropertyNode)
    {
    return;
    }

  // Set charts extent to effective range defined in volume property node
  double effectiveRange[2] = {0.0};
  volumePropertyNode->GetEffectiveRange(effectiveRange);
  if (effectiveRange[0] > effectiveRange[1])
    {
    if (!volumePropertyNode->CalculateEffectiveRange())
      {
      return; // Do not set undefined effective range
      }
    volumePropertyNode->GetEffectiveRange(effectiveRange);
    }
  bool wasBlocking = d->VolumePropertyNodeWidget->blockSignals(true);
  d->VolumePropertyNodeWidget->setChartsExtent(effectiveRange[0], effectiveRange[1]);
  d->VolumePropertyNodeWidget->blockSignals(wasBlocking);

  // Update presets slider range
  d->PresetComboBox->updatePresetSliderRange();
}
