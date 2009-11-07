#include "qSlicerTransformsModule.h" 

#include "ui_qSlicerTransformsModule.h" 

#include "qMRMLUtils.h"

#include "vtkSlicerTransformLogic.h"
#include "vtkMRMLLinearTransformNode.h"

#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

#include <QButtonGroup>
#include <QFileDialog>
#include <QVector>

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerTransformsModule::qInternal : public Ui::qSlicerTransformsModule
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
qSlicerTransformsModule::qSlicerTransformsModule(QWidget *parent) : Superclass(parent)
{
}

//-----------------------------------------------------------------------------
qSlicerTransformsModule::~qSlicerTransformsModule()
{
  if (this->initialized()) { delete this->Internal; }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::initializer()
{
  qDebug() << "qSlicerTransformsModule::initializer";
  this->Superclass::initializer();
  
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
  
  // Initialize translation min/max limit
  double minRange = this->Internal->MinTranslationLimitInput->value(); 
  double maxRange = this->Internal->MaxTranslationLimitInput->value(); 
  this->Internal->TranslationSliders->setRange(minRange, maxRange);
  this->Internal->RotationSliders->setRange(minRange, maxRange);
  
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
                
  // Connect revert button 
  this->connect(this->Internal->InvertPushButton, 
                SIGNAL(pressed()),
                SLOT(onInvertButtonPressed()));
  
  /* Connect node selector with translation sliders
  this->connect(this->Internal->TransformNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
                this->Internal->TranslationSliders, SLOT(setMRMLTransformNode(vtkMRMLNode*)));
  */
  /* Connect node selector with rotation sliders
  this->connect(this->Internal->TransformNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
                this->Internal->RotationSliders, SLOT(setMRMLTransformNode(vtkMRMLNode*)));
  */
  // Connect node selector with module itself
  this->connect(this->Internal->TransformNodeSelector, 
                SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
                SLOT(onNodeSelected(vtkMRMLNode*)));
  
  /* Connect node selector with matrix widget
  this->connect(this->Internal->TransformNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)), 
                this->Internal->MatrixWidget, SLOT(setMRMLTransformNode(vtkMRMLNode*)));
  */
  /* Reset Rotation sliders if at least one of the translation sliders is moved
  this->connect(this->Internal->TranslationSliders, SIGNAL(sliderMoved()), 
                this->Internal->RotationSliders, SLOT(reset())); 
  */          
  /* Connect min/max translation limit input with translation sliders
  this->connect(this->Internal->MinTranslationLimitInput, SIGNAL(valueEdited(double)), 
    this->Internal->TranslationSliders, SLOT(setMinimumRange(double))); 
  this->connect(this->Internal->MaxTranslationLimitInput, SIGNAL(valueEdited(double)), 
    this->Internal->TranslationSliders, SLOT(setMaximumRange(double))); 
  */
  this->connect(this->Internal->LoadTransformPushButton, SIGNAL(clicked()),
                SLOT(loadTransform()));
  QIcon openIcon = 
    QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon);
  this->Internal->LoadTransformPushButton->setIcon(openIcon);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModule::helpText()
{
  QString help = 
    "The Volumes Module loads and adjusts display parameters of volume data.</br>"
    "<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:Volumes-Documentation-3.4</a></br>"
    "</br>"
    "The Diffusion Editor allows modifying parameters (gradients, bValues, measurement frame) "
    "of DWI data and provides a quick way to interpret them. For that it estimates a tensor and "
    "shows glyphs and tracts for visual exploration. Help for Diffusion Editor:"
    "<a>%1/Modules:Volumes:Diffusion_Editor-Documentation</a>";
    
  return help.arg(this->slicerWikiUrl()); 
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModule::aboutText()
{
  QString about = 
    "The Transforms Module creates and edits transforms.</br>"
    "<a>%1/Modules:Transforms-Documentation-3.4</a>"; 
    
  return about.arg(this->slicerWikiUrl()); 
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::onCoordinateReferenceButtonPressed(int id)
{
  qMRMLTransformSliders::CoordinateReferenceType ref = 
    (id == qMRMLTransformSliders::GLOBAL) ? qMRMLTransformSliders::GLOBAL : qMRMLTransformSliders::LOCAL; 
  this->Internal->TranslationSliders->setCoordinateReference(ref);
  this->Internal->RotationSliders->setCoordinateReference(ref);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::onNodeSelected(vtkMRMLNode* node)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node); 
  
  // Enable/Disable CoordinateReference, identity buttons, MatrixViewGroupBox, 
  // Min/Max translation inputs
  this->Internal->CoordinateReferenceGroupBox->setEnabled(transformNode != 0); 
  this->Internal->IdentityPushButton->setEnabled(transformNode != 0); 
  this->Internal->InvertPushButton->setEnabled(transformNode != 0);
  this->Internal->MatrixViewGroupBox->setEnabled(transformNode != 0); 
  this->Internal->MinTranslationLimitLabel->setEnabled(transformNode != 0); 
  this->Internal->MaxTranslationLimitLabel->setEnabled(transformNode != 0); 
  this->Internal->MinTranslationLimitInput->setEnabled(transformNode != 0); 
  this->Internal->MaxTranslationLimitInput->setEnabled(transformNode != 0); 
  
  // Listen for Transform node changes
  this->qvtkReConnect(this->Internal->MRMLTransformNode, transformNode, 
    vtkMRMLTransformableNode::TransformModifiedEvent, 
    this, SLOT(onMRMLTransformNodeModified(void*,vtkObject*)));
  
  this->Internal->MRMLTransformNode = transformNode; 
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::onIdentityButtonPressed()
{
  if (!this->Internal->MRMLTransformNode) { return; }
  
  this->Internal->MRMLTransformNode->GetMatrixTransformToParent()->Identity(); 
  this->Internal->RotationSliders->reset(); 
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::onInvertButtonPressed()
{
  if (!this->Internal->MRMLTransformNode) { return; }
  
  this->Internal->MRMLTransformNode->GetMatrixTransformToParent()->Invert();
  this->Internal->RotationSliders->reset();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::onMRMLTransformNodeModified(void* /*call_data*/, vtkObject* caller)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
  if (!transformNode) { return; }
  
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(this->Internal->MRMLTransformNode, 
    this->coordinateReference() == qMRMLTransformSliders::GLOBAL, transform);
  
  vtkMatrix4x4 * mat = transform->GetMatrix();
  double minmax[2] = {0, 0}; 
  this->extractMinMaxTranslationValue(mat, minmax, 0.3);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::extractMinMaxTranslationValue(
  vtkMatrix4x4 * mat, double minmax[2], float expand)
{
  Q_ASSERT(mat);
  if (!mat)
    {
    return; 
    }
    
  for (int i=0; i <3; i++)
    {
    if (mat->GetElement(i,3) < minmax[0])
      {
      minmax[0] = mat->GetElement(i,3) - expand * fabs(mat->GetElement(i,3));
      }
  
    if (mat->GetElement(i,3) > minmax[1])
      {
      minmax[1] = mat->GetElement(i,3) + expand * fabs(mat->GetElement(i,3));
      }
    }
}

//-----------------------------------------------------------------------------
int qSlicerTransformsModule::coordinateReference()
{
  return this->Internal->CoordinateReferenceButtonGroup->checkedId(); 
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::loadTransform()
{
  QString fileName = QFileDialog::getOpenFileName(this);
  this->loadTransform(fileName);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModule::loadTransform(const QString& fileName)
{
  if (fileName.isEmpty())
    {
    return;
    }
  vtkSlicerTransformLogic *logic = vtkSlicerTransformLogic::New();
  logic->AddTransform(fileName.toLatin1(), this->mrmlScene());
  logic->Delete();
}
