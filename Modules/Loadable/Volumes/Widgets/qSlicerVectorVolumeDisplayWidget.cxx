/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// Slicer includes
#include "qSlicerVectorVolumeDisplayWidget.h"
#include "ui_qSlicerVectorVolumeDisplayWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLVectorVolumeNode.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qSlicerVectorVolumeDisplayWidgetPrivate : public Ui_qSlicerVectorVolumeDisplayWidget
{
  Q_DECLARE_PUBLIC(qSlicerVectorVolumeDisplayWidget);

protected:
  qSlicerVectorVolumeDisplayWidget* const q_ptr;

public:
  qSlicerVectorVolumeDisplayWidgetPrivate(qSlicerVectorVolumeDisplayWidget& object);
  void init();

  /// Vector field display node of the volume, or nullptr if it does not have one.
  /// If createIfMissing is true then one is created and added to the volume.
  vtkMRMLVectorFieldDisplayNode* vectorFieldDisplayNode(bool createIfMissing = false) const;

  vtkWeakPointer<vtkMRMLVectorVolumeNode> VolumeNode;
};

//-----------------------------------------------------------------------------
qSlicerVectorVolumeDisplayWidgetPrivate::qSlicerVectorVolumeDisplayWidgetPrivate(qSlicerVectorVolumeDisplayWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerVectorVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qSlicerVectorVolumeDisplayWidget);
  this->setupUi(q);
  // The display node is created the first time the section is opened, so that loading a
  // vector volume does not create display nodes that nobody asked for
  QObject::connect(this->VectorFieldGroupBox, SIGNAL(toggled(bool)), q, SLOT(updateWidgetFromMRML()));
}

//-----------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* qSlicerVectorVolumeDisplayWidgetPrivate::vectorFieldDisplayNode(bool createIfMissing /*=false*/) const
{
  if (!this->VolumeNode)
  {
    return nullptr;
  }
  int numberOfDisplayNodes = this->VolumeNode->GetNumberOfDisplayNodes();
  for (int i = 0; i < numberOfDisplayNodes; ++i)
  {
    vtkMRMLVectorFieldDisplayNode* displayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(this->VolumeNode->GetNthDisplayNode(i));
    if (displayNode)
    {
      return displayNode;
    }
  }
  if (!createIfMissing || !this->VolumeNode->GetScene())
  {
    return nullptr;
  }
  vtkMRMLVectorFieldDisplayNode* displayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(this->VolumeNode->GetScene()->AddNewNodeByClass("vtkMRMLVectorFieldDisplayNode"));
  if (!displayNode)
  {
    return nullptr;
  }
  // The display node is created just because the section is opened, which should not change
  // what is shown in the views: the user can show the vector field by turning on visibility.
  displayNode->SetVisibility(false);
  displayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeFilePlasma.txt");
  this->VolumeNode->AddAndObserveDisplayNodeID(displayNode->GetID());
  return displayNode;
}

// --------------------------------------------------------------------------
qSlicerVectorVolumeDisplayWidget::qSlicerVectorVolumeDisplayWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerVectorVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qSlicerVectorVolumeDisplayWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerVectorVolumeDisplayWidget::~qSlicerVectorVolumeDisplayWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLVectorVolumeNode* qSlicerVectorVolumeDisplayWidget::volumeNode() const
{
  Q_D(const qSlicerVectorVolumeDisplayWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
qMRMLVectorFieldDisplayWidget* qSlicerVectorVolumeDisplayWidget::vectorFieldDisplayWidget() const
{
  Q_D(const qSlicerVectorVolumeDisplayWidget);
  return d->VectorFieldDisplayWidget;
}

// --------------------------------------------------------------------------
vtkMRMLVectorFieldDisplayNode* qSlicerVectorVolumeDisplayWidget::vectorFieldDisplayNode() const
{
  Q_D(const qSlicerVectorVolumeDisplayWidget);
  return d->vectorFieldDisplayNode();
}

// --------------------------------------------------------------------------
void qSlicerVectorVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLVectorVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerVectorVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLVectorVolumeNode* volumeNode)
{
  Q_D(qSlicerVectorVolumeDisplayWidget);
  if (d->VolumeNode == volumeNode)
  {
    return;
  }
  // The image display properties (window/level, threshold) are edited by the same widget
  // that scalar volumes use.
  d->ScalarVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);

  qvtkReconnect(d->VolumeNode, volumeNode, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->VolumeNode = volumeNode;

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerVectorVolumeDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVectorVolumeDisplayWidget);

  // Only volumes whose voxels have at least 3 components store a vector field
  vtkImageData* imageData = d->VolumeNode ? d->VolumeNode->GetImageData() : nullptr;
  bool hasVectorField = (imageData && imageData->GetNumberOfScalarComponents() >= 3);
  d->VectorFieldGroupBox->setEnabled(hasVectorField);

  vtkMRMLVectorFieldDisplayNode* displayNode = d->vectorFieldDisplayNode();
  if (!displayNode && !d->VectorFieldGroupBox->collapsed())
  {
    // The section is open, so the user is editing the vector field display
    displayNode = d->vectorFieldDisplayNode(hasVectorField);
  }
  d->VectorFieldDisplayWidget->setMRMLVectorFieldDisplayNode(displayNode);
}
