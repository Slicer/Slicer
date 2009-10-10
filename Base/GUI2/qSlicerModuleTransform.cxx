#include "qSlicerModuleTransform.h" 

#include "ui_qSlicerModuleTransform.h" 

#include "vtkMRMLLinearTransformNode.h"

#include <QVector>
#include <QButtonGroup>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerModuleTransform::qInternal : public Ui::qSlicerModuleTransform
{
public:
  qInternal()
    {
    this->CoordinateReferenceButtonGroup = 0; 
    this->MRMLTransformNode = 0; 
    }
  QButtonGroup*                 CoordinateReferenceButtonGroup; 
  vtkMRMLLinearTransformNode*   MRMLTransformNode;
};

//-----------------------------------------------------------------------------
qSlicerModuleTransform::qSlicerModuleTransform(QWidget *parent) : Superclass(parent)
{
  this->Internal = new qInternal;
  this->Internal->setupUi(this);

  // TODO After the TransformSliders widget is added to QtDesigner, 
  // the TypeOfTransform, title property, singleStep will be set from the designer itself
  this->Internal->TranslationSliders->setTypeOfTransform(qMRMLTransformSliders::TRANSLATION);
  this->Internal->TranslationSliders->setSingleStep(0.1);
  this->Internal->RotationSliders->setTypeOfTransform(qMRMLTransformSliders::ROTATION);
  this->Internal->RotationSliders->setTitle("Rotation");
  this->Internal->RotationSliders->setSingleStep(0.1);
  
  // TODO Range should be dynamic (function of the input data/transform OR set by the user)
  this->Internal->TranslationSliders->setRange(-200.00, 200.00);
  this->Internal->RotationSliders->setRange(-200.00, 200.00);

  // TODO After the SelectorNode widget is added to QtDesigner
  // the NodeType property will be set from the designed itself
  // Set selector node type
  this->Internal->TransformNodeSelector->setNodeType("vtkMRMLLinearTransformNode"); 
  
  // Add coordinate reference button to a button group
  this->Internal->CoordinateReferenceButtonGroup = 
    new QButtonGroup(this->Internal->CoordinateReferenceGroupBox); 
  this->Internal->CoordinateReferenceButtonGroup->addButton(
    this->Internal->GlobalRadioButton, qMRMLTransformSliders::GLOBAL); 
  this->Internal->CoordinateReferenceButtonGroup->addButton(
    this->Internal->LocalRadioButton, qMRMLTransformSliders::LOCAL); 
    
  // Connect button group 
  this->connect(this->Internal->CoordinateReferenceButtonGroup, 
                SIGNAL(buttonPressed(int)),
                SLOT(onCoordinateReferenceButtonPressed(int)));
  
  // Connect identity button 
  this->connect(this->Internal->IdentityPushButton, 
                SIGNAL(pressed()),
                SLOT(onIdentityButtonPressed()));
  
  // Connect node selector with translation sliders
  this->connect(this->Internal->TransformNodeSelector, SIGNAL(nodeSelected(vtkMRMLNode*)), 
                this->Internal->TranslationSliders, SLOT(setMRMLTransformNode(vtkMRMLNode*)));
    
  // Connect node selector with rotation sliders
  this->connect(this->Internal->TransformNodeSelector, SIGNAL(nodeSelected(vtkMRMLNode*)), 
                this->Internal->RotationSliders, SLOT(setMRMLTransformNode(vtkMRMLNode*)));
    
  // Connect node selector with module itself
  this->connect(this->Internal->TransformNodeSelector, 
                SIGNAL(nodeSelected(vtkMRMLNode*)), 
                SLOT(onNodeSelected(vtkMRMLNode*)));
    
  // Connect node selector with matrix widget
  this->connect(this->Internal->TransformNodeSelector, SIGNAL(nodeSelected(vtkMRMLNode*)), 
                this->Internal->MatrixWidget, SLOT(setMRMLTransformNode(vtkMRMLNode*)));
  
  // Reset Rotation sliders if at least one of the translation sliders is moved
  this->connect(this->Internal->TranslationSliders, SIGNAL(sliderMoved()), 
                this->Internal->RotationSliders, SLOT(reset())); 
  
}

//-----------------------------------------------------------------------------
qSlicerModuleTransform::~qSlicerModuleTransform()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerModuleTransform::dumpObjectInfo()
{
  this->Superclass::dumpObjectInfo();
}

//-----------------------------------------------------------------------------
void qSlicerModuleTransform::onCoordinateReferenceButtonPressed(int id)
{
  qMRMLTransformSliders::CoordinateReferenceType ref = 
    (id == qMRMLTransformSliders::GLOBAL) ? qMRMLTransformSliders::GLOBAL : qMRMLTransformSliders::LOCAL; 
  this->Internal->TranslationSliders->setCoordinateReference(ref);
  this->Internal->RotationSliders->setCoordinateReference(ref);
}

//-----------------------------------------------------------------------------
void qSlicerModuleTransform::onNodeSelected(vtkMRMLNode* node)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node); 
  
  // Enable/Disable CoordinateReference, identity buttons, MatrixViewGroupBox
  this->Internal->CoordinateReferenceGroupBox->setEnabled(transformNode != 0); 
  this->Internal->IdentityPushButton->setEnabled(transformNode != 0); 
  this->Internal->MatrixViewGroupBox->setEnabled(transformNode != 0); 
  
  this->Internal->MRMLTransformNode = transformNode; 
}

//-----------------------------------------------------------------------------
void qSlicerModuleTransform::onIdentityButtonPressed()
{
  if (!this->Internal->MRMLTransformNode) { return; }
  
  this->Internal->MRMLTransformNode->GetMatrixTransformToParent()->Identity(); 
  this->Internal->RotationSliders->reset(); 
}
