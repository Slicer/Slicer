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

#include "qMRMLMarkupsCurveSettingsWidget.h"
#include "ui_qMRMLMarkupsCurveSettingsWidget.h"

#include "vtkSlicerDijkstraGraphGeodesicPath.h"

// MRML Markups includes
#include <vtkMRMLMarkupsClosedCurveNode.h>
#include <vtkMRMLMarkupsCurveNode.h>
#include <vtkMRMLMarkupsNode.h>

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkNew.h>

// Qt includes
#include <QTimer>

// --------------------------------------------------------------------------
class qMRMLMarkupsCurveSettingsWidget;

// --------------------------------------------------------------------------
class qMRMLMarkupsCurveSettingsWidgetPrivate
  : public Ui_qMRMLMarkupsCurveSettingsWidget
{
public:
  qMRMLMarkupsCurveSettingsWidgetPrivate(qMRMLMarkupsCurveSettingsWidget &widget);

  static const char* getCurveTypeAsHumanReadableString(int curveType);
  static const char* getCostFunctionAsHumanReadableString(int costFunction);

  void setupUi(QWidget* widget);

  virtual void setupUi(qMRMLMarkupsCurveSettingsWidget*);

  QTimer* editScalarFunctionDelay;

protected:
  qMRMLMarkupsCurveSettingsWidget* const q_ptr;

private:
  Q_DECLARE_PUBLIC(qMRMLMarkupsCurveSettingsWidget);
};

// --------------------------------------------------------------------------
qMRMLMarkupsCurveSettingsWidgetPrivate::qMRMLMarkupsCurveSettingsWidgetPrivate(qMRMLMarkupsCurveSettingsWidget& widget)
  : q_ptr(&widget)
{
  this->editScalarFunctionDelay = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidgetPrivate::setupUi(qMRMLMarkupsCurveSettingsWidget* widget)
{
  Q_Q(qMRMLMarkupsCurveSettingsWidget);

  this->Ui_qMRMLMarkupsCurveSettingsWidget::setupUi(widget);

  this->curveTypeComboBox->clear();
  for (int curveType = 0; curveType < vtkCurveGenerator::CURVE_TYPE_LAST; ++curveType)
    {
    this->curveTypeComboBox->addItem(qMRMLMarkupsCurveSettingsWidgetPrivate::getCurveTypeAsHumanReadableString(curveType), curveType);
    }

  this->costFunctionComboBox->clear();
  for (int costFunction = 0; costFunction < vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_LAST; ++costFunction)
    {
    this->costFunctionComboBox->addItem(qMRMLMarkupsCurveSettingsWidgetPrivate::getCostFunctionAsHumanReadableString(costFunction), costFunction);
    }

  this->editScalarFunctionDelay = new QTimer(q);
  this->editScalarFunctionDelay->setInterval(500);
  this->editScalarFunctionDelay->setSingleShot(true);



  QObject::connect(this->editScalarFunctionDelay, SIGNAL(timeout()),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->curveTypeComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->modelNodeSelector, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->costFunctionComboBox, SIGNAL(currentIndexChanged(int)),
                   q, SLOT(onCurveTypeParameterChanged()));
  QObject::connect(this->scalarFunctionLineEdit, SIGNAL(textChanged(QString)),
                   this->editScalarFunctionDelay, SLOT(start()));
  QObject::connect(this->projectCurveMaxSearchRadiusSliderWidget, SIGNAL(valueChanged(double)),
                   q, SLOT(onProjectCurveMaximumSearchRadiusChanged()));
  QObject::connect(this->resampleCurveButton, SIGNAL(clicked()),
                   q, SLOT(onApplyCurveResamplingPushButtonClicked()));

  q_ptr->setEnabled(q_ptr->MarkupsNode != nullptr);
}

//------------------------------------------------------------------------------
const char* qMRMLMarkupsCurveSettingsWidgetPrivate::getCurveTypeAsHumanReadableString(int curveType)
{
  switch (curveType)
    {
    case vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE:
      {
      return "Linear";
      }
    case vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE:
      {
      return "Spline";
      }
    case vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE:
      {
      return "Kochanek spline";
      }
    case vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL:
      {
      return "Polynomial";
      }
    case vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE:
      {
      return "Shortest distance on surface";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown curve type: " << curveType);
      return "Unknown";
      }
    }
}

//------------------------------------------------------------------------------
const char* qMRMLMarkupsCurveSettingsWidgetPrivate::getCostFunctionAsHumanReadableString(int costFunction)
{
  switch (costFunction)
    {
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE:
      {
      return "Distance";
      }
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_ADDITIVE:
      {
      return "Additive";
      }
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_MULTIPLICATIVE:
      {
      return "Multiplicative";
      }
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_INVERSE_SQUARED:
      {
      return "Inverse squared";
      }
    default:
      {
      return "";
      }
    }
}

// --------------------------------------------------------------------------
// qMRMLMarkupsCurveSettingsWidget methods

// --------------------------------------------------------------------------
qMRMLMarkupsCurveSettingsWidget::
qMRMLMarkupsCurveSettingsWidget(QWidget *parent)
  : Superclass(parent), d_ptr(new qMRMLMarkupsCurveSettingsWidgetPrivate(*this))
{
  this->setup();
}

// --------------------------------------------------------------------------
qMRMLMarkupsCurveSettingsWidget::~qMRMLMarkupsCurveSettingsWidget() = default;

// --------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidget::setup()
{
  Q_D(qMRMLMarkupsCurveSettingsWidget);
  d->setupUi(this);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLMarkupsCurveSettingsWidget);

  vtkMRMLMarkupsCurveNode *curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!curveNode)
    {
    return;
    }

  // Update displayed node types.
  // Since updating this list resets the previous node selection,
  // we save and restore previous selection.
  vtkMRMLNode* previousOutputNode = d->resampleCurveOutputNodeSelector->currentNode();
  d->resampleCurveOutputNodeSelector->setNodeTypes(QStringList(QString(curveNode->GetClassName())));
  if (previousOutputNode && previousOutputNode->IsA(curveNode->GetClassName()))
    {
    d->resampleCurveOutputNodeSelector->setCurrentNode(previousOutputNode);
    }
  else
    {
    d->resampleCurveOutputNodeSelector->setCurrentNode(nullptr);
    }

  bool wasBlocked = d->curveTypeComboBox->blockSignals(true);
  d->curveTypeComboBox->setCurrentIndex(d->curveTypeComboBox->findData(curveNode->GetCurveType()));
  d->curveTypeComboBox->blockSignals(wasBlocked);

  vtkMRMLModelNode* modelNode = curveNode->GetSurfaceConstraintNode();
  wasBlocked = d->modelNodeSelector->blockSignals(true);
  d->modelNodeSelector->setCurrentNode(modelNode);
  d->modelNodeSelector->blockSignals(wasBlocked);

  wasBlocked = d->costFunctionComboBox->blockSignals(true);
  int costFunction = curveNode->GetSurfaceCostFunctionType();
  d->costFunctionComboBox->setCurrentIndex(d->costFunctionComboBox->findData(costFunction));
  d->costFunctionComboBox->blockSignals(wasBlocked);

  wasBlocked = d->scalarFunctionLineEdit->blockSignals(true);
  int currentCursorPosition = d->scalarFunctionLineEdit->cursorPosition();
  d->scalarFunctionLineEdit->setText(curveNode->GetSurfaceDistanceWeightingFunction());
  d->scalarFunctionLineEdit->setCursorPosition(currentCursorPosition);
  d->scalarFunctionLineEdit->blockSignals(wasBlocked);

  wasBlocked = d->projectCurveMaxSearchRadiusSliderWidget->blockSignals(true);
  d->projectCurveMaxSearchRadiusSliderWidget->setValue(curveNode->GetSurfaceConstraintMaximumSearchRadiusTolerance() * 100.);
  d->projectCurveMaxSearchRadiusSliderWidget->blockSignals(wasBlocked);

  if (costFunction == vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE)
    {
    d->scalarFunctionLineEdit->setVisible(false);
    }
  else
    {
    d->scalarFunctionLineEdit->setVisible(true);
    }

  QString prefixString;
  QString suffixString;
  switch (costFunction)
    {
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_ADDITIVE:
      prefixString = "distance + ";
      break;
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_MULTIPLICATIVE:
      prefixString = "distance * ";
      break;
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_INVERSE_SQUARED:
      prefixString = "distance / (";
      suffixString = " ^ 2";
      break;
    default:
    case vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE:
      prefixString = "distance";
      break;
    }
  d->scalarFunctionPrefixLabel->setText(prefixString);
  d->scalarFunctionSuffixLabel->setText(suffixString);

  if (curveNode->GetCurveType() == vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE)
    {
    d->surfaceCurveCollapsibleButton->setEnabled(true);
    }
  else
    {
    d->surfaceCurveCollapsibleButton->setEnabled(false);
    }
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidget::onCurveTypeParameterChanged()
{
  Q_D(qMRMLMarkupsCurveSettingsWidget);

  vtkMRMLMarkupsCurveNode *curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!curveNode)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(curveNode);
  curveNode->SetCurveType(d->curveTypeComboBox->currentData().toInt());
  curveNode->SetAndObserveSurfaceConstraintNode(vtkMRMLModelNode::SafeDownCast(d->modelNodeSelector->currentNode()));
  std::string functionString = d->scalarFunctionLineEdit->text().toStdString();
  curveNode->SetSurfaceCostFunctionType(d->costFunctionComboBox->currentData().toInt());
  curveNode->SetSurfaceDistanceWeightingFunction(functionString.c_str());
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidget::onApplyCurveResamplingPushButtonClicked()
{
  Q_D(qMRMLMarkupsCurveSettingsWidget);

  double resampleNumberOfPoints = d->resampleCurveNumerOfOutputPointsSpinBox->value();
  if (resampleNumberOfPoints <= 1)
    {
    return;
    }

  vtkMRMLMarkupsCurveNode* inputNode = vtkMRMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!inputNode)
    {
    return;
    }
  bool isClosed = inputNode->GetCurveClosed();
  vtkMRMLMarkupsCurveNode* outputNode = vtkMRMLMarkupsCurveNode::SafeDownCast(d->resampleCurveOutputNodeSelector->currentNode());
  if (!outputNode)
    {
    outputNode = inputNode;
    }
  if(outputNode != inputNode)
    {
    MRMLNodeModifyBlocker blocker(outputNode);
    vtkNew<vtkPoints> originalControlPoints;
    inputNode->GetControlPointPositionsWorld(originalControlPoints);
    outputNode->SetControlPointPositionsWorld(originalControlPoints);
    vtkNew<vtkStringArray> originalLabels;
    inputNode->GetControlPointLabels(originalLabels);
    outputNode->SetControlPointLabels(originalLabels, originalControlPoints);
    outputNode->SetCurveType(inputNode->GetCurveType());
    outputNode->SetNumberOfPointsPerInterpolatingSegment(inputNode->GetNumberOfPointsPerInterpolatingSegment());
    outputNode->SetAndObserveSurfaceConstraintNode(inputNode->GetSurfaceConstraintNode());
    outputNode->SetSurfaceCostFunctionType(inputNode->GetSurfaceCostFunctionType());
    outputNode->SetSurfaceDistanceWeightingFunction(inputNode->GetSurfaceDistanceWeightingFunction());
    }
  unsigned int numberOfSegments = (isClosed ? resampleNumberOfPoints : resampleNumberOfPoints - 1);
  double sampleDist = outputNode->GetCurveLengthWorld() / numberOfSegments;
  outputNode->ResampleCurveWorld(sampleDist);
}

//-----------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidget::onProjectCurveMaximumSearchRadiusChanged()
{
  Q_D(qMRMLMarkupsCurveSettingsWidget);
  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(this->MarkupsNode);
  if (!curveNode)
  {
    return;
  }
  const double maximumSearchRadius = 0.01 * d->projectCurveMaxSearchRadiusSliderWidget->value();
  curveNode->SetSurfaceConstraintMaximumSearchRadiusTolerance(maximumSearchRadius);
}

//-----------------------------------------------------------------------------
bool qMRMLMarkupsCurveSettingsWidget::canManageMRMLMarkupsNode(vtkMRMLMarkupsNode *markupsNode) const
{
  Q_D(const qMRMLMarkupsCurveSettingsWidget);

  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(markupsNode);
  vtkMRMLMarkupsClosedCurveNode* closedCurveNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(markupsNode);
  if (!curveNode && !closedCurveNode)
    {
    return false;
    }

  return true;
}

// --------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidget::setMRMLMarkupsNode(vtkMRMLMarkupsNode* markupsNode)
{
  Q_D(qMRMLMarkupsCurveSettingsWidget);

  this->MarkupsNode = vtkMRMLMarkupsCurveNode::SafeDownCast(markupsNode);
  this->setEnabled(this->MarkupsNode!= nullptr);
}

// --------------------------------------------------------------------------
void qMRMLMarkupsCurveSettingsWidget::setMRMLScene(vtkMRMLScene *mrmlScene)
{
  Q_D(qMRMLMarkupsCurveSettingsWidget);

  Superclass::setMRMLScene(mrmlScene);
  d->modelNodeSelector->setMRMLScene(mrmlScene);
  d->resampleCurveOutputNodeSelector->setMRMLScene(mrmlScene);
}
