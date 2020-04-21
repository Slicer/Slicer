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
#include <QButtonGroup>
#include <QDebug>

// qMRML includes
#include "qMRMLSliceControllerWidget_p.h" // For updateSliceOrientationSelector
#include "qMRMLSliceInformationWidget_p.h"

// MRML includes
#include <vtkMRMLSliceNode.h>

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceInformationWidgetPrivate::qMRMLSliceInformationWidgetPrivate(qMRMLSliceInformationWidget& object)
  : q_ptr(&object)
  , SliceSpacingModeGroup(nullptr)
{
}

//---------------------------------------------------------------------------
qMRMLSliceInformationWidgetPrivate::~qMRMLSliceInformationWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  Q_Q(qMRMLSliceInformationWidget);

  this->Ui_qMRMLSliceInformationWidget::setupUi(widget);

  // LayoutName is readonly

  // Connect Orientation selector
  this->connect(this->SliceOrientationSelector, SIGNAL(currentIndexChanged(QString)),
                q, SLOT(setSliceOrientation(QString)));

  // Connect Slice visibility toggle
  this->connect(this->SliceVisibilityToggle, SIGNAL(clicked(bool)),
                q, SLOT(setSliceVisible(bool)));

  // Connect Widget visibility toggle
  this->connect(this->WidgetVisibilityToggle, SIGNAL(clicked(bool)),
                q, SLOT(setWidgetVisible(bool)));

  // Dimension and Field of View are readonly

  this->connect(this->ViewGroupSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setViewGroup(int)));

  // Connect LightBox layout
  this->connect(this->LightboxLayoutRowsSpinBox, SIGNAL(valueChanged(int)),
                q, SLOT(setLightboxLayoutRows(int)));
  this->connect(this->LightboxLayoutColumnsSpinBox, SIGNAL(valueChanged(int)),
                q, SLOT(setLightboxLayoutColumns(int)));

  // Connect SliceSpacingMode
  this->SliceSpacingModeGroup = new QButtonGroup(widget);
  this->SliceSpacingModeGroup->addButton(this->AutomaticSliceSpacingRadioButton,
                                         vtkMRMLSliceNode::AutomaticSliceSpacingMode);
  this->SliceSpacingModeGroup->addButton(this->PrescribedSliceSpacingRadioButton,
                                         vtkMRMLSliceNode::PrescribedSliceSpacingMode);
  this->connect(this->SliceSpacingModeGroup, SIGNAL(buttonReleased(int)),
                q, SLOT(setSliceSpacingMode(int)));

  // Connect Prescribed spacing
  this->connect(this->PrescribedSpacingSpinBox, SIGNAL(valueChanged(double)),
                q, SLOT(setPrescribedSliceSpacing(double)));
}

// --------------------------------------------------------------------------
void qMRMLSliceInformationWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  Q_Q(qMRMLSliceInformationWidget);

  q->setEnabled(this->MRMLSliceNode != nullptr);
  if (this->MRMLSliceNode == nullptr)
    {
    return;
    }

  //qDebug() << "qMRMLSliceInformationWidgetPrivate::updateWidgetFromMRMLSliceNode";

  // Update layout name
  this->LayoutNameLineEdit->setText(QString::fromUtf8(this->MRMLSliceNode->GetLayoutName()));

  qMRMLSliceControllerWidgetPrivate::updateSliceOrientationSelector(
        this->MRMLSliceNode, this->SliceOrientationSelector);

  // Update slice visibility toggle
  this->SliceVisibilityToggle->setChecked(this->MRMLSliceNode->GetSliceVisible());

  // Update widget visibility toggle
  this->WidgetVisibilityToggle->setChecked(this->MRMLSliceNode->GetWidgetVisible());

  // Update dimension
  int dimensions[3] = {0, 0, 0};
  this->MRMLSliceNode->GetDimensions(dimensions);
  double coordinatesInDouble[3];
  coordinatesInDouble[0] = dimensions[0];
  coordinatesInDouble[1] = dimensions[1];
  coordinatesInDouble[2] = dimensions[2];
  this->DimensionWidget->setCoordinates(coordinatesInDouble);

  // Update field of view
  double fieldOfView[3] = {0.0, 0.0, 0.0};
  this->MRMLSliceNode->GetFieldOfView(fieldOfView);
  coordinatesInDouble[0] = fieldOfView[0];
  coordinatesInDouble[1] = fieldOfView[1];
  coordinatesInDouble[2] = fieldOfView[2];
  this->FieldOfViewWidget->setCoordinates(coordinatesInDouble);

  this->ViewGroupSpinBox->setValue(this->MRMLSliceNode->GetViewGroup());

  // Update lightbox rows/columns entries
  this->LightboxLayoutRowsSpinBox->setValue(this->MRMLSliceNode->GetLayoutGridRows());
  this->LightboxLayoutColumnsSpinBox->setValue(this->MRMLSliceNode->GetLayoutGridColumns());

  // Update spacing mode
  if (this->MRMLSliceNode->GetSliceSpacingMode() == vtkMRMLSliceNode::AutomaticSliceSpacingMode)
    {
    this->AutomaticSliceSpacingRadioButton->setChecked(true);
    }
  else if (this->MRMLSliceNode->GetSliceSpacingMode() ==
           vtkMRMLSliceNode::PrescribedSliceSpacingMode)
    {
    this->PrescribedSliceSpacingRadioButton->setChecked(true);
    double prescribedSpacing[3] = {0.0, 0.0, 0.0};
    this->MRMLSliceNode->GetPrescribedSliceSpacing(prescribedSpacing);
    this->PrescribedSpacingSpinBox->setValue(prescribedSpacing[2]);
    }
}

// --------------------------------------------------------------------------
// qMRMLSliceView methods

// --------------------------------------------------------------------------
qMRMLSliceInformationWidget::qMRMLSliceInformationWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLSliceInformationWidgetPrivate(*this))
{
  Q_D(qMRMLSliceInformationWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qMRMLSliceInformationWidget::~qMRMLSliceInformationWidget() = default;

//---------------------------------------------------------------------------
vtkMRMLSliceNode* qMRMLSliceInformationWidget::mrmlSliceNode()const
{
  Q_D(const qMRMLSliceInformationWidget);
  return d->MRMLSliceNode;
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setMRMLSliceNode(vtkMRMLNode* newNode)
{
  vtkMRMLSliceNode * newSliceNode = vtkMRMLSliceNode::SafeDownCast(newNode);
  if (!newSliceNode)
    {
    return;
    }
  this->setMRMLSliceNode(newSliceNode);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode)
{
  Q_D(qMRMLSliceInformationWidget);

  if (newSliceNode == d->MRMLSliceNode)
    {
    return;
    }

  d->qvtkReconnect(d->MRMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromMRMLSliceNode()));

  d->MRMLSliceNode = newSliceNode;

  // Update widget state given the new node
  d->updateWidgetFromMRMLSliceNode();
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setSliceOrientation(const QString& orientation)
{
  Q_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetOrientation(orientation.toUtf8());
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setSliceVisible(bool visible)
{
  Q_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetSliceVisible(visible);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setViewGroup(int viewGroup)
{
  Q_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetViewGroup(viewGroup);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setWidgetVisible(bool visible)
{
  Q_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetWidgetVisible(visible);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setLightboxLayoutRows(int rowCount)
{
  Q_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetLayoutGridRows(rowCount);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setLightboxLayoutColumns(int columnCount)
{
  Q_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetLayoutGridColumns(columnCount);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setSliceSpacingMode(int spacingMode)
{
  Q_D(qMRMLSliceInformationWidget);

  if (spacingMode != vtkMRMLSliceNode::AutomaticSliceSpacingMode &&
      spacingMode != vtkMRMLSliceNode::PrescribedSliceSpacingMode)
    {
    qWarning() << "setSliceSpacingMode - Invalid spacingMode:" << spacingMode;
    return;
    }

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetSliceSpacingMode(spacingMode);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setPrescribedSliceSpacing(double spacing)
{
  Q_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  double spacingArray[3] = {0.0, 0.0, 0.0};
  d->MRMLSliceNode->GetPrescribedSliceSpacing(spacingArray);
  spacingArray[2] = spacing;
  d->MRMLSliceNode->SetPrescribedSliceSpacing(spacingArray);
}

