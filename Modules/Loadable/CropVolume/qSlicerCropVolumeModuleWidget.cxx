// Qt includes
#include <QDebug>
#include <QMessageBox>

// CTK includes
#include <ctkFlowLayout.h>

// SlicerQt includes
#include <qSlicerAbstractCoreModule.h>

// CropVolume includes
#include "qSlicerCropVolumeModuleWidget.h"
#include "ui_qSlicerCropVolumeModule.h"

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
#include <vtkMRMLLinearTransformNode.h>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CropVolume
class qSlicerCropVolumeModuleWidgetPrivate: public Ui_qSlicerCropVolumeModule
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


  vtkSmartPointer<vtkMRMLVolumeNode> inputVolume = vtkMRMLVolumeNode::SafeDownCast(this->InputVolumeComboBox->currentNode());

  if(!inputVolume)
    return false;

   vtkSmartPointer<vtkMRMLLinearTransformNode> volTransform  = vtkMRMLLinearTransformNode::SafeDownCast(inputVolume->GetParentTransformNode());

   if(volTransform)
       return true;


   return false;
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


  vtkSmartPointer<vtkMRMLLinearTransformNode> roiTransform  = vtkMRMLLinearTransformNode::SafeDownCast(inputROI->GetParentTransformNode());

  if(roiTransform)
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
  this->parametersNode = NULL;
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
  connect(d->InputVolumeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onInputVolumeChanged()));
  connect(d->InputROIComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
          this, SLOT(onInputROIChanged()));
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
  connect(d->VoxelBasedModeRadioButton,SIGNAL(toggled(bool)),this, SLOT(onVoxelBasedChecked(bool)) );
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::enter()
{
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

  // observe close event
  qvtkReconnect(this->mrmlScene(), vtkMRMLScene::EndCloseEvent, 
    this, SLOT(onEndCloseEvent()));
}

void qSlicerCropVolumeModuleWidget::initializeParameterNode(vtkMRMLScene* scene)
{
  vtkCollection* parameterNodes = scene->GetNodesByClass("vtkMRMLCropVolumeParametersNode");

  if(parameterNodes->GetNumberOfItems() > 0)
    {
    this->parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(parameterNodes->GetItemAsObject(0));
    if(!this->parametersNode)
      {
      qCritical() << "FATAL ERROR: Cannot instantiate CropVolumeParameterNode";
      Q_ASSERT(this->parametersNode);
      }
    }
  else
    {
    qDebug() << "No CropVolume parameter nodes found!";
    this->parametersNode = vtkMRMLCropVolumeParametersNode::New();
    scene->AddNode(this->parametersNode);
    this->parametersNode->Delete();
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
void qSlicerCropVolumeModuleWidget::onApply(){

  Q_D(const qSlicerCropVolumeModuleWidget);
  vtkSlicerCropVolumeLogic *logic = d->logic();
  if(!logic->Apply(this->parametersNode))
    {
    std::cerr << "Propagating to the selection node" << std::endl;
    vtkSlicerApplicationLogic *appLogic = this->module()->appLogic();
    vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
    selectionNode->SetReferenceActiveVolumeID(this->parametersNode->GetOutputVolumeNodeID());
    appLogic->PropagateVolumeSelection();
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputVolumeChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  Q_ASSERT(d->InputVolumeComboBox);
  Q_ASSERT(d->VoxelBasedModeRadioButton);

  if(!this->parametersNode)
    {
    return;
    }
  vtkMRMLNode* node = d->InputVolumeComboBox->currentNode();
  if(node)
    {
    this->parametersNode->SetInputVolumeNodeID(node->GetID());


    if(d->VoxelBasedModeRadioButton->isChecked())
      {
        if(d->checkForVolumeParentTransform())
          {
            d->showUnsupportedTransVolumeVoxelCroppingDialog();
            d->InputVolumeComboBox->setCurrentNode(NULL);
          }
        else
          {
            d->performROIVoxelGridAlignment();
          }
      }


    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputROIChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  Q_ASSERT(d->VoxelBasedModeRadioButton);

  if(!this->parametersNode)
    {
    return;
    }
  vtkMRMLAnnotationROINode* node = 
    vtkMRMLAnnotationROINode::SafeDownCast(d->InputROIComboBox->currentNode());
  if(node)
    {
    this->parametersNode->SetROINodeID(node->GetID());
    this->parametersNode->SetROIVisibility(node->GetDisplayVisibility());
    this->updateWidget();

    if(d->VoxelBasedModeRadioButton->isChecked())
      d->performROIVoxelGridAlignment();

    }
}


//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onROIVisibilityChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if(!this->parametersNode)
    {
    return;
    }
  this->parametersNode->SetROIVisibility(d->VisibilityButton->isChecked());
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
  vtkMRMLCropVolumeParametersNode *parameterNode = this->parametersNode;
  if(!this->parametersNode)
    {
    return;
    }
  if(d->NNRadioButton->isChecked())
    {
    parameterNode->SetInterpolationMode(1);
    }
  if(d->LinearRadioButton->isChecked())
    {
    parameterNode->SetInterpolationMode(2);
    }
  if(d->WSRadioButton->isChecked())
    {
    parameterNode->SetInterpolationMode(3);
    }
  if(d->BSRadioButton->isChecked())
    {
    parameterNode->SetInterpolationMode(4);
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onSpacingScalingValueChanged(double s)
{
  if(!this->parametersNode)
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *p = this->parametersNode;
  p->SetSpacingScalingConst(s);
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onIsotropicModeChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if(!this->parametersNode)
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *p = this->parametersNode;
  p->SetIsotropicResampling(d->IsotropicCheckbox->isChecked());
}

//-----------------------------------------------------------------------------
void
qSlicerCropVolumeModuleWidget::onVoxelBasedChecked(bool checked)
{
  Q_D(qSlicerCropVolumeModuleWidget);

  if (this->parametersNode)
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
  if(!this->parametersNode)
    return;
  vtkMRMLCropVolumeParametersNode *pNode = this->parametersNode;

  vtkMRMLNode *volumeNode = d->InputVolumeComboBox->currentNode();
  vtkMRMLNode *roiNode = d->InputROIComboBox->currentNode();

  if(volumeNode)
    pNode->SetInputVolumeNodeID(volumeNode->GetID());
  else
    pNode->SetInputVolumeNodeID(NULL);


  if(roiNode)
    pNode->SetROINodeID(roiNode->GetID());
  else
    pNode->SetROINodeID(NULL);

  pNode->SetROIVisibility(d->VisibilityButton->isChecked());

  if(d->NNRadioButton->isChecked())
    pNode->SetInterpolationMode(1);
  else if(d->LinearRadioButton->isChecked())
    pNode->SetInterpolationMode(2);
  else if(d->WSRadioButton->isChecked())
    pNode->SetInterpolationMode(3);
  else if(d->BSRadioButton->isChecked())
    pNode->SetInterpolationMode(4);

  if(d->IsotropicCheckbox->isChecked())
    pNode->SetIsotropicResampling(1);

  pNode->SetSpacingScalingConst(d->SpacingScalingSpinBox->value());
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::updateWidget()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if(!this->parametersNode || !this->mrmlScene())
    {
    return;
    }
  vtkMRMLCropVolumeParametersNode *parameterNode = this->parametersNode;

  vtkMRMLNode *volumeNode = this->mrmlScene()->GetNodeByID(parameterNode->GetInputVolumeNodeID());
  vtkMRMLNode *roiNode = this->mrmlScene()->GetNodeByID(parameterNode->GetROINodeID());

  if(volumeNode)
    d->InputVolumeComboBox->setCurrentNode(volumeNode);
  if(roiNode)
    d->InputROIComboBox->setCurrentNode(roiNode);

  d->VisibilityButton->setChecked(parameterNode->GetROIVisibility());
  switch(parameterNode->GetInterpolationMode())
    {
    case 1: d->NNRadioButton->setChecked(1); break;
    case 2: d->LinearRadioButton->setChecked(1); break;
    case 3: d->WSRadioButton->setChecked(1); break;
    case 4: d->BSRadioButton->setChecked(1); break;
    }
  d->IsotropicCheckbox->setChecked(parameterNode->GetIsotropicResampling());
  d->VisibilityButton->setChecked(parameterNode->GetROIVisibility());

  d->SpacingScalingSpinBox->setValue(parameterNode->GetSpacingScalingConst());
}



