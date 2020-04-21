#include "qSlicerScalarVolumeDisplayWidget.h"
#include "ui_qSlicerScalarVolumeDisplayWidget.h"

// Qt includes

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

// STD includes
#include <limits>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
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

  QObject::connect(this->InterpolateCheckbox, SIGNAL(toggled(bool)),
                   q, SLOT(setInterpolate(bool)));
  QObject::connect(this->ColorTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                   q, SLOT(setColorNode(vtkMRMLNode*)));

  QObject::connect(this->LockWindowLevelButton, SIGNAL(clicked()),
                   q, SLOT(onLockWindowLevelButtonClicked()));
  QObject::connect(this->CTBonePresetToolButton, SIGNAL(clicked()),
                   q, SLOT(onPresetButtonClicked()));
  QObject::connect(this->CTAirPresetToolButton, SIGNAL(clicked()),
                   q, SLOT(onPresetButtonClicked()));
  QObject::connect(this->PETPresetToolButton, SIGNAL(clicked()),
                   q, SLOT(onPresetButtonClicked()));
  QObject::connect(this->CTAbdomenPresetToolButton, SIGNAL(clicked()),
                   q, SLOT(onPresetButtonClicked()));
  QObject::connect(this->CTBrainPresetToolButton, SIGNAL(clicked()),
                   q, SLOT(onPresetButtonClicked()));
  QObject::connect(this->CTLungPresetToolButton, SIGNAL(clicked()),
                   q, SLOT(onPresetButtonClicked()));
  QObject::connect(this->DTIPresetToolButton, SIGNAL(clicked()),
                   q, SLOT(onPresetButtonClicked()));
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
vtkMRMLScalarVolumeNode* qSlicerScalarVolumeDisplayWidget::volumeNode()const
{
  Q_D(const qSlicerScalarVolumeDisplayWidget);
  return vtkMRMLScalarVolumeNode::SafeDownCast(
    d->MRMLWindowLevelWidget->mrmlVolumeNode());
}

// --------------------------------------------------------------------------
bool qSlicerScalarVolumeDisplayWidget::isColorTableComboBoxEnabled()const
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
bool qSlicerScalarVolumeDisplayWidget::isMRMLWindowLevelWidgetEnabled()const
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
vtkMRMLScalarVolumeDisplayNode* qSlicerScalarVolumeDisplayWidget::volumeDisplayNode()const
{
  vtkMRMLVolumeNode* volumeNode = this->volumeNode();
  return volumeNode ? vtkMRMLScalarVolumeDisplayNode::SafeDownCast(
    volumeNode->GetDisplayNode()) : nullptr;
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

  d->MRMLWindowLevelWidget->setMRMLVolumeNode(volumeNode);
  d->MRMLVolumeThresholdWidget->setMRMLVolumeNode(volumeNode);

  qvtkReconnect(oldVolumeDisplayNode, volumeNode ? volumeNode->GetDisplayNode() :nullptr,
                vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromMRML()));

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
    d->ColorTableComboBox->setCurrentNode(displayNode->GetColorNode());
    d->InterpolateCheckbox->setChecked(displayNode->GetInterpolate());
    bool lockedWindowLevel = displayNode->GetWindowLevelLocked();
    d->LockWindowLevelButton->setChecked(lockedWindowLevel);
    if (lockedWindowLevel)
      {
      d->LockWindowLevelButton->setIcon(QIcon(":Icons/Medium/SlicerLock.png"));
      d->LockWindowLevelButton->setToolTip(QString("Click to enable modification of Window/Level values"));
      }
    else
      {
      d->LockWindowLevelButton->setIcon(QIcon(":Icons/Medium/SlicerUnlock.png"));
      d->LockWindowLevelButton->setToolTip(QString("Click to prevent modification of Window/Level values"));
      }
    d->CTBonePresetToolButton->setEnabled(!lockedWindowLevel);
    d->CTAirPresetToolButton->setEnabled(!lockedWindowLevel);
    d->PETPresetToolButton->setEnabled(!lockedWindowLevel);
    d->CTAbdomenPresetToolButton->setEnabled(!lockedWindowLevel);
    d->CTBrainPresetToolButton->setEnabled(!lockedWindowLevel);
    d->CTLungPresetToolButton->setEnabled(!lockedWindowLevel);
    d->DTIPresetToolButton->setEnabled(!lockedWindowLevel);
    d->MRMLWindowLevelWidget->setEnabled(!lockedWindowLevel);
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

  // Update histogram

  // Screen resolution is limited, therefore it does not make sense to compute
  // many bin counts.
  const int maxBinCount = 1000;
  if (voxelValues->GetArrayType() == VTK_FLOAT || voxelValues->GetArrayType() == VTK_DOUBLE)
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

  // Update histogram background

  // from vtkKWWindowLevelThresholdEditor::UpdateTransferFunction
  double range[2] = {0,255};
  vtkMRMLScalarVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (displayNode)
    {
    displayNode->GetDisplayScalarRange(range);
    }
  else
    {
    imageData->GetScalarRange(range);
    }
  // AdjustRange call will take out points that are outside of the new
  // range, but it needs the points to be there in order to work, so call
  // RemoveAllPoints after it's done
  d->ColorTransferFunction->AdjustRange(range);
  d->ColorTransferFunction->RemoveAllPoints();

  double min = d->MRMLWindowLevelWidget->level() - 0.5 * d->MRMLWindowLevelWidget->window();
  double max = d->MRMLWindowLevelWidget->level() + 0.5 * d->MRMLWindowLevelWidget->window();
  double minVal = 0;
  double maxVal = 1;
  double low   = d->MRMLVolumeThresholdWidget->isOff() ? range[0] : d->MRMLVolumeThresholdWidget->lowerThreshold();
  double upper = d->MRMLVolumeThresholdWidget->isOff() ? range[1] : d->MRMLVolumeThresholdWidget->upperThreshold();

  d->ColorTransferFunction->SetColorSpaceToRGB();

  if (low >= max || upper <= min)
    {
    d->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
    }
  else
    {
    max = qMax(min+0.001, max);
    low = qMax(range[0] + 0.001, low);
    min = qMax(range[0] + 0.001, min);
    upper = qMin(range[1] - 0.001, upper);

    if (min <= low)
      {
      minVal = (low - min)/(max - min);
      min = low + 0.001;
      }

    if (max >= upper)
      {
      maxVal = (upper - min)/(max-min);
      max = upper - 0.001;
      }

    d->ColorTransferFunction->AddRGBPoint(range[0], 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(low, 0, 0, 0);
    d->ColorTransferFunction->AddRGBPoint(min, minVal, minVal, minVal);
    d->ColorTransferFunction->AddRGBPoint(max, maxVal, maxVal, maxVal);
    d->ColorTransferFunction->AddRGBPoint(upper, maxVal, maxVal, maxVal);
    if (upper+0.001 < range[1])
      {
      d->ColorTransferFunction->AddRGBPoint(upper+0.001, 0, 0, 0);
      d->ColorTransferFunction->AddRGBPoint(range[1], 0, 0, 0);
      }
    }

  d->ColorTransferFunction->SetAlpha(1.0);
  d->ColorTransferFunction->Build();
}

// -----------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::showEvent( QShowEvent * event )
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
void qSlicerScalarVolumeDisplayWidget::onLockWindowLevelButtonClicked()
{
  vtkMRMLScalarVolumeDisplayNode* displayNode = this->volumeDisplayNode();
  if (!displayNode)
    {
    return;
    }
  // toggle the lock
  int locked = displayNode->GetWindowLevelLocked();
  displayNode->SetWindowLevelLocked(!locked);
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::onPresetButtonClicked()
{
  QToolButton* preset = qobject_cast<QToolButton*>(this->sender());
  this->setPreset(preset->accessibleName());
}

// --------------------------------------------------------------------------
void qSlicerScalarVolumeDisplayWidget::setPreset(const QString& presetName)
{
  Q_D(qSlicerScalarVolumeDisplayWidget);
  QString colorNodeID;
  double window = -1.;
  double level = std::numeric_limits<double>::max();
  if (presetName == "CT-Bone")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 1000.;
    level = 400.;
    }
  else if (presetName == "CT-Air")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 1000.;
    level = -426.;
    }
  else if (presetName == "PET")
    {
    colorNodeID = "vtkMRMLColorTableNodeRainbow";
    window = 10000.;
    level = 6000.;
    }
  else if (presetName == "CT-Abdomen")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 350.;
    level = 40.;
    }
  else if (presetName == "CT-Brain")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 100.;
    level = 50.;
    }
  else if (presetName == "CT-Lung")
    {
    colorNodeID = "vtkMRMLColorTableNodeGrey";
    window = 1400.;
    level = -500.;
    }
  else if (presetName == "DTI")
    {
    colorNodeID = "vtkMRMLColorTableNodeRainbow";
    window = 1.0;
    level = 0.5;
    }
  vtkMRMLNode* colorNode = this->mrmlScene()->GetNodeByID(colorNodeID.toUtf8());
  if (colorNode)
    {
    this->setColorNode(colorNode);
    }
  if (window != -1 || level!= std::numeric_limits<double>::max())
    {
    d->MRMLWindowLevelWidget->setAutoWindowLevel(qMRMLWindowLevelWidget::Manual);
    }
  if (window != -1 && level != std::numeric_limits<double>::max())
    {
    d->MRMLWindowLevelWidget->setWindowLevel(window, level);
    }
  else if (window != -1)
    {
    d->MRMLWindowLevelWidget->setWindow(window);
    }
  else if (level != std::numeric_limits<double>::max())
    {
    d->MRMLWindowLevelWidget->setLevel(level);
    }
}
