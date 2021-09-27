/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

  ==============================================================================*/

#include "qMRMLMarkupsAngleMeasurementsWidget.h"
#include "ui_qMRMLMarkupsAngleMeasurementsWidget.h"

// MRML Markups includes
#include <vtkMRMLMarkupsNode.h>
#include <vtkMRMLMarkupsAngleNode.h>

// VTK includes
#include <vtkWeakPointer.h>

// --------------------------------------------------------------------------
class qMRMLMarkupsAngleMeasurementsWidget;

// --------------------------------------------------------------------------
class qMRMLMarkupsAngleMeasurementsWidgetPrivate:
  public Ui_qMRMLMarkupsAngleMeasurementsWidget
{

public:
  qMRMLMarkupsAngleMeasurementsWidgetPrivate(qMRMLMarkupsAngleMeasurementsWidget &widget);
  void setupUi(qMRMLMarkupsAngleMeasurementsWidget* widget);

protected:
  qMRMLMarkupsAngleMeasurementsWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qMRMLMarkupsAngleMeasurementsWidget);

};

// --------------------------------------------------------------------------
qMRMLMarkupsAngleMeasurementsWidgetPrivate::qMRMLMarkupsAngleMeasurementsWidgetPrivate(qMRMLMarkupsAngleMeasurementsWidget& widget)
  : q_ptr(&widget)
{
}

// --------------------------------------------------------------------------
void qMRMLMarkupsAngleMeasurementsWidgetPrivate::setupUi(qMRMLMarkupsAngleMeasurementsWidget* widget)
{
  Q_Q(qMRMLMarkupsAngleMeasurementsWidget);

  this->Ui_qMRMLMarkupsAngleMeasurementsWidget::setupUi(widget);

  QObject::connect(this->angleMeasurementModeComboBox, SIGNAL(currentIndexChanged(int)),
                   q_ptr, SLOT(onAngleMeasurementModeChanged()));
  QObject::connect(this->rotationAxisCoordinatesWidget, SIGNAL(coordinatesChanged(double*)),
                   q_ptr, SLOT(onRotationAxisChanged()));

  q_ptr->setEnabled(q_ptr->MarkupsNode != nullptr);
}

// --------------------------------------------------------------------------
// qMRMLMarkupsAngleMeasurementsWidget methods

// --------------------------------------------------------------------------
qMRMLMarkupsAngleMeasurementsWidget::
qMRMLMarkupsAngleMeasurementsWidget(QWidget *parent)
  : Superclass(parent), d_ptr(new qMRMLMarkupsAngleMeasurementsWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qMRMLMarkupsAngleMeasurementsWidget::~qMRMLMarkupsAngleMeasurementsWidget()
{
}

// --------------------------------------------------------------------------
void qMRMLMarkupsAngleMeasurementsWidget::setup()
{
  d_ptr->setupUi(this);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsAngleMeasurementsWidget::updateWidgetFromMRML()
{
  vtkMRMLMarkupsAngleNode* angleNode = vtkMRMLMarkupsAngleNode::SafeDownCast(this->MarkupsNode);
  if (!angleNode)
    {
    return;
    }

    double axisVector[3] = {0.0, 0.0, 0.0};
    angleNode->GetOrientationRotationAxis(axisVector);
    bool wasBlocked = d_ptr->rotationAxisCoordinatesWidget->blockSignals(true);
    d_ptr->rotationAxisCoordinatesWidget->setCoordinates(axisVector);
    d_ptr->rotationAxisCoordinatesWidget->setEnabled(angleNode->GetAngleMeasurementMode() != vtkMRMLMarkupsAngleNode::Minimal);
    d_ptr->rotationAxisCoordinatesWidget->blockSignals(wasBlocked);
    d_ptr->angleMeasurementModeComboBox->setCurrentIndex(angleNode->GetAngleMeasurementMode());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsAngleMeasurementsWidget::onAngleMeasurementModeChanged()
{
  Q_D(qMRMLMarkupsAngleMeasurementsWidget);
  vtkMRMLMarkupsAngleNode* markupsAngleNode = vtkMRMLMarkupsAngleNode::SafeDownCast(this->MarkupsNode);
  if (!markupsAngleNode)
    {
    return;
    }

  markupsAngleNode->SetAngleMeasurementMode(d_ptr->angleMeasurementModeComboBox->currentIndex());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsAngleMeasurementsWidget::onRotationAxisChanged()
{
  vtkMRMLMarkupsAngleNode* markupsAngleNode = vtkMRMLMarkupsAngleNode::SafeDownCast(this->MarkupsNode);
  if (!markupsAngleNode)
    {
    return;
    }
  markupsAngleNode->SetOrientationRotationAxis(const_cast<double*>(d_ptr->rotationAxisCoordinatesWidget->coordinates()));
}

//-----------------------------------------------------------------------------
bool qMRMLMarkupsAngleMeasurementsWidget::canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const
{
  vtkMRMLMarkupsAngleNode* angleNode = vtkMRMLMarkupsAngleNode::SafeDownCast(markupsNode);
  if (!angleNode)
    {
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsAngleMeasurementsWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  this->MarkupsNode = vtkMRMLMarkupsAngleNode::SafeDownCast(markupsNode);

  if(!this->MarkupsNode)
    {
    return;
    }

  this->qvtkReconnect(this->MarkupsNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));

  this->updateWidgetFromMRML();
  this->setEnabled(markupsNode != nullptr);
}
