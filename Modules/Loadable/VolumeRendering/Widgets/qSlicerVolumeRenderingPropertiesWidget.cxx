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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// qSlicerVolumeRendering includes
#include "qSlicerVolumeRenderingPropertiesWidget.h"
#include "vtkMRMLVolumeRenderingDisplayNode.h"

// MRML includes
#include "vtkMRMLVolumeNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingPropertiesWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerVolumeRenderingPropertiesWidget);
protected:
  qSlicerVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qSlicerVolumeRenderingPropertiesWidgetPrivate(qSlicerVolumeRenderingPropertiesWidget& object);

  vtkMRMLVolumeRenderingDisplayNode* VolumeRenderingDisplayNode;
  vtkMRMLVolumeNode* VolumeNode;
};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingPropertiesWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingPropertiesWidgetPrivate
::qSlicerVolumeRenderingPropertiesWidgetPrivate(
  qSlicerVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
  this->VolumeRenderingDisplayNode = nullptr;
  this->VolumeNode = nullptr;
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingPropertiesWidget
::qSlicerVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerVolumeRenderingPropertiesWidgetPrivate(*this) )
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingPropertiesWidget::~qSlicerVolumeRenderingPropertiesWidget() = default;

//-----------------------------------------------------------------------------
vtkMRMLNode* qSlicerVolumeRenderingPropertiesWidget::mrmlNode()const
{
  return vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(
    this->mrmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
vtkMRMLVolumeRenderingDisplayNode* qSlicerVolumeRenderingPropertiesWidget
::mrmlVolumeRenderingDisplayNode()const
{
  Q_D(const qSlicerVolumeRenderingPropertiesWidget);
  return d->VolumeRenderingDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLVolumeNode* qSlicerVolumeRenderingPropertiesWidget
::mrmlVolumeNode()const
{
  Q_D(const qSlicerVolumeRenderingPropertiesWidget);
  return d->VolumeNode;
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingPropertiesWidget
::setMRMLNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeRenderingDisplayNode(
    vtkMRMLVolumeRenderingDisplayNode::SafeDownCast(node));
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingPropertiesWidget
::setMRMLVolumeRenderingDisplayNode(vtkMRMLVolumeRenderingDisplayNode* displayNode)
{
  Q_D(qSlicerVolumeRenderingPropertiesWidget);
  qvtkReconnect(d->VolumeRenderingDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));

  d->VolumeRenderingDisplayNode = displayNode;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingPropertiesWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerVolumeRenderingPropertiesWidget);
  vtkMRMLVolumeNode* newVolumeNode =
    d->VolumeRenderingDisplayNode ? d->VolumeRenderingDisplayNode->GetVolumeNode() : nullptr;
  qvtkReconnect(d->VolumeNode, newVolumeNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRMLVolumeNode()));
  d->VolumeNode = newVolumeNode;
  this->updateWidgetFromMRMLVolumeNode();
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingPropertiesWidget::updateWidgetFromMRMLVolumeNode()
{
}
