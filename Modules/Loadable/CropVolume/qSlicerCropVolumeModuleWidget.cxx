// Qt includes
#include <QDebug>
#include <QMessageBox>

// CTK includes
#include <ctkFlowLayout.h>

// SlicerQt includes
#include <qSlicerAbstractCoreModule.h>

// CropVolume includes
#include "qSlicerCropVolumeModuleWidget.h"
#include "ui_qSlicerCropVolumeModuleWidget.h"

// CropVolume Logic includes
#include <vtkSlicerCropVolumeLogic.h>

// qMRML includes
#include <qMRMLNodeFactory.h>

// MRMLAnnotation includes
#include <vtkMRMLAnnotationROINode.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

#include <vtkNew.h>
#include <vtkMatrix4x4.h>


// MRML includes
#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkMRMLVolumeNode.h>
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

  void performROIVoxelGridAlignment();
  bool checkForVolumeParentTransform() const;
  void showUnsupportedTransVolumeVoxelCroppingDialog() const;

};

//-----------------------------------------------------------------------------
// qSlicerCropVolumeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidgetPrivate::qSlicerCropVolumeModuleWidgetPrivate(qSlicerCropVolumeModuleWidget& object) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidgetPrivate::~qSlicerCropVolumeModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic* qSlicerCropVolumeModuleWidgetPrivate::logic() const
{
  Q_Q(const qSlicerCropVolumeModuleWidget);
  return vtkSlicerCropVolumeLogic::SafeDownCast(q->logic());
}


//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidgetPrivate::showUnsupportedTransVolumeVoxelCroppingDialog() const
{
  QMessageBox::information(NULL,"Crop Volume","The selected volume is under a transform. Voxel based cropping is only supported for non transformed volumes!");
}
//-----------------------------------------------------------------------------
bool qSlicerCropVolumeModuleWidgetPrivate::checkForVolumeParentTransform() const
{
  Q_ASSERT(this->InputVolumeComboBox);

  vtkMRMLVolumeNode* inputVolume = vtkMRMLVolumeNode::SafeDownCast(this->InputVolumeComboBox->currentNode());
  if(!inputVolume)
    {
    return false;
    }
  vtkMRMLTransformNode* volTransform = inputVolume->GetParentTransformNode();
  if(!volTransform)
    {
    return false;
    }
  // we ignore non-linear transforms
  return volTransform->IsTransformToWorldLinear();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidgetPrivate::performROIVoxelGridAlignment()
{
  Q_ASSERT(this->InputVolumeComboBox);
  Q_ASSERT(this->InputROIComboBox);
  Q_ASSERT(this->InterpolationModeRadioButton);
  Q_ASSERT(this->VoxelBasedModeRadioButton);

  vtkSmartPointer<vtkMRMLVolumeNode> inputVolume = vtkMRMLVolumeNode::SafeDownCast(this->InputVolumeComboBox->currentNode());
  vtkSmartPointer<vtkMRMLAnnotationROINode> inputROI = vtkMRMLAnnotationROINode::SafeDownCast(this->InputROIComboBox->currentNode());

  if( !inputVolume || !inputROI || this->VoxelBasedModeRadioButton->isChecked() == false)
    return;

  vtkNew<vtkMatrix4x4> volRotMat;
  bool volumeTilted = vtkSlicerCropVolumeLogic::IsVolumeTiltedInRAS(inputVolume,volRotMat.GetPointer());


  vtkMRMLTransformNode* roiTransform = inputROI->GetParentTransformNode();

  if(roiTransform && roiTransform->IsTransformToWorldLinear())
    {
      vtkNew<vtkMatrix4x4> parentTransform;
      roiTransform->GetMatrixTransformToWorld(parentTransform.GetPointer());

      bool same = true;

      for(int i=0; i < 4; ++i)
        {
          for(int j=0; j < 4; ++j)
            {
                if(parentTransform->GetElement(i,j) != volRotMat->GetElement(i,j))
                  {
                    same = false;
                    break;
                  }
            }

          if (same == false)
            break;
        }
      if(!same)
        {
          QString message = "The selected ROI has a transform which is neither an identity transform nor a Crop Volume voxelgrid alignment transform for the selected volume. In order to perform voxel based cropping a new transform will be applied to the ROI.\n\nDo you want to continue and reset the ROI's transform?";
          int ret = QMessageBox::information(NULL,"Crop Volume",message,QMessageBox::Yes,QMessageBox::No);

          if(ret == QMessageBox::Yes)
            {
              inputROI->SetAndObserveTransformNodeID(0);
            }
          else if(ret == QMessageBox::No)
            {
              this->InputROIComboBox->setCurrentNode(NULL);
              return;
            }
        }
    }

  if(volumeTilted)
    {
      vtkSlicerCropVolumeLogic* logic = this->logic();
      Q_ASSERT(logic);
      logic->SnapROIToVoxelGrid(inputROI,inputVolume);
    }

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
qSlicerCropVolumeModuleWidget::~qSlicerCropVolumeModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setup()
{
  Q_D(qSlicerCropVolumeModuleWidget);

  d->setupUi(this);
  ctkFlowLayout* flowLayout = ctkFlowLayout::replaceLayout(d->InterpolatorWidget);
  flowLayout->setPreferredExpandingDirections(Qt::Vertical);

  this->Superclass::setup();

  connect(d->InputROIComboBox->nodeFactory(), SIGNAL(nodeInitialized(vtkMRMLNode*)),
          this, SLOT(initializeNode(vtkMRMLNode*)));

  connect(d->CropButton, SIGNAL(clicked()),
          this, SLOT(onApply()) );

  connect(d->ParametersNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(updateWidget()));
  connect(d->ParametersNodeComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
          this, SLOT(updateWidget()));

  connect(d->InputVolumeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onInputVolumeChanged()));
  // observe nodeAdded signal so can select the new one if nothing is selected
  connect(d->InputVolumeComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)),
          this, SLOT(onInputVolumeAdded(vtkMRMLNode*)));

  connect(d->InputROIComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onInputROIChanged()));
  connect(d->InputROIComboBox, SIGNAL(nodeAdded(vtkMRMLNode*)),
          this, SLOT(onInputROIAdded(vtkMRMLNode*)));

  connect(d->VisibilityButton, SIGNAL(toggled(bool)),
          this, SLOT(onROIVisibilityChanged()));
  connect(d->LinearRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->NNRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->WSRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->BSRadioButton, SIGNAL(toggled(bool)),
          this, SLOT(onInterpolationModeChanged()));
  connect(d->IsotropicCheckbox, SIGNAL(toggled(bool)),
          this, SLOT(onIsotropicModeChanged()));
  connect(d->SpacingScalingSpinBox, SIGNAL(valueChanged(double)),
          this, SLOT(onSpacingScalingValueChanged(double)));
  connect(d->VoxelBasedModeRadioButton,SIGNAL(toggled(bool)),
          this, SLOT(onVoxelBasedChecked(bool)) );
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::enter()
{
  // make sure that there's a parameters node so if there are already some
  // volumes or ROIs in the scene, they can be set up for use
  this->initializeParameterNode(this->mrmlScene());

  this->onInputVolumeChanged();
  this->onInputROIChanged();

  this->Superclass::enter();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);
  if(scene == NULL)
    {
    return;
    }

  this->initializeParameterNode(scene);

  this->updateWidget();

  // observe close event so can re-add a parameters node if necessary
  qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent,
                this, SLOT(onEndCloseEvent()));

}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::initializeParameterNode(vtkMRMLScene* scene)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!scene)
    {
    return;
    }

  vtkCollection* parameterNodes = scene->GetNodesByClass("vtkMRMLCropVolumeParametersNode");

  if (parameterNodes->GetNumberOfItems() > 0)
    {
    vtkMRMLCropVolumeParametersNode *parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(parameterNodes->GetItemAsObject(0));
    if (!parametersNode)
      {
      qCritical() << "FATAL ERROR: Cannot get CropVolumeParameterNode";
      Q_ASSERT(parametersNode);
      }
    }
  else
    {
    vtkMRMLCropVolumeParametersNode *parametersNode = vtkMRMLCropVolumeParametersNode::New();
    scene->AddNode(parametersNode);
    d->ParametersNodeComboBox->setCurrentNode(parametersNode);
    parametersNode->Delete();
    }

  parameterNodes->Delete();
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::initializeNode(vtkMRMLNode *n)
{
  vtkMRMLScene* scene = qobject_cast<qMRMLNodeFactory*>(this->sender())->mrmlScene();
  vtkMRMLAnnotationROINode::SafeDownCast(n)->Initialize(scene);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onApply()
{

  Q_D(const qSlicerCropVolumeModuleWidget);
  vtkSlicerCropVolumeLogic *logic = d->logic();

  if(!d->ParametersNodeComboBox->currentNode() ||
     !d->InputVolumeComboBox->currentNode() ||
     !d->InputROIComboBox->currentNode())
    {
    return;
    }

  vtkMRMLCropVolumeParametersNode *parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  parametersNode->SetInputVolumeNodeID(d->InputVolumeComboBox->currentNode()->GetID());
  parametersNode->SetROINodeID(d->InputROIComboBox->currentNode()->GetID());

  if (!logic->Apply(parametersNode))
    {
    vtkSlicerApplicationLogic *appLogic = this->module()->appLogic();
    vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
    selectionNode->SetReferenceActiveVolumeID(parametersNode->GetOutputVolumeNodeID());
    appLogic->PropagateVolumeSelection();
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputVolumeChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  Q_ASSERT(d->InputVolumeComboBox);
  Q_ASSERT(d->VoxelBasedModeRadioButton);

  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  if (d->ParametersNodeComboBox->currentNode())
    {
    parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
    }

  vtkMRMLNode* node = d->InputVolumeComboBox->currentNode();
  if (node)
    {
    if (d->VoxelBasedModeRadioButton->isChecked())
      {
      if (d->checkForVolumeParentTransform())
        {
        d->showUnsupportedTransVolumeVoxelCroppingDialog();
        d->InputVolumeComboBox->setCurrentNode(NULL);
        }
      else
        {
        d->performROIVoxelGridAlignment();
        }
      }
    if (parametersNode)
      {
      parametersNode->SetInputVolumeNodeID(node->GetID());
      if (this->mrmlScene() &&
          !this->mrmlScene()->IsClosing() &&
          !this->mrmlScene()->IsBatchProcessing())
        {
        // set it to be active in the slice windows
        vtkSlicerApplicationLogic *appLogic = this->module()->appLogic();
        vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
        selectionNode->SetReferenceActiveVolumeID(parametersNode->GetInputVolumeNodeID());
        appLogic->PropagateVolumeSelection();
        }
      }
    }
  else
    {
    if (parametersNode)
      {
      parametersNode->SetInputVolumeNodeID(NULL);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputVolumeAdded(vtkMRMLNode *mrmlNode)
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!mrmlNode)
    {
    return;
    }
  // check if there's a parameter node first since if there isn't, updateWidget
  // will reset this to unselected
  if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  if (d->InputVolumeComboBox->currentNode() != NULL)
    {
    // there's already a selected node, don't reset it
    return;
    }
  d->InputVolumeComboBox->setCurrentNode(mrmlNode);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputROIChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  Q_ASSERT(d->VoxelBasedModeRadioButton);

  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  if (d->ParametersNodeComboBox->currentNode())
    {
    parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
    }

  vtkMRMLNode *node =  d->InputROIComboBox->currentNode();
  if (node)
    {
    vtkMRMLAnnotationROINode* roiNode =
      vtkMRMLAnnotationROINode::SafeDownCast(node);
    if (roiNode)
      {
      if (d->VoxelBasedModeRadioButton->isChecked())
        {
        d->performROIVoxelGridAlignment();
        }
      d->VisibilityButton->setChecked(roiNode->GetDisplayVisibility());
      if (parametersNode)
        {
        parametersNode->SetROINodeID(roiNode->GetID());
        }
      }
    else
      {
      if (parametersNode)
        {
        parametersNode->SetROINodeID(NULL);
        }
      }
    }
  else
    {
    if (parametersNode)
      {
      parametersNode->SetROINodeID(NULL);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputROIAdded(vtkMRMLNode *mrmlNode)
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!mrmlNode)
    {
    return;
    }
  // check if there's a parameter node first since if there isn't, updateWidget
  // will reset this to unselected
  if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  if (d->InputROIComboBox->currentNode() != NULL)
    {
    // there's already a selected node, don't reset it
    return;
    }
  d->InputROIComboBox->setCurrentNode(mrmlNode);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onROIVisibilityChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    return;
    }
  parametersNode->SetROIVisibility(d->VisibilityButton->isChecked());
  vtkMRMLAnnotationROINode* node =
    vtkMRMLAnnotationROINode::SafeDownCast(d->InputROIComboBox->currentNode());
  if (node)
    {
    node->SetDisplayVisibility(d->VisibilityButton->isChecked());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInterpolationModeChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
   if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
      return;
    }
  if(d->NNRadioButton->isChecked())
    {
    parametersNode->SetInterpolationMode(1);
    }
  if(d->LinearRadioButton->isChecked())
    {
    parametersNode->SetInterpolationMode(2);
    }
  if(d->WSRadioButton->isChecked())
    {
    parametersNode->SetInterpolationMode(3);
    }
  if(d->BSRadioButton->isChecked())
    {
    parametersNode->SetInterpolationMode(4);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onSpacingScalingValueChanged(double s)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    return;
    }
  parametersNode->SetSpacingScalingConst(s);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onIsotropicModeChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    return;
    }
  parametersNode->SetIsotropicResampling(d->IsotropicCheckbox->isChecked());
}

//-----------------------------------------------------------------------------
void
qSlicerCropVolumeModuleWidget::onVoxelBasedChecked(bool checked)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    return;
    }
  parametersNode->SetVoxelBased(checked);

  if (d->checkForVolumeParentTransform())
    {
    d->showUnsupportedTransVolumeVoxelCroppingDialog();
    d->InputVolumeComboBox->setCurrentNode(NULL);
    }
  else
    {
    d->performROIVoxelGridAlignment();
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onEndCloseEvent()
{
  this->initializeParameterNode(this->mrmlScene());
}


//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::updateParameters()
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (!d->ParametersNodeComboBox->currentNode())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    return;
    }

  vtkMRMLNode *volumeNode = d->InputVolumeComboBox->currentNode();
  vtkMRMLNode *roiNode = d->InputROIComboBox->currentNode();

  if (volumeNode)
    {
    parametersNode->SetInputVolumeNodeID(volumeNode->GetID());
    }
  else
    {
    parametersNode->SetInputVolumeNodeID(NULL);
    }

  if (roiNode)
    {
    parametersNode->SetROINodeID(roiNode->GetID());
    }
  else
    {
    parametersNode->SetROINodeID(NULL);
    }

  parametersNode->SetVoxelBased(d->VoxelBasedModeRadioButton->isChecked());
  parametersNode->SetROIVisibility(d->VisibilityButton->isChecked());

  if (d->NNRadioButton->isChecked())
    parametersNode->SetInterpolationMode(1);
  else if (d->LinearRadioButton->isChecked())
    parametersNode->SetInterpolationMode(2);
  else if (d->WSRadioButton->isChecked())
    parametersNode->SetInterpolationMode(3);
  else if (d->BSRadioButton->isChecked())
    parametersNode->SetInterpolationMode(4);

  parametersNode->SetIsotropicResampling(d->IsotropicCheckbox->isChecked());

  parametersNode->SetSpacingScalingConst(d->SpacingScalingSpinBox->value());
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::updateWidget()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if (!this->mrmlScene())
    {
    return;
    }

  if (!d->ParametersNodeComboBox->currentNode())
    {
    // reset widget to defaults from node class
    d->InputVolumeComboBox->setCurrentNode(NULL);
    d->InputROIComboBox->setCurrentNode(NULL);

    d->VoxelBasedModeRadioButton->setChecked(false);
    d->VisibilityButton->setChecked(true);

    d->IsotropicCheckbox->setChecked(false);
    d->SpacingScalingSpinBox->setValue(1.0);
    d->LinearRadioButton->setChecked(1);

    return;
    }

  vtkMRMLCropVolumeParametersNode *parametersNode = NULL;
  parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(d->ParametersNodeComboBox->currentNode());
  if (!parametersNode)
    {
    return;
    }

  char *volumeNodeID = parametersNode->GetInputVolumeNodeID();
  int nodeCount = d->InputVolumeComboBox->nodeCount();
  if (!volumeNodeID && nodeCount != 0)
    {
    // the parameters node doesn't have a volume set, but there is a volume in the
    // combo box, so use the last one
    int nodeIndex = nodeCount - 1;
    volumeNodeID = d->InputVolumeComboBox->nodeFromIndex(nodeIndex)->GetID();
    // then reset it on the param node?
    parametersNode->SetInputVolumeNodeID(volumeNodeID);
    }
  vtkMRMLNode *volumeNode = this->mrmlScene()->GetNodeByID(volumeNodeID);
  d->InputVolumeComboBox->setCurrentNode(volumeNode);

  char *roiNodeID = parametersNode->GetROINodeID();
  if (!roiNodeID && d->InputROIComboBox->nodeCount() != 0)
    {
    int nodeIndex =  d->InputROIComboBox->nodeCount() - 1;
    roiNodeID =  d->InputROIComboBox->nodeFromIndex(nodeIndex)->GetID();
    parametersNode->SetROINodeID(roiNodeID);
    }
  vtkMRMLNode *roiNode = this->mrmlScene()->GetNodeByID(roiNodeID);
  d->InputROIComboBox->setCurrentNode(roiNode);

  if (parametersNode->GetVoxelBased())
    {
    d->VoxelBasedModeRadioButton->setChecked(true);
    }
  else
    {
    d->InterpolationModeRadioButton->setChecked(true);
    }
  d->VisibilityButton->setChecked(parametersNode->GetROIVisibility());

  switch (parametersNode->GetInterpolationMode())
    {
    case 1: d->NNRadioButton->setChecked(1); break;
    case 2: d->LinearRadioButton->setChecked(1); break;
    case 3: d->WSRadioButton->setChecked(1); break;
    case 4: d->BSRadioButton->setChecked(1); break;
    }
  d->IsotropicCheckbox->setChecked(parametersNode->GetIsotropicResampling());
  d->VisibilityButton->setChecked(parametersNode->GetROIVisibility());

  d->SpacingScalingSpinBox->setValue(parametersNode->GetSpacingScalingConst());
}
