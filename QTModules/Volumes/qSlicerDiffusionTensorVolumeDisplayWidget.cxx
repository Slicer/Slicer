/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qSlicerDiffusionTensorVolumeDisplayWidget.h"
#include "ui_qSlicerDiffusionTensorVolumeDisplayWidget.h"

// Qt includes
#include <QDebug>

// MRML includes
#include "vtkMRMLDiffusionTensorVolumeNode.h"

// VTK includes
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

// STD includes
#include <limits>

//-----------------------------------------------------------------------------
class qSlicerDiffusionTensorVolumeDisplayWidgetPrivate
  : public Ui_qSlicerDiffusionTensorVolumeDisplayWidget
{
  Q_DECLARE_PUBLIC(qSlicerDiffusionTensorVolumeDisplayWidget);
protected:
  qSlicerDiffusionTensorVolumeDisplayWidget* const q_ptr;
public:
  qSlicerDiffusionTensorVolumeDisplayWidgetPrivate(qSlicerDiffusionTensorVolumeDisplayWidget& object);
  ~qSlicerDiffusionTensorVolumeDisplayWidgetPrivate();
  void init();
  vtkMRMLDiffusionTensorVolumeNode* VolumeNode;
};

//-----------------------------------------------------------------------------
qSlicerDiffusionTensorVolumeDisplayWidgetPrivate
::qSlicerDiffusionTensorVolumeDisplayWidgetPrivate(
  qSlicerDiffusionTensorVolumeDisplayWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerDiffusionTensorVolumeDisplayWidgetPrivate
::~qSlicerDiffusionTensorVolumeDisplayWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qSlicerDiffusionTensorVolumeDisplayWidget);

  this->setupUi(q);
}

// --------------------------------------------------------------------------
qSlicerDiffusionTensorVolumeDisplayWidget
::qSlicerDiffusionTensorVolumeDisplayWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerDiffusionTensorVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qSlicerDiffusionTensorVolumeDisplayWidget);
  d->init();

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qSlicerDiffusionTensorVolumeDisplayWidget
::~qSlicerDiffusionTensorVolumeDisplayWidget()
{
}

// --------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeNode* qSlicerDiffusionTensorVolumeDisplayWidget
::volumeNode()const
{
  Q_D(const qSlicerDiffusionTensorVolumeDisplayWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode* qSlicerDiffusionTensorVolumeDisplayWidget::volumeDisplayNode()const
{
  vtkMRMLDiffusionTensorVolumeNode* volumeNode = this->volumeNode();
  return volumeNode ? vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(
    volumeNode->GetDisplayNode()) : 0;
}

// --------------------------------------------------------------------------
vtkMRMLDiffusionTensorVolumeDisplayNode* qSlicerDiffusionTensorVolumeDisplayWidget::sliceDisplayNode()const
{
  vtkMRMLDiffusionTensorVolumeNode* volumeNode = this->volumeNode();
  return volumeNode ? vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(
    volumeNode->GetDisplayNode()) : 0;
}


// --------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLDiffusionTensorVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLDiffusionTensorVolumeNode* volumeNode)
{
  Q_D(qSlicerDiffusionTensorVolumeDisplayWidget);

  vtkMRMLDiffusionTensorVolumeDisplayNode* oldVolumeDisplayNode = this->volumeDisplayNode();

  d->MRMLWindowLevelWidget->setMRMLVolumeNode(volumeNode);
  d->MRMLVolumeThresholdWidget->setMRMLVolumeNode(volumeNode);

  qvtkReconnect(oldVolumeDisplayNode, volumeNode ? volumeNode->GetDisplayNode() :0,
                vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->Histogram->setDataArray(volumeNode &&
                             volumeNode->GetImageData() &&
                             volumeNode->GetImageData()->GetPointData() ?
                             volumeNode->GetImageData()->GetPointData()->GetScalars() :
                             0);
  d->Histogram->build();
  this->setEnabled(volumeNode != 0);

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerDiffusionTensorVolumeDisplayWidget);
  vtkMRMLDiffusionTensorVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (displayNode)
    {
    d->ColorTableComboBox->setCurrentNode(displayNode->GetColorNode());
    d->InterpolateCheckbox->setChecked(displayNode->GetInterpolate());
    }
  if (this->isVisible())
    {
    this->updateTransferFunction();
    }
}

//----------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::updateTransferFunction()
{
  Q_D(qSlicerDiffusionTensorVolumeDisplayWidget);
  // from vtkKWWindowLevelThresholdEditor::UpdateTransferFunction
  vtkMRMLVolumeNode* volumeNode = d->MRMLWindowLevelWidget->mrmlVolumeNode();
  Q_ASSERT(volumeNode == d->MRMLVolumeThresholdWidget->mrmlVolumeNode());
  vtkImageData* imageData = volumeNode ? volumeNode->GetImageData() : 0;
  if (imageData == 0)
    {
    d->ColorTransferFunction->RemoveAllPoints();
    return;
    }
  double range[2] = {0,255};
  imageData->GetScalarRange(range);
  // AdjustRange call will take out points that are outside of the new
  // range, but it needs the points to be there in order to work, so call
  // RemoveAllPoints after it's done
  d->ColorTransferFunction->AdjustRange(range);
  d->ColorTransferFunction->RemoveAllPoints();

  double min = d->MRMLWindowLevelWidget->level() - 0.5 * d->MRMLWindowLevelWidget->window();
  double max = d->MRMLWindowLevelWidget->level() + 0.5 * d->MRMLWindowLevelWidget->window();
  double minVal = 0;
  double maxVal = 1;
  double low   = d->MRMLVolumeThresholdWidget->isOff() ? range[0] : d->MRMLVolumeThresholdWidget->lowerThreshold();
  double upper = d->MRMLVolumeThresholdWidget->isOff() ? range[1] : d->MRMLVolumeThresholdWidget->upperThreshold();

  d->ColorTransferFunction->SetColorSpaceToRGB();

  if (low >= max || upper <= min)
    {
    d->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
    }
  else
    {
    max = qMax(min+0.001, max);
    low = qMax(range[0] + 0.001, low);
    min = qMax(range[0] + 0.001, min);
    upper = qMin(range[1] - 0.001, upper);

    if (min <= low)
      {
      minVal = (low - min)/(max - min);
      min = low + 0.001;
      }

    if (max >= upper)
      {
      maxVal = (upper - min)/(max-min);
      max = upper - 0.001;
      }

    d->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(low, 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(min, minVal, minVal, minVal);
    d->ColorTransferFunction->AddRGBPoint(max, maxVal, maxVal, maxVal);
    d->ColorTransferFunction->AddRGBPoint(upper, maxVal, maxVal, maxVal);
    if (upper+0.001 < range[1])
      {
      d->ColorTransferFunction->AddRGBPoint(upper+0.001, 0, 0, 0);
      d->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
      }
    }

  d->ColorTransferFunction->SetAlpha(1.0);
  d->ColorTransferFunction->Build();
}

// -----------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::showEvent( QShowEvent * event )
{
  this->updateTransferFunction();
  this->Superclass::showEvent(event);
}

// --------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::setInterpolate(bool interpolate)
{
  vtkMRMLDiffusionTensorVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode)
    {
    return;
    }
  displayNode->SetInterpolate(interpolate);
}

// --------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::setColorNode(vtkMRMLNode* colorNode)
{
  vtkMRMLDiffusionTensorVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode || !colorNode)
    {
    return;
    }
  Q_ASSERT(vtkMRMLColorNode::SafeDownCast(colorNode));
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
}

// --------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::onPresetButtonClicked()
{
  QToolButton* preset = qobject_cast<QToolButton*>(this->sender());
  this->setPreset(preset->accessibleName());
}

// --------------------------------------------------------------------------
void qSlicerDiffusionTensorVolumeDisplayWidget::setPreset(const QString& presetName)
{
  Q_D(qSlicerDiffusionTensorVolumeDisplayWidget);
  QString colorNodeID;
  double window = -1.;
  double level = std::numeric_limits<double>::max();
  if (presetName == "CT-Bone")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 1000.;
    level = 400.;
    }
  else if (presetName == "CT-Air")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 1000.;
    level = -426.;
    }
  else if (presetName == "PET")
    {
    colorNodeID = "vtkMRMLColorTableNodeRainbow";
    window = 10000.;
    level = 6000.;
    }
  else if (presetName == "CT-Abdomen")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 350.;
    level = 40.;
    }
  else if (presetName == "CT-Brain")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 100.;
    level = 50.;
    }
  else if (presetName == "CT-Lung")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 1400.;
    level = -500.;
    }

  vtkMRMLNode* colorNode = this->mrmlScene()->GetNodeByID(colorNodeID.toLatin1());
  if (colorNode)
    {
    this->setColorNode(colorNode);
    }
  if (window != -1 || level!= std::numeric_limits<double>::max())
    {
    d->MRMLWindowLevelWidget->setAutoWindowLevel(qMRMLWindowLevelWidget::Manual);
    }
  if (window != -1 && level != std::numeric_limits<double>::max())
    {
    d->MRMLWindowLevelWidget->setWindowLevel(window, level);
    }
  else if (window != -1)
    {
    d->MRMLWindowLevelWidget->setWindow(window);
    }
  else if (level != std::numeric_limits<double>::max())
    {
    d->MRMLWindowLevelWidget->setLevel(level);
    }
}
