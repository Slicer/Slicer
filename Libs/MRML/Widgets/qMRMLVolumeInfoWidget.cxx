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

// QT includes
#include <QDebug>

// CTK includes
#include <ctkUtils.h>

// qMRML includes
#include "qMRMLVolumeInfoWidget.h"
#include "ui_qMRMLVolumeInfoWidget.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLStorageNode.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTrivialProducer.h>
#include <vtkWeakPointer.h>

//------------------------------------------------------------------------------
class qMRMLVolumeInfoWidgetPrivate: public Ui_qMRMLVolumeInfoWidget
{
  Q_DECLARE_PUBLIC(qMRMLVolumeInfoWidget);

protected:
  qMRMLVolumeInfoWidget* const q_ptr;

public:
  qMRMLVolumeInfoWidgetPrivate(qMRMLVolumeInfoWidget& object);
  void init();
  bool centeredOrigin(double* origin)const;

  vtkWeakPointer<vtkMRMLVolumeNode> VolumeNode;
};

//------------------------------------------------------------------------------
qMRMLVolumeInfoWidgetPrivate::qMRMLVolumeInfoWidgetPrivate(qMRMLVolumeInfoWidget& object)
  : q_ptr(&object)
{
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidgetPrivate::init()
{
  Q_Q(qMRMLVolumeInfoWidget);
  this->setupUi(q);

  // populate Scalar Types
  for (int i = VTK_VOID; i < VTK_OBJECT; ++i)
    {
    this->ScalarTypeComboBox->addItem(vtkImageScalarTypeNameMacro(i), i);
    }
  // populate scan order
  this->ScanOrderComboBox->addItem("Sagittal LR", "LR");
  this->ScanOrderComboBox->addItem("Sagittal RL", "RL");
  this->ScanOrderComboBox->addItem("Coronal PA", "PA");
  this->ScanOrderComboBox->addItem("Coronal AP", "AP");
  this->ScanOrderComboBox->addItem("Axial IS", "IS");
  this->ScanOrderComboBox->addItem("Axial SI", "SI");

  // Image dimension is read-only
  QObject::connect(this->ImageSpacingWidget, SIGNAL(coordinatesChanged(double*)),
                   q, SLOT(setImageSpacing(double*)));
  QObject::connect(this->ImageOriginWidget, SIGNAL(coordinatesChanged(double*)),
                   q, SLOT(setImageOrigin(double*)));
  QObject::connect(this->CenterVolumePushButton, SIGNAL(clicked()),
                   q, SLOT(center()));

  // setScanOrder is dangerous, it can loose orientation information because
  // ComputeScanOrderFromIJKToRAS is not the exact opposite of
  // ComputeIJKToRASFromScanOrder
  // As an example, ComputeScanOrderFromIJKToRAS returns IS for a trace of -1 1 -1
  // but ComputeIJKToRASFromScanOrder -1 -1 -1 for IS
  // So we should change the scan order only if the user decides to (activated
  // is fired only when the user selects an item)
  QObject::connect(this->ScanOrderComboBox, SIGNAL(activated(int)),
                   q, SLOT(setScanOrder(int)));
  QObject::connect(this->NumberOfScalarsSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setNumberOfScalars(int)));
  QObject::connect(this->ScalarTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(setScalarType(int)));

  // Window level presets are read-only
  q->setDataTypeEditable(false);
  q->setEnabled(this->VolumeNode != 0);
}

//------------------------------------------------------------------------------
bool qMRMLVolumeInfoWidgetPrivate::centeredOrigin(double* origin)const
{
  vtkImageData *imageData = this->VolumeNode ? this->VolumeNode->GetImageData() : 0;
  if (!imageData)
    {
    qWarning() << __FUNCTION__ << "No image data, can't retrieve origin.";
    return false;
    }

  int *dims = imageData->GetDimensions();
  double dimsH[4];
  dimsH[0] = dims[0] - 1;
  dimsH[1] = dims[1] - 1;
  dimsH[2] = dims[2] - 1;
  dimsH[3] = 0.;

  vtkNew<vtkMatrix4x4> ijkToRAS;
  this->VolumeNode->GetIJKToRASMatrix(ijkToRAS.GetPointer());
  double rasCorner[4];
  ijkToRAS->MultiplyPoint(dimsH, rasCorner);

  origin[0] = -0.5 * rasCorner[0];
  origin[1] = -0.5 * rasCorner[1];
  origin[2] = -0.5 * rasCorner[2];

  return true;
}

//------------------------------------------------------------------------------
qMRMLVolumeInfoWidget::qMRMLVolumeInfoWidget(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qMRMLVolumeInfoWidgetPrivate(*this))
{
  Q_D(qMRMLVolumeInfoWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLVolumeInfoWidget::~qMRMLVolumeInfoWidget()
{
}


//------------------------------------------------------------------------------
vtkMRMLVolumeNode* qMRMLVolumeInfoWidget::volumeNode()const
{
  Q_D(const qMRMLVolumeInfoWidget);
  return d->VolumeNode;
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setVolumeNode(vtkMRMLNode* node)
{
  this->setVolumeNode(vtkMRMLVolumeNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setVolumeNode(vtkMRMLVolumeNode* volumeNode)
{
  Q_D(qMRMLVolumeInfoWidget);
  qvtkReconnect(d->VolumeNode, volumeNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  qvtkReconnect(d->VolumeNode, volumeNode, vtkMRMLVolumeNode::ImageDataModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));
  d->VolumeNode = volumeNode;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setDataTypeEditable(bool enable)
{
  Q_D(qMRMLVolumeInfoWidget);
  d->ScanOrderComboBox->setEnabled(enable);
  d->NumberOfScalarsSpinBox->setEnabled(enable);
  d->ScalarTypeComboBox->setEnabled(enable);
}

//------------------------------------------------------------------------------
bool qMRMLVolumeInfoWidget::isDataTypeEditable()const
{
  Q_D(const qMRMLVolumeInfoWidget);
  Q_ASSERT(d->ScanOrderComboBox->isEnabledTo(const_cast<qMRMLVolumeInfoWidget*>(this)) ==
           d->NumberOfScalarsSpinBox->isEnabledTo(const_cast<qMRMLVolumeInfoWidget*>(this)));
  Q_ASSERT(d->ScanOrderComboBox->isEnabledTo(const_cast<qMRMLVolumeInfoWidget*>(this)) ==
           d->ScalarTypeComboBox->isEnabledTo(const_cast<qMRMLVolumeInfoWidget*>(this)));
  return d->ScanOrderComboBox->isEnabledTo(const_cast<qMRMLVolumeInfoWidget*>(this));
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVolumeInfoWidget);
  this->setEnabled(d->VolumeNode != 0);
  if (!d->VolumeNode)
    {
    double dimensions[3] = {0.,0.,0.};
    d->ImageDimensionsWidget->setCoordinates(dimensions);

    double spacing[3] = {1.,1.,1.};
    d->ImageSpacingWidget->setCoordinates(spacing);

    double origin[3] = {0.,0.,0.};
    d->ImageOriginWidget->setCoordinates(origin);

    d->MinScalarDoubleSpinBox->setRange(0., 0.);
    d->MaxScalarDoubleSpinBox->setRange(0., 0.);
    d->MinScalarDoubleSpinBox->setValue(0.);
    d->MaxScalarDoubleSpinBox->setValue(0.);

    d->ScanOrderComboBox->setCurrentIndex(-1);

    d->NumberOfScalarsSpinBox->setValue(1);

    d->ScalarTypeComboBox->setCurrentIndex(-1);

    d->FileNameLineEdit->setText("");

    d->VolumeTagLabel->setText("");

    d->WindowLevelPresetsListWidget->clear();

    return;
    }
  vtkImageData* image = d->VolumeNode->GetImageData();
  double dimensions[3] = {0.,0.,0.};
  int* dims = image ? image->GetDimensions() : 0;
  if (dims)
    {
    dimensions[0] = dims[0];
    dimensions[1] = dims[1];
    dimensions[2] = dims[2];
    }
  d->ImageDimensionsWidget->setCoordinates(dimensions);

  double* spacing = d->VolumeNode->GetSpacing();
  d->ImageSpacingWidget->setCoordinates(spacing);

  double* origin = d->VolumeNode->GetOrigin();
  d->ImageOriginWidget->setCoordinates(origin);

  double IJKToRASDirections[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} };
  d->VolumeNode->GetIJKToRASDirections(IJKToRASDirections);
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      d->IJKToRASDirectionMatrixWidget->setValue(i,j, IJKToRASDirections[i][j]);
      }
    }

  d->CenterVolumePushButton->setEnabled(!this->isCentered());

  vtkNew<vtkMatrix4x4> mat;
  d->VolumeNode->GetIJKToRASMatrix(mat.GetPointer());
  d->ScanOrderComboBox->setCurrentIndex(d->ScanOrderComboBox->findData(
    vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(mat.GetPointer())));

  d->NumberOfScalarsSpinBox->setValue(
    image ? image->GetNumberOfScalarComponents() : 0);

  d->ScalarTypeComboBox->setCurrentIndex( d->ScalarTypeComboBox->findData(
    image ? image->GetScalarType() : -1));

  if (image)
    {
    double typeRange[2];
    vtkDataArray::GetDataTypeRange(image->GetScalarType(), typeRange);
    d->MinScalarDoubleSpinBox->setRange(typeRange[0], typeRange[1]);
    d->MaxScalarDoubleSpinBox->setRange(typeRange[0], typeRange[1]);

    double* scalarRange = image->GetScalarRange();
    d->MinScalarDoubleSpinBox->setValue(scalarRange[0]);
    d->MaxScalarDoubleSpinBox->setValue(scalarRange[1]);
    }
  else
    {
    d->MinScalarDoubleSpinBox->setRange(0., 0.);
    d->MaxScalarDoubleSpinBox->setRange(0., 0.);
    d->MinScalarDoubleSpinBox->setValue(0.);
    d->MaxScalarDoubleSpinBox->setValue(0.);
    }

  vtkMRMLStorageNode* storageNode = d->VolumeNode->GetStorageNode();
  d->FileNameLineEdit->setText(storageNode ? storageNode->GetFileName() : "");

  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::SafeDownCast( d->VolumeNode );

  // Remove "Volume" postfix from node tag name to get only the volume type
  QString volumeType(d->VolumeNode->GetNodeTagName());
  if (volumeType.endsWith("Volume"))
    {
    volumeType.chop(6);
    // Workaround for not having the "Scalar" tag in scalar volumes
    if (volumeType.isEmpty())
      {
      volumeType = QString("Scalar");
      }
    }
  else
    {
    qWarning() << __FUNCTION__ << "Invalid volume node tag '" << volumeType << "'!";
    }
  d->VolumeTagLabel->setText(volumeType);

  vtkMRMLScalarVolumeDisplayNode *displayNode =
    scalarNode ? scalarNode->GetScalarVolumeDisplayNode() : 0;
  if (displayNode)
    {
    // populate the win/level presets
    for (int p = 0; p < displayNode->GetNumberOfWindowLevelPresets(); ++p)
      {
      QString windowLevel;
      windowLevel += QString::number(displayNode->GetWindowPreset(p));
      windowLevel += " | ";
      windowLevel += QString::number(displayNode->GetLevelPreset(p));
      d->WindowLevelPresetsListWidget->addItem(windowLevel);
      }
    }
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setImageSpacing(double* spacing)
{
  Q_D(qMRMLVolumeInfoWidget);
  if (d->VolumeNode == 0)
    {
    return;
    }
  d->VolumeNode->SetSpacing(spacing);
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setImageOrigin(double* origin)
{
  Q_D(qMRMLVolumeInfoWidget);
  if (d->VolumeNode == 0)
    {
    return;
    }
  d->VolumeNode->SetOrigin(origin);
}

//------------------------------------------------------------------------------
bool qMRMLVolumeInfoWidget::isCentered()const
{
  Q_D(const qMRMLVolumeInfoWidget);
  double centerOrigin[3];
  if (!d->centeredOrigin(centerOrigin))
    {
    return false;
    }
  double* volumeOrigin = d->VolumeNode->GetOrigin();
  return qFuzzyCompare(centerOrigin[0], volumeOrigin[0]) &&
         qFuzzyCompare(centerOrigin[1], volumeOrigin[1]) &&
         qFuzzyCompare(centerOrigin[2], volumeOrigin[2]);
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::center()
{
  Q_D(qMRMLVolumeInfoWidget);
  double origin[3];
  if (!d->centeredOrigin(origin))
    {
    return;
    }
  d->VolumeNode->SetOrigin(origin);
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setScanOrder(int index)
{
  Q_D(qMRMLVolumeInfoWidget);
  if (d->VolumeNode == 0)
    {
    return;
    }
  QString scanOrder = d->ScanOrderComboBox->itemData(index).toString();
  vtkNew<vtkMatrix4x4> IJKToRAS;
  if (vtkMRMLVolumeNode::ComputeIJKToRASFromScanOrder(
    scanOrder.toLatin1(),
    d->VolumeNode->GetSpacing(),
    d->VolumeNode->GetImageData()->GetDimensions(),
    this->isCentered(),
    IJKToRAS.GetPointer()))
    {
    if (!this->isCentered())
      {
      IJKToRAS->SetElement(0, 3, d->VolumeNode->GetOrigin()[0]);
      IJKToRAS->SetElement(1, 3, d->VolumeNode->GetOrigin()[1]);
      IJKToRAS->SetElement(2, 3, d->VolumeNode->GetOrigin()[2]);
      }
    d->VolumeNode->SetIJKToRASMatrix(IJKToRAS.GetPointer());
    }
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setNumberOfScalars(int number)
{
  Q_D(qMRMLVolumeInfoWidget);
  vtkImageData* imageData = d->VolumeNode ? d->VolumeNode->GetImageData() : 0;
  if (imageData == 0)
    {
    return;
    }
  vtkNew<vtkTrivialProducer> tp;
  tp->SetOutput(imageData);
  vtkInformation* outInfo = tp->GetOutputInformation(0);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo,
      vtkImageData::GetScalarType(outInfo), number);
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setScalarType(int index)
{
  Q_D(qMRMLVolumeInfoWidget);
  vtkImageData* imageData = d->VolumeNode ? d->VolumeNode->GetImageData() : 0;
  if (imageData == 0)
    {
    return;
    }
  int type = d->ScalarTypeComboBox->itemData(index).toInt();
  vtkNew<vtkTrivialProducer> tp;
  tp->SetOutput(imageData);
  vtkInformation* outInfo = tp->GetOutputInformation(0);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, type,
    vtkImageData::GetNumberOfScalarComponents(outInfo));
}
