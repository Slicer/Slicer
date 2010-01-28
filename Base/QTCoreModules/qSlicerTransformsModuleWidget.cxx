/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#include "qSlicerTransformsModuleWidget.h"
#include "ui_qSlicerTransformsModule.h"

// SlicerQT includes
//#include "qSlicerApplication.h"
//#include "qSlicerIOManager.h"

// vtkSlicerLogic includes
#include "vtkSlicerTransformLogic.h"

// qMRMLWidgets includes
#include <qMRMLUtils.h>

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkMatrix4x4.h>

// QT includes
#include <QButtonGroup>
#include <QFileDialog>
#include <QVector>
#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerTransformsModuleWidgetPrivate: public qCTKPrivate<qSlicerTransformsModuleWidget>,
                                            public Ui_qSlicerTransformsModule
{
public:
  qSlicerTransformsModuleWidgetPrivate()
    {
    this->CoordinateReferenceButtonGroup = 0;
    this->MRMLTransformNode = 0;
    }
  vtkSlicerTransformLogic*      logic()const;
  QButtonGroup*                 CoordinateReferenceButtonGroup;
  vtkMRMLLinearTransformNode*   MRMLTransformNode;
};

//-----------------------------------------------------------------------------
vtkSlicerTransformLogic* qSlicerTransformsModuleWidgetPrivate::logic()const
{
  QCTK_P(const qSlicerTransformsModuleWidget);
  return vtkSlicerTransformLogic::SafeDownCast(p->logic());
}

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerTransformsModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::setup()
{
  QCTK_D(qSlicerTransformsModuleWidget);
  d->setupUi(this);

  // Add coordinate reference button to a button group
  d->CoordinateReferenceButtonGroup =
    new QButtonGroup(d->CoordinateReferenceGroupBox);
  d->CoordinateReferenceButtonGroup->addButton(
    d->GlobalRadioButton, qMRMLTransformSliders::GLOBAL);
  d->CoordinateReferenceButtonGroup->addButton(
    d->LocalRadioButton, qMRMLTransformSliders::LOCAL);

  // Connect button group
  this->connect(d->CoordinateReferenceButtonGroup,
                SIGNAL(buttonPressed(int)),
                SLOT(onCoordinateReferenceButtonPressed(int)));

  // Connect identity button
  this->connect(d->IdentityPushButton,
                SIGNAL(pressed()),
                SLOT(onIdentityButtonPressed()));

  // Connect revert button
  this->connect(d->InvertPushButton,
                SIGNAL(pressed()),
                SLOT(onInvertButtonPressed()));

  // Connect node selector with module itself
  this->connect(d->TransformNodeSelector,
                SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onNodeSelected(vtkMRMLNode*)));

  this->connect(d->LoadTransformPushButton, SIGNAL(clicked()),
                SLOT(loadTransform()));
  QIcon openIcon =
    QApplication::style()->standardIcon(QStyle::SP_DirOpenIcon);
  d->LoadTransformPushButton->setIcon(openIcon);
}

//-----------------------------------------------------------------------------
QAction* qSlicerTransformsModuleWidget::showModuleAction()
{
  return new QAction(QIcon(":/Icons/Transforms.png"), tr("Show Transforms module"), this);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onCoordinateReferenceButtonPressed(int id)
{
  QCTK_D(qSlicerTransformsModuleWidget);
  
  qMRMLTransformSliders::CoordinateReferenceType ref =
    (id == qMRMLTransformSliders::GLOBAL) ? qMRMLTransformSliders::GLOBAL : qMRMLTransformSliders::LOCAL;
  d->TranslationSliders->setCoordinateReference(ref);
  d->RotationSliders->setCoordinateReference(ref);
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onNodeSelected(vtkMRMLNode* node)
{
  QCTK_D(qSlicerTransformsModuleWidget);
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);

  // Enable/Disable CoordinateReference, identity buttons, MatrixViewGroupBox,
  // Min/Max translation inputs
  d->CoordinateReferenceGroupBox->setEnabled(transformNode != 0);
  d->IdentityPushButton->setEnabled(transformNode != 0);
  d->InvertPushButton->setEnabled(transformNode != 0);
  d->MatrixViewGroupBox->setEnabled(transformNode != 0);

  // Listen for Transform node changes
  this->qvtkReconnect(d->MRMLTransformNode, transformNode,
    vtkMRMLTransformableNode::TransformModifiedEvent,
    this, SLOT(onMRMLTransformNodeModified(vtkObject*)));

  d->MRMLTransformNode = transformNode;
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onIdentityButtonPressed()
{
  QCTK_D(qSlicerTransformsModuleWidget);
  
  if (!d->MRMLTransformNode) { return; }

  d->MRMLTransformNode->GetMatrixTransformToParent()->Identity();
  d->RotationSliders->reset();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onInvertButtonPressed()
{
  QCTK_D(qSlicerTransformsModuleWidget);
  
  if (!d->MRMLTransformNode) { return; }

  d->MRMLTransformNode->GetMatrixTransformToParent()->Invert();
  d->RotationSliders->reset();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::onMRMLTransformNodeModified(vtkObject* caller)
{
  QCTK_D(qSlicerTransformsModuleWidget);
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
  if (!transformNode) { return; }

  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  qMRMLUtils::getTransformInCoordinateSystem(d->MRMLTransformNode,
    this->coordinateReference() == qMRMLTransformSliders::GLOBAL, transform);

  // The matrix can be changed externally. The min/max values shall be updated 
  //accordingly to the new matrix if needed.
  vtkMatrix4x4 * mat = transform->GetMatrix();
  double min = 0.;
  double max = 0.;
  this->extractMinMaxTranslationValue(mat, min, max);
  if (min < d->TranslationSliders->minimum())
    {
    min = min - 0.3 * fabs(min);
    d->TranslationSliders->setMinimum(min);
    }
  if (max > d->TranslationSliders->maximum())
    {
    max = max + 0.3 * fabs(max);
    d->TranslationSliders->setMaximum(max);
    }
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::extractMinMaxTranslationValue(
  vtkMatrix4x4 * mat, double& min, double& max)
{
  if (!mat)
    {
    Q_ASSERT(mat);
    return;
    }
  for (int i=0; i <3; i++)
    {
    min = qMin(min, mat->GetElement(i,3));
    max = qMax(max, mat->GetElement(i,3));
    }
}

//-----------------------------------------------------------------------------
int qSlicerTransformsModuleWidget::coordinateReference()
{
  return qctk_d()->CoordinateReferenceButtonGroup->checkedId();
}

//-----------------------------------------------------------------------------
void qSlicerTransformsModuleWidget::loadTransform()
{
  Q_ASSERT(this->mrmlScene());
  QString fileName = QFileDialog::getOpenFileName(this);
  if (!fileName.isEmpty())
    {
    qctk_d()->logic()->AddTransform(fileName.toLatin1(), this->mrmlScene());
    }
}
