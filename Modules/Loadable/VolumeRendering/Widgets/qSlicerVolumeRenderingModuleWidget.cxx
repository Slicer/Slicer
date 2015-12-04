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

// Qt includes
#include <QSettings>

// CTK includes
#include <ctkUtils.h>

// qMRMLWidgets include
#include "qMRMLSceneModel.h"

// qSlicerVolumeRendering includes
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "ui_qSlicerVolumeRenderingModuleWidget.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkSlicerVolumeRenderingLogic.h"
#include "qSlicerCPURayCastVolumeRenderingPropertiesWidget.h"
#include "qSlicerGPURayCastVolumeRenderingPropertiesWidget.h"

// MRML includes
#include "vtkMRMLAnnotationROINode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumePropertyNode.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVector.h>
#include <vtkVolumeProperty.h>
#include <vtkWeakPointer.h>

// STD includes
#include <cassert>
#include <vector>

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
  vtkMRMLVolumeRenderingDisplayNode* createVolumeRenderingDisplayNode(
    vtkMRMLVolumeNode* volumeNode);
  void populateRenderingTechniqueComboBox();
  void populatePresetsIcons(qMRMLNodeComboBox* presetsNodeComboBox);

  vtkMRMLVolumeRenderingDisplayNode* DisplayNode;
  QMap<int, int>                     LastTechniques;
  double                             OldPresetPosition;
  QMap<QString, QWidget*>            RenderingMethodWidgets;
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
qSlicerVolumeRenderingModuleWidgetPrivate::~qSlicerVolumeRenderingModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidgetPrivate::setupUi(qSlicerVolumeRenderingModuleWidget* q)
{
  this->Ui_qSlicerVolumeRenderingModuleWidget::setupUi(q);

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

  // Rendering
  QObject::connect(this->ROICropCheckBox,
                   SIGNAL(toggled(bool)),
                   q, SLOT(onCropToggled(bool)));
  QObject::connect(this->ROIFitPushButton,
                   SIGNAL(clicked()),
                   q, SLOT(fitROIToVolume()));

  // Techniques
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());
  std::map<std::string, std::string> methods =
    volumeRenderingLogic->GetRenderingMethods();
  std::map<std::string, std::string>::const_iterator it;
  for (it = methods.begin(); it != methods.end(); ++it)
    {
    this->RenderingMethodComboBox->addItem(
      QString::fromStdString(it->first), QString::fromStdString(it->second));
    }
  QObject::connect(this->RenderingMethodComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurrentRenderingMethodChanged(int)));
  // Add empty widget at index 0 for the volume rendering methods with no widget.
  this->RenderingMethodStackedWidget->addWidget(new QWidget());
  q->addRenderingMethodWidget("vtkMRMLCPURayCastVolumeRenderingDisplayNode",
                              new qSlicerCPURayCastVolumeRenderingPropertiesWidget);
  q->addRenderingMethodWidget("vtkMRMLGPURayCastVolumeRenderingDisplayNode",
                              new qSlicerGPURayCastVolumeRenderingPropertiesWidget);
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

  // Volume Properties
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

  // ensure that the view node combo box only shows view nodes,
  // not slice nodes or chart nodes
  this->ViewCheckableNodeComboBox->setNodeTypes(QStringList(QString("vtkMRMLViewNode")));
}

// --------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingModuleWidgetPrivate
::createVolumeRenderingDisplayNode(vtkMRMLVolumeNode* volumeNode)
{
  Q_Q(qSlicerVolumeRenderingModuleWidget);
  vtkSlicerVolumeRenderingLogic *logic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(q->logic());

  vtkMRMLVolumeRenderingDisplayNode* displayNode =
    logic->CreateVolumeRenderingDisplayNode();
  q->mrmlScene()->AddNode(displayNode);
  displayNode->Delete();

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
  foreach (vtkMRMLAbstractViewNode* viewNode,
           this->ViewCheckableNodeComboBox->checkedViewNodes())
    {
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

  // if display node is not referenced by current volume, add the reference
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
void qSlicerVolumeRenderingModuleWidget::addRenderingMethodWidget(
  const QString& methodClassName,
  qSlicerVolumeRenderingPropertiesWidget* widget)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  this->connect(d->DisplayNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                widget, SLOT(setMRMLNode(vtkMRMLNode*)));
  d->RenderingMethodStackedWidget->addWidget(widget);
  d->RenderingMethodWidgets[methodClassName] = widget;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::updateFromMRMLDisplayNode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);

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
  QString defaultRenderingMethod =
    settings.value("VolumeRendering/RenderingMethod",
                   QString("vtkMRMLCPURayCastVolumeRenderinDisplayNode")).toString();
  QString currentVolumeMapper = d->DisplayNode ?
    QString(d->DisplayNode->GetClassName()) : defaultRenderingMethod;
  d->RenderingMethodComboBox->setCurrentIndex(
    d->RenderingMethodComboBox->findData(currentVolumeMapper) );
  int index = d->DisplayNode ?
    d->MemorySizeComboBox->findData(QVariant(d->DisplayNode->GetGPUMemorySize())) : -1;
  d->MemorySizeComboBox->setCurrentIndex(index);
  d->QualityControlComboBox->setCurrentIndex(
    d->DisplayNode ? d->DisplayNode->GetPerformanceControl() : -1);
  if (d->DisplayNode)
    {
    d->FramerateSliderWidget->setValue(d->DisplayNode->GetExpectedFPS());
    }
  d->FramerateSliderWidget->setEnabled(
    d->DisplayNode && d->DisplayNode->GetPerformanceControl() ==
    vtkMRMLVolumeRenderingDisplayNode::Adaptative);
  // Opacity/color
  bool follow = d->DisplayNode ? d->DisplayNode->GetFollowVolumeDisplayNode() != 0 : false;
  if (follow)
    {
    d->SynchronizeScalarDisplayNodeButton->setCheckState(Qt::Checked);
    }
  d->SynchronizeScalarDisplayNodeButton->setChecked(follow);
  d->IgnoreVolumesThresholdCheckBox->setChecked( d->DisplayNode ?
    d->DisplayNode->GetIgnoreVolumeDisplayNodeThreshold() != 0 : false);

  // Properties
  if (d->RenderingMethodWidgets[currentVolumeMapper])
    {
    d->RenderingMethodStackedWidget->setCurrentWidget(
      d->RenderingMethodWidgets[currentVolumeMapper]);
    }
  else
    {
    // index 0 is an empty widget
    d->RenderingMethodStackedWidget->setCurrentIndex(0);
    }
}


// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::updateFromMRMLDisplayROINode()
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->ROIWidget->mrmlROINode())
    {
    return;
    }
  //ROI visibility
  d->ROICropDisplayCheckBox->setChecked(
        d->ROIWidget->mrmlROINode()->GetDisplayVisibility());
}


// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::addVolumeIntoView(vtkMRMLNode* viewNode)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  d->ViewCheckableNodeComboBox->check(viewNode);
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
  this->qvtkReconnect(d->DisplayNode->GetROINode(), roiNode,
                        vtkMRMLDisplayableNode::DisplayModifiedEvent,
                        this, SLOT(updateFromMRMLDisplayROINode()));

  d->DisplayNode->SetAndObserveROINodeID(roiNode ? roiNode->GetID() : 0);
  this->updateFromMRMLDisplayROINode();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::onCurrentRenderingMethodChanged(int index)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (!d->DisplayNode)
    {
    return;
    }
  QString renderingClassName = d->RenderingMethodComboBox->itemData(index).toString();
  // Display node is already the right type, don't change anything
  if (renderingClassName.isEmpty() ||
      renderingClassName == d->DisplayNode->GetClassName())
    {
    return;
    }
  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  vtkMRMLVolumeRenderingDisplayNode* displayNode =
    volumeRenderingLogic->CreateVolumeRenderingDisplayNode(
      renderingClassName.toLatin1());
  this->mrmlScene()->AddNode(displayNode);
  displayNode->Delete();
  vtkWeakPointer<vtkMRMLVolumeRenderingDisplayNode> oldDisplayNode = d->DisplayNode;
  displayNode->vtkMRMLVolumeRenderingDisplayNode::Copy(d->DisplayNode);
  d->DisplayNodeComboBox->setCurrentNode(displayNode);
  if (oldDisplayNode.GetPointer())
    {
    this->mrmlScene()->RemoveNode(oldDisplayNode);
    }
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
  volumeRenderingLogic->CopyDisplayToVolumeRenderingDisplayNode(
    d->DisplayNode);
}


// --------------------------------------------------------------------------
void qSlicerVolumeRenderingModuleWidget::setFollowVolumeDisplayNode(bool follow)
{
  Q_D(qSlicerVolumeRenderingModuleWidget);
  if (d->DisplayNode == 0)
    {
    return;
    }
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
    d->DisplayNode->SetCroppingEnabled(toggle);
    }

  int numberOfDisplayNodes =
      d->ROIWidget->mrmlROINode()->GetNumberOfDisplayNodes();

  std::vector<int> wasModifying(numberOfDisplayNodes);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    wasModifying[index] =
        d->ROIWidget->mrmlROINode()->GetNthDisplayNode(index)->StartModify();
    }

  d->ROIWidget->mrmlROINode()->SetDisplayVisibility(toggle);

  for(int index = 0; index < numberOfDisplayNodes; index++)
    {
    d->ROIWidget->mrmlROINode()->GetNthDisplayNode(index)->EndModify(
          wasModifying[index]);
    }
}
