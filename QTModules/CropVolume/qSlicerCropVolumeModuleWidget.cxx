// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerCropVolumeModuleWidget.h"
#include "qMRMLNodeFactory.h"
#include "ui_qSlicerCropVolumeModule.h"
#include "vtkMRMLAnnotationROINode.h"

#include <QMessageBox>

#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkSlicerCropVolumeLogic.h>

//-----------------------------------------------------------------------------
class qSlicerCropVolumeModuleWidgetPrivate: public Ui_qSlicerCropVolumeModule
{
  Q_DECLARE_PUBLIC(qSlicerCropVolumeModuleWidget);
protected:
  qSlicerCropVolumeModuleWidget* const q_ptr;
public:

  qSlicerCropVolumeModuleWidgetPrivate(qSlicerCropVolumeModuleWidget& object);
  ~qSlicerCropVolumeModuleWidgetPrivate();

  vtkSlicerCropVolumeLogic*
    logic() const;

protected slots:

protected:

private:

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
vtkSlicerCropVolumeLogic*
qSlicerCropVolumeModuleWidgetPrivate::logic() const
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
}

//-----------------------------------------------------------------------------
qSlicerCropVolumeModuleWidget::~qSlicerCropVolumeModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModuleWidget::setup()
{
  qDebug() << "setting up CropVolume widget";

  Q_D(qSlicerCropVolumeModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  connect( d->InputROIComboBox->nodeFactory(), 
    SIGNAL(nodeInitialized(vtkMRMLNode*)),
    this, SLOT(initializeNode(vtkMRMLNode*)));

  connect( d->CropButton, SIGNAL(clicked()),
    this, SLOT(onApply()) );
  connect( d->InputVolumeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onInputVolumeChanged()));
  connect( d->InputROIComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onInputROIChanged()));
  connect( d->OutputVolumeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onInputROIChanged()));
  connect( d->VisibilityButton, SIGNAL(toggled(bool)),
    this, SLOT(onROIVisibilityChanged()));
  connect( d->LinearRadioButton, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationModeChanged()));
  connect( d->NNRadioButton, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationModeChanged()));
  connect( d->CubicRadioButton, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationModeChanged()));
  connect( d->IsotropicCheckbox, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationModeChanged()));
{

  Q_D(qSlicerCropVolumeModuleWidget);
  vtkSlicerCropVolumeLogic *l = vtkSlicerCropVolumeLogic::SafeDownCast(d->logic());
  vtkMRMLScene* scene = l->GetMRMLScene();
  vtkCollection *pnc = scene->GetNodesByClass("vtkMRMLCropVolumeParametersNode");
  if(pnc->GetNumberOfItems()){
    this->parametersNode = vtkMRMLCropVolumeParametersNode::SafeDownCast(pnc->GetItemAsObject(0));
    if(!this->parametersNode){
      qDebug() << "Fatal error";
      abort();
    }
    std::cerr << "FOUND crop vol param node" << std::endl;

  } else {
    qDebug() << "No CropVolume parameter nodes found!";
    this->parametersNode = vtkMRMLCropVolumeParametersNode::New();
    scene->AddNodeNoNotify(this->parametersNode);
    this->parametersNode->Delete();
  }
  pnc->Delete();

  this->updateWidget();
  //std::cout << "Scene is set to " <<
  }
}

void qSlicerCropVolumeModuleWidget::initializeNode(vtkMRMLNode *n)
{
  vtkMRMLScene* scene = qobject_cast<qMRMLNodeFactory*>(this->sender())->mrmlScene();
  vtkMRMLAnnotationROINode::SafeDownCast(n)->Initialize(scene);
}

void qSlicerCropVolumeModuleWidget::onApply(){
  QMessageBox::information(this, tr("CropVolume"),
                       tr("This module is under development and not ready for your use at this time."));

  Q_D(const qSlicerCropVolumeModuleWidget);
  vtkSlicerCropVolumeLogic *l = vtkSlicerCropVolumeLogic::SafeDownCast(d->logic());
  l->Apply(this->parametersNode);
}

void qSlicerCropVolumeModuleWidget::onInputVolumeChanged(){
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  vtkMRMLNode* n = d->InputVolumeComboBox->currentNode();
  if(n)
    this->parametersNode->SetAndObserveInputVolumeNodeID(n->GetID());
}

void qSlicerCropVolumeModuleWidget::onInputROIChanged(){
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  vtkMRMLNode* n = d->InputROIComboBox->currentNode();
  if(n)
    this->parametersNode->SetAndObserveROINodeID(n->GetID());
}

void qSlicerCropVolumeModuleWidget::onOutputVolumeChanged(){
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  vtkMRMLNode* n = d->OutputVolumeComboBox->currentNode();
  if(n)
    this->parametersNode->SetAndObserveOutputVolumeNodeID(n->GetID());
}

void qSlicerCropVolumeModuleWidget::onROIVisibilityChanged(){
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  this->parametersNode->SetROIVisibility(d->VisibilityButton->isChecked());
}

void qSlicerCropVolumeModuleWidget::onInterpolationModeChanged(){
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);

}

void qSlicerCropVolumeModuleWidget::updateWidget()
{
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  if(d->InputVolumeComboBox->currentNode() != this->parametersNode->GetInputVolumeNode())
    d->InputVolumeComboBox->setCurrentNode(this->parametersNode->GetInputVolumeNode());

}
