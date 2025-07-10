#include "qSlicerScalarVolumeDisplayWidget.h"
#include "ui_qSlicerScalarVolumeDisplayWidget.h"

// Qt includes
#include <QToolButton>

// CTK includes
#include <ctkVTKColorTransferFunction.h>
#include <ctkTransferFunctionGradientItem.h>
#include <ctkTransferFunctionScene.h>
#include <ctkTransferFunctionBarsItem.h>
#include <ctkVTKHistogram.h>

// MRML includes
#include "vtkMRMLColorNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "vtkSlicerVolumesLogic.h"

// STD includes
#include <limits>

//-----------------------------------------------------------------------------
class qSlicerScalarVolumeDisplayWidgetPrivate
  : public Ui_qSlicerScalarVolumeDisplayWidget
{
  Q_DECLARE_PUBLIC(qSlicerScalarVolumeDisplayWidget);
protected:
  qSlicerScalarVolumeDisplayWidget* const q_ptr;
public:
  qSlicerScalarVolumeDisplayWidgetPrivate(qSlicerScalarVolumeDisplayWidget& object);
  ~qSlicerScalarVolumeDisplayWidgetPrivate();
  void init();

  ctkVTKHistogram* Histogram;
  vtkWeakPointer<vtkDataArray> HistogramVoxelValues;
  vtkMTimeType HistogramVoxelValuesMTime;
  vtkSmartPointer<vtkColorTransferFunction> ColorTransferFunction;
};

//-----------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidgetPrivate::qSlicerScalarVolumeDisplayWidgetPrivate(
  qSlicerScalarVolumeDisplayWidget& object)
  : q_ptr(&object)
{
  this->Histogram = new ctkVTKHistogram();
  this->ColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
}

//-----------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidgetPrivate::~qSlicerScalarVolumeDisplayWidgetPrivate()
{
  delete this->Histogram;
}

//-----------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qSlicerScalarVolumeDisplayWidget);

  this->setupUi(q);

  ctkTransferFunctionScene* scene = qobject_cast<ctkTransferFunctionScene*>(
    this->TransferFunctionView->scene());
  // Transfer Function
  ctkVTKColorTransferFunction* transferFunction =
    new ctkVTKColorTransferFunction(this->ColorTransferFunction, q);

  ctkTransferFunctionGradientItem* gradientItem =
    new ctkTransferFunctionGradientItem(transferFunction);
  scene->addItem(gradientItem);
  // Histogram
  //scene->setTransferFunction(this->Histogram);
  ctkTransferFunctionBarsItem* barsItem =
    new ctkTransferFunctionBarsItem(this->Histogram);
  barsItem->setBarWidth(1.);
  scene->addItem(barsItem);

  // Add mapping from presets defined in the Volumes module logic (VolumeDisplayPresets.json)

    // read volume preset names from volumes logic
  vtkSlicerVolumesLogic* volumesModuleLogic = (qSlicerCoreApplication::application() ? vtkSlicerVolumesLogic::SafeDownCast(
    qSlicerCoreApplication::application()->moduleLogic("Volumes")) : nullptr);
  if (volumesModuleLogic)
  {
    QLayout* volumeDisplayPresetsLayout = this->PresetsGroupBox->layout();
    if (!volumeDisplayPresetsLayout)
    {
      volumeDisplayPresetsLayout = new QHBoxLayout;
      this->PresetsGroupBox->setLayout(volumeDisplayPresetsLayout);
    }
    std::vector<std::string> presetIds = volumesModuleLogic->GetVolumeDisplayPresetIDs();
    for (const auto& presetId : presetIds)
    {
      vtkSlicerVolumesLogic::VolumeDisplayPreset preset = volumesModuleLogic->GetVolumeDisplayPreset(presetId);
      QString presetIdStr = QString::fromStdString(presetId);
      QString presetName = qSlicerScalarVolumeDisplayWidget::tr(preset.name.c_str());
      QToolButton* presetButton = new QToolButton();
      presetButton->setObjectName(presetIdStr);
      presetButton->setToolTip(qSlicerScalarVolumeDisplayWidget::tr(preset.name.c_str()) + "\n"
        + qSlicerScalarVolumeDisplayWidget::tr(preset.description.c_str()));
      if (!preset.icon.empty())
      {
        presetButton->setIcon(QIcon(QString::fromStdString(preset.icon)));
        presetButton->setIconSize(QSize(45, 45));
      }
      volumeDisplayPresetsLayout->addWidget(presetButton);
      QObject::connect(presetButton, SIGNAL(clicked()),
        q, SLOT(onPresetButtonClicked()));
    }
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: Module logic 'Volumes' not found. No volume display presets will be added.";
    return;
  }

  QObject::connect(this->InterpolateCheckbox, SIGNAL(toggled(bool)),
                   q, SLOT(setInterpolate(bool)));
  QObject::connect(this->InvertDisplayScalarRangeCheckbox, SIGNAL(toggled(bool)),
                   q, SLOT(setInvert(bool)));
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setColorNode(vtkMRMLNode*)));
  QObject::connect(this->HistogramGroupBox, SIGNAL(toggled(bool)),
                   q, SLOT(onHistogramSectionExpanded(bool)));
}

// --------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidget::qSlicerScalarVolumeDisplayWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerScalarVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qSlicerScalarVolumeDisplayWidget);
  d->init();

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qSlicerScalarVolumeDisplayWidget::~qSlicerScalarVolumeDisplayWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* qSlicerScalarVolumeDisplayWidget::volumeNode() const
{
  Q_D(const qSlicerScalarVolumeDisplayWidget);
  return vtkMRMLScalarVolumeNode::SafeDownCast(
    d->MRMLWindowLevelWidget->mrmlVolumeNode());
}

// --------------------------------------------------------------------------
bool qSlicerScalarVolumeDisplayWidget::isColorTableComboBoxEnabled() const
{
  Q_D(const qSlicerScalarVolumeDisplayWidget);
  return d->ColorTableComboBox->isEnabled();
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setColorTableComboBoxEnabled(bool enable)
{
  Q_D(qSlicerScalarVolumeDisplayWidget);
  d->ColorTableComboBox->setEnabled(enable);
}

// --------------------------------------------------------------------------
bool qSlicerScalarVolumeDisplayWidget::isMRMLWindowLevelWidgetEnabled() const
{
  Q_D(const qSlicerScalarVolumeDisplayWidget);
  return d->MRMLWindowLevelWidget->isEnabled();
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setMRMLWindowLevelWidgetEnabled(bool enable)
{
  Q_D(qSlicerScalarVolumeDisplayWidget);
  d->MRMLWindowLevelWidget->setEnabled(enable);
}

// --------------------------------------------------------------------------
vtkMRMLScalarVolumeDisplayNode* qSlicerScalarVolumeDisplayWidget::volumeDisplayNode() const
{
  vtkMRMLVolumeNode* volumeNode = this->volumeNode();
  return volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetVolumeDisplayNode()) : nullptr;
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLScalarVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLScalarVolumeNode* volumeNode)
{
  Q_D(qSlicerScalarVolumeDisplayWidget);

  vtkMRMLScalarVolumeDisplayNode* oldVolumeDisplayNode = this->volumeDisplayNode();

  qvtkReconnect(oldVolumeDisplayNode, volumeNode ? volumeNode->GetVolumeDisplayNode() :nullptr,
                vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));

  d->MRMLWindowLevelWidget->setMRMLVolumeNode(volumeNode);
  d->MRMLVolumeThresholdWidget->setMRMLVolumeNode(volumeNode);

  this->setEnabled(volumeNode != nullptr);

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::updateWidgetFromMRML()
{
  Q_D(qSlicerScalarVolumeDisplayWidget);
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (displayNode)
  {
    QSignalBlocker blocker1(d->ColorTableComboBox);
    QSignalBlocker blocker2(d->InterpolateCheckbox);
    QSignalBlocker blocker3(d->InvertDisplayScalarRangeCheckbox);
    d->ColorTableComboBox->setCurrentNode(displayNode->GetColorNode());
    d->InterpolateCheckbox->setChecked(displayNode->GetInterpolate());
    d->InvertDisplayScalarRangeCheckbox->setChecked(displayNode->GetInvertDisplayScalarRange());
  }
  this->updateHistogram();
}

//----------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::updateHistogram()
{
  Q_D(qSlicerScalarVolumeDisplayWidget);

  // Get voxel array
  vtkMRMLScalarVolumeNode* volumeNode = this->volumeNode();
  vtkImageData* imageData = volumeNode ? volumeNode->GetImageData() : nullptr;
  vtkPointData* pointData = imageData ? imageData->GetPointData() : nullptr;
  vtkDataArray* voxelValues = pointData ? pointData->GetScalars() : nullptr;

  // If there are no voxel values then we completely hide the histogram section
  d->HistogramGroupBox->setVisible(voxelValues != nullptr);

  d->Histogram->setDataArray(voxelValues);
  // Calling histogram build() with an empty volume causes heap corruption
  // (reported by VS2013 in debug mode), therefore we only build
  // the histogram if there are voxels (otherwise histogram is hidden).

  if (!voxelValues || !this->isVisible() || d->HistogramGroupBox->collapsed())
  {
    d->ColorTransferFunction->RemoveAllPoints();
    return;
  }

  // Update histogram data if needed
  if (d->HistogramVoxelValues != voxelValues || d->HistogramVoxelValuesMTime < voxelValues->GetMTime())
  {
    d->Histogram->setDataArray(voxelValues);

    // Screen resolution is limited, therefore it does not make sense to compute
    // many bin counts.
    const int maxBinCount = 1000;
    if (voxelValues->GetDataType() == VTK_FLOAT || voxelValues->GetDataType() == VTK_DOUBLE)
    {
      d->Histogram->setNumberOfBins(maxBinCount);
    }
    else
    {
      double* range = voxelValues->GetRange();
      int binCount = static_cast<int>(range[1] - range[0] + 1);
      if (binCount > maxBinCount)
      {
        binCount = maxBinCount;
      }
      if (binCount < 1)
      {
        binCount = 1;
      }
      d->Histogram->setNumberOfBins(binCount);
    }
    d->Histogram->build();

    // Update min, center, max labels
    double* range = voxelValues->GetRange();
    double center = (range[0] + range[1]) / 2.0;

    d->MinValueLabel->setText(QString::number(range[0], 'g', 5));
    d->CenterValueLabel->setText(QString::number(center, 'g', 5));
    d->MaxValueLabel->setText(QString::number(range[1], 'g', 5));

    d->HistogramVoxelValues = voxelValues;
    d->HistogramVoxelValuesMTime = voxelValues->GetMTime();
  }

  // Update histogram background

  // from vtkKWWindowLevelThresholdEditor::UpdateTransferFunction
  double histogramRange[2] = {0,255};
  vtkMRMLScalarVolumeDisplayNode* displayNode = this->volumeDisplayNode();
  if (displayNode)
  {
    displayNode->GetDisplayScalarRange(histogramRange);
  }
  else
  {
    imageData->GetScalarRange(histogramRange);
  }
  // AdjustRange call will take out points that are outside of the new
  // range, but it needs the points to be there in order to work, so call
  // RemoveAllPoints after it's done
  d->ColorTransferFunction->AdjustRange(histogramRange);
  d->ColorTransferFunction->RemoveAllPoints();

  double colorTableRange[2] = { displayNode->GetWindowLevelMin(), displayNode->GetWindowLevelMax() };
  double thresholdRange[2] = { histogramRange[0], histogramRange[1] };
  if (displayNode->GetApplyThreshold())
  {
    thresholdRange[0] = displayNode->GetLowerThreshold();
    thresholdRange[1] = displayNode->GetUpperThreshold();
  }

  d->ColorTransferFunction->SetColorSpaceToRGB();

  vtkScalarsToColors* mapToColors = nullptr;
  if (displayNode && displayNode->GetScalarRangeFlag() != vtkMRMLDisplayNode::UseDirectMapping)
  {
    mapToColors = displayNode->GetLookupTable();
  }

  // Background color for thresholded region
  double thresholdedRGB[3] = { 0.2, 0.2, 0.2 };

  const double eps = (histogramRange[1] - histogramRange[0]) / 1e5;

  // Fill lower thresholded region
  double currentPosition = histogramRange[0]; // can point from here
  if (currentPosition + eps < thresholdRange[0])
  {
    d->ColorTransferFunction->AddRGBPoint(currentPosition + eps, thresholdedRGB[0], thresholdedRGB[1], thresholdedRGB[2]);
    currentPosition = thresholdRange[0];
    d->ColorTransferFunction->AddRGBPoint(currentPosition, thresholdedRGB[0], thresholdedRGB[1], thresholdedRGB[2]);
  }
  // Draw color gradient between thresholded regions
  if (currentPosition + eps < thresholdRange[1])
  {
    double colorTableVisibleRange[2] =
    {
      qBound(thresholdRange[0], colorTableRange[0], thresholdRange[1]),
      qBound(thresholdRange[0], colorTableRange[1], thresholdRange[1])
    };
    double colorTableIndexRange[2] = { 0, 0 };
    if (colorTableRange[1] - colorTableRange[0] > eps)
    {
      colorTableIndexRange[0] = (colorTableVisibleRange[0] - colorTableRange[0]) / (colorTableRange[1] - colorTableRange[0]) * 255.0;
      colorTableIndexRange[1] = (colorTableVisibleRange[1] - colorTableRange[0]) / (colorTableRange[1] - colorTableRange[0]) * 255.0;
    };
    // Paint if there is a saturated region below the color table
    if (currentPosition + eps < colorTableVisibleRange[0])
    {
      double belowColor[3] = { 0.0, 0.0, 0.0 };
      if (mapToColors)
      {
        mapToColors->GetColor(0, belowColor);
      }
      d->ColorTransferFunction->AddRGBPoint(currentPosition + eps, belowColor[0], belowColor[1], belowColor[2]);
      currentPosition = colorTableVisibleRange[0];
      d->ColorTransferFunction->AddRGBPoint(currentPosition, belowColor[0], belowColor[1], belowColor[2]);
    }
    // Paint the color gradient
    if (currentPosition + eps < colorTableRange[1])
    {
      if (mapToColors)
      {
        int resolution = 50;
        double rgb[3] = { 0.0, 0.0, 0.0 };
        for (int i = 0; i < resolution; i++)
        {
          currentPosition = colorTableVisibleRange[0] + (colorTableVisibleRange[1] - colorTableVisibleRange[0]) * double(i) / (resolution - 1) + eps;
          mapToColors->GetColor((colorTableIndexRange[0] + (colorTableIndexRange[1] - colorTableIndexRange[0]) * double(i) / (resolution - 1)), rgb);
          d->ColorTransferFunction->AddRGBPoint(currentPosition, rgb[0], rgb[1], rgb[2]);
        }
      }
      else
      {
        d->ColorTransferFunction->AddRGBPoint(currentPosition + eps,
          colorTableIndexRange[0] / 255.0, colorTableIndexRange[0] / 255.0, colorTableIndexRange[0] / 255.0);
        currentPosition = colorTableVisibleRange[1];
        d->ColorTransferFunction->AddRGBPoint(currentPosition,
          colorTableIndexRange[1] / 255.0, colorTableIndexRange[1] / 255.0, colorTableIndexRange[1] / 255.0);
      }
    }
    // Paint if there is a saturated region above the color table
    if (currentPosition + eps < thresholdRange[1])
    {
      double aboveColor[3] = { 1.0, 1.0, 1.0 };
      if (mapToColors)
      {
        mapToColors->GetColor(255, aboveColor);
      }
      d->ColorTransferFunction->AddRGBPoint(currentPosition + eps, aboveColor[0], aboveColor[1], aboveColor[2]);
      currentPosition = thresholdRange[1];
      d->ColorTransferFunction->AddRGBPoint(currentPosition, aboveColor[0], aboveColor[1], aboveColor[2]);
    }

  }
  // Fill upper thresholded region
  if (currentPosition + eps < histogramRange[1])
  {
    d->ColorTransferFunction->AddRGBPoint(currentPosition + eps, thresholdedRGB[0], thresholdedRGB[1], thresholdedRGB[2]);
    currentPosition = histogramRange[1];
    d->ColorTransferFunction->AddRGBPoint(currentPosition, thresholdedRGB[0], thresholdedRGB[1], thresholdedRGB[2]);
  }

  d->ColorTransferFunction->SetAlpha(1.0);
  d->ColorTransferFunction->Build();
}

// -----------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::showEvent( QShowEvent* event )
{
  this->updateHistogram();
  this->Superclass::showEvent(event);
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::onHistogramSectionExpanded(bool expanded)
{
  Q_UNUSED(expanded);
  this->updateHistogram();
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setInterpolate(bool interpolate)
{
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode)
  {
    return;
  }
  displayNode->SetInterpolate(interpolate);
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setInvert(bool invert)
{
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode)
  {
    return;
  }
  displayNode->SetInvertDisplayScalarRange(invert);
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setColorNode(vtkMRMLNode* colorNode)
{
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode || !colorNode)
  {
    return;
  }
  Q_ASSERT(vtkMRMLColorNode::SafeDownCast(colorNode));
  displayNode->SetAndObserveColorNodeID(colorNode->GetID());
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::onPresetButtonClicked()
{
  QToolButton* preset = qobject_cast<QToolButton*>(this->sender());
  this->setPreset(preset->objectName());
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setPreset(const QString& presetId)
{
  Q_D(qSlicerScalarVolumeDisplayWidget);
  vtkSlicerVolumesLogic* volumesModuleLogic = vtkSlicerVolumesLogic::SafeDownCast(qSlicerApplication::application()->moduleLogic("Volumes"));
  if (!volumesModuleLogic)
  {
    qCritical() << Q_FUNC_INFO << " failed: volumes module logic is not available";
    return;
  }
  volumesModuleLogic->ApplyVolumeDisplayPreset(this->volumeDisplayNode(), presetId.toStdString());
}
