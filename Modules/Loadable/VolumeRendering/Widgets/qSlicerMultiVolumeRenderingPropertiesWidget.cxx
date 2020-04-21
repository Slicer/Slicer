/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

// qSlicerGPURayCastVolumeRendering includes
#include "qSlicerMultiVolumeRenderingPropertiesWidget.h"
#include "vtkMRMLMultiVolumeRenderingDisplayNode.h"
#include "ui_qSlicerMultiVolumeRenderingPropertiesWidget.h"

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerMultiVolumeRenderingPropertiesWidgetPrivate
  : public Ui_qSlicerMultiVolumeRenderingPropertiesWidget
{
  Q_DECLARE_PUBLIC(qSlicerMultiVolumeRenderingPropertiesWidget);
protected:
  qSlicerMultiVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qSlicerMultiVolumeRenderingPropertiesWidgetPrivate(
    qSlicerMultiVolumeRenderingPropertiesWidget& object);
  virtual ~qSlicerMultiVolumeRenderingPropertiesWidgetPrivate();

  virtual void setupUi(qSlicerMultiVolumeRenderingPropertiesWidget*);
  void populateRenderingTechniqueComboBox();
};

// --------------------------------------------------------------------------
qSlicerMultiVolumeRenderingPropertiesWidgetPrivate
::qSlicerMultiVolumeRenderingPropertiesWidgetPrivate(
  qSlicerMultiVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
qSlicerMultiVolumeRenderingPropertiesWidgetPrivate::
~qSlicerMultiVolumeRenderingPropertiesWidgetPrivate() = default;

// --------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingPropertiesWidgetPrivate
::setupUi(qSlicerMultiVolumeRenderingPropertiesWidget* widget)
{
  this->Ui_qSlicerMultiVolumeRenderingPropertiesWidget::setupUi(widget);
  this->populateRenderingTechniqueComboBox();
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechnique(int)));
  QObject::connect(this->SurfaceSmoothingCheckBox, SIGNAL(toggled(bool)),
                   widget, SLOT(setSurfaceSmoothing(bool)));
}

// --------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingPropertiesWidgetPrivate::populateRenderingTechniqueComboBox()
{
  this->RenderingTechniqueComboBox->clear();
  this->RenderingTechniqueComboBox->addItem(
    "Composite With Shading", vtkMRMLViewNode::Composite);
  this->RenderingTechniqueComboBox->addItem(
    "Maximum Intensity Projection", vtkMRMLViewNode::MaximumIntensityProjection);
  this->RenderingTechniqueComboBox->addItem(
    "Minimum Intensity Projection", vtkMRMLViewNode::MinimumIntensityProjection);
}

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingPropertiesWidget
::qSlicerMultiVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerMultiVolumeRenderingPropertiesWidgetPrivate(*this) )
{
  Q_D(qSlicerMultiVolumeRenderingPropertiesWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingPropertiesWidget::~qSlicerMultiVolumeRenderingPropertiesWidget() = default;

//-----------------------------------------------------------------------------
vtkMRMLMultiVolumeRenderingDisplayNode* qSlicerMultiVolumeRenderingPropertiesWidget
::mrmlDisplayNode()
{
  return vtkMRMLMultiVolumeRenderingDisplayNode::SafeDownCast(
    this->mrmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingPropertiesWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerMultiVolumeRenderingPropertiesWidget);

  vtkMRMLMultiVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  vtkMRMLViewNode* firstViewNode = displayNode->GetFirstViewNode();
  if (!firstViewNode)
    {
    return;
    }

  int technique = firstViewNode->GetRaycastTechnique();
  int index = d->RenderingTechniqueComboBox->findData(QVariant(technique));
  if (index == -1)
    {
    index = 0;
    }
  bool wasBlocked = d->RenderingTechniqueComboBox->blockSignals(true);
  d->RenderingTechniqueComboBox->setCurrentIndex(index);
  d->RenderingTechniqueComboBox->blockSignals(wasBlocked);

  wasBlocked = d->SurfaceSmoothingCheckBox->blockSignals(true);
  d->SurfaceSmoothingCheckBox->setChecked(firstViewNode->GetVolumeRenderingSurfaceSmoothing());
  d->SurfaceSmoothingCheckBox->blockSignals(wasBlocked);
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingPropertiesWidget::setRenderingTechnique(int index)
{
  Q_D(qSlicerMultiVolumeRenderingPropertiesWidget);
  vtkMRMLMultiVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }
  int technique = d->RenderingTechniqueComboBox->itemData(index).toInt();

  std::vector<vtkMRMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetRaycastTechnique(technique);
      }
    }
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingPropertiesWidget::setSurfaceSmoothing(bool on)
{
  Q_D(qSlicerMultiVolumeRenderingPropertiesWidget);
  vtkMRMLMultiVolumeRenderingDisplayNode* displayNode = this->mrmlDisplayNode();
  if (!displayNode)
    {
    return;
    }

  std::vector<vtkMRMLNode*> viewNodes;
  displayNode->GetScene()->GetNodesByClass("vtkMRMLViewNode", viewNodes);
  for (std::vector<vtkMRMLNode*>::iterator it=viewNodes.begin(); it!=viewNodes.end(); ++it)
    {
    vtkMRMLViewNode* viewNode = vtkMRMLViewNode::SafeDownCast(*it);
    if (displayNode->IsDisplayableInView(viewNode->GetID()))
      {
      viewNode->SetVolumeRenderingSurfaceSmoothing(on);
      }
    }
}
