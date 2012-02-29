// Qt includes
#include <QDebug>

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

// MRML includes
#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkMRMLVolumeNode.h>
#include <vtkMRMLSelectionNode.h>

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

}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::enter()
{
  this->onInputVolumeChanged();
  this->onInputROIChanged();
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
  if(!this->parametersNode)
    {
    return;
    }
  vtkMRMLNode* node = d->InputVolumeComboBox->currentNode();
  if(node)
    {
    this->parametersNode->SetInputVolumeNodeID(node->GetID());
    }
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::onInputROIChanged()
{
  Q_D(qSlicerCropVolumeModuleWidget);
  if(!this->parametersNode)
    {
    return;
    }
  vtkMRMLAnnotationROINode* node = 
    vtkMRMLAnnotationROINode::SafeDownCast(d->InputROIComboBox->currentNode());
  if(node)
    {
    this->parametersNode->SetROINodeID(node->GetID());
    this->parametersNode->SetROIVisibility(node->GetVisibility());
    this->updateWidget();
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
    node->SetVisibility(d->VisibilityButton->isChecked());
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
