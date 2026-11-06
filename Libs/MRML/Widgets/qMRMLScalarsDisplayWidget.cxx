/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2020 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// Qt includes
#include <QStandardItem>

// CTK includes
#include <ctkVTKDataSetModel.h>
#include "ctkUtils.h"

// qMRML includes
#include "qMRMLScalarsDisplayWidget.h"
#include "ui_qMRMLScalarsDisplayWidget.h"

// MRML include
#include <vtkMRMLScene.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLVectorFieldDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>

// VTK includes
#include <vtkPointSet.h>

// STD includes
#include <algorithm>

//-----------------------------------------------------------------------------
namespace
{
/// Marks the "(Default)" item that stands for the host's array. Must not clash with the data set model's own
/// roles (ctkVTK::PointerRole is Qt::UserRole + 1).
const int DefaultArrayItemRole = Qt::UserRole + 100;
} // namespace

//-----------------------------------------------------------------------------
class qMRMLScalarsDisplayWidgetPrivate : public Ui_qMRMLScalarsDisplayWidget
{
  Q_DECLARE_PUBLIC(qMRMLScalarsDisplayWidget);

protected:
  qMRMLScalarsDisplayWidget* const q_ptr;

public:
  qMRMLScalarsDisplayWidgetPrivate(qMRMLScalarsDisplayWidget& object);
  void init();

  /// The array the host falls back to.
  QString DefaultScalarArrayName;

  /// Put the "(Default)" entry at the top of the list of arrays, or take it out. The list
  /// is rebuilt from the data set whenever that changes, so this runs after every update.
  void updateDefaultArrayItem();
  /// Whether the "(Default)" entry heads the list at the moment. Asked of the list each time:
  /// the list is rebuilt, without the entry, whenever the data set is modified.
  bool isDefaultArrayItemPresent() const;

  /// Whether the threshold controls are shown at all. A host with its own thresholding turns them off,
  /// and they stay off whatever the display node.
  bool ThresholdVisible{ true };

  /// Thresholding helpers that work with any display node type that supports it
  /// (model and glyph display nodes).
  static bool isThresholdSupported(vtkMRMLDisplayNode* displayNode);
  static bool thresholdEnabled(vtkMRMLDisplayNode* displayNode);
  static void setThresholdEnabled(vtkMRMLDisplayNode* displayNode, bool enabled);
  static void thresholdRange(vtkMRMLDisplayNode* displayNode, double range[2]);
  static void setThresholdRange(vtkMRMLDisplayNode* displayNode, double min, double max);

public:
  QList<vtkWeakPointer<vtkMRMLDisplayNode>> CurrentDisplayNodes;

  // Store what data range was used to automatically slider range of display range,
  // to prevent resetting slider range when user moves the slider.
  double DataRangeUsedForAutoDisplayRange[2];
};

//------------------------------------------------------------------------------
qMRMLScalarsDisplayWidgetPrivate::qMRMLScalarsDisplayWidgetPrivate(qMRMLScalarsDisplayWidget& object)
  : q_ptr(&object)
{
  this->DataRangeUsedForAutoDisplayRange[0] = 0.0;
  this->DataRangeUsedForAutoDisplayRange[1] = 0.0;
}

// --------------------------------------------------------------------------
void qMRMLScalarsDisplayWidgetPrivate::init()
{
  Q_Q(qMRMLScalarsDisplayWidget);

  this->setupUi(q);

  // Scalar
  QObject::connect(this->ScalarsVisibilityCheckBox, SIGNAL(toggled(bool)), q, SLOT(setScalarsVisibility(bool)));
  QObject::connect(this->ActiveScalarComboBox, SIGNAL(activated(int)), q, SLOT(onCurrentArrayActivated()));
  QObject::connect(this->ActiveScalarComponentSpinBox, SIGNAL(valueChanged(int)), q, SLOT(setActiveScalarComponent(int)));
  QObject::connect(this->ScalarsColorNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(setScalarsColorNode(vtkMRMLNode*)));
  // scalar range
  QObject::connect(this->DisplayedScalarRangeModeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setScalarRangeMode(int)));
  QObject::connect(this->DisplayedScalarRangeWidget, SIGNAL(valuesChanged(double, double)), q, SLOT(setScalarsDisplayRange(double, double)));

  // Thresholding
  this->ThresholdCheckBox->setChecked(false);
  this->ThresholdRangeWidget->setEnabled(false);
  QObject::connect(this->ThresholdCheckBox, SIGNAL(toggled(bool)), q, SLOT(setTresholdEnabled(bool)));
  QObject::connect(this->ThresholdRangeWidget, SIGNAL(valuesChanged(double, double)), q, SLOT(setThresholdRange(double, double)));

  // Set default range mode
  q->setScalarRangeMode(vtkMRMLDisplayNode::UseDataScalarRange);
}

//------------------------------------------------------------------------------
// Model and vector field display nodes have the same threshold methods but no common base class declaring
// them, so these helpers dispatch on the node type.

//------------------------------------------------------------------------------
bool qMRMLScalarsDisplayWidgetPrivate::isThresholdSupported(vtkMRMLDisplayNode* displayNode)
{
  return vtkMRMLModelDisplayNode::SafeDownCast(displayNode) != nullptr //
         || vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode) != nullptr;
}

//------------------------------------------------------------------------------
bool qMRMLScalarsDisplayWidgetPrivate::thresholdEnabled(vtkMRMLDisplayNode* displayNode)
{
  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
  if (modelDisplayNode)
  {
    return modelDisplayNode->GetThresholdEnabled();
  }
  vtkMRMLVectorFieldDisplayNode* glyphDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode);
  if (glyphDisplayNode)
  {
    return glyphDisplayNode->GetThresholdEnabled();
  }
  return false;
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidgetPrivate::setThresholdEnabled(vtkMRMLDisplayNode* displayNode, bool enabled)
{
  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
  if (modelDisplayNode)
  {
    modelDisplayNode->SetThresholdEnabled(enabled);
    return;
  }
  vtkMRMLVectorFieldDisplayNode* glyphDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode);
  if (glyphDisplayNode)
  {
    glyphDisplayNode->SetThresholdEnabled(enabled);
  }
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidgetPrivate::thresholdRange(vtkMRMLDisplayNode* displayNode, double range[2])
{
  range[0] = 0.0;
  range[1] = -1.0; // invalid range
  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
  if (modelDisplayNode)
  {
    modelDisplayNode->GetThresholdRange(range);
    return;
  }
  vtkMRMLVectorFieldDisplayNode* glyphDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode);
  if (glyphDisplayNode)
  {
    glyphDisplayNode->GetThresholdRange(range);
  }
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidgetPrivate::setThresholdRange(vtkMRMLDisplayNode* displayNode, double min, double max)
{
  double oldRange[2] = { 0.0, -1.0 };
  qMRMLScalarsDisplayWidgetPrivate::thresholdRange(displayNode, oldRange);
  if (oldRange[0] == min && oldRange[1] == max)
  {
    return;
  }
  vtkMRMLModelDisplayNode* modelDisplayNode = vtkMRMLModelDisplayNode::SafeDownCast(displayNode);
  if (modelDisplayNode)
  {
    modelDisplayNode->SetThresholdRange(min, max);
    return;
  }
  vtkMRMLVectorFieldDisplayNode* glyphDisplayNode = vtkMRMLVectorFieldDisplayNode::SafeDownCast(displayNode);
  if (glyphDisplayNode)
  {
    glyphDisplayNode->SetThresholdRange(min, max);
  }
}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------
qMRMLScalarsDisplayWidget::qMRMLScalarsDisplayWidget(QWidget* parentWidget)
  : qMRMLWidget(parentWidget)
  , d_ptr(new qMRMLScalarsDisplayWidgetPrivate(*this))
{
  Q_D(qMRMLScalarsDisplayWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLScalarsDisplayWidget::~qMRMLScalarsDisplayWidget() = default;

//------------------------------------------------------------------------------
vtkMRMLDisplayNode* qMRMLScalarsDisplayWidget::mrmlDisplayNode() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  if (d->CurrentDisplayNodes.size() > 0)
  {
    return d->CurrentDisplayNodes[0];
  }

  return nullptr;
}

//------------------------------------------------------------------------------
QList<vtkMRMLDisplayNode*> qMRMLScalarsDisplayWidget::mrmlDisplayNodes() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  QList<vtkMRMLDisplayNode*> displayNodes; // this list will only contain valid (non-null) display node pointers
  for (vtkMRMLDisplayNode* displayNode : d->CurrentDisplayNodes)
  {
    if (displayNode)
    {
      displayNodes << displayNode;
    }
  }
  return displayNodes;
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setMRMLDisplayNodes(QList<vtkMRMLDisplayNode*> displayNodes)
{
  Q_D(qMRMLScalarsDisplayWidget);

  displayNodes.removeAll(nullptr);

  // Return if the list of display nodes did not change to avoid unnecessary updates.
  if (displayNodes.size() == d->CurrentDisplayNodes.size())
  {
    bool same = true;
    for (int i = 0; i < displayNodes.size(); ++i)
    {
      if (displayNodes[i] != d->CurrentDisplayNodes[i])
      {
        same = false;
        break;
      }
    }
    if (same)
    {
      return;
    }
  }

  // Only the first display node is observed
  qvtkReconnect((d->CurrentDisplayNodes.size() > 0 ? d->CurrentDisplayNodes[0] : nullptr),
                (displayNodes.size() > 0 ? displayNodes[0] : nullptr),
                vtkCommand::ModifiedEvent,
                this,
                SLOT(updateWidgetFromMRML()));

  d->CurrentDisplayNodes.clear();
  for (vtkMRMLDisplayNode* displayNode : displayNodes)
  {
    d->CurrentDisplayNodes << displayNode;
  }

  if (d->CurrentDisplayNodes.size() > 0)
  {
    d->CurrentDisplayNodes[0]->UpdateScalarRange();
  }

  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setMRMLDisplayNode(vtkMRMLNode* node)
{
  this->setMRMLDisplayNode(vtkMRMLDisplayNode::SafeDownCast(node));
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setMRMLDisplayNode(vtkMRMLDisplayNode* displayNode)
{
  Q_D(qMRMLScalarsDisplayWidget);
  QList<vtkMRMLDisplayNode*> displayNodes;
  if (displayNode)
  {
    displayNodes << displayNode;
  }
  this->setMRMLDisplayNodes(displayNodes);
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setScalarsVisibility(bool visible)
{
  Q_D(qMRMLScalarsDisplayWidget);

  // Iterate through a copy of d->CurrentDisplayNodes, as the list may change due to callbacks
  QList<vtkWeakPointer<vtkMRMLDisplayNode>> displayNodes = d->CurrentDisplayNodes;
  for (vtkMRMLDisplayNode* const displayNode : displayNodes)
  {
    if (!displayNode)
    {
      continue;
    }
    displayNode->SetScalarVisibility(visible);
    displayNode->UpdateAssignedAttribute();
  }
}

//------------------------------------------------------------------------------
bool qMRMLScalarsDisplayWidget::scalarsVisibility() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  return d->ScalarsVisibilityCheckBox->isChecked();
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::onCurrentArrayActivated()
{
  Q_D(qMRMLScalarsDisplayWidget);
  if (d->isDefaultArrayItemPresent() && d->ActiveScalarComboBox->currentIndex() == 0)
  {
    // "(Default)" entry: select the actual array first, because the attribute location is read from the
    // selected row and the default row holds no array.
    bool wasBlocking = d->ActiveScalarComboBox->blockSignals(true);
    d->ActiveScalarComboBox->setCurrentArray(d->DefaultScalarArrayName);
    d->ActiveScalarComboBox->blockSignals(wasBlocking);
    this->setActiveScalarName(d->DefaultScalarArrayName);
    return;
  }
  this->setActiveScalarName(d->ActiveScalarComboBox->currentArrayName());
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setActiveScalarName(const QString& arrayName)
{
  Q_D(qMRMLScalarsDisplayWidget);

  // Iterate through a copy of d->CurrentDisplayNodes, as the list may change due to callbacks
  QList<vtkWeakPointer<vtkMRMLDisplayNode>> displayNodes = d->CurrentDisplayNodes;
  for (vtkMRMLDisplayNode* const displayNode : displayNodes)
  {
    if (!displayNode)
    {
      continue;
    }
    int wasModified = displayNode->StartModify();

    displayNode->SetActiveScalar(arrayName.toUtf8(), d->ActiveScalarComboBox->currentArrayLocation());

    // if there's no color node set for a non empty array name, use a default
    if (!arrayName.isEmpty() && displayNode->GetColorNodeID() == nullptr)
    {
      const char* colorNodeID = "vtkMRMLColorTableNodeFileViridis.txt";
      displayNode->SetAndObserveColorNodeID(colorNodeID);
    }
    displayNode->EndModify(wasModified);
  }
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setActiveScalarComponent(int component)
{
  Q_D(qMRMLScalarsDisplayWidget);
  // Iterate through a copy of d->CurrentDisplayNodes, as the list may change due to callbacks
  QList<vtkWeakPointer<vtkMRMLDisplayNode>> displayNodes = d->CurrentDisplayNodes;
  for (vtkMRMLDisplayNode* const displayNode : displayNodes)
  {
    if (displayNode)
    {
      displayNode->SetActiveScalarComponent(component);
    }
  }
}

//------------------------------------------------------------------------------
int qMRMLScalarsDisplayWidget::activeScalarComponent() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  return d->ActiveScalarComponentSpinBox->value();
}

//------------------------------------------------------------------------------
QString qMRMLScalarsDisplayWidget::activeScalarName() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  // TODO: use currentArrayName()
  vtkAbstractArray* array = d->ActiveScalarComboBox->currentArray();
  return array ? array->GetName() : "";
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setScalarsColorNode(vtkMRMLNode* colorNode)
{
  this->setScalarsColorNode(vtkMRMLColorNode::SafeDownCast(colorNode));
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setScalarsColorNode(vtkMRMLColorNode* colorNode)
{
  Q_D(qMRMLScalarsDisplayWidget);

  // Iterate through a copy of d->CurrentDisplayNodes, as the list may change due to callbacks
  QList<vtkWeakPointer<vtkMRMLDisplayNode>> displayNodes = d->CurrentDisplayNodes;
  for (vtkMRMLDisplayNode* const displayNode : displayNodes)
  {
    if (displayNode)
    {
      displayNode->SetAndObserveColorNodeID(colorNode ? colorNode->GetID() : nullptr);
    }
  }
}

//------------------------------------------------------------------------------
vtkMRMLColorNode* qMRMLScalarsDisplayWidget::scalarsColorNode() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  return vtkMRMLColorNode::SafeDownCast(d->ScalarsColorNodeComboBox->currentNode());
}

// --------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setScalarRangeMode(vtkMRMLDisplayNode::ScalarRangeFlagType mode)
{
  Q_D(qMRMLScalarsDisplayWidget);

  bool modified = false;
  // Iterate through a copy of d->CurrentDisplayNodes, as the list may change due to callbacks
  QList<vtkWeakPointer<vtkMRMLDisplayNode>> displayNodes = d->CurrentDisplayNodes;
  for (vtkMRMLDisplayNode* const displayNode : displayNodes)
  {
    if (!displayNode)
    {
      continue;
    }
    int currentScalarRangeMode = displayNode->GetScalarRangeFlag();
    if (currentScalarRangeMode != mode)
    {
      displayNode->SetScalarRangeFlag(mode);
      modified = true;
    }
  }
  if (modified)
  {
    emit this->scalarRangeModeValueChanged(mode);
  }
}

// --------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setScalarRangeMode(int scalarRangeMode)
{
  this->setScalarRangeMode((vtkMRMLDisplayNode::ScalarRangeFlagType)scalarRangeMode);
}

// --------------------------------------------------------------------------
vtkMRMLDisplayNode::ScalarRangeFlagType qMRMLScalarsDisplayWidget::scalarRangeMode() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  return (vtkMRMLDisplayNode::ScalarRangeFlagType)d->DisplayedScalarRangeModeComboBox->currentIndex();
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setScalarsDisplayRange(double min, double max)
{
  Q_D(qMRMLScalarsDisplayWidget);

  // Iterate through a copy of d->CurrentDisplayNodes, as the list may change due to callbacks
  QList<vtkWeakPointer<vtkMRMLDisplayNode>> displayNodes = d->CurrentDisplayNodes;
  for (vtkMRMLDisplayNode* const displayNode : displayNodes)
  {
    if (!displayNode)
    {
      continue;
    }
    double* range = displayNode->GetScalarRange();
    if (range[0] == min && range[1] == max)
    {
      continue;
    }
    displayNode->SetScalarRange(min, max);
  }
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setTresholdEnabled(bool b)
{
  Q_D(qMRMLScalarsDisplayWidget);

  for (vtkMRMLDisplayNode* const displayNode : d->CurrentDisplayNodes)
  {
    qMRMLScalarsDisplayWidgetPrivate::setThresholdEnabled(displayNode, b);
  }
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidgetPrivate::updateDefaultArrayItem()
{
  // Build the row completely before handing it to the model (as the model does for "no array"): the model
  // reads an array from each row and would look for one in a row that is still being populated.
  ctkVTKDataSetModel* dataSetModel = qobject_cast<ctkVTKDataSetModel*>(this->ActiveScalarComboBox->model());
  if (!dataSetModel)
  {
    return;
  }
  bool wanted = !this->DefaultScalarArrayName.isEmpty();
  bool present = this->isDefaultArrayItemPresent();
  if (wanted && !present)
  {
    QStandardItem* defaultItem = new QStandardItem();
    defaultItem->setData(QVariant::fromValue(qlonglong(0)), ctkVTK::PointerRole);
    defaultItem->setData(dataSetModel->nullItemLocation(), ctkVTK::LocationRole);
    defaultItem->setData(true, DefaultArrayItemRole);
    defaultItem->setText(qMRMLScalarsDisplayWidget::tr("(Default)"));
    dataSetModel->insertRow(0, defaultItem);
  }
  else if (!wanted && present)
  {
    dataSetModel->removeRow(0);
  }
}

//------------------------------------------------------------------------------
bool qMRMLScalarsDisplayWidgetPrivate::isDefaultArrayItemPresent() const
{
  ctkVTKDataSetModel* dataSetModel = qobject_cast<ctkVTKDataSetModel*>(this->ActiveScalarComboBox->model());
  QStandardItem* firstItem = (dataSetModel ? dataSetModel->item(0) : nullptr);
  return (firstItem && firstItem->data(DefaultArrayItemRole).toBool());
}

//------------------------------------------------------------------------------
QString qMRMLScalarsDisplayWidget::defaultScalarArrayName() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  return d->DefaultScalarArrayName;
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setDefaultScalarArrayName(const QString& arrayName)
{
  Q_D(qMRMLScalarsDisplayWidget);
  if (d->DefaultScalarArrayName == arrayName)
  {
    return;
  }
  d->DefaultScalarArrayName = arrayName;
  this->updateWidgetFromMRML();
}

//------------------------------------------------------------------------------
bool qMRMLScalarsDisplayWidget::isThresholdVisible() const
{
  Q_D(const qMRMLScalarsDisplayWidget);
  return d->ThresholdVisible;
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setThresholdVisible(bool visible)
{
  Q_D(qMRMLScalarsDisplayWidget);
  d->ThresholdVisible = visible;
  d->ThresholdLabel->setVisible(visible);
  d->ThresholdCheckBox->setVisible(visible);
  d->ThresholdRangeWidget->setVisible(visible);
}

//------------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setThresholdRange(double min, double max)
{
  Q_D(qMRMLScalarsDisplayWidget);

  for (vtkMRMLDisplayNode* const displayNode : d->CurrentDisplayNodes)
  {
    qMRMLScalarsDisplayWidgetPrivate::setThresholdRange(displayNode, min, max);
  }
}

// --------------------------------------------------------------------------
double qMRMLScalarsDisplayWidget::minimumValue() const
{
  Q_D(const qMRMLScalarsDisplayWidget);

  return d->DisplayedScalarRangeWidget->minimumValue();
}

// --------------------------------------------------------------------------
double qMRMLScalarsDisplayWidget::maximumValue() const
{
  Q_D(const qMRMLScalarsDisplayWidget);

  return d->DisplayedScalarRangeWidget->maximumValue();
}

// --------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setMinimumValue(double min)
{
  Q_D(const qMRMLScalarsDisplayWidget);

  if (d->CurrentDisplayNodes.size() > 1)
  {
    qWarning() << Q_FUNC_INFO << ": Multi-selection not supported for this property";
  }

  d->DisplayedScalarRangeWidget->setMinimumValue(min);
}

// --------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::setMaximumValue(double max)
{
  Q_D(const qMRMLScalarsDisplayWidget);

  if (d->CurrentDisplayNodes.size() > 1)
  {
    qWarning() << Q_FUNC_INFO << ": Multi-selection not supported for this property";
  }

  d->DisplayedScalarRangeWidget->setMaximumValue(max);
}

// --------------------------------------------------------------------------
void qMRMLScalarsDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLScalarsDisplayWidget);

  vtkMRMLDisplayNode* firstDisplayNode = (d->CurrentDisplayNodes.size() > 0 ? d->CurrentDisplayNodes[0] : nullptr);
  // The Threshold section is only available for display nodes that support thresholding
  bool thresholdSupported = (qMRMLScalarsDisplayWidgetPrivate::isThresholdSupported(firstDisplayNode) && d->ThresholdVisible);
  d->ThresholdLabel->setVisible(thresholdSupported);
  d->ThresholdCheckBox->setVisible(thresholdSupported);
  d->ThresholdRangeWidget->setVisible(thresholdSupported);

  this->setEnabled(firstDisplayNode != nullptr);
  if (!firstDisplayNode)
  {
    emit displayNodeChanged();
    return;
  }

  bool wasBlocking = false;

  if (d->ScalarsVisibilityCheckBox->isChecked() != (bool)firstDisplayNode->GetScalarVisibility())
  {
    wasBlocking = d->ScalarsVisibilityCheckBox->blockSignals(true);
    d->ScalarsVisibilityCheckBox->setChecked(firstDisplayNode->GetScalarVisibility());
    d->ScalarsVisibilityCheckBox->blockSignals(wasBlocking);
  }

  double dataRange[2] = { 0.0, 0.0 };
  vtkDataArray* dataArray = firstDisplayNode->GetActiveScalarArray();
  if (dataArray)
  {
    dataArray->GetRange(dataRange);
  }

  // Update scalar values, range, decimals and single step
  double precision = 1.0;
  double dataMin = 0.0;
  double dataMax = 0.0;
  int decimals = 0;
  bool resetSliderRange = false;
  if (dataRange[0] < dataRange[1])
  {
    if (d->DataRangeUsedForAutoDisplayRange[0] != dataRange[0] //
        || d->DataRangeUsedForAutoDisplayRange[1] != dataRange[1])
    {
      d->DataRangeUsedForAutoDisplayRange[0] = dataRange[0];
      d->DataRangeUsedForAutoDisplayRange[1] = dataRange[1];
      resetSliderRange = true;
    }
    // Begin with a precision of 1% of the range
    precision = dataRange[1] / 100.0 - dataRange[0] / 100.0;
    // Extend min/max by 20% to give some room to work with
    dataMin = (floor(dataRange[0] / precision) - 20) * precision;
    dataMax = (ceil(dataRange[1] / precision) + 20) * precision;
    // Use closest power of ten value as a step value
    precision = ctk::closestPowerOfTen(precision);
    // Find significant decimals to show
    double stepDecimals = ctk::significantDecimals(precision);
    double minDecimals = ctk::significantDecimals(dataRange[0]);
    double maxDecimals = ctk::significantDecimals(dataRange[1]);
    decimals = std::max(stepDecimals, std::max(minDecimals, maxDecimals));
  }

  double* displayRange = firstDisplayNode->GetScalarRange();

  wasBlocking = d->DisplayedScalarRangeWidget->blockSignals(true);
  double sliderRange[2] = { 0.0, 0.0 };
  if (resetSliderRange)
  {
    sliderRange[0] = std::min(dataMin, displayRange[0]);
    sliderRange[1] = std::max(dataMax, displayRange[1]);
  }
  else
  {
    double currentRange[2] = { 0.0 };
    d->DisplayedScalarRangeWidget->range(currentRange);
    sliderRange[0] = std::min(currentRange[0], displayRange[0]);
    sliderRange[1] = std::max(currentRange[1], displayRange[1]);
  }
  d->DisplayedScalarRangeWidget->setRange(sliderRange[0], sliderRange[1]);
  d->DisplayedScalarRangeWidget->setValues(displayRange[0], displayRange[1]);
  // If the data range is empty (missing or constant active scalar array) then the default
  // 1.0 step may be larger than the slider range, and the range widget would reject it
  // (logging a warning on every update). Compute a valid step from the slider range instead.
  double displayedPrecision = precision;
  int displayedDecimals = decimals;
  double sliderRangeWidth = sliderRange[1] - sliderRange[0];
  if (sliderRangeWidth > 0.0 && displayedPrecision > sliderRangeWidth)
  {
    displayedPrecision = ctk::closestPowerOfTen(sliderRangeWidth / 100.0);
    if (displayedPrecision <= 0.0)
    {
      displayedPrecision = sliderRangeWidth;
    }
    displayedDecimals = std::max(decimals, static_cast<int>(ctk::significantDecimals(displayedPrecision)));
  }
  d->DisplayedScalarRangeWidget->setDecimals(displayedDecimals);
  d->DisplayedScalarRangeWidget->setSingleStep(displayedPrecision);
  d->DisplayedScalarRangeWidget->setEnabled(firstDisplayNode->GetScalarRangeFlag() == vtkMRMLDisplayNode::UseManualScalarRange);
  d->DisplayedScalarRangeWidget->blockSignals(wasBlocking);

  double thresholdRange[2] = { 0.0, 0.0 };
  if (thresholdSupported)
  {
    qMRMLScalarsDisplayWidgetPrivate::thresholdRange(firstDisplayNode, thresholdRange);
  }
  bool isThresholdEnabled = thresholdSupported && qMRMLScalarsDisplayWidgetPrivate::thresholdEnabled(firstDisplayNode);

  wasBlocking = d->ThresholdRangeWidget->blockSignals(true);
  d->ThresholdRangeWidget->setEnabled(isThresholdEnabled);
  d->ThresholdRangeWidget->setRange(dataRange[0] - precision, dataRange[1] + precision);
  if (thresholdRange[0] <= thresholdRange[1])
  {
    // there is a valid threshold range
    // If current threshold values do not fit in the current data range
    // then we move the slider handles to make them fit,
    // but values in the display node will not be changed until the user moves the handles.
    d->ThresholdRangeWidget->setValues(std::max(dataRange[0] - precision, thresholdRange[0]), std::min(dataRange[1] + precision, thresholdRange[1]));
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
  d->ThresholdCheckBox->setEnabled(thresholdSupported);
  d->ThresholdCheckBox->setChecked(isThresholdEnabled);
  d->ThresholdCheckBox->blockSignals(wasBlocking);

  wasBlocking = d->DisplayedScalarRangeModeComboBox->blockSignals(true);
  d->DisplayedScalarRangeModeComboBox->setCurrentIndex(firstDisplayNode->GetScalarRangeFlag());
  d->DisplayedScalarRangeModeComboBox->blockSignals(wasBlocking);

  wasBlocking = d->ActiveScalarComboBox->blockSignals(true);
  d->ActiveScalarComboBox->setEnabled(firstDisplayNode);
  if (firstDisplayNode)
  {
    if (d->ActiveScalarComboBox->dataSet() != firstDisplayNode->GetScalarDataSet())
    {
      d->ActiveScalarComboBox->setDataSet(firstDisplayNode->GetScalarDataSet());
    }
    d->updateDefaultArrayItem();
    QString activeScalarName = QString::fromUtf8(firstDisplayNode->GetActiveScalarName() ? firstDisplayNode->GetActiveScalarName() : "");
    if (d->isDefaultArrayItemPresent() && activeScalarName == d->DefaultScalarArrayName)
    {
      // Following the array the host chose is shown as such, not as that array by name
      d->ActiveScalarComboBox->setCurrentIndex(0);
    }
    else if (d->ActiveScalarComboBox->currentArrayName() != activeScalarName)
    {
      d->ActiveScalarComboBox->setCurrentArray(activeScalarName);
      // Array location would need to be set in d->ActiveScalarComboBox if
      // same scalar name is used in multiple locations.
    }
  }
  d->ActiveScalarComboBox->blockSignals(wasBlocking);

  // The component of a multi-component array that is shown; a single-component array has
  // nothing to choose
  int numberOfComponents = (firstDisplayNode ? firstDisplayNode->GetActiveScalarNumberOfComponents() : 1);
  wasBlocking = d->ActiveScalarComponentSpinBox->blockSignals(true);
  d->ActiveScalarComponentSpinBox->setVisible(numberOfComponents > 1);
  d->ActiveScalarComponentSpinBox->setMaximum(std::max(0, numberOfComponents - 1));
  int component = (firstDisplayNode ? firstDisplayNode->GetActiveScalarComponent() : -1);
  // A component that the array does not have shows the magnitude, which is what is drawn
  d->ActiveScalarComponentSpinBox->setValue(component < numberOfComponents ? component : -1);
  d->ActiveScalarComponentSpinBox->blockSignals(wasBlocking);

  wasBlocking = d->ScalarsColorNodeComboBox->blockSignals(true);
  if (d->ScalarsColorNodeComboBox->mrmlScene() != this->mrmlScene())
  {
    d->ScalarsColorNodeComboBox->setMRMLScene(this->mrmlScene());
  }
  if (d->ScalarsColorNodeComboBox->currentNodeID() != firstDisplayNode->GetColorNodeID())
  {
    d->ScalarsColorNodeComboBox->setCurrentNodeID(firstDisplayNode->GetColorNodeID());
  }
  d->ScalarsColorNodeComboBox->setEnabled(firstDisplayNode->GetScalarRangeFlag() != vtkMRMLDisplayNode::UseDirectMapping);
  d->ScalarsColorNodeComboBox->blockSignals(wasBlocking);

  emit displayNodeChanged();
}
