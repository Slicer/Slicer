// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCropVolumeModuleWidget.h"
#include "qMRMLNodeFactory.h"
#include "ui_qSlicerCropVolumeModule.h"
#include "vtkMRMLAnnotationROINode.h"

#include <QMessageBox>

#include <vtkMRMLCropVolumeParametersNode.h>
#include <vtkSlicerCropVolumeLogic.h>
#include <vtkMRMLSelectionNode.h>

#include <vtkMRMLApplicationLogic.h>

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
  connect( d->VisibilityButton, SIGNAL(toggled(bool)),
    this, SLOT(onROIVisibilityChanged()));
  connect( d->LinearRadioButton, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationModeChanged()));
  connect( d->NNRadioButton, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationModeChanged()));
  connect( d->WSRadioButton, SIGNAL(toggled(bool)),
    this, SLOT(onInterpolationModeChanged()));
  connect( d->BSRadioButton, SIGNAL(toggled(bool)),
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
    //InitializeEventListeners(this->parametersNode);
  } else {
    qDebug() << "No CropVolume parameter nodes found!";
    this->parametersNode = vtkMRMLCropVolumeParametersNode::New();
    scene->AddNodeNoNotify(this->parametersNode);
    this->parametersNode->Delete();
  }

  pnc->Delete();

  this->updateWidget();
  }
}

void qSlicerCropVolumeModuleWidget::initializeNode(vtkMRMLNode *n)
{
  vtkMRMLScene* scene = qobject_cast<qMRMLNodeFactory*>(this->sender())->mrmlScene();
  vtkMRMLAnnotationROINode::SafeDownCast(n)->Initialize(scene);
}

void qSlicerCropVolumeModuleWidget::onApply(){

  Q_D(const qSlicerCropVolumeModuleWidget);
  vtkSlicerCropVolumeLogic *l = vtkSlicerCropVolumeLogic::SafeDownCast(d->logic());

  if(!l->Apply(this->parametersNode)){
    std::cerr << "Propagating to the selection node" << std::endl;
    vtkMRMLApplicationLogic *appLogic =
      qSlicerCoreApplication::application()->mrmlApplicationLogic();
    vtkMRMLSelectionNode *selectionNode = appLogic->GetSelectionNode();
    selectionNode->SetReferenceActiveVolumeID(this->parametersNode->GetOutputVolumeNodeID());
    appLogic->PropagateVolumeSelection();
  }
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
  if(n){
    this->parametersNode->SetAndObserveROINodeID(n->GetID());
    this->parametersNode->SetROIVisibility(this->parametersNode->GetROINode()->GetVisibility());
    this->updateWidget();    
  }
}

void qSlicerCropVolumeModuleWidget::onROIVisibilityChanged(){
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  this->parametersNode->SetROIVisibility((int)d->VisibilityButton->isChecked());
  this->parametersNode->GetROINode()->SetVisibility((int)d->VisibilityButton->isChecked());
}

void qSlicerCropVolumeModuleWidget::onInterpolationModeChanged()
{
  if(!this->parametersNode)
    return;
  vtkMRMLCropVolumeParametersNode *p = this->parametersNode;
  Q_D(qSlicerCropVolumeModuleWidget);
  if(d->NNRadioButton->isChecked())
    p->SetInterpolationMode(1);
  if(d->LinearRadioButton->isChecked())
    p->SetInterpolationMode(2);
  if(d->WSRadioButton->isChecked())
    p->SetInterpolationMode(3);
  if(d->BSRadioButton->isChecked())
    p->SetInterpolationMode(4);
}

void qSlicerCropVolumeModuleWidget::updateWidget()
{
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  vtkMRMLCropVolumeParametersNode *p = this->parametersNode;

  if(d->InputVolumeComboBox->currentNode() != (vtkMRMLNode*) p->GetInputVolumeNode())
    d->InputVolumeComboBox->setCurrentNode((vtkMRMLNode*)p->GetInputVolumeNode());

  d->VisibilityButton->setChecked(p->GetROIVisibility());
  switch(p->GetInterpolationMode()){
    case 1: d->NNRadioButton->setChecked(1); break;
    case 2: d->LinearRadioButton->setChecked(1); break;
    case 3: d->WSRadioButton->setChecked(1); break;
    case 4: d->BSRadioButton->setChecked(1); break;
  }
  d->IsotropicCheckbox->setChecked(p->GetIsotropicResampling());
  if (this->parametersNode->GetROINode())
    {   
    d->VisibilityButton->setChecked(p->GetROIVisibility());
    }
}

//void qSlicerCropVolumeModuleWidget::InitializeEventListeners(vtkMRMLCropVolumeParametersNode *n){
//  //vtkIntArray *events = vtkIntArray::New();
//  //events->InsertNextValue(vtkCommand::ModifiedEvent);
//  //
//}
