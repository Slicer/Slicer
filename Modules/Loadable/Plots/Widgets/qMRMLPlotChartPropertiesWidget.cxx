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

namespace
{
//----------------------------------------------------------------------------
template <typename T> std::string NumberToString(T V)
{
  std::string stringValue;
  std::stringstream strstream;
  strstream << V;
  strstream >> stringValue;
  return stringValue;
}

//----------------------------------------------------------------------------
std::string DoubleToString(double Value)
{
  return NumberToString<double>(Value);
}

}// end namespace

//--------------------------------------------------------------------------
// qMRMLPlotViewViewPrivate methods

//---------------------------------------------------------------------------
qMRMLPlotChartPropertiesWidgetPrivate::qMRMLPlotChartPropertiesWidgetPrivate(qMRMLPlotChartPropertiesWidget& object)
  : q_ptr(&object)
{
  this->PlotChartNode = 0;
}

//---------------------------------------------------------------------------
qMRMLPlotChartPropertiesWidgetPrivate::~qMRMLPlotChartPropertiesWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  Q_Q(qMRMLPlotChartPropertiesWidget);

  this->Ui_qMRMLPlotChartPropertiesWidget::setupUi(widget);

  // PlotChart Properties
  this->connect(this->fontTypeComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onFontTypeChanged(const QString&)));
  this->connect(this->titleFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onTitleFontSizeChanged(double)));
  this->connect(this->axisTitleFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onAxisTitleFontSizeChanged(double)));
  this->connect(this->axisLabelFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onAxisLabelFontSizeChanged(double)));

  QObject::connect(this->titleLineEdit, SIGNAL(textEdited(const QString&)),
    q, SLOT(setTitle(const QString&)));
  QObject::connect(this->xAxisLabelLineEdit, SIGNAL(textEdited(const QString&)),
    q, SLOT(setXAxisLabel(const QString&)));
  QObject::connect(this->yAxisLabelLineEdit, SIGNAL(textEdited(const QString&)),
    q, SLOT(setYAxisLabel(const QString&)));
  this->connect(this->legendVisibleCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(legendVisibility(bool)));
  this->connect(this->gridVisibleCheckBox, SIGNAL(toggled(bool)),
    q, SLOT(gridVisibility(bool)));

  this->connect(this->plotSeriesComboBox, SIGNAL(checkedNodesChanged()), this, SLOT(onPlotSeriesNodesSelected()));
  this->connect(this->plotSeriesComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)), this, SLOT(onPlotSeriesNodeAdded(vtkMRMLNode*)));

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->plotSeriesComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLPlotChartPropertiesWidget);

  q->setEnabled(this->PlotChartNode != 0 && q->mrmlScene() != NULL);

  if (!this->PlotChartNode || !q->mrmlScene())
    {
    this->titleLineEdit->clear();
    this->xAxisLabelLineEdit->clear();
    this->yAxisLabelLineEdit->clear();

    this->fontTypeComboBox->setCurrentIndex(-1);
    this->titleFontSizeDoubleSpinBox->setValue(20);
    this->axisTitleFontSizeDoubleSpinBox->setValue(16);
    this->axisLabelFontSizeDoubleSpinBox->setValue(12);

    bool plotBlockSignals = this->plotSeriesComboBox->blockSignals(true);
    for (int idx = 0; idx < this->plotSeriesComboBox->nodeCount(); idx++)
      {
      this->plotSeriesComboBox->setCheckState(this->plotSeriesComboBox->nodeFromIndex(idx), Qt::Unchecked);
      }
    this->plotSeriesComboBox->blockSignals(plotBlockSignals);

    return;
    }

  this->fontTypeComboBox->setCurrentIndex(this->fontTypeComboBox->findText(
    this->PlotChartNode->GetFontType() ? this->PlotChartNode->GetFontType() : ""));

  this->titleFontSizeDoubleSpinBox->setValue(this->PlotChartNode->GetTitleFontSize());
  this->axisTitleFontSizeDoubleSpinBox->setValue(this->PlotChartNode->GetAxisTitleFontSize());
  this->axisLabelFontSizeDoubleSpinBox->setValue(this->PlotChartNode->GetAxisLabelFontSize());

  // Titles, axis labels (checkboxes AND text widgets)
  this->titleLineEdit->setText(this->PlotChartNode->GetTitle() ? this->PlotChartNode->GetTitle() : "");
  this->xAxisLabelLineEdit->setText(this->PlotChartNode->GetXAxisTitle() ? this->PlotChartNode->GetXAxisTitle() : "");
  this->yAxisLabelLineEdit->setText(this->PlotChartNode->GetYAxisTitle() ? this->PlotChartNode->GetYAxisTitle() : "");

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
    if (plotSeriesNode == NULL)
      {
      continue;
      }
    this->plotSeriesComboBox->setCheckState(plotSeriesNode, Qt::Checked);
    }
  this->plotSeriesComboBox->blockSignals(plotBlockSignals);

}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::onFontTypeChanged(const QString &type)
{
  if (!this->PlotChartNode)
    {
    return;
    }
  this->PlotChartNode->SetFontType(type.toStdString().c_str());
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::onTitleFontSizeChanged(double size)
{
  if (!this->PlotChartNode)
    {
    return;
    }
  this->PlotChartNode->SetTitleFontSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::onAxisTitleFontSizeChanged(double size)
{
  if (!this->PlotChartNode)
    {
    return;
    }
  this->PlotChartNode->SetAxisTitleFontSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidgetPrivate::onAxisLabelFontSizeChanged(double size)
{
  if (!this->PlotChartNode)
    {
    return;
    }
  this->PlotChartNode->SetAxisLabelFontSize(size);
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
qMRMLPlotChartPropertiesWidget::~qMRMLPlotChartPropertiesWidget()
{
}

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
void qMRMLPlotChartPropertiesWidget::gridVisibility(bool show)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetGridVisibility(show);
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::legendVisibility(bool show)
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
  d->PlotChartNode->SetTitle(str.toLatin1().constData());
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setXAxisLabel(const QString &str)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetXAxisTitle(str.toLatin1().constData());
}

//---------------------------------------------------------------------------
void qMRMLPlotChartPropertiesWidget::setYAxisLabel(const QString &str)
{
  Q_D(qMRMLPlotChartPropertiesWidget);
  if (!d->PlotChartNode)
  {
    return;
  }
  d->PlotChartNode->SetYAxisTitle(str.toLatin1().constData());
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
