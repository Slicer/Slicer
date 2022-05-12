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

// Segmentations includes
#include "qMRMLSegmentationGeometryWidget.h"

#include "ui_qMRMLSegmentationGeometryWidget.h"

#include "vtkSlicerSegmentationGeometryLogic.h"
#include "vtkSlicerSegmentationsModuleLogic.h"
#include "vtkMRMLSegmentationNode.h"

// SegmentationCore includes
#include "vtkSegmentation.h"
#include "vtkSegmentationConverter.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkCalculateOversamplingFactor.h"

// MRML includes
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkAddonMathUtilities.h>
#include <vtkGeneralTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkTransform.h>
#include <vtkWeakPointer.h>

// Qt includes
#include <QDebug>

//-----------------------------------------------------------------------------
class qMRMLSegmentationGeometryWidgetPrivate: public Ui_qMRMLSegmentationGeometryWidget
{
  Q_DECLARE_PUBLIC(qMRMLSegmentationGeometryWidget);

protected:
  qMRMLSegmentationGeometryWidget* const q_ptr;
public:
  qMRMLSegmentationGeometryWidgetPrivate(qMRMLSegmentationGeometryWidget& object);
  virtual ~qMRMLSegmentationGeometryWidgetPrivate();
  void init();

  /// Fill geometry info section from geometry image data containing geometry information \sa GeometryImageData
  void updateGeometryWidgets();

public:
  /// Segmentation MRML node
  vtkWeakPointer<vtkMRMLSegmentationNode> SegmentationNode;

  /// Segmentation geometry logic
  vtkSlicerSegmentationGeometryLogic* Logic;

  /// Flag indicating whether editing is enabled in the widget
  bool EditEnabled;
};

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryWidgetPrivate::qMRMLSegmentationGeometryWidgetPrivate(qMRMLSegmentationGeometryWidget& object)
  : q_ptr(&object)
  , EditEnabled(false)
{
  this->Logic = vtkSlicerSegmentationGeometryLogic::New();
  this->Logic->PadOutputGeometryOff();
}

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryWidgetPrivate::~qMRMLSegmentationGeometryWidgetPrivate()
{
  if (this->Logic)
    {
    this->Logic->Delete();
    this->Logic = nullptr;
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidgetPrivate::init()
{
  Q_Q(qMRMLSegmentationGeometryWidget);
  this->setupUi(q);

  // Make connections
  QObject::connect(this->MRMLNodeComboBox_SourceGeometryNode, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(onSourceNodeChanged(vtkMRMLNode*)) );
  QObject::connect(this->DoubleSpinBox_OversamplingFactor, SIGNAL(valueChanged(double)),
    q, SLOT(onOversamplingFactorChanged(double)) );
  QObject::connect(this->checkBox_IsotropicSpacing, SIGNAL(toggled(bool)),
    q, SLOT(onIsotropicSpacingChanged(bool)) );
  QObject::connect(this->MRMLCoordinatesWidget_Spacing, SIGNAL(coordinatesChanged(double*)),
    q, SLOT(onUserSpacingChanged(double*)) );
  QObject::connect(this->CheckBox_PadSegmentation, SIGNAL(toggled(bool)),
    q, SLOT(onPadSegmentationChanged(bool)));

  q->setEnabled(this->SegmentationNode.GetPointer());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidgetPrivate::updateGeometryWidgets()
{
  Q_Q(qMRMLSegmentationGeometryWidget);

  vtkOrientedImageData* geometryImageData = this->Logic->GetOutputGeometryImageData();
  if (!geometryImageData)
    {
    qCritical() << Q_FUNC_INFO << ": Invalid geometry image data";
    return;
    }

  int dims[3] = { 0 };
  geometryImageData->GetDimensions(dims);
  double dimsDouble[3] = { (double)dims[0], (double)dims[1], (double)dims[2] };
  this->MRMLCoordinatesWidget_Dimensions->setCoordinates(dimsDouble);

  // Apply inverse source axis permutation
  int sourceAxisIndexForInputAxis[3] = { 0 };
  this->Logic->GetSourceAxisIndexForInputAxis(sourceAxisIndexForInputAxis);
  double spacing[3] = { 0.0 };
  geometryImageData->GetSpacing(spacing);
  double outputSpacing[3]  = {
    spacing[sourceAxisIndexForInputAxis[0]],
    spacing[sourceAxisIndexForInputAxis[1]],
    spacing[sourceAxisIndexForInputAxis[2]] };

  bool blocked = this->MRMLCoordinatesWidget_Spacing->blockSignals(true);
  this->MRMLCoordinatesWidget_Spacing->setCoordinates(outputSpacing);
  this->MRMLCoordinatesWidget_Spacing->blockSignals(blocked);

  double origin[3] = { 0.0 };
  geometryImageData->GetOrigin(origin);
  this->MRMLCoordinatesWidget_Origin->setCoordinates(origin);

  vtkNew<vtkMatrix4x4> directions;
  geometryImageData->GetDirectionMatrix(directions.GetPointer());
  for (int i=0; i<3; ++i)
    {
    for (int j=0; j<3; ++j)
      {
      this->MatrixWidget_Directions->setValue(i, j, directions->GetElement(i,j));
      }
    }

  this->CheckBox_PadSegmentation->setIcon(QIcon());
  this->CheckBox_PadSegmentation->setText("");
  if (this->SegmentationNode && this->SegmentationNode->GetSegmentation() && !this->Logic->GetPadOutputGeometry())
    {
    std::string commonLabelmapGeometryString = this->SegmentationNode->GetSegmentation()->DetermineCommonLabelmapGeometry(
      vtkSegmentation::EXTENT_UNION_OF_EFFECTIVE_SEGMENTS);
    vtkNew<vtkOrientedImageData> segmentationGeometry;
    vtkSegmentationConverter::DeserializeImageGeometry(commonLabelmapGeometryString, segmentationGeometry, false/*don't allocate*/);
    if (vtkSlicerSegmentationsModuleLogic::IsSegmentationExentOutsideReferenceGeometry(geometryImageData, segmentationGeometry))
      {
      QIcon warningIcon = q->style()->standardIcon(QStyle::SP_MessageBoxWarning);
      this->CheckBox_PadSegmentation->setIcon(warningIcon);
      this->CheckBox_PadSegmentation->setText("The current segmentation may not fit into the new geometry.");
      }
    }
}


//-----------------------------------------------------------------------------
// qMRMLSegmentationGeometryWidget methods

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryWidget::qMRMLSegmentationGeometryWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLSegmentationGeometryWidgetPrivate(*this))
{
  Q_D(qMRMLSegmentationGeometryWidget);
  d->init();
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
qMRMLSegmentationGeometryWidget::~qMRMLSegmentationGeometryWidget() = default;

//-----------------------------------------------------------------------------
vtkMRMLSegmentationNode* qMRMLSegmentationGeometryWidget::segmentationNode()const
{
  Q_D(const qMRMLSegmentationGeometryWidget);
  return d->SegmentationNode;
}

//-----------------------------------------------------------------------------
QString qMRMLSegmentationGeometryWidget::segmentationNodeID()const
{
  Q_D(const qMRMLSegmentationGeometryWidget);
  return (d->SegmentationNode.GetPointer() ? d->SegmentationNode->GetID() : QString());
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setSegmentationNode(vtkMRMLSegmentationNode* node)
{
  Q_D(qMRMLSegmentationGeometryWidget);

  if (d->SegmentationNode == node)
    {
    return;
    }

  if (node)
    {
    this->setMRMLScene(node->GetScene());
    }

  qvtkReconnect(d->SegmentationNode, node, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  vtkMRMLSegmentationNode* segmentationNode = vtkMRMLSegmentationNode::SafeDownCast(node);
  d->SegmentationNode = segmentationNode;

  d->Logic->SetInputSegmentationNode(segmentationNode);

  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationGeometryWidget::editEnabled()const
{
  Q_D(const qMRMLSegmentationGeometryWidget);
  return d->EditEnabled;
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setEditEnabled(bool aEditEnabled)
{
  Q_D(qMRMLSegmentationGeometryWidget);
  d->EditEnabled = aEditEnabled;
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
vtkMRMLNode* qMRMLSegmentationGeometryWidget::sourceNode()const
{
  Q_D(const qMRMLSegmentationGeometryWidget);
  return d->MRMLNodeComboBox_SourceGeometryNode->currentNode();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setSourceNode(vtkMRMLNode* sourceNode)
{
  Q_D(qMRMLSegmentationGeometryWidget);
  if (sourceNode && sourceNode->GetScene() != this->mrmlScene())
    {
    qCritical() << Q_FUNC_INFO << ": MRML scene of the given source node and the widget are different, cannot set node";
    return;
    }

  qvtkReconnect(this->sourceNode(), sourceNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));

  d->MRMLNodeComboBox_SourceGeometryNode->setCurrentNode(sourceNode);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::onSourceNodeChanged(vtkMRMLNode* sourceNode)
{
  Q_D(qMRMLSegmentationGeometryWidget);

  d->Logic->SetSourceGeometryNode(vtkMRMLDisplayableNode::SafeDownCast(sourceNode));

  // updateWidgetFromMRML() only updates GUI if input is valid, so we need to make sure
  // that widgets are up-to-date

  // Spacing
  // When changing source node, current geometry spacing may differ from
  // the spacing that the user set. By default, show the user spacing.
  int sourceAxisIndexForInputAxis[3] = { 0 };
  d->Logic->GetSourceAxisIndexForInputAxis(sourceAxisIndexForInputAxis);
  double spacing[3] = { 0.0 };
  d->Logic->GetUserSpacing(spacing);
  double outputSpacing[3] = {
    spacing[sourceAxisIndexForInputAxis[0]],
    spacing[sourceAxisIndexForInputAxis[1]],
    spacing[sourceAxisIndexForInputAxis[2]] };
  bool wasBlocked = d->MRMLCoordinatesWidget_Spacing->blockSignals(true);
  d->MRMLCoordinatesWidget_Spacing->setCoordinates(outputSpacing);
  d->MRMLCoordinatesWidget_Spacing->blockSignals(wasBlocked);

  // Calculate output geometry and update UI
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::onOversamplingFactorChanged(double oversamplingFactor)
{
  Q_D(qMRMLSegmentationGeometryWidget);

  d->Logic->SetOversamplingFactor(oversamplingFactor);

  // Calculate output geometry and update UI
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::onIsotropicSpacingChanged(bool isotropicSpacing)
{
  Q_D(qMRMLSegmentationGeometryWidget);

  d->Logic->SetIsotropicSpacing(isotropicSpacing);

  // Calculate output geometry and update UI
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::onUserSpacingChanged(double* userSpacing)
{
  Q_D(qMRMLSegmentationGeometryWidget);

  d->Logic->SetUserSpacing(userSpacing);

  // Calculate output geometry and update UI
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
double qMRMLSegmentationGeometryWidget::oversamplingFactor()const
{
  Q_D(const qMRMLSegmentationGeometryWidget);
  return d->DoubleSpinBox_OversamplingFactor->value();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setOversamplingFactor(double aOversamplingFactor)
{
  Q_D(qMRMLSegmentationGeometryWidget);
  d->DoubleSpinBox_OversamplingFactor->setValue(aOversamplingFactor);
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationGeometryWidget::isotropicSpacing()const
{
  Q_D(const qMRMLSegmentationGeometryWidget);
  return d->checkBox_IsotropicSpacing->isChecked();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setIsotropicSpacing(bool aIsotropicSpacing)
{
  Q_D(qMRMLSegmentationGeometryWidget);
  d->checkBox_IsotropicSpacing->setChecked(aIsotropicSpacing);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setSpacing(double aSpacing[3])
{
  Q_D(qMRMLSegmentationGeometryWidget);
  d->MRMLCoordinatesWidget_Spacing->setCoordinates(aSpacing);
}

//-----------------------------------------------------------------------------
bool qMRMLSegmentationGeometryWidget::padSegmentation()const
{
  Q_D(const qMRMLSegmentationGeometryWidget);
  return d->CheckBox_PadSegmentation->isChecked();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setPadSegmentation(bool aPadSegmentation)
{
  Q_D(qMRMLSegmentationGeometryWidget);
  d->CheckBox_PadSegmentation->setChecked(aPadSegmentation);
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::onPadSegmentationChanged(bool padSegmentation)
{
  Q_D(qMRMLSegmentationGeometryWidget);

  d->Logic->SetPadOutputGeometry(padSegmentation);
  this->updateWidgetFromMRML();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLSegmentationGeometryWidget);

  // Reset geometry
  d->Logic->ResetGeometryImageData();

  // Sanity check
  d->label_Error->setVisible(false);
  this->setEnabled(d->SegmentationNode.GetPointer());
  if (!d->SegmentationNode.GetPointer())
    {
    d->frame_SourceGeometry->setVisible(false);
    d->groupBox_VolumeSpacingOptions->setVisible(false);
    d->MRMLCoordinatesWidget_Spacing->setEnabled(false);
    d->label_Error->setText("No segmentation node specified!");
    d->label_Error->setVisible(true);
    d->updateGeometryWidgets();
    return;
    }

  // Get source node
  vtkMRMLTransformableNode* sourceNode = vtkMRMLTransformableNode::SafeDownCast(
    d->MRMLNodeComboBox_SourceGeometryNode->currentNode() );
  // Get possible source volumes
  vtkMRMLScalarVolumeNode* sourceVolumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(sourceNode);
  bool sourceIsVolume = (sourceVolumeNode != nullptr) || d->Logic->IsSourceSegmentationWithBinaryLabelmapMaster();

  // If editing is disabled, then hide source node selector and use no source node even if was previously selected
  if (d->EditEnabled)
    {
    d->frame_SourceGeometry->setVisible(true);
    }
  else
    {
    d->frame_SourceGeometry->setVisible(false);
    sourceNode = nullptr;
    }

  // If volume node is selected, then show volume spacing options box
  d->groupBox_VolumeSpacingOptions->setVisible(sourceNode != nullptr && sourceIsVolume);
  // Otherwise enable spacing widget to allow editing if it's allowed
  d->MRMLCoordinatesWidget_Spacing->setEnabled(sourceNode != nullptr && !sourceIsVolume && d->EditEnabled);

  // If no source node is selected, then show the current labelmap geometry
  if (!sourceNode)
    {
    d->groupBox_VolumeSpacingOptions->setVisible(false);
    std::string geometryString = d->SegmentationNode->GetSegmentation()->GetConversionParameter(
      vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
    if (!geometryString.empty())
      {
      vtkOrientedImageData* geometryImageData = d->Logic->GetOutputGeometryImageData();
      vtkSegmentationConverter::DeserializeImageGeometry(geometryString, geometryImageData, false);
      }
    d->updateGeometryWidgets();
    return;
    }

  // Calculate output geometry based on selection
  std::string errorMessage = d->Logic->CalculateOutputGeometry();
  if (!errorMessage.empty())
    {
    d->label_Error->setText(errorMessage.c_str());
    d->label_Error->setVisible(true);
    qCritical() << Q_FUNC_INFO << ": " << errorMessage.c_str();

    // We must not call d->updateGeometryWidgets() here, because it could
    // overwrite spacing, which would be very annoying (e.g., the user
    // enters 0 into one of the spacing fields and all spacing fields
    // would be reset and because of 0 spacing, directions matrix
    // would be invalid).
    }
  else
    {
    // Fill output geometry in the widget
    d->updateGeometryWidgets();
    }
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::setReferenceImageGeometryForSegmentationNode()
{
  Q_D(qMRMLSegmentationGeometryWidget);
  if (!d->SegmentationNode.GetPointer())
    {
    qCritical() << Q_FUNC_INFO << "No input segmentation specified";
    return;
    }

  // Save reference geometry
  vtkOrientedImageData* geometryImageData = d->Logic->GetOutputGeometryImageData();
  std::string geometryString = vtkSegmentationConverter::SerializeImageGeometry(geometryImageData);
  d->SegmentationNode->GetSegmentation()->SetConversionParameter(
    vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), geometryString );

  // Save reference geometry node (this is shown in Segmentations module
  // to gives a hint about which node the current geometry is based on)
  const char* referenceGeometryNodeID = nullptr;
  if (d->Logic->GetSourceGeometryNode())
    {
    referenceGeometryNodeID = d->Logic->GetSourceGeometryNode()->GetID();
    }

  // If the reference geometry node is the same as the segmentation node, then don't change the node reference
  if (vtkMRMLSegmentationNode::SafeDownCast(d->Logic->GetSourceGeometryNode()) != d->SegmentationNode)
    {
    d->SegmentationNode->SetNodeReferenceID(
      vtkMRMLSegmentationNode::GetReferenceImageGeometryReferenceRole().c_str(), referenceGeometryNodeID);
    }

  // Note: it could be also useful to save oversampling value and isotropic flag,
  // we could then allow the user to modify these settings instead of always
  // setting from scratch.

  qDebug() << Q_FUNC_INFO << "Reference image geometry of " << d->SegmentationNode->GetName()
    << " has been set to '" << geometryString.c_str() << "'";
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::resampleLabelmapsInSegmentationNode()
{
  Q_D(qMRMLSegmentationGeometryWidget);
  d->Logic->ResampleLabelmapsInSegmentationNode();
}

//-----------------------------------------------------------------------------
void qMRMLSegmentationGeometryWidget::geometryImageData(vtkOrientedImageData* outputGeometry)
{
  Q_D(qMRMLSegmentationGeometryWidget);
  if (!outputGeometry)
    {
    return;
    }
  outputGeometry->ShallowCopy(d->Logic->GetOutputGeometryImageData());
}
