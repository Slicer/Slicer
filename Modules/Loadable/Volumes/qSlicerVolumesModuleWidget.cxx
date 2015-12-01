/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes

// CTK includes
//#include <ctkModelTester.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLLabelMapVolumeNode.h>

// Volumes includes
#include "qSlicerVolumesModuleWidget.h"
#include "ui_qSlicerVolumesModuleWidget.h"

#include "vtkSlicerVolumesLogic.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerVolumesModuleWidgetPrivate: public Ui_qSlicerVolumesModuleWidget
{
public:
};

//-----------------------------------------------------------------------------
qSlicerVolumesModuleWidget::qSlicerVolumesModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesModuleWidgetPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesModuleWidget::~qSlicerVolumesModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::setup()
{
  Q_D(qSlicerVolumesModuleWidget);
  d->setupUi(this);

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   d->MRMLVolumeInfoWidget, SLOT(setVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   d->VolumeDisplayWidget, SLOT(setMRMLVolumeNode(vtkMRMLNode*)));

  QObject::connect(d->ActiveVolumeNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(nodeSelectionChanged(vtkMRMLNode*)));

  // Set up labelmap conversion
  d->ConvertToLabelMapFrame->setVisible(false);
  QObject::connect(d->ConvertToLabelMapButton, SIGNAL(clicked()),
                   this, SLOT(convertToLabelmap()));
  QObject::connect(d->TargetLabelMapSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   this, SLOT(nodeSelectionChanged(vtkMRMLNode*)));

  //ctkModelTester* tester = new ctkModelTester(this);
  //tester->setModel(d->ActiveVolumeNodeSelector->model());
}

//------------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::nodeSelectionChanged(vtkMRMLNode* node)
{
  Q_UNUSED(node);
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVolumesModuleWidget);

  vtkMRMLVolumeNode* currentVolumeNode = vtkMRMLVolumeNode::SafeDownCast(
    d->ActiveVolumeNodeSelector->currentNode() );
  if (!currentVolumeNode)
    {
    d->ConvertToLabelMapFrame->setVisible(false);
    return;
    }

  // Show convert to labelmap frame only if the exact type is scalar volume
  // (not labelmap, vector, tensor, DTI, etc.)
  bool labelMapCoversionPossible =
    !strcmp(currentVolumeNode->GetClassName(), "vtkMRMLScalarVolumeNode");
  d->ConvertToLabelMapFrame->setVisible(labelMapCoversionPossible);

  // Set base name of target labelmap node
  d->TargetLabelMapSelector->setBaseName(QString("%1_Label").arg(currentVolumeNode->GetName()));

  d->ConvertToLabelMapButton->setEnabled(
    d->TargetLabelMapSelector->currentNode() != NULL );
}

//------------------------------------------------------------------------------
void qSlicerVolumesModuleWidget::convertToLabelmap()
{
  Q_D(qSlicerVolumesModuleWidget);

  vtkMRMLScalarVolumeNode* currentScalarVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(
    d->ActiveVolumeNodeSelector->currentNode() );
  vtkMRMLLabelMapVolumeNode* targetLabelMapNode = vtkMRMLLabelMapVolumeNode::SafeDownCast(
    d->TargetLabelMapSelector->currentNode() );
  if (!currentScalarVolumeNode || !targetLabelMapNode)
    {
    return;
    }

  vtkSlicerVolumesLogic* logic = vtkSlicerVolumesLogic::SafeDownCast(this->logic());
  logic->CreateLabelVolumeFromVolume(this->mrmlScene(), targetLabelMapNode, currentScalarVolumeNode);
}
