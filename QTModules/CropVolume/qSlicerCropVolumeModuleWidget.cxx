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
  connect( d->OutputVolumeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    this, SLOT(onOutputVolumeChanged()));
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

  if(!strcmp(this->parametersNode->GetInputVolumeNodeID(),
             this->parametersNode->GetOutputVolumeNodeID())){
    QMessageBox::warning(this, QString("CropVolume"),
       QString("Output and input volumes cannot be the same!"));
    return;
  };

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
  if(n){
    this->parametersNode->SetAndObserveROINodeID(n->GetID());
    this->updateWidget();
  }
}

void qSlicerCropVolumeModuleWidget::onOutputVolumeChanged(){
  //std::cerr << "Output volume changed!" << std::endl;
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  vtkMRMLNode* n = d->OutputVolumeComboBox->currentNode();
  //std::cerr << "Got current node" << std::endl;

  if(n){
    this->parametersNode->SetAndObserveOutputVolumeNodeID(n->GetID());
    //std::cerr << "Output volume node is now " << n->GetID() << std::endl;
  }
}

void qSlicerCropVolumeModuleWidget::onROIVisibilityChanged(){
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  this->parametersNode->SetROIVisibility(d->VisibilityButton->isChecked());
  this->parametersNode->GetROINode()->SetVisibility(d->VisibilityButton->isChecked());
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
  if(d->CubicRadioButton->isChecked())
    p->SetInterpolationMode(3);
}

void qSlicerCropVolumeModuleWidget::updateWidget()
{
  if(!this->parametersNode)
    return;

  Q_D(qSlicerCropVolumeModuleWidget);
  vtkMRMLCropVolumeParametersNode *p = this->parametersNode;

  if(d->InputVolumeComboBox->currentNode() != p->GetInputVolumeNode())
    d->InputVolumeComboBox->setCurrentNode(p->GetInputVolumeNode());
  if(d->OutputVolumeComboBox->currentNode() != p->GetOutputVolumeNode()){
    d->OutputVolumeComboBox->setCurrentNode(p->GetOutputVolumeNode());
  }

  d->VisibilityButton->setChecked(p->GetROIVisibility());
  switch(p->GetInterpolationMode()){
    case 1: d->NNRadioButton->setChecked(1); break;
    case 2: d->LinearRadioButton->setChecked(1); break;
    case 3: d->CubicRadioButton->setChecked(1); break;
  }
  d->IsotropicCheckbox->setChecked(p->GetIsotropicResampling());
  if (this->parametersNode->GetROINode())
    {
    this->parametersNode->GetROINode()->SetVisibility(p->GetROIVisibility());
    }
}

void qSlicerCropVolumeModuleWidget::InitializeEventListeners(vtkMRMLCropVolumeParametersNode *n){
  //vtkIntArray *events = vtkIntArray::New();
  //events->InsertNextValue(vtkCommand::ModifiedEvent);
  //
}
