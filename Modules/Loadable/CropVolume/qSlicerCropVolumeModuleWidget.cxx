// Qt includes
#include <QDebug>
#include <QMessageBox>

// CTK includes
#include <ctkFlowLayout.h>

// Slicer includes
#include <qSlicerAbstractCoreModule.h>
#include <qSlicerApplication.h>
#include <qSlicerLayoutManager.h>

// CropVolume includes
#include "qSlicerCropVolumeModuleWidget.h"
#include "ui_qSlicerCropVolumeModuleWidget.h"

// CropVolume Logic includes
#include <vtkSlicerCropVolumeLogic.h>

// qMRML includes
#include <qMRMLNodeFactory.h>
#include <qMRMLSliceWidget.h>

// MRMLAnnotation includes
#include <vtkMRMLAnnotationROINode.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLSliceLogic.h>

#include <vtkNew.h>
#include <vtkMatrix4x4.h>
#include <vtkImageData.h>


// MRML includes
#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTransformNode.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CropVolume
class qSlicerCropVolumeModuleWidgetPrivate: public Ui_qSlicerCropVolumeModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerCropVolumeModuleWidget);
protected:
  qSlicerCropVolumeModuleWidget* const q_ptr;
public:

  qSlicerCropVolumeModuleWidgetPrivate(qSlicerCropVolumeModuleWidget& object);
  ~qSlicerCropVolumeModuleWidgetPrivate();

  vtkSlicerCropVolumeLogic* logic() const;

  /// Return true if inputs are correct, cropping may be enabled
  bool checkInputs(bool& autoFixAvailable, QString& message, bool autoFixProblems);

  vtkWeakPointer<vtkMRMLCropVolumeParametersNode> ParametersNode;
  vtkWeakPointer<vtkMRMLVolumeNode> InputVolumeNode;
  vtkWeakPointer<vtkMRMLAnnotationROINode> InputROINode;
};

//-----------------------------------------------------------------------------
// qSlicerCropVolumeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidgetPrivate::qSlicerCropVolumeModuleWidgetPrivate(qSlicerCropVolumeModuleWidget& object) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidgetPrivate::~qSlicerCropVolumeModuleWidgetPrivate() = default;

//-----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic* qSlicerCropVolumeModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerCropVolumeModuleWidget);
  return vtkSlicerCropVolumeLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
bool qSlicerCropVolumeModuleWidgetPrivate::checkInputs(bool& autoFixAvailable, QString& message, bool autoFixProblems)
{
  message.clear();
  autoFixAvailable = false;

  if (this->ParametersNode == nullptr)
    {
    message = qSlicerCropVolumeModuleWidget::tr("Select or create a new parameter node.");
    autoFixAvailable = true;
    if (autoFixProblems)
      {
      this->ParametersNodeComboBox->addNode();
      }
    return false;
    }

  if (this->ParametersNode->GetInputVolumeNode() == nullptr)
    {
    message = qSlicerCropVolumeModuleWidget::tr("Select an input volume.");
    return false;
    }

  bool roiExists = true;
  bool inputVolumeTransformValid = true;
  bool roiTransformValid = true;
  bool outputVolumeTransformValid = true;
  QStringList problemsDescription;

  // Common cropping problems
  if (this->ParametersNode->GetROINode())
    {
    if (this->ParametersNode->GetROINode()->GetParentTransformNode()
      && !this->ParametersNode->GetROINode()->GetParentTransformNode()->IsTransformToWorldLinear())
      {
      roiTransformValid = false;
      problemsDescription << qSlicerCropVolumeModuleWidget::tr("Input ROI is under a non-linear tansform.");
      }
    }
  else
    {
    roiExists = false;
    problemsDescription << qSlicerCropVolumeModuleWidget::tr("Select or create a new input ROI.");
    }
  if (this->ParametersNode->GetOutputVolumeNode()
    && this->ParametersNode->GetOutputVolumeNode()->GetParentTransformNode()
    && !this->ParametersNode->GetOutputVolumeNode()->GetParentTransformNode()->IsTransformToWorldLinear())
    {
    outputVolumeTransformValid = false;
    problemsDescription << qSlicerCropVolumeModuleWidget::tr("Output volume is under a non-linear tansform.");
    }

  // Non-interpolated cropping problem
  if (this->ParametersNode->GetVoxelBased())
    {
    if (this->ParametersNode->GetInputVolumeNode()
      && this->ParametersNode->GetInputVolumeNode()->GetParentTransformNode()
      && !this->ParametersNode->GetInputVolumeNode()->GetParentTransformNode()->IsTransformToWorldLinear())
      {
      inputVolumeTransformValid = false;
      problemsDescription << qSlicerCropVolumeModuleWidget::tr("Interpolation is disabled and input volume is under a non-linear tansform");
      }
    // Only report ROI errors, if ROI is valid (avoid overloading the user with too much info)
    if (roiExists && roiTransformValid && !vtkSlicerCropVolumeLogic::IsROIAlignedWithInputVolume(this->ParametersNode))
      {
      roiTransformValid = false;
      problemsDescription += qSlicerCropVolumeModuleWidget::tr("Interpolation is disabled and input ROI is not aligned with input volume axes.");
      }
    }

  if (!autoFixProblems || problemsDescription.isEmpty())
    {
    // nothing to fix or fix is not requested
    autoFixAvailable = !problemsDescription.isEmpty();
    message = problemsDescription.join(" ");
    return problemsDescription.isEmpty();
    }

  if (!roiExists)
    {
    this->InputROIComboBox->addNode();
    }

  // Non-interpolated cropping problem
  if (!inputVolumeTransformValid)
    {
    this->ParametersNode->GetInputVolumeNode()->SetAndObserveTransformNodeID(nullptr);
    if (this->ParametersNode->GetVoxelBased())
      {
      this->logic()->SnapROIToVoxelGrid(this->ParametersNode);
      roiTransformValid = true;
      }
    }
  if (!roiTransformValid)
    {
    this->logic()->SnapROIToVoxelGrid(this->ParametersNode);
    }
  if (!outputVolumeTransformValid)
    {
    const char* newParentTransformNodeID = nullptr;
    if (this->ParametersNode->GetInputVolumeNode())
      {
      newParentTransformNodeID = this->ParametersNode->GetInputVolumeNode()->GetTransformNodeID();
      }
    this->ParametersNode->GetOutputVolumeNode()->SetAndObserveTransformNodeID(newParentTransformNodeID);
    }

  message = problemsDescription.join(" ");
  return problemsDescription.isEmpty();
}
//-----------------------------------------------------------------------------
// qSlicerCropVolumeModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidget::qSlicerCropVolumeModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerCropVolumeModuleWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidget::~qSlicerCropVolumeModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setup()
{
  Q_D(qSlicerCropVolumeModuleWidget);

  d->setupUi(this);

  this->Superclass::setup();

  connect(d->ParametersNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(setParametersNode(vtkMRMLNode*)));

  connect(d->InputVolumeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(setInputVolume(vtkMRMLNode*)));

  connect(d->InputROIComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(setInputROI(vtkMRMLNode*)));
  connect(d->InputROIComboBox->nodeFactory(), SIGNAL(nodeInitialized(vtkMRMLNode*)),
          this, SLOT(initializeInputROI(vtkMRMLNode*)));
  connect(d->InputROIComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)),
          this, SLOT(onInputROIAdded(vtkMRMLNode*)));

  connect(d->OutputVolumeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(setOutputVolume(vtkMRMLNode*)));

  connect(d->VisibilityButton, SIGNAL(toggled(bool)),
          this, SLOT(onROIVisibilityChanged(bool)));
  connect(d->ROIFitPushButton, SIGNAL(clicked()),
    this, SLOT(onROIFit()));

  connect(d->InterpolationEnabledCheckBox, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationEnabled(bool)));
  connect(d->SpacingScalingSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(onSpacingScalingValueChanged(double)));
  connect(d->IsotropicCheckbox, SIGNAL(toggled(bool)),
    this, SLOT(onIsotropicModeChanged(bool)));

  connect(d->LinearRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->NNRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->WSRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->BSRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));

  connect(d->FillValueSpinBox, SIGNAL(valueChanged(double)),
    this, SLOT(onFillValueChanged(double)));

  // Observe info section, only update content if opened
  this->connect(d->VolumeInformationCollapsibleButton,
    SIGNAL(clicked(bool)),
    SLOT(onVolumeInformationSectionClicked(bool)));

  d->InputErrorLabel->setVisible(false);
  d->InputErrorFixButton->setVisible(false);
  connect(d->InputErrorFixButton, SIGNAL(clicked()),
    this, SLOT(onFixAlignment()));

  connect(d->CropButton, SIGNAL(clicked()),
    this, SLOT(onApply()));

}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::enter()
{
  Q_D(qSlicerCropVolumeModuleWidget);

  // For user's convenience, create a default ROI parameter node if
  // none exists yet.
  vtkMRMLScene* scene = this->mrmlScene();
  if (!scene)
    {
    qWarning("qSlicerCropVolumeModuleWidget::enter: invalid scene");
    return;
    }
  if (scene->GetNumberOfNodesByClass("vtkMRMLCropVolumeParametersNode") == 0)
    {
    vtkNew<vtkMRMLCropVolumeParametersNode> parametersNode;
    scene->AddNode(parametersNode.GetPointer());

    // Use first background volume node in any of the displayed slice views as input volume
    qSlicerApplication * app = qSlicerApplication::application();
    if (app && app->layoutManager())
      {
      foreach(QString sliceViewName, app->layoutManager()->sliceViewNames())
        {
        qMRMLSliceWidget* sliceWidget = app->layoutManager()->sliceWidget(sliceViewName);
        const char* backgroundVolumeNodeID = sliceWidget->sliceLogic()->GetSliceCompositeNode()->GetBackgroundVolumeID();
        if (backgroundVolumeNodeID != nullptr)
          {
          parametersNode->SetInputVolumeNodeID(backgroundVolumeNodeID);
          break;
          }
        }
      }

    // Use first visible ROI node (or last ROI node, if all are invisible)
    vtkMRMLAnnotationROINode* foundROINode = nullptr;
    std::vector<vtkMRMLNode *> roiNodes;
    scene->GetNodesByClass("vtkMRMLAnnotationROINode", roiNodes);
    for (unsigned int i = 0; i < roiNodes.size(); ++i)
      {
      vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(roiNodes[i]);
      if (!roiNode)
        {
        continue;
        }
      foundROINode = roiNode;
      if (foundROINode->GetDisplayVisibility())
        {
        break;
        }
      }
    if (foundROINode)
      {
      parametersNode->SetROINodeID(foundROINode->GetID());
      }

    d->ParametersNodeComboBox->setCurrentNode(parametersNode.GetPointer());
    }
  else
    {
    // There is at least one parameter node.
    // If none is selected then select the first one.
    if (d->ParametersNodeComboBox->currentNode() == nullptr)
      {
      d->ParametersNodeComboBox->setCurrentNode(scene->GetFirstNodeByClass("vtkMRMLCropVolumeParametersNode"));
      }
    }

  this->Superclass::enter();

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  // observe close event so can re-add a parameters node if necessary
  qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndImportEvent, this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndBatchProcessEvent, this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent, this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndRestoreEvent, this, SLOT(onMRMLSceneEndBatchProcessEvent()));
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::initializeInputROI(vtkMRMLNode *n)
{
  Q_D(const qSlicerCropVolumeModuleWidget);
  vtkMRMLScene* scene = qobject_cast<qMRMLNodeFactory*>(this->sender())->mrmlScene();
  vtkMRMLAnnotationROINode::SafeDownCast(n)->Initialize(scene);
  if (d->ParametersNode && d->ParametersNode->GetInputVolumeNode())
    {
    this->setInputROI(n);
    this->onROIFit();
    this->updateWidgetFromMRML();
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onApply()
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if(!d->ParametersNode.GetPointer() ||
    !d->ParametersNode->GetInputVolumeNode() ||
    !d->ParametersNode->GetROINode())
    {
    qWarning() << Q_FUNC_INFO << ": invalid inputs";
    return;
    }

  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  vtkMRMLNode* oldOutputNode = d->ParametersNode->GetOutputVolumeNode();
  if (!d->logic()->Apply(d->ParametersNode))
    {
    // no errors
    if (d->ParametersNode->GetOutputVolumeNode() != oldOutputNode)
      {
      // New output volume is created, show it in slice viewers
      vtkSlicerApplicationLogic *appLogic = this->module()->appLogic();
      vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
      selectionNode->SetActiveVolumeID(d->ParametersNode->GetOutputVolumeNodeID());
      appLogic->PropagateVolumeSelection();
      }
    }
  QApplication::restoreOverrideCursor();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onFixAlignment()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  bool autoFixAvailable = false;
  QString errorMessages;
  d->checkInputs(autoFixAvailable, errorMessages, true /* auto-fix problems */ );
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setInputVolume(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNode.GetPointer())
    {
    // setInputVolume may be triggered by calling setScene on InputVolumeComboBox
    // before ParametersNodeComboBox is initialized, so don't log a warning here
    return;
    }

  qvtkReconnect(d->InputVolumeNode, volumeNode, vtkCommand::ModifiedEvent, this, SLOT(updateVolumeInfo()));
  d->InputVolumeNode = volumeNode;
  d->ParametersNode->SetInputVolumeNodeID(volumeNode ? volumeNode->GetID() : nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setOutputVolume(vtkMRMLNode* volumeNode)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  vtkMRMLCropVolumeParametersNode *parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    if (volumeNode != nullptr)
      {
      qWarning() << Q_FUNC_INFO << ": invalid parameter node";
      }
    return;
    }

  parametersNode->SetOutputVolumeNodeID(volumeNode ? volumeNode->GetID() : nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setInputROI(vtkMRMLNode* node)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNode.GetPointer())
    {
    if (node != nullptr)
      {
      qWarning() << Q_FUNC_INFO << ": invalid parameter node";
      }
    return;
    }
  vtkMRMLAnnotationROINode* roiNode = vtkMRMLAnnotationROINode::SafeDownCast(node);
  qvtkReconnect(d->InputROINode, roiNode, vtkCommand::ModifiedEvent, this, SLOT(updateVolumeInfo()));
  d->InputROINode = roiNode;
  d->ParametersNode->SetROINodeID(roiNode ? roiNode->GetID() : nullptr);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputROIAdded(vtkMRMLNode* node)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNode.GetPointer())
    {
    return;
    }
  if (!d->ParametersNode->GetROINode() && node)
    {
    // There was no ROI selected and the user just added one
    // use that for cropping.
    d->ParametersNode->SetROINodeID(node->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onROIVisibilityChanged(bool visible)
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!d->ParametersNode || !d->ParametersNode->GetROINode())
    {
    return;
    }
  d->ParametersNode->GetROINode()->SetDisplayVisibility(visible);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onROIFit()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  d->logic()->SnapROIToVoxelGrid(d->ParametersNode);
  d->logic()->FitROIToInputVolume(d->ParametersNode);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInterpolationModeChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!d->ParametersNode)
    {
    return;
    }
  if(d->NNRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkMRMLCropVolumeParametersNode::InterpolationNearestNeighbor);
    }
  if(d->LinearRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkMRMLCropVolumeParametersNode::InterpolationLinear);
    }
  if(d->WSRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkMRMLCropVolumeParametersNode::InterpolationWindowedSinc);
    }
  if(d->BSRadioButton->isChecked())
    {
    d->ParametersNode->SetInterpolationMode(vtkMRMLCropVolumeParametersNode::InterpolationBSpline);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onSpacingScalingValueChanged(double s)
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!d->ParametersNode)
    {
    return;
    }
  d->ParametersNode->SetSpacingScalingConst(s);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onFillValueChanged(double s)
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!d->ParametersNode)
  {
    return;
  }
  d->ParametersNode->SetFillValue(s);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onIsotropicModeChanged(bool isotropic)
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!d->ParametersNode)
    {
    return;
    }
  d->ParametersNode->SetIsotropicResampling(isotropic);
}

//-----------------------------------------------------------------------------
void
qSlicerCropVolumeModuleWidget::onInterpolationEnabled(bool interpolationEnabled)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNode)
    {
    return;
    }
  d->ParametersNode->SetVoxelBased(!interpolationEnabled);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onMRMLSceneEndBatchProcessEvent()
{
  if (!this->mrmlScene() || this->mrmlScene()->IsBatchProcessing())
    {
    return;
    }
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!this->mrmlScene())
    {
    return;
    }

  QString inputCheckErrorMessage;
  bool autoFixAvailable = false;
  if (d->checkInputs(autoFixAvailable, inputCheckErrorMessage, false))
    {
    d->InputErrorLabel->setVisible(false);
    d->InputErrorFixButton->setVisible(false);
    }
  else
    {
    inputCheckErrorMessage.prepend("<span style = \"color:#FF0000;\">");
    inputCheckErrorMessage.append("</span>");
    d->InputErrorLabel->setText(inputCheckErrorMessage);
    d->InputErrorLabel->setVisible(true);
    d->InputErrorFixButton->setVisible(autoFixAvailable);
    }

  if (!d->ParametersNode)
    {
    // reset widget to defaults from node class
    d->InputVolumeComboBox->setCurrentNode(nullptr);
    d->InputROIComboBox->setCurrentNode(nullptr);
    d->OutputVolumeComboBox->setCurrentNode(nullptr);

    d->InterpolationEnabledCheckBox->setChecked(true);
    d->VisibilityButton->setChecked(true);

    d->IsotropicCheckbox->setChecked(false);
    d->SpacingScalingSpinBox->setValue(1.0);
    d->LinearRadioButton->setChecked(true);
    d->FillValueSpinBox->setValue(0.0);

    this->updateVolumeInfo();

    d->CropButton->setEnabled(false);
    return;
    }

  d->CropButton->setEnabled(inputCheckErrorMessage.isEmpty());

  d->InputVolumeComboBox->setCurrentNode(d->ParametersNode->GetInputVolumeNode());
  d->InputROIComboBox->setCurrentNode(d->ParametersNode->GetROINode());
  d->OutputVolumeComboBox->setCurrentNode(d->ParametersNode->GetOutputVolumeNode());
  d->InterpolationEnabledCheckBox->setChecked(!d->ParametersNode->GetVoxelBased());

  switch (d->ParametersNode->GetInterpolationMode())
    {
    case vtkMRMLCropVolumeParametersNode::InterpolationNearestNeighbor: d->NNRadioButton->setChecked(true); break;
    case vtkMRMLCropVolumeParametersNode::InterpolationLinear: d->LinearRadioButton->setChecked(true); break;
    case vtkMRMLCropVolumeParametersNode::InterpolationWindowedSinc: d->WSRadioButton->setChecked(true); break;
    case vtkMRMLCropVolumeParametersNode::InterpolationBSpline: d->BSRadioButton->setChecked(true); break;
    }
  d->IsotropicCheckbox->setChecked(d->ParametersNode->GetIsotropicResampling());
  d->VisibilityButton->setChecked(d->ParametersNode->GetROINode() && (d->ParametersNode->GetROINode()->GetDisplayVisibility() != 0));

  d->SpacingScalingSpinBox->setValue(d->ParametersNode->GetSpacingScalingConst());

  d->FillValueSpinBox->setValue(d->ParametersNode->GetFillValue());

  this->updateVolumeInfo();
}

//-----------------------------------------------------------
bool qSlicerCropVolumeModuleWidget::setEditedNode(vtkMRMLNode* node,
                                                  QString role /* = QString()*/,
                                                  QString context /* = QString()*/)
{
  Q_D(qSlicerCropVolumeModuleWidget);
  Q_UNUSED(role);
  Q_UNUSED(context);

  if (vtkMRMLCropVolumeParametersNode::SafeDownCast(node))
    {
    d->ParametersNodeComboBox->setCurrentNode(node);
    return true;
    }
  return false;
}

//------------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setParametersNode(vtkMRMLNode* node)
{
  vtkMRMLCropVolumeParametersNode* parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(node);
  Q_D(qSlicerCropVolumeModuleWidget);
  qvtkReconnect(d->ParametersNode, parametersNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->ParametersNode = parametersNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::updateVolumeInfo()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (d->VolumeInformationCollapsibleButton->collapsed())
    {
    return;
    }

  vtkMRMLVolumeNode* inputVolumeNode = nullptr;
  if (d->ParametersNode != nullptr)
    {
    inputVolumeNode = d->ParametersNode->GetInputVolumeNode();
    }
  if (inputVolumeNode != nullptr && inputVolumeNode->GetImageData() != nullptr)
    {
    int *dimensions = inputVolumeNode->GetImageData()->GetDimensions();
    d->InputDimensionsWidget->setCoordinates(dimensions[0], dimensions[1], dimensions[2]);
    d->InputSpacingWidget->setCoordinates(inputVolumeNode->GetSpacing());
    }
  else
    {
    d->InputDimensionsWidget->setCoordinates(0, 0, 0);
    d->InputSpacingWidget->setCoordinates(0, 0, 0);
    }

  int outputExtent[6] = { 0, -1, -0, -1, 0, -1 };
  double outputSpacing[3] = { 0 };
  if (d->ParametersNode != nullptr && d->ParametersNode->GetInputVolumeNode())
    {
    if (d->ParametersNode->GetVoxelBased())
      {
      d->logic()->GetVoxelBasedCropOutputExtent(d->ParametersNode->GetROINode(), d->ParametersNode->GetInputVolumeNode(), outputExtent);
      d->ParametersNode->GetInputVolumeNode()->GetSpacing(outputSpacing);
      }
    else
      {
      d->logic()->GetInterpolatedCropOutputGeometry(d->ParametersNode->GetROINode(), d->ParametersNode->GetInputVolumeNode(),
        d->ParametersNode->GetIsotropicResampling(), d->ParametersNode->GetSpacingScalingConst(),
        outputExtent, outputSpacing);
      }
    }

  d->CroppedDimensionsWidget->setCoordinates(outputExtent[1] - outputExtent[0] + 1,
    outputExtent[3] - outputExtent[2] + 1, outputExtent[5] - outputExtent[4] + 1);
  d->CroppedSpacingWidget->setCoordinates(outputSpacing);
}

//------------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onVolumeInformationSectionClicked(bool isOpen)
{
  if (isOpen)
  {
    this->updateVolumeInfo();
  }
}
