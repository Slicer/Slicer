/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// Qt includes
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QLineEdit>

// qMRML includes
#include "qMRMLPlotChartPropertiesWidget.h"
#include "qMRMLPlotChartPropertiesWidget_p.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotSeriesNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkPen.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>

// stream includes
#include <sstream>

//--------------------------------------------------------------------------
// qMRMLPlotViewViewPrivate methods

//---------------------------------------------------------------------------
qMRMLPlotChartPropertiesWidgetPrivate::qMRMLPlotChartPropertiesWidgetPrivate(qMRMLPlotChartPropertiesWidget& object)
  : q_ptr(&object)
{
  this->PlotChartNode = nullptr;
}

//---------------------------------------------------------------------------
qMRMLPlotChartPropertiesWidgetPrivate::~qMRMLPlotChartPropertiesWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  Q_Q(qMRMLPlotChartPropertiesWidget);

  this->Ui_qMRMLPlotChartPropertiesWidget::setupUi(widget);

  // PlotChart Properties
  this->connect(this->fontTypeComboBox, SIGNAL(currentIndexChanged(const QString&)),
    q, SLOT(setFontType(const QString&)));
  this->connect(this->titleFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setTitleFontSize(double)));
  this->connect(this->legendFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setLegendFontSize(double)));
  this->connect(this->axisTitleFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setAxisTitleFontSize(double)));
  this->connect(this->axisLabelFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setAxisLabelFontSize(double)));

  QObject::connect(this->titleLineEdit, SIGNAL(textEdited(const QString&)),
    q, SLOT(setTitle(const QString&)));
  QObject::connect(this->xAxisLabelLineEdit, SIGNAL(textEdited(const QString&)),
    q, SLOT(setXAxisLabel(const QString&)));
  QObject::connect(this->yAxisLabelLineEdit, SIGNAL(textEdited(const QString&)),
    q, SLOT(setYAxisLabel(const QString&)));
  this->connect(this->legendVisibleCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setLegendVisibility(bool)));
  this->connect(this->gridVisibleCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setGridVisibility(bool)));

  this->connect(this->xAxisManualRangeCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setXAxisManualRangeEnabled(bool)));
  this->connect(this->xAxisRangeMinDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setXAxisRangeMin(double)));
  this->connect(this->xAxisRangeMaxDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setXAxisRangeMax(double)));

  this->connect(this->yAxisManualRangeCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setYAxisManualRangeEnabled(bool)));
  this->connect(this->yAxisRangeMinDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setYAxisRangeMin(double)));
  this->connect(this->yAxisRangeMaxDoubleSpinBox, SIGNAL(valueChanged(double)),
    q, SLOT(setYAxisRangeMax(double)));

  this->connect(this->xAxisLogScaleCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setXAxisLogScale(bool)));
  this->connect(this->yAxisLogScaleCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(setYAxisLogScale(bool)));

  this->connect(this->plotSeriesComboBox, SIGNAL(checkedNodesChanged()), this, SLOT(onPlotSeriesNodesSelected()));
  this->connect(this->plotSeriesComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this, SLOT(onPlotSeriesNodeAdded(vtkMRMLNode*)));

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->plotSeriesComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLPlotChartPropertiesWidget);

  q->setEnabled(this->PlotChartNode != nullptr && q->mrmlScene() != nullptr);

  if (!this->PlotChartNode || !q->mrmlScene())
    {
    this->titleLineEdit->clear();
    this->xAxisLabelLineEdit->clear();
    this->yAxisLabelLineEdit->clear();

    this->xAxisManualRangeCheckBox->setChecked(false);
    this->yAxisManualRangeCheckBox->setChecked(false);

    this->xAxisRangeMinDoubleSpinBox->setValue(0);
    this->xAxisRangeMaxDoubleSpinBox->setValue(0);
    this->yAxisRangeMinDoubleSpinBox->setValue(0);
    this->yAxisRangeMaxDoubleSpinBox->setValue(0);

    this->fontTypeComboBox->setCurrentIndex(-1);
    this->titleFontSizeDoubleSpinBox->setValue(20);
    this->legendFontSizeDoubleSpinBox->setValue(16);
    this->axisTitleFontSizeDoubleSpinBox->setValue(16);
    this->axisLabelFontSizeDoubleSpinBox->setValue(12);

    this->legendVisibleCheckBox->setChecked(false);
    this->gridVisibleCheckBox->setChecked(false);

    bool plotBlockSignals = this->plotSeriesComboBox->blockSignals(true);
    for (int idx = 0; idx < this->plotSeriesComboBox->nodeCount(); idx++)
      {
      this->plotSeriesComboBox->setCheckState(this->plotSeriesComboBox->nodeFromIndex(idx), Qt::Unchecked);
      }
    this->plotSeriesComboBox->blockSignals(plotBlockSignals);

    return;
    }

  this->xAxisManualRangeCheckBox->setChecked(!this->PlotChartNode->GetXAxisRangeAuto());
  if (!this->PlotChartNode->GetXAxisRangeAuto())
    {
    double* range = this->PlotChartNode->GetXAxisRange();
    this->xAxisRangeMinDoubleSpinBox->setValue(range[0]);
    this->xAxisRangeMaxDoubleSpinBox->setValue(range[1]);
    }
  else
    {
    bool wasBlocked = this->xAxisRangeMinDoubleSpinBox->blockSignals(true);
    this->xAxisRangeMinDoubleSpinBox->setValue(0);
    this->xAxisRangeMinDoubleSpinBox->blockSignals(wasBlocked);
    wasBlocked = this->xAxisRangeMaxDoubleSpinBox->blockSignals(true);
    this->xAxisRangeMaxDoubleSpinBox->setValue(0);
    this->xAxisRangeMaxDoubleSpinBox->blockSignals(wasBlocked);
    }

  this->yAxisManualRangeCheckBox->setChecked(!this->PlotChartNode->GetYAxisRangeAuto());
  if (!this->PlotChartNode->GetYAxisRangeAuto())
    {
    double* range = this->PlotChartNode->GetYAxisRange();
    this->yAxisRangeMinDoubleSpinBox->setValue(range[0]);
    this->yAxisRangeMaxDoubleSpinBox->setValue(range[1]);
    }
  else
    {
    bool wasBlocked = this->yAxisRangeMinDoubleSpinBox->blockSignals(true);
    this->yAxisRangeMinDoubleSpinBox->setValue(0);
    this->yAxisRangeMinDoubleSpinBox->blockSignals(wasBlocked);
    wasBlocked = this->yAxisRangeMaxDoubleSpinBox->blockSignals(true);
    this->yAxisRangeMaxDoubleSpinBox->setValue(0);
    this->yAxisRangeMaxDoubleSpinBox->blockSignals(wasBlocked);
    }

  bool blockedLogScale = this->xAxisLogScaleCheckBox->blockSignals(true);
  this->xAxisLogScaleCheckBox->setChecked(this->PlotChartNode->GetXAxisLogScale());
  this->xAxisLogScaleCheckBox->blockSignals(blockedLogScale);
  blockedLogScale = this->yAxisLogScaleCheckBox->blockSignals(true);
  this->yAxisLogScaleCheckBox->setChecked(this->PlotChartNode->GetYAxisLogScale());
  this->yAxisLogScaleCheckBox->blockSignals(blockedLogScale);


  this->fontTypeComboBox->setCurrentIndex(this->fontTypeComboBox->findText(
    this->PlotChartNode->GetFontType() ? this->PlotChartNode->GetFontType() : ""));

  this->titleFontSizeDoubleSpinBox->setValue(this->PlotChartNode->GetTitleFontSize());
  this->legendFontSizeDoubleSpinBox->setValue(this->PlotChartNode->GetLegendFontSize());
  this->axisTitleFontSizeDoubleSpinBox->setValue(this->PlotChartNode->GetAxisTitleFontSize());
  this->axisLabelFontSizeDoubleSpinBox->setValue(this->PlotChartNode->GetAxisLabelFontSize());

  // Titles, axis labels
  this->titleLineEdit->setText(this->PlotChartNode->GetTitle() ? this->PlotChartNode->GetTitle() : "");
  this->xAxisLabelLineEdit->setText(this->PlotChartNode->GetXAxisTitle() ? this->PlotChartNode->GetXAxisTitle() : "");
  this->yAxisLabelLineEdit->setText(this->PlotChartNode->GetYAxisTitle() ? this->PlotChartNode->GetYAxisTitle() : "");

  this->legendVisibleCheckBox->setChecked(this->PlotChartNode->GetLegendVisibility());
  this->gridVisibleCheckBox->setChecked(this->PlotChartNode->GetGridVisibility());

    // Plot series selector
  bool plotBlockSignals = this->plotSeriesComboBox->blockSignals(true);
  for (int idx = 0; idx < this->plotSeriesComboBox->nodeCount(); idx++)
    {
    vtkMRMLNode* node = this->plotSeriesComboBox->nodeFromIndex(idx);
    this->plotSeriesComboBox->setCheckState(node, Qt::Unchecked);
    }
  std::vector<std::string> plotSeriesNodesIDs;
  this->PlotChartNode->GetPlotSeriesNodeIDs(plotSeriesNodesIDs);
  for (std::vector<std::string>::iterator it = plotSeriesNodesIDs.begin();
    it != plotSeriesNodesIDs.end(); ++it)
    {
    vtkMRMLPlotSeriesNode *plotSeriesNode = vtkMRMLPlotSeriesNode::SafeDownCast
      (q->mrmlScene()->GetNodeByID((*it).c_str()));
    if (plotSeriesNode == nullptr)
      {
      continue;
      }
    this->plotSeriesComboBox->setCheckState(plotSeriesNode, Qt::Checked);
    }
  this->plotSeriesComboBox->blockSignals(plotBlockSignals);

}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setFontType(const QString &type)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetFontType(type.toStdString().c_str());
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setTitleFontSize(double size)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetTitleFontSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setLegendFontSize(double size)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetLegendFontSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setAxisTitleFontSize(double size)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetAxisTitleFontSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setAxisLabelFontSize(double size)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetAxisLabelFontSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::onPlotSeriesNodesSelected()
{
  if (!this->PlotChartNode)
    {
    return;
    }

  std::vector<std::string> plotSeriesNodesIDs;
  this->PlotChartNode->GetPlotSeriesNodeIDs(plotSeriesNodesIDs);

  // loop over arrays in the widget
  for (int idx = 0; idx < this->plotSeriesComboBox->nodeCount(); idx++)
    {
    vtkMRMLPlotSeriesNode *dn = vtkMRMLPlotSeriesNode::SafeDownCast(this->plotSeriesComboBox->nodeFromIndex(idx));

    bool checked = (this->plotSeriesComboBox->checkState(dn) == Qt::Checked);

    // is the node in the Plot?
    bool found = false;
    std::vector<std::string>::iterator it = plotSeriesNodesIDs.begin();
    for (; it != plotSeriesNodesIDs.end(); ++it)
      {
      if (!strcmp(dn->GetID(), (*it).c_str()))
        {
        if (!checked)
          {
          // plot is not checked but currently in the LayoutPlot, remove it
          // (might want to cache the old name in case user adds it back)
          this->PlotChartNode->RemovePlotSeriesNodeID((*it).c_str());
          }
        found = true;
        break;
        }
      }
    if (!found)
      {
      if (checked)
        {
        // plot is checked but not currently in the LayoutPlot, add it
        this->PlotChartNode->AddAndObservePlotSeriesNodeID(dn->GetID());
        }
      }
  }
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::onPlotSeriesNodeAdded(vtkMRMLNode *node)
{
  Q_Q(qMRMLPlotChartPropertiesWidget);
  if (!this->PlotChartNode)
    {
    return;
    }
  vtkMRMLPlotSeriesNode *plotSeriesNode = vtkMRMLPlotSeriesNode::SafeDownCast(node);
  if (!plotSeriesNode)
    {
    return;
    }
  // Add the reference of the PlotSeriesNode in the active PlotChartNode
  this->PlotChartNode->AddAndObservePlotSeriesNodeID(plotSeriesNode->GetID());

  emit q->seriesNodeAddedByUser(node);
}

// --------------------------------------------------------------------------
// qMRMLPlotChartPropertiesWidget methods

// --------------------------------------------------------------------------
qMRMLPlotChartPropertiesWidget::qMRMLPlotChartPropertiesWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLPlotChartPropertiesWidgetPrivate(*this))
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qMRMLPlotChartPropertiesWidget::~qMRMLPlotChartPropertiesWidget() = default;

//---------------------------------------------------------------------------
 vtkMRMLPlotChartNode* qMRMLPlotChartPropertiesWidget::mrmlPlotChartNode()const
{
  Q_D(const qMRMLPlotChartPropertiesWidget);
  return d->PlotChartNode;
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setMRMLPlotChartNode(vtkMRMLNode* node)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  vtkMRMLPlotChartNode* plotChartNode = vtkMRMLPlotChartNode::SafeDownCast(node);
  this->setMRMLPlotChartNode(plotChartNode);
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setMRMLPlotChartNode(vtkMRMLPlotChartNode* plotChartNode)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (plotChartNode == d->PlotChartNode)
    {
    return;
    }

  d->qvtkReconnect(d->PlotChartNode, plotChartNode, vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));
  d->PlotChartNode = plotChartNode;

  d->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setGridVisibility(bool show)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetGridVisibility(show);
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setLegendVisibility(bool show)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetLegendVisibility(show);
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setTitle(const QString &str)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetTitle(str.toUtf8().constData());
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setXAxisLabel(const QString &str)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetXAxisTitle(str.toUtf8().constData());
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setYAxisLabel(const QString &str)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetYAxisTitle(str.toUtf8().constData());
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLPlotChartPropertiesWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }

  // Disable the node selectors as they would fire signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means
  // that the current node was not in the combo box list menu before.
  bool plotBlockSignals = d->plotSeriesComboBox->blockSignals(true);
  this->Superclass::setMRMLScene(newScene);
  d->plotSeriesComboBox->blockSignals(plotBlockSignals);

  Superclass::setMRMLScene(newScene);

   d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent,
                    d, SLOT(updateWidgetFromMRML()));

  if (this->mrmlScene())
    {
    d->updateWidgetFromMRML();
    }
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setXAxisManualRangeEnabled(bool manual)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetXAxisRangeAuto(!manual);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setXAxisRangeMin(double rangeMin)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  double range[2] = { 0 };
  d->PlotChartNode->GetXAxisRange(range);
  range[0] = rangeMin;
  d->PlotChartNode->SetXAxisRange(range);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setXAxisRangeMax(double rangeMax)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  double range[2] = { 0 };
  d->PlotChartNode->GetXAxisRange(range);
  range[1] = rangeMax;
  d->PlotChartNode->SetXAxisRange(range);
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setYAxisManualRangeEnabled(bool manual)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetYAxisRangeAuto(!manual);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setYAxisRangeMin(double rangeMin)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  double range[2] = { 0 };
  d->PlotChartNode->GetYAxisRange(range);
  range[0] = rangeMin;
  d->PlotChartNode->SetYAxisRange(range);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setYAxisRangeMax(double rangeMax)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  double range[2] = { 0 };
  d->PlotChartNode->GetYAxisRange(range);
  range[1] = rangeMax;
  d->PlotChartNode->SetYAxisRange(range);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setXAxisLogScale(bool logScale)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetXAxisLogScale(logScale);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setYAxisLogScale(bool logScale)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
    {
    return;
    }
  d->PlotChartNode->SetYAxisLogScale(logScale);
}
