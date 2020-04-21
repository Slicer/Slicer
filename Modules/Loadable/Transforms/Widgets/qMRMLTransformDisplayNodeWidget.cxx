/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

==============================================================================*/


// qMRML includes
#include "qMRMLTransformDisplayNodeWidget.h"
#include "ui_qMRMLTransformDisplayNodeWidget.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLTransformDisplayNode.h>
#include <vtkMRMLProceduralColorNode.h>

// VTK includes
#include "vtkColorTransferFunction.h"

// CTK includes
#include "ctkVTKScalarsToColorsView.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Transforms
class qMRMLTransformDisplayNodeWidgetPrivate
  : public Ui_qMRMLTransformDisplayNodeWidget
{
  Q_DECLARE_PUBLIC(qMRMLTransformDisplayNodeWidget);
protected:
  qMRMLTransformDisplayNodeWidget* const q_ptr;
public:
  qMRMLTransformDisplayNodeWidgetPrivate(qMRMLTransformDisplayNodeWidget& object);
  ~qMRMLTransformDisplayNodeWidgetPrivate();
  void init();

  vtkMRMLTransformDisplayNode* TransformDisplayNode;
  vtkColorTransferFunction* ColorTransferFunction;
};

//-----------------------------------------------------------------------------
// qMRMLTransformDisplayNodeWidgetPrivate methods

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidgetPrivate
::qMRMLTransformDisplayNodeWidgetPrivate(qMRMLTransformDisplayNodeWidget& object)
  : q_ptr(&object)
{
  this->TransformDisplayNode = nullptr;
  this->ColorTransferFunction = vtkColorTransferFunction::New();
}

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidgetPrivate
::~qMRMLTransformDisplayNodeWidgetPrivate()
{
  this->ColorTransferFunction->Delete();
  this->ColorTransferFunction = nullptr;
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidgetPrivate
::init()
{
  Q_Q(qMRMLTransformDisplayNodeWidget);
  this->setupUi(q);

  double validBounds[4] = {VTK_DOUBLE_MIN, VTK_DOUBLE_MAX, 0., 1.};

  this->ColorMapWidget->view()->setValidBounds(validBounds);
  this->ColorMapWidget->view()->addColorTransferFunction(nullptr);
  this->ColorMapWidget->view()->setColorTransferFunctionToPlots(this->ColorTransferFunction);

  double chartBounds[8]={0};
  this->ColorMapWidget->view()->chartBounds(chartBounds);
  chartBounds[2] = 0;
  chartBounds[3] = 100;
  this->ColorMapWidget->view()->setChartUserBounds(chartBounds);
  chartBounds[2] = 1;
  chartBounds[3] = 50;
  this->ColorMapWidget->view()->setPlotsUserBounds(chartBounds);
  this->ColorMapWidget->view()->update();

  q->qvtkConnect(this->ColorTransferFunction, vtkCommand::EndInteractionEvent,
                    q, SLOT(onColorInteractionEvent()), 0., Qt::QueuedConnection);
  q->qvtkConnect(this->ColorTransferFunction, vtkCommand::EndEvent,
                    q, SLOT(onColorInteractionEvent()), 0., Qt::QueuedConnection);
  q->qvtkConnect(this->ColorTransferFunction, vtkCommand::ModifiedEvent,
                    q, SLOT(onColorModifiedEvent()), 0., Qt::QueuedConnection);


  this->AdvancedParameters->setCollapsed(true);

  // Interaction panel
  QObject::connect(this->InteractionVisibleCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorVisibility(bool)));
  QObject::connect(this->InteractiveTranslationCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorTranslationEnabled(bool)));
  QObject::connect(this->InteractiveRotationCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorRotationEnabled(bool)));
  QObject::connect(this->InteractiveScalingCheckBox, SIGNAL(toggled(bool)), q, SLOT(setEditorScalingEnabled(bool)));
  QObject::connect(this->UpdateBoundsPushButton, SIGNAL(clicked()), q, SLOT(updateEditorBounds()));

  this->InteractiveAdvancedOptionsFrame->hide();

  // Visualization panel
  // by default the glyph option is selected, so hide the parameter sets for the other options
  this->GlyphOptions->show();
  this->ContourOptions->hide();
  this->GridOptions->hide();

  QObject::connect(this->VisibleCheckBox, SIGNAL(toggled(bool)), q, SLOT(setVisibility(bool)));
  QObject::connect(this->Visible2dCheckBox, SIGNAL(toggled(bool)), q, SLOT(setVisibility2d(bool)));
  QObject::connect(this->Visible3dCheckBox, SIGNAL(toggled(bool)), q, SLOT(setVisibility3d(bool)));

  QObject::connect(this->RegionNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(regionNodeChanged(vtkMRMLNode*)));

  QObject::connect(this->GlyphToggle, SIGNAL(toggled(bool)), q, SLOT(setGlyphVisualizationMode(bool)));
  QObject::connect(this->GridToggle, SIGNAL(toggled(bool)), q, SLOT(setGridVisualizationMode(bool)));
  QObject::connect(this->ContourToggle, SIGNAL(toggled(bool)), q, SLOT(setContourVisualizationMode(bool)));

  // Glyph Parameters
  QObject::connect(this->GlyphPointsNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SLOT(glyphPointsNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->GlyphSpacingMm, SIGNAL(valueChanged(double)), q, SLOT(setGlyphSpacingMm(double)));
  QObject::connect(this->GlyphScalePercent, SIGNAL(valueChanged(double)), q, SLOT(setGlyphScalePercent(double)));
  QObject::connect(this->GlyphDisplayRangeMm, SIGNAL(valuesChanged(double, double)), q, SLOT(setGlyphDisplayRangeMm(double, double)));
  QObject::connect(this->GlyphTypeComboBox, SIGNAL(currentIndexChanged(int)), q, SLOT(setGlyphType(int)));
  // 3D Glyph Parameters
  QObject::connect(this->GlyphDiameterMm, SIGNAL(valueChanged(double)), q, SLOT(setGlyphDiameterMm(double)));
  QObject::connect(this->GlyphTipLengthPercent, SIGNAL(valueChanged(double)), q, SLOT(setGlyphTipLengthPercent(double)));
  QObject::connect(this->GlyphShaftDiameterPercent, SIGNAL(valueChanged(double)), q, SLOT(setGlyphShaftDiameterPercent(double)));
  QObject::connect(this->GlyphResolution, SIGNAL(valueChanged(double)), q, SLOT(setGlyphResolution(double)));

  // Grid Parameters
  QObject::connect(this->GridScalePercent, SIGNAL(valueChanged(double)), q, SLOT(setGridScalePercent(double)));
  QObject::connect(this->GridSpacingMm, SIGNAL(valueChanged(double)), q, SLOT(setGridSpacingMm(double)));
  QObject::connect(this->GridLineDiameterMm, SIGNAL(valueChanged(double)), q, SLOT(setGridLineDiameterMm(double)));
  QObject::connect(this->GridResolutionMm, SIGNAL(valueChanged(double)), q, SLOT(setGridResolutionMm(double)));
  QObject::connect(this->GridShowNonWarped, SIGNAL(toggled(bool)), q, SLOT(setGridShowNonWarped(bool)));

  // Contour Parameters
  QRegExp rx("^(([0-9]+(.[0-9]+)?)[ ]?)*([0-9]+(.[0-9]+)?)[ ]?$");
  this->ContourLevelsMm->setValidator(new QRegExpValidator(rx,q));
  QObject::connect(this->ContourLevelsMm, SIGNAL(textChanged(QString)), q, SLOT(setContourLevelsMm(QString)));
  QObject::connect(this->ContourResolutionMm, SIGNAL(valueChanged(double)), q, SLOT(setContourResolutionMm(double)));
  QObject::connect(this->ContourOpacityPercent, SIGNAL(valueChanged(double)), q, SLOT(setContourOpacityPercent(double)));

  q->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
// qMRMLTransformDisplayNodeWidget methods

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidget
::qMRMLTransformDisplayNodeWidget(QWidget *newParent) :
    Superclass(newParent)
  , d_ptr(new qMRMLTransformDisplayNodeWidgetPrivate(*this))
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  d->init();
}

//-----------------------------------------------------------------------------
qMRMLTransformDisplayNodeWidget
::~qMRMLTransformDisplayNodeWidget() = default;

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget
::setMRMLTransformNode(vtkMRMLNode* transformNode)
{
  setMRMLTransformNode(vtkMRMLTransformNode::SafeDownCast(transformNode));
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget
::setMRMLTransformNode(vtkMRMLTransformNode* transformNode)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  vtkMRMLTransformDisplayNode* displayNode = nullptr;
  if (transformNode!=nullptr)
    {
    displayNode=vtkMRMLTransformDisplayNode::SafeDownCast(transformNode->GetDisplayNode());
    }

  qvtkReconnect(d->TransformDisplayNode, displayNode, vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDisplayNode()));

  d->TransformDisplayNode = displayNode;
  this->updateWidgetFromDisplayNode();
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget
::updateWidgetFromDisplayNode()
{
  Q_D(qMRMLTransformDisplayNodeWidget);

  this->setEnabled(d->TransformDisplayNode != nullptr);

  if (!d->TransformDisplayNode)
    {
    return;
    }

  // Update widget if different from MRML node

  // Interaction

  d->InteractionVisibleCheckBox->setChecked(d->TransformDisplayNode->GetEditorVisibility());
  d->InteractiveTranslationCheckBox->setChecked(d->TransformDisplayNode->GetEditorTranslationEnabled());
  d->InteractiveRotationCheckBox->setChecked(d->TransformDisplayNode->GetEditorRotationEnabled());
  d->InteractiveScalingCheckBox->setChecked(d->TransformDisplayNode->GetEditorScalingEnabled());

  // Display

  d->VisibleCheckBox->setChecked(d->TransformDisplayNode->GetVisibility());
  d->Visible2dCheckBox->setChecked(d->TransformDisplayNode->GetVisibility2D());
  d->Visible3dCheckBox->setChecked(d->TransformDisplayNode->GetVisibility3D());

  switch (d->TransformDisplayNode->GetVisualizationMode())
    {
    case vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH: d->GlyphToggle->setChecked(true); break;
    case vtkMRMLTransformDisplayNode::VIS_MODE_GRID: d->GridToggle->setChecked(true); break;
    case vtkMRMLTransformDisplayNode::VIS_MODE_CONTOUR: d->ContourToggle->setChecked(true); break;
    }

  d->RegionNodeComboBox->setCurrentNode(d->TransformDisplayNode->GetRegionNode());

  // Update Visualization Parameters
  // Glyph Parameters
  d->GlyphPointsNodeComboBox->setCurrentNode(d->TransformDisplayNode->GetGlyphPointsNode());
  d->GlyphSpacingMm->setValue(d->TransformDisplayNode->GetGlyphSpacingMm());
  d->GlyphSpacingMm->setEnabled(d->TransformDisplayNode->GetGlyphPointsNode() == nullptr);
  d->GlyphScalePercent->setValue(d->TransformDisplayNode->GetGlyphScalePercent());
  d->GlyphDisplayRangeMm->setMaximumValue(d->TransformDisplayNode->GetGlyphDisplayRangeMaxMm());
  d->GlyphDisplayRangeMm->setMinimumValue(d->TransformDisplayNode->GetGlyphDisplayRangeMinMm());
  d->GlyphTypeComboBox->setCurrentIndex(d->TransformDisplayNode->GetGlyphType());
  // 3D Glyph Parameters
  d->GlyphDiameterMm->setValue(d->TransformDisplayNode->GetGlyphDiameterMm());
  d->GlyphTipLengthPercent->setValue(d->TransformDisplayNode->GetGlyphTipLengthPercent());
  d->GlyphShaftDiameterPercent->setValue(d->TransformDisplayNode->GetGlyphShaftDiameterPercent());
  d->GlyphResolution->setValue(d->TransformDisplayNode->GetGlyphResolution());

  // Grid Parameters
  d->GridScalePercent->setValue(d->TransformDisplayNode->GetGridScalePercent());
  d->GridSpacingMm->setValue(d->TransformDisplayNode->GetGridSpacingMm());
  d->GridLineDiameterMm->setValue(d->TransformDisplayNode->GetGridLineDiameterMm());
  d->GridResolutionMm->setValue(d->TransformDisplayNode->GetGridResolutionMm());
  d->GridShowNonWarped->setChecked(d->TransformDisplayNode->GetGridShowNonWarped());

  // Contour Parameters
  d->ContourResolutionMm->setValue(d->TransformDisplayNode->GetContourResolutionMm());
  d->ContourOpacityPercent->setValue(d->TransformDisplayNode->GetContourOpacity()*100.0);
  // Only update the text in the editbox if it is changed (to not interfere with editing of the values)
  std::vector<double> levelsInWidget=vtkMRMLTransformDisplayNode::ConvertContourLevelsFromString(d->ContourLevelsMm->text().toUtf8());
  std::vector<double> levelsInMRML;
  d->TransformDisplayNode->GetContourLevelsMm(levelsInMRML);
  if (!vtkMRMLTransformDisplayNode::IsContourLevelEqual(levelsInWidget,levelsInMRML))
    {
    d->ContourLevelsMm->setText(QLatin1String(d->TransformDisplayNode->GetContourLevelsMmAsString().c_str()));
    }

  // Update ColorMap
  vtkColorTransferFunction* colorTransferFunctionInNode=d->TransformDisplayNode->GetColorMap();
  if (colorTransferFunctionInNode)
    {
    if (!vtkMRMLProceduralColorNode::IsColorMapEqual(d->ColorTransferFunction,colorTransferFunctionInNode))
      {
      // only update the range if the colormap is changed to avoid immediate update,
      // because we don't want to change the colormap plot range while dragging the control point
      d->ColorTransferFunction->DeepCopy(colorTransferFunctionInNode);
      this->colorUpdateRange();
      }
    }

  // Interaction
  d->InteractionVisibleCheckBox->setChecked(d->TransformDisplayNode->GetEditorVisibility());
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::updateGlyphSourceOptions(int glyphType)
{
  Q_D(qMRMLTransformDisplayNodeWidget);

  if (glyphType == vtkMRMLTransformDisplayNode::GLYPH_TYPE_ARROW)
    {
    d->GlyphDiameterMmLabel->setVisible(true);
    d->GlyphDiameterMm->setVisible(true);
    d->GlyphShaftDiameterLabel->setVisible(true);
    d->GlyphShaftDiameterPercent->setVisible(true);
    d->GlyphTipLengthLabel->setVisible(true);
    d->GlyphTipLengthPercent->setVisible(true);
    }
  else if (glyphType == vtkMRMLTransformDisplayNode::GLYPH_TYPE_CONE)
    {
    d->GlyphDiameterMmLabel->setVisible(true);
    d->GlyphDiameterMm->setVisible(true);
    d->GlyphShaftDiameterLabel->setVisible(false);
    d->GlyphShaftDiameterPercent->setVisible(false);
    d->GlyphTipLengthLabel->setVisible(false);
    d->GlyphTipLengthPercent->setVisible(false);
    }
  else if (glyphType == vtkMRMLTransformDisplayNode::GLYPH_TYPE_SPHERE)
    {
    d->GlyphDiameterMmLabel->setVisible(false);
    d->GlyphDiameterMm->setVisible(false);
    d->GlyphShaftDiameterLabel->setVisible(false);
    d->GlyphShaftDiameterPercent->setVisible(false);
    d->GlyphTipLengthLabel->setVisible(false);
    d->GlyphTipLengthPercent->setVisible(false);
    }
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::regionNodeChanged(vtkMRMLNode* node)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetAndObserveRegionNode(node);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::glyphPointsNodeChanged(vtkMRMLNode* node)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetAndObserveGlyphPointsNode(node);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphSpacingMm(double spacing)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGlyphSpacingMm(spacing);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphScalePercent(double scale)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGlyphScalePercent(scale);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphDisplayRangeMm(double min, double max)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  int oldModify=d->TransformDisplayNode->StartModify();
  d->TransformDisplayNode->SetGlyphDisplayRangeMinMm(min);
  d->TransformDisplayNode->SetGlyphDisplayRangeMaxMm(max);
  d->TransformDisplayNode->EndModify(oldModify);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphType(int glyphType)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGlyphType(glyphType);
  this->updateGlyphSourceOptions(glyphType);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphTipLengthPercent(double length)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGlyphTipLengthPercent(length);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphDiameterMm(double diameterMm)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGlyphDiameterMm(diameterMm);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphShaftDiameterPercent(double diameterPercent)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGlyphShaftDiameterPercent(diameterPercent);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphResolution(double resolution)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGlyphResolution(resolution);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGridScalePercent(double scale)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGridScalePercent(scale);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGridSpacingMm(double spacing)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGridSpacingMm(spacing);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGridLineDiameterMm(double diameterMm)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGridLineDiameterMm(diameterMm);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGridResolutionMm(double resolutionMm)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGridResolutionMm(resolutionMm);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGridShowNonWarped(bool show)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetGridShowNonWarped(show);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setContourLevelsMm(QString values_str)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetContourLevelsMmFromString(values_str.toUtf8());
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setContourResolutionMm(double resolutionMm)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetContourResolutionMm(resolutionMm);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setContourOpacityPercent(double opacityPercent)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetContourOpacity(opacityPercent*0.01);
}


//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGlyphVisualizationMode(bool activate)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!activate)
    {
    return;
    }
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GLYPH);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setGridVisualizationMode(bool activate)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!activate)
    {
    return;
    }
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_GRID);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setContourVisualizationMode(bool activate)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!activate)
    {
    return;
    }
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetVisualizationMode(vtkMRMLTransformDisplayNode::VIS_MODE_CONTOUR);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setEditorVisibility(bool enabled)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetEditorVisibility(enabled);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setEditorTranslationEnabled(bool enabled)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetEditorTranslationEnabled(enabled);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setEditorRotationEnabled(bool enabled)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetEditorRotationEnabled(enabled);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setEditorScalingEnabled(bool enabled)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetEditorScalingEnabled(enabled);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setVisibility(bool visible)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetVisibility(visible);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setVisibility2d(bool visible)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetVisibility2D(visible);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setVisibility3d(bool visible)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetVisibility3D(visible);
}

//-----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::setColorTableNode(vtkMRMLNode* colorTableNode)
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetAndObserveColorNodeID(colorTableNode?colorTableNode->GetID():nullptr);
}

// ----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::colorUpdateRange()
{
  Q_D(qMRMLTransformDisplayNodeWidget);

  if (!d->TransformDisplayNode)
    {
    return;
    }
  // Rescale the chart so that all the points are visible
  vtkColorTransferFunction* colorMap=d->TransformDisplayNode->GetColorMap();
  if (colorMap==nullptr)
    {
    return;
    }
  double range[2] = {0.0, 10.0};
  colorMap->GetRange(range);
  double chartBounds[8] = {0};
  d->ColorMapWidget->view()->chartBounds(chartBounds);
  chartBounds[2] = 0;
  chartBounds[3] = range[1]*1.1;
  d->ColorMapWidget->view()->setChartUserBounds(chartBounds);
  d->ColorMapWidget->view()->update();
}

// ----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::onColorInteractionEvent()
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  colorUpdateRange();
}

// ----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::onColorModifiedEvent()
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->SetColorMap(d->ColorTransferFunction);
}

// ----------------------------------------------------------------------------
void qMRMLTransformDisplayNodeWidget::updateEditorBounds()
{
  Q_D(qMRMLTransformDisplayNodeWidget);
  if (!d->TransformDisplayNode)
    {
    return;
    }
  d->TransformDisplayNode->UpdateEditorBounds();
}
