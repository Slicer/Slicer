
// Qt includes
#include <QDebug>
#include <QMenu>
#include <QProxyStyle>

// CTK includes
#include <ctkVTKSliceView.h>
#include <ctkLogger.h>

// qMRML includes
#include "qMRMLSliceInformationWidget.h"
#include "qMRMLSliceInformationWidget_p.h"

// MRML includes
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkImageData.h>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLSliceInformationWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate methods

//---------------------------------------------------------------------------
qMRMLSliceInformationWidgetPrivate::qMRMLSliceInformationWidgetPrivate()
{
  this->MRMLSliceNode = 0;
}

//---------------------------------------------------------------------------
qMRMLSliceInformationWidgetPrivate::~qMRMLSliceInformationWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  CTK_P(qMRMLSliceInformationWidget);

  this->Ui_qMRMLSliceInformationWidget::setupUi(widget);

  // LayoutName is readonly

  // Connect Orientation selector
  this->connect(this->SliceOrientationSelector, SIGNAL(currentIndexChanged(QString)),
                p, SLOT(setSliceOrientation(QString)));

  // Connect Slice visibility toggle
  this->connect(this->SliceVisibilityToggle, SIGNAL(clicked(bool)),
                p, SLOT(setSliceVisible(bool)));

  // Connect Widget visibility toggle
  this->connect(this->WidgetVisibilityToggle, SIGNAL(clicked(bool)),
                p, SLOT(setWidgetVisible(bool)));

  // Dimension and Field of View are readonly

  // Connect LightBox layout
  this->connect(this->LightboxLayoutRowsSpinBox, SIGNAL(valueChanged(int)),
                p, SLOT(setLightboxLayoutRows(int)));
  this->connect(this->LightboxLayoutColumnsSpinBox, SIGNAL(valueChanged(int)),
                p, SLOT(setLightboxLayoutColumns(int)));

  // Connect SliceSpacingMode
  this->SliceSpacingModeGroup = new QButtonGroup(widget);
  this->SliceSpacingModeGroup->addButton(this->AutomaticSliceSpacingRadioButton,
                                         vtkMRMLSliceNode::AutomaticSliceSpacingMode);
  this->SliceSpacingModeGroup->addButton(this->PrescribedSliceSpacingRadioButton,
                                         vtkMRMLSliceNode::PrescribedSliceSpacingMode);
  this->connect(this->SliceSpacingModeGroup, SIGNAL(buttonReleased(int)),
                p, SLOT(setSliceSpacingMode(int)));

  // Connect Prescribed spacing
  this->connect(this->PrescribedSpacingSpinBox, SIGNAL(valueChanged(double)),
                p, SLOT(setPrescribedSliceSpacing(double)));
}

// --------------------------------------------------------------------------
void qMRMLSliceInformationWidgetPrivate::updateWidgetFromMRMLSliceNode()
{
  Q_ASSERT(this->MRMLSliceNode);

  logger.trace("updateWidgetFromMRMLSliceNode");

  // Update layout name
  this->LayoutNameLineEdit->setText(QLatin1String(this->MRMLSliceNode->GetLayoutName()));

  // Update orientation selector state
  int index = this->SliceOrientationSelector->findText(
      QString::fromStdString(this->MRMLSliceNode->GetOrientationString()));
  Q_ASSERT(index>=0 && index <=4);
  this->SliceOrientationSelector->setCurrentIndex(index);

  // Update slice visibility toggle
  this->SliceVisibilityToggle->setChecked(this->MRMLSliceNode->GetSliceVisible());

  // Update widget visibility toggle
  this->WidgetVisibilityToggle->setChecked(this->MRMLSliceNode->GetWidgetVisible());

  // Update dimension
  unsigned int dimensions[3] = {0, 0, 0};
  this->MRMLSliceNode->GetDimensions(dimensions);
  this->DimensionXEdit->setText(QString::number(dimensions[0]));
  this->DimensionYEdit->setText(QString::number(dimensions[1]));
  this->DimensionZEdit->setText(QString::number(dimensions[2]));

  // Update field of view
  double fieldOfView[3] = {0.0, 0.0, 0.0};
  this->MRMLSliceNode->GetFieldOfView(fieldOfView);
  this->FieldOfViewXEdit->setText(QString::number(fieldOfView[0]));
  this->FieldOfViewYEdit->setText(QString::number(fieldOfView[1]));
  this->FieldOfViewZEdit->setText(QString::number(fieldOfView[2]));

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
{
  CTK_INIT_PRIVATE(qMRMLSliceInformationWidget);
  CTK_D(qMRMLSliceInformationWidget);
  d->setupUi(this);
}

//---------------------------------------------------------------------------
CTK_GET_CXX(qMRMLSliceInformationWidget, vtkMRMLSliceNode*, mrmlSliceNode, MRMLSliceNode);

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
  CTK_D(qMRMLSliceInformationWidget);

  if (newSliceNode == d->MRMLSliceNode)
    {
    return;
    }

  // Enable/disable widget
  this->setDisabled(newSliceNode == 0);

  d->qvtkReconnect(d->MRMLSliceNode, newSliceNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromMRMLSliceNode()));

  d->MRMLSliceNode = newSliceNode;

  if (d->MRMLSliceNode)
    {
    // Update widget state given the new node
    d->updateWidgetFromMRMLSliceNode();
    }
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setSliceOrientation(const QString& orientation)
{
  CTK_D(qMRMLSliceInformationWidget);

#ifndef QT_NO_DEBUG
  QStringList expectedOrientation;
  expectedOrientation << "Axial" << "Sagittal" << "Coronal" << "Reformat";
  Q_ASSERT(expectedOrientation.contains(orientation));
#endif

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetOrientationString(orientation.toLatin1());
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setSliceVisible(bool visible)
{
  CTK_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetSliceVisible(visible);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setWidgetVisible(bool visible)
{
  CTK_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetWidgetVisible(visible);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setLightboxLayoutRows(int rowCount)
{
  CTK_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetLayoutGridRows(rowCount);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setLightboxLayoutColumns(int columnCount)
{
  CTK_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  d->MRMLSliceNode->SetLayoutGridColumns(columnCount);
}

//---------------------------------------------------------------------------
void qMRMLSliceInformationWidget::setSliceSpacingMode(int spacingMode)
{
  CTK_D(qMRMLSliceInformationWidget);

  if (spacingMode != vtkMRMLSliceNode::AutomaticSliceSpacingMode &&
      spacingMode != vtkMRMLSliceNode::PrescribedSliceSpacingMode)
  {
    logger.warn(QString("setSliceSpacingMode - Invalid spacingMode: %1").arg(spacingMode));
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
  CTK_D(qMRMLSliceInformationWidget);

  if (!d->MRMLSliceNode)
    {
    return;
    }

  double spacingArray[3] = {0.0, 0.0, 0.0};
  d->MRMLSliceNode->GetPrescribedSliceSpacing(spacingArray);
  spacingArray[2] = spacing;
  d->MRMLSliceNode->SetPrescribedSliceSpacing(spacingArray);
}

