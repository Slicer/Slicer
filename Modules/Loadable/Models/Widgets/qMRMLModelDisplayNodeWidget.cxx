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
#include <QColor>

// CTK includes
#include <ctkUtils.h>

// qMRML includes
#include "qMRMLModelDisplayNodeWidget.h"
#include "ui_qMRMLModelDisplayNodeWidget.h"

// Subject hierarchy includes
#include <qSlicerSubjectHierarchyFolderPlugin.h>
#include <qSlicerSubjectHierarchyPluginHandler.h>

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

// VTK includes
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkPointSet.h>
#include <vtkProperty.h>
#include <vtkSmartPointer.h>

static const int REPRESENTATION_POINTS = 0;
static const int REPRESENTATION_WIREFRAME = 1;
static const int REPRESENTATION_SURFACE = 2;

//------------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
class qMRMLModelDisplayNodeWidgetPrivate: public QWidget, public Ui_qMRMLModelDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLModelDisplayNodeWidget);

protected:
  qMRMLModelDisplayNodeWidget* const q_ptr;
  typedef QWidget Superclass;

public:
  qMRMLModelDisplayNodeWidgetPrivate(qMRMLModelDisplayNodeWidget& object);
  void init();

  vtkWeakPointer<vtkMRMLModelDisplayNode> ModelDisplayNode;
  vtkWeakPointer<vtkMRMLDisplayNode> DisplayNode;
  vtkSmartPointer<vtkProperty> Property;
  vtkIdType CurrentSubjectHierarchyItemID;
  // Store what data range was used to automatically slider range of display range,
  // to prevent resetting slider range when user moves the slider.
  double DataRangeUsedForAutoDisplayRange[2];
};

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidgetPrivate::qMRMLModelDisplayNodeWidgetPrivate(qMRMLModelDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->Property = vtkSmartPointer<vtkProperty>::New();
  this->CurrentSubjectHierarchyItemID = vtkMRMLSubjectHierarchyNode::INVALID_ITEM_ID;
  this->DataRangeUsedForAutoDisplayRange[0] = 0.0;
  this->DataRangeUsedForAutoDisplayRange[1] = 0.0;
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidgetPrivate::init()
{
  Q_Q(qMRMLModelDisplayNodeWidget);
  this->setupUi(q);

  this->MaterialPropertyWidget->setProperty(this->Property);
  q->qvtkConnect(this->Property, vtkCommand::ModifiedEvent,
    q, SLOT(updateNodeFromProperty()));

  q->connect(this->VisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setVisibility(bool)));
  q->connect(this->ClippingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setClipping(bool)));
  q->connect(this->ConfigureClippingPushButton, SIGNAL(clicked()),
    q, SIGNAL(clippingConfigurationButtonClicked()));
  this->ConfigureClippingPushButton->setVisible(false);

  q->connect(this->SliceIntersectionVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setSliceIntersectionVisible(bool)));
  q->connect(this->SliceDisplayModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setSliceDisplayMode(int)));
  q->connect(this->SliceIntersectionThicknessSpinBox, SIGNAL(valueChanged(int)),
    q, SLOT(setSliceIntersectionThickness(int)));
  q->connect(this->SliceIntersectionOpacitySlider, SIGNAL(valueChanged(double)),
    q, SLOT(setSliceIntersectionOpacity(double)));
  q->connect(this->DistanceToColorNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
    q, SLOT(setDistanceToColorNode(vtkMRMLNode*)));

  q->connect(this->RepresentationComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setRepresentation(int)));
  q->connect(this->PointSizeSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setPointSize(double)));
  q->connect(this->LineWidthSliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setLineWidth(double)));
  q->connect(this->ShowFacesComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setShowFaces(int)));
  q->connect(this->ColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(setColor(QColor)));
  q->connect(this->OpacitySliderWidget, SIGNAL(valueChanged(double)),
    q, SLOT(setOpacity(double)));
  q->connect(this->EdgeColorPickerButton, SIGNAL(colorChanged(QColor)),
    q, SLOT(setEdgeColor(QColor)));

  q->connect(this->LightingCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setLighting(bool)));
  q->connect(this->InterpolationComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setInterpolation(int)));

  // Without this infinite loop of widget update/VTK data set update occurs when updating a VTK data set
  // that is generated by an algorithm that  temporarily removes all arrays from its output temporarily
  // (for example the vtkGlyph3D filter behaves like this).
  // The root cause of the problem is that if none option is not enabled then the combobox
  // automatically selects the first array, which triggers a data set change, which removes all arrays,
  // which triggers a widget update, etc. - until stack overflow occurs.
  //this->ActiveScalarComboBox->setNoneEnabled(true);

  // Scalar
  QObject::connect(this->ScalarsVisibilityCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setScalarsVisibility(bool)));
  QObject::connect(this->ActiveScalarComboBox, SIGNAL(currentArrayChanged(QString)),
    q, SLOT(setActiveScalarName(QString)));
  QObject::connect(this->ScalarsColorNodeComboBox,
    SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(setScalarsColorNode(vtkMRMLNode*)));
  // scalar range
  QObject::connect(this->DisplayedScalarRangeModeComboBox, SIGNAL(currentIndexChanged(int)),
    q, SLOT(setScalarRangeMode(int)));
  QObject::connect(this->DisplayedScalarRangeWidget, SIGNAL(valuesChanged(double,double)),
    q, SLOT(setScalarsDisplayRange(double,double)));

  // Thresholding
  this->ThresholdCheckBox->setChecked(false);
  this->ThresholdRangeWidget->setEnabled(false);
  QObject::connect(this->ThresholdCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setTresholdEnabled(bool)));
  QObject::connect(this->ThresholdRangeWidget, SIGNAL(valuesChanged(double,double)),
                   q, SLOT(setThresholdRange(double,double)));

  // update range mode
  q->setScalarRangeMode(qMRMLModelDisplayNodeWidget::Data); // former auto

  if (this->ModelDisplayNode.GetPointer())
  {
    q->setEnabled(true);
    q->setMRMLModelDisplayNode(this->ModelDisplayNode);
  }
}

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::qMRMLModelDisplayNodeWidget(QWidget* _parent)
  : qMRMLWidget(_parent)
  , d_ptr(new qMRMLModelDisplayNodeWidgetPrivate(*this))
{
  Q_D(qMRMLModelDisplayNodeWidget);
  d->init();
}

//------------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::~qMRMLModelDisplayNodeWidget()
= default;

//------------------------------------------------------------------------------
vtkMRMLModelDisplayNode* qMRMLModelDisplayNodeWidget::mrmlModelDisplayNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ModelDisplayNode;
}

//------------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLModelDisplayNodeWidget::mrmlDisplayNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->DisplayNode;
}

//------------------------------------------------------------------------------
vtkIdType qMRMLModelDisplayNodeWidget::currentSubjectHierarchyItemID()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->CurrentSubjectHierarchyItemID;
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setCurrentSubjectHierarchyItemID(vtkIdType currentItemID)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (d->CurrentSubjectHierarchyItemID == currentItemID)
    {
    return;
    }
  vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(this->mrmlScene());
  if (!shNode)
    {
    qCritical() << Q_FUNC_INFO << ": Failed to access subject hierarchy node";
    return;
    }

  d->CurrentSubjectHierarchyItemID = currentItemID;

  vtkMRMLDisplayNode* displayNode = nullptr;

  // Can be set from model or folder
  vtkMRMLModelNode* modelNode = vtkMRMLModelNode::SafeDownCast(shNode->GetItemDataNode(currentItemID));
  if (modelNode)
    {
    // Note: Formerly the last display node was chosen that was model display node typem (or the proper fiber type)
    displayNode = modelNode->GetDisplayNode();
    }

  qSlicerSubjectHierarchyFolderPlugin* folderPlugin = qobject_cast<qSlicerSubjectHierarchyFolderPlugin*>(
    qSlicerSubjectHierarchyPluginHandler::instance()->pluginByName("Folder") );
  if (folderPlugin->canOwnSubjectHierarchyItem(currentItemID) > 0.0)
    {
    displayNode = folderPlugin->createDisplayNodeForItem(currentItemID);
    }

  this->setMRMLDisplayNode(displayNode);
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLModelDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLModelDisplayNode(vtkMRMLModelDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLModelDisplayNode(vtkMRMLModelDisplayNode* modelDisplayNode)
{
  this->setMRMLDisplayNode(modelDisplayNode);
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMRMLDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (d->DisplayNode == displayNode)
    {
    return;
    }

  qvtkReconnect(d->DisplayNode, displayNode, vtkCommand::ModifiedEvent, this, SLOT(updateWidgetFromMRML()));
  d->DisplayNode = displayNode;
  d->ModelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsVisibility(bool visible)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }

  d->DisplayNode->SetScalarVisibility(visible);
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::scalarsVisibility()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ScalarsVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setActiveScalarName(const QString& arrayName)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->ModelDisplayNode.GetPointer())
    {
    return;
    }
  int wasModified = d->DisplayNode->StartModify();
  d->ModelDisplayNode->SetActiveScalar(arrayName.toLatin1(), d->ActiveScalarComboBox->currentArrayLocation());

  // if there's no color node set for a non empty array name, use a default
  if (!arrayName.isEmpty() && d->ModelDisplayNode->GetColorNodeID() == nullptr)
    {
    const char* colorNodeID = "vtkMRMLColorTableNodeFileViridis.txt";
    d->ModelDisplayNode->SetAndObserveColorNodeID(colorNodeID);
    }
  d->ModelDisplayNode->EndModify(wasModified);
}

//------------------------------------------------------------------------------
QString qMRMLModelDisplayNodeWidget::activeScalarName()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  // TODO: use currentArrayName()
  vtkAbstractArray* array = d->ActiveScalarComboBox->currentArray();
  return array ? array->GetName() : "";
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsColorNode(vtkMRMLNode* colorNode)
{
  this->setScalarsColorNode(vtkMRMLColorNode::SafeDownCast(colorNode));
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsColorNode(vtkMRMLColorNode* colorNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }

  d->DisplayNode->SetAndObserveColorNodeID(colorNode ? colorNode->GetID() : nullptr);
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLModelDisplayNodeWidget::scalarsColorNode()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return vtkMRMLColorNode::SafeDownCast(d->ScalarsColorNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarRangeMode(ControlMode controlMode)
{
  Q_D(qMRMLModelDisplayNodeWidget);

  if (!d->ModelDisplayNode.GetPointer())
    {
    return;
    }

  int newScalarRangeMode = vtkMRMLDisplayNode::UseManualScalarRange;
  switch (controlMode)
    {
    case Data: newScalarRangeMode = vtkMRMLDisplayNode::UseDataScalarRange; break;
    case LUT: newScalarRangeMode = vtkMRMLDisplayNode::UseColorNodeScalarRange; break;
    case DataType: newScalarRangeMode = vtkMRMLDisplayNode::UseDataTypeScalarRange; break;
    case Manual: newScalarRangeMode = vtkMRMLDisplayNode::UseManualScalarRange; break;
    case DirectMapping: newScalarRangeMode = vtkMRMLDisplayNode::UseDirectMapping; break;
    }

  int currentScalarRangeMode = d->ModelDisplayNode->GetScalarRangeFlag();
  if (currentScalarRangeMode == newScalarRangeMode)
    {
    // no change
    return;
    }

  d->ModelDisplayNode->SetScalarRangeFlag(newScalarRangeMode);
  emit this->scalarRangeModeValueChanged(controlMode);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarRangeMode(int scalarRangeMode)
{
  switch(scalarRangeMode)
    {
    case 0:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::Data);
      break;
    case 1:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::LUT);
      break;
    case 2:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::DataType);
      break;
    case 3:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::Manual);
      break;
    case 4:
      this->setScalarRangeMode(qMRMLModelDisplayNodeWidget::DirectMapping);
      break;
    default:
      break;
    }
}

// --------------------------------------------------------------------------
qMRMLModelDisplayNodeWidget::ControlMode qMRMLModelDisplayNodeWidget::scalarRangeMode() const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  switch( d->DisplayedScalarRangeModeComboBox->currentIndex() )
    {
    case 0:
      return qMRMLModelDisplayNodeWidget::Data;
      break;
    case 1:
      return qMRMLModelDisplayNodeWidget::LUT;
      break;
    case 2:
      return qMRMLModelDisplayNodeWidget::DataType;
      break;
    case 3:
      return qMRMLModelDisplayNodeWidget::Manual;
      break;
    case 4:
      return qMRMLModelDisplayNodeWidget::DirectMapping;
      break;
    default:
      break;
    }
  return qMRMLModelDisplayNodeWidget::Data;
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setScalarsDisplayRange(double min, double max)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  double *range = d->DisplayNode->GetScalarRange();
  if (range[0] == min && range[1] == max)
    {
    return;
    }
  d->DisplayNode->SetScalarRange(min, max);
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setTresholdEnabled(bool b)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->ModelDisplayNode.GetPointer())
    {
    return;
    }
  d->ModelDisplayNode->SetThresholdEnabled(b);
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setThresholdRange(double min, double max)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->ModelDisplayNode.GetPointer())
    {
    return;
    }
  double oldMin = d->ModelDisplayNode->GetThresholdMin();
  double oldMax = d->ModelDisplayNode->GetThresholdMax();
  if (oldMin == min && oldMax == max)
    {
    return;
    }
  d->ModelDisplayNode->SetThresholdRange(min, max);
}

// --------------------------------------------------------------------------
double qMRMLModelDisplayNodeWidget::minimumValue() const
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  return d->DisplayedScalarRangeWidget->minimumValue();
}

// --------------------------------------------------------------------------
double qMRMLModelDisplayNodeWidget::maximumValue() const
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  return d->DisplayedScalarRangeWidget->maximumValue();
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMinimumValue(double min)
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  d->DisplayedScalarRangeWidget->setMinimumValue(min);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setMaximumValue(double max)
{
  Q_D(const qMRMLModelDisplayNodeWidget);

  d->DisplayedScalarRangeWidget->setMaximumValue(max);
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::updateWidgetFromMRML()
{
  bool wasBlocking;

  Q_D(qMRMLModelDisplayNodeWidget);
  this->setEnabled(d->DisplayNode.GetPointer() != nullptr);
  if (!d->DisplayNode.GetPointer())
    {
    emit displayNodeChanged();
    return;
    }

  d->VisibilityCheckBox->setChecked(d->DisplayNode->GetVisibility());
  d->DisplayNodeViewComboBox->setMRMLDisplayNode(d->DisplayNode);
  d->ClippingCheckBox->setChecked(d->DisplayNode->GetClipping());
  d->SliceIntersectionVisibilityCheckBox->setChecked(d->DisplayNode->GetVisibility2D());
  d->SliceIntersectionThicknessSpinBox->setValue(d->DisplayNode->GetSliceIntersectionThickness());
  bool showSliceIntersectionThickness =
    (d->ModelDisplayNode && d->ModelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayIntersection);
  d->SliceIntersectionThicknessSpinBox->setEnabled(showSliceIntersectionThickness);
  d->SliceIntersectionOpacitySlider->setValue(d->DisplayNode->GetSliceIntersectionOpacity());

  d->SliceDisplayModeComboBox->setEnabled(d->ModelDisplayNode);
  if (d->ModelDisplayNode)
    {
    d->SliceDisplayModeComboBox->setCurrentIndex(d->ModelDisplayNode->GetSliceDisplayMode());
    }

  wasBlocking = d->DistanceToColorNodeComboBox->blockSignals(true);
  if (d->DistanceToColorNodeComboBox->mrmlScene() != this->mrmlScene())
    {
    d->DistanceToColorNodeComboBox->setMRMLScene(this->mrmlScene());
    }
  if ( d->ModelDisplayNode
    && d->DistanceToColorNodeComboBox->currentNodeID() != d->ModelDisplayNode->GetDistanceEncodedProjectionColorNodeID() )
    {
    d->DistanceToColorNodeComboBox->setCurrentNodeID(d->ModelDisplayNode->GetDistanceEncodedProjectionColorNodeID());
    }
  d->DistanceToColorNodeComboBox->setEnabled( d->ModelDisplayNode &&
    d->ModelDisplayNode->GetSliceDisplayMode() == vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection );
  d->DistanceToColorNodeComboBox->blockSignals(wasBlocking);

  // Representation
  switch (d->DisplayNode->GetRepresentation())
    {
    case REPRESENTATION_POINTS:
      d->RepresentationComboBox->setCurrentIndex(0);
      break;
    case REPRESENTATION_WIREFRAME:
      d->RepresentationComboBox->setCurrentIndex(1);
      break;
    case REPRESENTATION_SURFACE:
      if (d->DisplayNode->GetEdgeVisibility())
        {
        d->RepresentationComboBox->setCurrentIndex(3);
        }
      else
        {
        d->RepresentationComboBox->setCurrentIndex(2);
        }
      break;
    }

  d->PointSizeSliderWidget->setValue(d->DisplayNode->GetPointSize());
  bool showPointSize = d->DisplayNode->GetRepresentation() == REPRESENTATION_POINTS;
  d->PointSizeSliderWidget->setEnabled(showPointSize);

  d->LineWidthSliderWidget->setValue(d->DisplayNode->GetLineWidth());
  bool showLineWidth = (d->DisplayNode->GetRepresentation() == REPRESENTATION_WIREFRAME)
    || (d->DisplayNode->GetRepresentation() == REPRESENTATION_SURFACE && d->DisplayNode->GetEdgeVisibility());
  d->LineWidthSliderWidget->setEnabled(showLineWidth);

  if (!d->DisplayNode->GetFrontfaceCulling() && d->DisplayNode->GetBackfaceCulling())
    {
    // show only front faces
    d->ShowFacesComboBox->setCurrentIndex(1);
    }
  else if (d->DisplayNode->GetFrontfaceCulling() && !d->DisplayNode->GetBackfaceCulling())
    {
    // show only back faces
    d->ShowFacesComboBox->setCurrentIndex(2);
    }
  else
    {
    // show all faces
    d->ShowFacesComboBox->setCurrentIndex(0);
    }

  double* c = d->DisplayNode->GetColor();
  d->ColorPickerButton->setColor(QColor::fromRgbF(qMin(c[0], 1.), qMin(c[1], 1.), qMin(c[2], 1.)));
  d->OpacitySliderWidget->setValue(d->DisplayNode->GetOpacity());
  double* ec = d->DisplayNode->GetEdgeColor();
  d->EdgeColorPickerButton->setColor(
    QColor::fromRgbF(qMin(ec[0], 1.), qMin(ec[1], 1.), qMin(ec[2], 1.)));
  bool showEdgeColor =
    (d->DisplayNode->GetRepresentation() == REPRESENTATION_SURFACE && d->DisplayNode->GetEdgeVisibility());
  d->EdgeColorPickerButton->setEnabled(showEdgeColor);

  d->LightingCheckBox->setChecked(d->DisplayNode->GetLighting());
  d->InterpolationComboBox->setCurrentIndex(d->DisplayNode->GetInterpolation());

  // Material
  d->Property->SetAmbient(d->DisplayNode->GetAmbient());
  d->Property->SetDiffuse(d->DisplayNode->GetDiffuse());
  d->Property->SetSpecular(d->DisplayNode->GetSpecular());
  d->Property->SetSpecularPower(d->DisplayNode->GetPower());

  // Scalars

  if (d->ScalarsVisibilityCheckBox->isChecked() != (bool)d->DisplayNode->GetScalarVisibility())
    {
    wasBlocking = d->ScalarsVisibilityCheckBox->blockSignals(true);
    d->ScalarsVisibilityCheckBox->setChecked(d->DisplayNode->GetScalarVisibility());
    d->ScalarsVisibilityCheckBox->blockSignals(wasBlocking);
    }

  double dataRange[2] = { 0.0, 0.0 };
  if (d->ModelDisplayNode)
    {
    vtkDataArray* dataArray = d->ModelDisplayNode->GetActiveScalarArray();
    if (dataArray)
      {
      dataArray->GetRange(dataRange);
      }
    }

  // Update scalar values, range, decimals and single step
  double precision = 1.0;
  double dataMin = 0.0;
  double dataMax = 0.0;
  int decimals = 0;
  bool resetSliderRange = false;
  if (dataRange[0] < dataRange[1])
    {
    if (d->DataRangeUsedForAutoDisplayRange[0] != dataRange[0]
      || d->DataRangeUsedForAutoDisplayRange[1] != dataRange[1])
      {
      d->DataRangeUsedForAutoDisplayRange[0] = dataRange[0];
      d->DataRangeUsedForAutoDisplayRange[1] = dataRange[1];
      resetSliderRange = true;
      }
    // Begin with a precision of 1% of the range
    precision = dataRange[1]/100.0 - dataRange[0]/100.0;
    // Extend min/max by 20% to give some room to work with
    dataMin = (floor(dataRange[0]/precision) - 20 ) * precision;
    dataMax = (ceil(dataRange[1]/precision) + 20 ) * precision;
    // Use closest power of ten value as a step value
    precision = ctk::closestPowerOfTen(precision);
    // Find significant decimals to show
    double stepDecimals = ctk::significantDecimals(precision);
    double minDecimals = ctk::significantDecimals(dataRange[0]);
    double maxDecimals = ctk::significantDecimals(dataRange[1]);
    decimals = std::max(stepDecimals, std::max(minDecimals, maxDecimals));
    }

  double* displayRange = d->DisplayNode->GetScalarRange();

  wasBlocking = d->DisplayedScalarRangeWidget->blockSignals(true);
  if (resetSliderRange)
    {
    d->DisplayedScalarRangeWidget->setRange(std::min(dataMin, displayRange[0]), std::max(dataMax, displayRange[1]));
    }
  else
    {
    double currentRange[2] = { 0.0 };
    d->DisplayedScalarRangeWidget->range(currentRange);
    d->DisplayedScalarRangeWidget->setRange(std::min(currentRange[0], displayRange[0]),
      std::max(currentRange[1], displayRange[1]));
    }
  d->DisplayedScalarRangeWidget->setValues(displayRange[0], displayRange[1]);
  d->DisplayedScalarRangeWidget->setDecimals(decimals);
  d->DisplayedScalarRangeWidget->setSingleStep(precision);
  d->DisplayedScalarRangeWidget->setEnabled(d->DisplayNode->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseManualScalarRange);
  d->DisplayedScalarRangeWidget->blockSignals(wasBlocking);

  double thresholdRange[2] = { 0.0, 0.0 };
  if (d->ModelDisplayNode)
    {
    d->ModelDisplayNode->GetThresholdRange(thresholdRange);
    }

  wasBlocking = d->ThresholdRangeWidget->blockSignals(true);
  d->ThresholdRangeWidget->setEnabled(d->ModelDisplayNode && d->ModelDisplayNode->GetThresholdEnabled());
  d->ThresholdRangeWidget->setRange(dataRange[0] - precision, dataRange[1] + precision);
  if (thresholdRange[0] <= thresholdRange[1])
    {
    // there is a valid threshold range
    // If current threshold values do not fit in the current data range
    // then we move the slider handles to make them fit,
    // but values in the display node will not be changed until the user moves the handles.
    d->ThresholdRangeWidget->setValues(std::max(dataRange[0] - precision, thresholdRange[0]),
      std::min(dataRange[1] + precision, thresholdRange[1]));
    }
  else
    {
    // no valid threshold range is set move handles to the center
    d->ThresholdRangeWidget->setValues(dataRange[0], dataRange[1]);
    }
  d->ThresholdRangeWidget->setDecimals(decimals);
  d->ThresholdRangeWidget->setSingleStep(precision);
  d->ThresholdRangeWidget->blockSignals(wasBlocking);

  wasBlocking = d->ThresholdCheckBox->blockSignals(true);
  d->ThresholdCheckBox->setEnabled(d->ModelDisplayNode);
  d->ThresholdCheckBox->setChecked(d->ModelDisplayNode && d->ModelDisplayNode->GetThresholdEnabled());
  d->ThresholdCheckBox->blockSignals(wasBlocking);

  ControlMode controlMode = qMRMLModelDisplayNodeWidget::Manual;
  switch (d->DisplayNode->GetScalarRangeFlag())
    {
    case vtkMRMLDisplayNode::UseDataScalarRange: controlMode = qMRMLModelDisplayNodeWidget::Data; break;
    case vtkMRMLDisplayNode::UseColorNodeScalarRange: controlMode = qMRMLModelDisplayNodeWidget::LUT; break;
    case vtkMRMLDisplayNode::UseDataTypeScalarRange: controlMode = qMRMLModelDisplayNodeWidget::DataType; break;
    case vtkMRMLDisplayNode::UseManualScalarRange: controlMode = qMRMLModelDisplayNodeWidget::Manual; break;
    case vtkMRMLDisplayNode::UseDirectMapping: controlMode = qMRMLModelDisplayNodeWidget::DirectMapping; break;
    }

  wasBlocking = d->DisplayedScalarRangeModeComboBox->blockSignals(true);
  d->DisplayedScalarRangeModeComboBox->setCurrentIndex(controlMode);
  d->DisplayedScalarRangeModeComboBox->blockSignals(wasBlocking);

  wasBlocking = d->ActiveScalarComboBox->blockSignals(true);
  d->ActiveScalarComboBox->setEnabled(d->ModelDisplayNode);
  if (d->ModelDisplayNode)
    {
    d->ActiveScalarComboBox->setDataSet(d->ModelDisplayNode->GetInputMesh());
    if (d->ActiveScalarComboBox->currentArrayName() != d->ModelDisplayNode->GetActiveScalarName())
      {
      d->ActiveScalarComboBox->setCurrentArray(d->ModelDisplayNode->GetActiveScalarName());
      // Array location would need to be set in d->ActiveScalarComboBox if
      // same scalar name is used in multiple locations.
      }
    }
  d->ActiveScalarComboBox->blockSignals(wasBlocking);

  wasBlocking = d->ScalarsColorNodeComboBox->blockSignals(true);
  if (d->ScalarsColorNodeComboBox->mrmlScene() != this->mrmlScene())
    {
    d->ScalarsColorNodeComboBox->setMRMLScene(this->mrmlScene());
    }
  if ( d->ModelDisplayNode
    && d->ScalarsColorNodeComboBox->currentNodeID() != d->ModelDisplayNode->GetColorNodeID() )
    {
    d->ScalarsColorNodeComboBox->setCurrentNodeID(d->ModelDisplayNode->GetColorNodeID());
    }
  d->ScalarsColorNodeComboBox->setEnabled( d->ModelDisplayNode &&
    d->ModelDisplayNode->GetScalarRangeFlag() != vtkMRMLDisplayNode::UseDirectMapping );
  d->ScalarsColorNodeComboBox->blockSignals(wasBlocking);
  emit displayNodeChanged();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  d->DisplayNode->SetVisibility(visible);
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::visibility()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->VisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setClipping(bool clip)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  d->DisplayNode->SetClipping(clip);
  emit clippingToggled(clip);
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::clipping()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ClippingCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceIntersectionVisible(bool visible)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  d->DisplayNode->SetVisibility2D(visible);
}

//------------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::sliceIntersectionVisible()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->SliceIntersectionVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceIntersectionThickness(int thickness)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  d->DisplayNode->SetSliceIntersectionThickness(thickness);
}

//------------------------------------------------------------------------------
int qMRMLModelDisplayNodeWidget::sliceIntersectionThickness()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->SliceIntersectionThicknessSpinBox->value();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceIntersectionOpacity(double opacity)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  d->DisplayNode->SetSliceIntersectionOpacity(opacity);
}

//------------------------------------------------------------------------------
double qMRMLModelDisplayNodeWidget::sliceIntersectionOpacity()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->SliceIntersectionOpacitySlider->value();
}

//------------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::updateNodeFromProperty()
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  int wasModifying = d->DisplayNode->StartModify();
  // Lighting
  d->DisplayNode->SetLighting(d->Property->GetLighting());
  d->DisplayNode->SetInterpolation(d->Property->GetInterpolation());
  d->DisplayNode->SetShading(d->Property->GetShading());
  // Material
  d->DisplayNode->SetAmbient(d->Property->GetAmbient());
  d->DisplayNode->SetDiffuse(d->Property->GetDiffuse());
  d->DisplayNode->SetSpecular(d->Property->GetSpecular());
  d->DisplayNode->SetPower(d->Property->GetSpecularPower());
  d->DisplayNode->EndModify(wasModifying);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setRepresentation(int newRepresentation)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  switch (newRepresentation)
    {
    case 0: // points
      d->DisplayNode->SetRepresentation(REPRESENTATION_POINTS);
      break;
    case 1: // wireframe
      d->DisplayNode->SetRepresentation(REPRESENTATION_WIREFRAME);
      break;
    case 2: // surface
    case 3: // surface with edges
      {
      int wasModified = d->DisplayNode->StartModify();
      d->DisplayNode->SetRepresentation(REPRESENTATION_SURFACE);
      d->DisplayNode->SetEdgeVisibility(newRepresentation == 3);
      d->DisplayNode->EndModify(wasModified);
      break;
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setSliceDisplayMode(int newMode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->ModelDisplayNode.GetPointer())
  {
    return;
  }
  int wasModified = d->ModelDisplayNode->StartModify();
  // Select a color node if none is selected yet
  if (d->ModelDisplayNode->GetSliceDisplayMode()
    != vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection
    && newMode == vtkMRMLModelDisplayNode::SliceDisplayDistanceEncodedProjection
    && d->ModelDisplayNode->GetDistanceEncodedProjectionColorNodeID() == nullptr)
    {
    d->ModelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID("vtkMRMLFreeSurferProceduralColorNodeRedGreen");
    }
  d->ModelDisplayNode->SetSliceDisplayMode(newMode);
  d->ModelDisplayNode->EndModify(wasModified);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setPointSize(double newPointSize)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
    {
    return;
    }
  d->DisplayNode->SetPointSize(newPointSize);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setLineWidth(double newLineWidth)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  d->DisplayNode->SetLineWidth(newLineWidth);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setShowFaces(int newShowFaces)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  int wasModified = d->DisplayNode->StartModify();
  switch (newShowFaces)
    {
    case 0: // show all
      d->DisplayNode->SetFrontfaceCulling(false);
      d->DisplayNode->SetBackfaceCulling(false);
      break;
    case 1: // show front only
      d->DisplayNode->SetFrontfaceCulling(false);
      d->DisplayNode->SetBackfaceCulling(true);
      break;
    case 2: // show back only
      d->DisplayNode->SetFrontfaceCulling(true);
      d->DisplayNode->SetBackfaceCulling(false);
      break;
    }
  d->DisplayNode->EndModify(wasModified);

}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setColor(const QColor& newColor)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  d->DisplayNode->SetColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setOpacity(double newOpacity)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  d->DisplayNode->SetOpacity(newOpacity);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setEdgeVisibility(bool newEdgeVisibility)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  d->Property->SetEdgeVisibility(newEdgeVisibility);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setEdgeColor(const QColor& newColor)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  d->DisplayNode->SetEdgeColor(newColor.redF(), newColor.greenF(), newColor.blueF());
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setLighting(bool newLighting)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  d->Property->SetLighting(newLighting);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setInterpolation(int newInterpolation)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->DisplayNode.GetPointer())
  {
    return;
  }
  d->Property->SetInterpolation(newInterpolation);
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setDistanceToColorNode(vtkMRMLNode* colorNode)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  if (!d->ModelDisplayNode.GetPointer())
    {
    return;
    }
  d->ModelDisplayNode->SetAndObserveDistanceEncodedProjectionColorNodeID(
    colorNode ? colorNode->GetID() : nullptr);
}

// --------------------------------------------------------------------------
bool qMRMLModelDisplayNodeWidget::clippingConfigurationButtonVisible()const
{
  Q_D(const qMRMLModelDisplayNodeWidget);
  return d->ConfigureClippingPushButton->isVisible();
}

// --------------------------------------------------------------------------
void qMRMLModelDisplayNodeWidget::setClippingConfigurationButtonVisible(bool show)
{
  Q_D(qMRMLModelDisplayNodeWidget);
  d->ConfigureClippingPushButton->setVisible(show);
}
