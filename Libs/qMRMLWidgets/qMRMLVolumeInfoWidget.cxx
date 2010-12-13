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

// QT includes
#include <QDebug>

// qMRML includes
#include "qMRMLVolumeInfoWidget.h"
#include "ui_qMRMLVolumeInfoWidget.h"

// MRML includes
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>

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

  vtkMRMLVolumeNode* VolumeNode;
};

//------------------------------------------------------------------------------
qMRMLVolumeInfoWidgetPrivate::qMRMLVolumeInfoWidgetPrivate(qMRMLVolumeInfoWidget& object)
  : q_ptr(&object)
{
  this->VolumeNode = 0;
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
  QObject::connect(this->ScanOrderComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(setScanOrder(int)));
  QObject::connect(this->NumberOfScalarsSpinBox, SIGNAL(valueChanged(int)),
                   q, SLOT(setNumberOfScalars(int)));
  QObject::connect(this->ScalarTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(setScalarType(int)));
  // Filename is read-only
  QObject::connect(this->LabelMapCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setLabelMap(bool)));
  // Window level presets are read-only
  
  q->setDataTypeEditable(false);
  q->setLabelMapEditable(true);
  q->setEnabled(this->VolumeNode != 0);
}

//------------------------------------------------------------------------------
bool qMRMLVolumeInfoWidgetPrivate::centeredOrigin(double* origin)const
{
  vtkImageData *imageData = this->VolumeNode ? this->VolumeNode->GetImageData() : 0;
  if (!imageData)
    {
    qWarning() << "qMRMLVolumeInfoWidget::centerVolume(): No image data, can't center volume.";
    return false;
    }
 
  int *dims = imageData->GetDimensions();
  double dimsH[4];
  dimsH[0] = dims[0] - 1;
  dimsH[1] = dims[1] - 1;
  dimsH[2] = dims[2] - 1;
  dimsH[3] = 0.;

  vtkSmartPointer<vtkMatrix4x4> ijkToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  this->VolumeNode->GetIJKToRASMatrix(ijkToRAS);
  double rasCorner[4];
  ijkToRAS->MultiplyPoint(dimsH, rasCorner);

  origin[0] = -0.5 * rasCorner[0];
  origin[1] = -0.5 * rasCorner[1];
  origin[2] = -0.5 * rasCorner[2];
  
  return true;
}

//------------------------------------------------------------------------------
qMRMLVolumeInfoWidget::qMRMLVolumeInfoWidget(QWidget *_parent)
  : QWidget(_parent)
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
void qMRMLVolumeInfoWidget::setLabelMapEditable(bool enable)
{
  Q_D(qMRMLVolumeInfoWidget);
  d->LabelMapCheckBox->setEnabled(enable);
}

//------------------------------------------------------------------------------
bool qMRMLVolumeInfoWidget::isLabelMapEditable()const
{
  Q_D(const qMRMLVolumeInfoWidget);
  return d->LabelMapCheckBox->isEnabledTo(const_cast<qMRMLVolumeInfoWidget*>(this));
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLVolumeInfoWidget);
  this->setEnabled(d->VolumeNode != 0);
  if (!d->VolumeNode)
    {
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
  
  d->CenterVolumePushButton->setEnabled(!this->isCentered());
  
  vtkSmartPointer<vtkMatrix4x4> mat  = vtkSmartPointer<vtkMatrix4x4>::New();
  d->VolumeNode->GetIJKToRASMatrix(mat);
  d->ScanOrderComboBox->setCurrentIndex(d->ScanOrderComboBox->findData(
    vtkMRMLVolumeNode::ComputeScanOrderFromIJKToRAS(mat)));
  
  d->NumberOfScalarsSpinBox->setValue(
    image ? image->GetNumberOfScalarComponents() : 0);
  
  d->ScalarTypeComboBox->setCurrentIndex( d->ScalarTypeComboBox->findData(
    image ? image->GetScalarType() : -1));
  
  vtkMRMLStorageNode* storageNode = d->VolumeNode->GetStorageNode();
  d->FileNameLineEdit->setText(storageNode ? storageNode->GetFileName() : "");
  
  d->LabelMapCheckBox->setEnabled(d->VolumeNode->IsA("vtkMRMLScalarVolumeNode")
                               && !d->VolumeNode->IsA("vtkMRMLTensorVolumeNode") );
  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::SafeDownCast( d->VolumeNode );
  d->LabelMapCheckBox->setChecked(scalarNode ? scalarNode->GetLabelMap() : false);

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
  //volumeNode->SetDisableModifiedEvent(1);
  d->VolumeNode->SetOrigin(origin);
  //volumeNode->SetDisableModifiedEvent(0);
  //volumeNode->InvokePendingModifiedEvent();
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
  vtkSmartPointer<vtkMatrix4x4> IJKToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
  if (vtkMRMLVolumeNode::ComputeIJKToRASFromScanOrder(
    scanOrder.toLatin1(),
    d->VolumeNode->GetSpacing(),
    d->VolumeNode->GetImageData()->GetDimensions(),
    this->isCentered(),
    IJKToRAS))
    {
    d->VolumeNode->SetIJKToRASMatrix(IJKToRAS);
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
  imageData->SetNumberOfScalarComponents(number);
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
  imageData->SetScalarType(type);
}

//------------------------------------------------------------------------------
void qMRMLVolumeInfoWidget::setLabelMap(bool enable)
{
  Q_D(qMRMLVolumeInfoWidget);
  vtkMRMLScalarVolumeNode *scalarNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(d->VolumeNode);
  if (scalarNode == 0 ||
      scalarNode->IsA("vtkMRMLTensorVolume") ||
      static_cast<bool>(scalarNode->GetLabelMap()) == enable)
    {
    return;
    }
  vtkMRMLDisplayNode *oldDisplayNode = scalarNode->GetDisplayNode();
  if (oldDisplayNode)
    {
    scalarNode->GetScene()->RemoveNode(oldDisplayNode);
    }
  vtkMRMLVolumeDisplayNode* displayNode = 0;
  if (enable )
    {
    displayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
    }
  else
    {
    displayNode = vtkMRMLScalarVolumeDisplayNode::New();
    }
  scalarNode->GetScene()->AddNodeNoNotify(displayNode);
  displayNode->SetAndObserveColorNodeID (enable ? "vtkMRMLColorTableNodeLabels" : "vtkMRMLColorTableNodeGrey");
  scalarNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  scalarNode->SetLabelMap( enable );
  //scalarNode->GetScene()->Edited();
  displayNode->Delete();
}
