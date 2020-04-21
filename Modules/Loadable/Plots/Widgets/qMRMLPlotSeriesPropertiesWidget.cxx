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
#include "qMRMLPlotSeriesPropertiesWidget_p.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotSeriesNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkPen.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>

// stream includes
#include <sstream>

//--------------------------------------------------------------------------
// qMRMLPlotSeriesPropertiesWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLPlotSeriesPropertiesWidgetPrivate::qMRMLPlotSeriesPropertiesWidgetPrivate(qMRMLPlotSeriesPropertiesWidget& object)
  : q_ptr(&object)
{
  this->PlotSeriesNode = nullptr;
}

//---------------------------------------------------------------------------
qMRMLPlotSeriesPropertiesWidgetPrivate::~qMRMLPlotSeriesPropertiesWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  Q_Q(qMRMLPlotSeriesPropertiesWidget);

  this->Ui_qMRMLPlotSeriesPropertiesWidget::setupUi(widget);

  this->connect(this->inputTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                this, SLOT(onInputTableNodeChanged(vtkMRMLNode*)));
  this->connect(this->xAxisComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onXAxisChanged(int)));
  this->connect(this->labelsComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onLabelsChanged(int)));
  this->connect(this->yAxisComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onYAxisChanged(int)));
  this->connect(this->plotTypeComboBox, SIGNAL(currentIndexChanged(int)),
                this, SLOT(onPlotTypeChanged(int)));
  this->connect(this->markersStyleComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onMarkersStyleChanged(const QString&)));
  this->connect(this->markersSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onMarkersSizeChanged(double)));
  this->connect(this->lineStyleComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onLineStyleChanged(const QString&)));
  this->connect(this->lineWidthDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onLineWidthChanged(double)));
  this->connect(this->plotSeriesColorPickerButton, SIGNAL(colorChanged(const QColor&)),
                this, SLOT(onPlotSeriesColorChanged(const QColor&)));
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLPlotSeriesPropertiesWidget);

  q->setEnabled(this->PlotSeriesNode != nullptr);

  if (!this->PlotSeriesNode)
    {
    this->inputTableComboBox->setCurrentNode(nullptr);
    this->xAxisComboBox->clear();
    this->labelsComboBox->clear();
    this->yAxisComboBox->clear();
    this->plotTypeComboBox->setCurrentIndex(0);
    this->markersStyleComboBox->setCurrentIndex(0);
    this->plotSeriesColorPickerButton->setColor(QColor(127,127,127));
    return;
    }

  // Update the TableNode ComboBox
  vtkMRMLTableNode* mrmlTableNode = this->PlotSeriesNode->GetTableNode();
  this->inputTableComboBox->setCurrentNode(mrmlTableNode);

  // Update the xAxis and yAxis ComboBoxes
  bool xAxisBlockSignals = this->xAxisComboBox->blockSignals(true);
  bool labelsBlockSignals = this->labelsComboBox->blockSignals(true);
  bool yAxisBlockSignals = this->yAxisComboBox->blockSignals(true);

  bool xColumnRequired = this->PlotSeriesNode->IsXColumnRequired();

  this->xAxisComboBox->clear();
  this->labelsComboBox->clear();
  this->yAxisComboBox->clear();
  if (mrmlTableNode)
    {
    if (this->labelsComboBox->findData(QString()) == -1)
      {
      this->labelsComboBox->addItem("(none)", QString());
      }
    for (int columnIndex = 0; columnIndex < mrmlTableNode->GetNumberOfColumns(); columnIndex++)
      {
      std::string columnName = mrmlTableNode->GetColumnName(columnIndex);
      int columnType = mrmlTableNode->GetColumnType(columnName);
      if (columnType == VTK_STRING)
        {
        if (this->labelsComboBox->findData(QString(columnName.c_str())) == -1)
          {
          this->labelsComboBox->addItem(columnName.c_str(), QString(columnName.c_str()));
          }
        }
      else if (columnType != VTK_BIT)
        {
        if (xColumnRequired && this->xAxisComboBox->findData(QString(columnName.c_str())) == -1)
          {
          this->xAxisComboBox->addItem(columnName.c_str(), QString(columnName.c_str()));
          }
        if (this->yAxisComboBox->findData(QString(columnName.c_str())) == -1)
          {
          this->yAxisComboBox->addItem(columnName.c_str(), QString(columnName.c_str()));
          }
        }
      }
    }

  if (xColumnRequired)
    {
    std::string xAxisName = this->PlotSeriesNode->GetXColumnName();
    int xAxisIndex = this->xAxisComboBox->findData(QString(xAxisName.c_str()));
    if (xAxisIndex < 0)
      {
      this->xAxisComboBox->addItem(xAxisName.c_str(), QString(xAxisName.c_str()));
      xAxisIndex = this->xAxisComboBox->findData(QString(xAxisName.c_str()));
      }
    this->xAxisComboBox->setCurrentIndex(xAxisIndex);
    this->xAxisComboBox->setToolTip("");
    }
  else
    {
    this->xAxisComboBox->addItem("(Indexes)", QString());
    this->xAxisComboBox->setCurrentIndex(0);
    this->xAxisComboBox->setToolTip(tr("This plot type uses indexes as X axis values. Switch to scatter plot type to allow column selection."));
    }

  std::string labelsName = this->PlotSeriesNode->GetLabelColumnName();
  int labelsIndex = this->labelsComboBox->findData(QString(labelsName.c_str()));
  if (labelsIndex < 0)
    {
    this->labelsComboBox->addItem(labelsName.c_str(), QString(labelsName.c_str()));
    labelsIndex = this->labelsComboBox->findData(QString(labelsName.c_str()));
    }
  this->labelsComboBox->setCurrentIndex(labelsIndex);
  this->labelsComboBox->setEnabled(mrmlTableNode != nullptr);

  std::string yAxisName = this->PlotSeriesNode->GetYColumnName();
  int yAxisIndex = this->yAxisComboBox->findData(QString(yAxisName.c_str()));
  if (yAxisIndex < 0)
    {
    this->yAxisComboBox->addItem(yAxisName.c_str(), QString(yAxisName.c_str()));
    yAxisIndex = this->yAxisComboBox->findData(QString(yAxisName.c_str()));
    }
  this->yAxisComboBox->setCurrentIndex(yAxisIndex);

  this->xAxisComboBox->blockSignals(xAxisBlockSignals);
  this->labelsComboBox->blockSignals(labelsBlockSignals);
  this->yAxisComboBox->blockSignals(yAxisBlockSignals);
  this->xAxisComboBox->setEnabled(mrmlTableNode != nullptr && this->PlotSeriesNode->IsXColumnRequired());

  this->yAxisComboBox->setEnabled(mrmlTableNode != nullptr);

  // Update the PlotType ComboBox
  bool wasBlocked = this->plotTypeComboBox->blockSignals(true);
  this->plotTypeComboBox->setCurrentIndex(this->PlotSeriesNode->GetPlotType());
  this->plotTypeComboBox->blockSignals(wasBlocked);

  // Update Markers Style
  wasBlocked = this->markersStyleComboBox->blockSignals(true);
  const char* plotMarkersStyle = this->PlotSeriesNode->GetMarkerStyleAsString(this->PlotSeriesNode->GetMarkerStyle());
  // After Qt5 migration, the next line can be replaced by this call:
  // this->markersStyleComboBox->setCurrentText(plotMarkersStyle);
  this->markersStyleComboBox->setCurrentIndex(this->markersStyleComboBox->findText(plotMarkersStyle));
  this->markersStyleComboBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeLine);
  this->markersStyleComboBox->blockSignals(wasBlocked);

  // Update Markers Size
  wasBlocked = this->markersSizeDoubleSpinBox->blockSignals(true);
  this->markersSizeDoubleSpinBox->setValue(this->PlotSeriesNode->GetMarkerSize());
  this->markersSizeDoubleSpinBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeLine);
  this->markersSizeDoubleSpinBox->blockSignals(wasBlocked);

  // Update Line Style
  wasBlocked = this->lineStyleComboBox->blockSignals(true);
  const char* plotLineStyle = this->PlotSeriesNode->GetLineStyleAsString(this->PlotSeriesNode->GetLineStyle());
  // After Qt5 migration, the next line can be replaced by this call:
  // this->markersStyleComboBox->setCurrentText(plotMarkersStyle);
  this->lineStyleComboBox->setCurrentIndex(this->lineStyleComboBox->findText(plotLineStyle));
  this->lineStyleComboBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeLine);
  this->lineStyleComboBox->blockSignals(wasBlocked);

  // Update Line Width
  wasBlocked = this->lineWidthDoubleSpinBox->blockSignals(true);
  this->lineWidthDoubleSpinBox->setValue(this->PlotSeriesNode->GetLineWidth());
  this->lineWidthDoubleSpinBox->setEnabled(
    this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeScatter
    || this->PlotSeriesNode->GetPlotType() == vtkMRMLPlotSeriesNode::PlotTypeLine);
  this->lineWidthDoubleSpinBox->blockSignals(wasBlocked);

  // Update PlotSeriesColorPickerButton
  wasBlocked = this->plotSeriesColorPickerButton->blockSignals(true);
  double* rgb = this->PlotSeriesNode->GetColor();
  QColor color;
  color.setRedF(rgb[0]);
  color.setGreenF(rgb[1]);
  color.setBlueF(rgb[2]);
  color.setAlphaF(this->PlotSeriesNode->GetOpacity());
  this->plotSeriesColorPickerButton->setColor(color);
  this->plotSeriesColorPickerButton->blockSignals(wasBlocked);
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onPlotSeriesNodeChanged(vtkMRMLNode *node)
{
  vtkMRMLPlotSeriesNode *mrmlPlotSeriesNode = vtkMRMLPlotSeriesNode::SafeDownCast(node);

  if (this->PlotSeriesNode == mrmlPlotSeriesNode)
    {
    return;
    }

  this->qvtkReconnect(this->PlotSeriesNode, mrmlPlotSeriesNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRMLPlotSeriesNode()));

  this->PlotSeriesNode = mrmlPlotSeriesNode;

  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onInputTableNodeChanged(vtkMRMLNode *node)
{
  vtkMRMLTableNode *mrmlTableNode = vtkMRMLTableNode::SafeDownCast(node);

  if (!this->PlotSeriesNode || this->PlotSeriesNode->GetTableNode() == mrmlTableNode)
    {
    return;
    }

  if (mrmlTableNode)
    {
    this->PlotSeriesNode->SetAndObserveTableNodeID(mrmlTableNode->GetID());
    }
  else
    {
    this->PlotSeriesNode->SetAndObserveTableNodeID(nullptr);
    }
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onXAxisChanged(int index)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetXColumnName(this->xAxisComboBox->itemData(index).toString().toUtf8().constData());
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onLabelsChanged(int index)
{
  if (!this->PlotSeriesNode)
  {
    return;
  }

  this->PlotSeriesNode->SetLabelColumnName(this->labelsComboBox->itemData(index).toString().toUtf8().constData());
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onYAxisChanged(int index)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetYColumnName(this->yAxisComboBox->itemData(index).toString().toUtf8().constData());
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onPlotTypeChanged(int index)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetPlotType(index);
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onMarkersStyleChanged(const QString &style)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetMarkerStyle(this->PlotSeriesNode->
    GetMarkerStyleFromString(style.toStdString().c_str()));
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onMarkersSizeChanged(double size)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetMarkerSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onLineStyleChanged(const QString &style)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetLineStyle(this->PlotSeriesNode->
    GetLineStyleFromString(style.toStdString().c_str()));
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onLineWidthChanged(double width)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }

  this->PlotSeriesNode->SetLineWidth(width);
}

// --------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidgetPrivate::onPlotSeriesColorChanged(const QColor & color)
{
  if (!this->PlotSeriesNode)
    {
    return;
    }
  double rgb[3] = { color.redF() , color.greenF() , color.blueF() };
  this->PlotSeriesNode->SetColor(rgb);
  this->PlotSeriesNode->SetOpacity(color.alphaF());
}

// --------------------------------------------------------------------------
// qMRMLPlotViewView methods

// --------------------------------------------------------------------------
qMRMLPlotSeriesPropertiesWidget::qMRMLPlotSeriesPropertiesWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLPlotSeriesPropertiesWidgetPrivate(*this))
{
  Q_D(qMRMLPlotSeriesPropertiesWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qMRMLPlotSeriesPropertiesWidget::~qMRMLPlotSeriesPropertiesWidget() = default;


//---------------------------------------------------------------------------
 vtkMRMLPlotSeriesNode* qMRMLPlotSeriesPropertiesWidget::mrmlPlotSeriesNode()const
{
  Q_D(const qMRMLPlotSeriesPropertiesWidget);
  return d->PlotSeriesNode;
}

//---------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidget::setMRMLPlotSeriesNode(vtkMRMLNode* node)
{
  Q_D(qMRMLPlotSeriesPropertiesWidget);
  vtkMRMLPlotSeriesNode* plotSeriesNode = vtkMRMLPlotSeriesNode::SafeDownCast(node);
  this->setMRMLPlotSeriesNode(plotSeriesNode);
}

//---------------------------------------------------------------------------
void qMRMLPlotSeriesPropertiesWidget::setMRMLPlotSeriesNode(vtkMRMLPlotSeriesNode* plotSeriesNode)
{
  Q_D(qMRMLPlotSeriesPropertiesWidget);
  if (plotSeriesNode == d->PlotSeriesNode)
    {
    return;
    }

  d->qvtkReconnect(d->PlotSeriesNode, plotSeriesNode, vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));
  d->PlotSeriesNode = plotSeriesNode;

  d->updateWidgetFromMRML();
}
