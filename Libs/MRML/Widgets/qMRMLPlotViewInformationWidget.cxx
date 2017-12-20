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
#include "qMRMLPlotViewInformationWidget_p.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotDataNode.h>
#include <vtkMRMLPlotViewNode.h>
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
qMRMLPlotViewInformationWidgetPrivate::qMRMLPlotViewInformationWidgetPrivate(qMRMLPlotViewInformationWidget& object)
  : q_ptr(&object)
{
  this->PlotViewNode = 0;
  this->PlotDataNode = 0;
}

//---------------------------------------------------------------------------
qMRMLPlotViewInformationWidgetPrivate::~qMRMLPlotViewInformationWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::setupUi(qMRMLWidget* widget)
{
  Q_Q(qMRMLPlotViewInformationWidget);

  this->Ui_qMRMLPlotViewInformationWidget::setupUi(widget);

  this->connect(this->viewGroupSpinBox, SIGNAL(valueChanged(int)),
                q, SLOT(setViewGroup(int)));

  // PlotChart Properties
  this->connect(this->colorNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                this, SLOT(onColorNodeChanged(vtkMRMLNode*)));
  this->connect(this->fontTypeComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onFontTypeChanged(const QString&)));
  this->connect(this->titleFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onTitleFontSizeChanged(double)));
  this->connect(this->axisTitleFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onAxisTitleFontSizeChanged(double)));
  this->connect(this->axisLabelFontSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onAxisLabelFontSizeChanged(double)));
  this->connect(this->clickAndDragXCheckBox, SIGNAL(toggled(bool)),
                this, SLOT(activateClickAndDragX(bool)));
  this->connect(this->clickAndDragYCheckBox, SIGNAL(toggled(bool)),
                this, SLOT(activateClickAndDragY(bool)));

  // PlotData Properties
  this->connect(this->plotDataNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                this, SLOT(onPlotDataNodeChanged(vtkMRMLNode*)));
  this->connect(this->InputTableComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                this, SLOT(onInputTableNodeChanged(vtkMRMLNode*)));
  this->connect(this->xAxisComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onXAxisChanged(const QString&)));
  this->connect(this->yAxisComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onYAxisChanged(const QString&)));
  this->connect(this->plotTypeComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onPlotTypeChanged(const QString&)));
  this->connect(this->markersStyleComboBox, SIGNAL(currentIndexChanged(const QString&)),
                this, SLOT(onMarkersStyleChanged(const QString&)));
  this->connect(this->markersSizeDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onMarkersSizeChanged(double)));
  this->connect(this->lineWidthDoubleSpinBox, SIGNAL(valueChanged(double)),
                this, SLOT(onLineWidthChanged(double)));
  this->connect(this->plotDataColorPickerButton, SIGNAL(colorChanged(const QColor&)),
                this, SLOT(onPlotDataColorChanged(const QColor&)));
  this->connect(this->copyPlotDataNodePushButton, SIGNAL(clicked()),
                this, SLOT(onCopyPlotDataNodeClicked()));

}

// --------------------------------------------------------------------------
vtkMRMLPlotChartNode *qMRMLPlotViewInformationWidgetPrivate::GetPlotChartNodeFromView()
{
  Q_Q(qMRMLPlotViewInformationWidget);

  if (!this->PlotViewNode || !q->mrmlScene())
    {
    // qDebug() << "No PlotViewNode or no Scene";
    return NULL;
    }

  // Get the current PlotChart node
  vtkMRMLPlotChartNode *PlotChartNodeFromViewNode =
    vtkMRMLPlotChartNode::SafeDownCast(q->mrmlScene()->GetNodeByID
      (this->PlotViewNode->GetPlotChartNodeID()));

  return PlotChartNodeFromViewNode;
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::updateWidgetFromMRMLPlotViewNode()
{
  Q_Q(qMRMLPlotViewInformationWidget);

  q->setEnabled(this->PlotViewNode != 0);
  if (!this->PlotViewNode)
    {
    return;
    }

  // Update layout name
  this->layoutNameLineEdit->setText(QLatin1String(this->PlotViewNode->GetLayoutName()));

  this->viewGroupSpinBox->setValue(this->PlotViewNode->GetViewGroup());

  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();

  if (!mrmlPlotChartNode)
    {
    this->colorNodeComboBox->setCurrentNode(NULL);
    this->fontTypeComboBox->setCurrentIndex(0);
    this->titleFontSizeDoubleSpinBox->setValue(20);
    this->axisTitleFontSizeDoubleSpinBox->setValue(16);
    this->axisLabelFontSizeDoubleSpinBox->setValue(12);
    this->clickAndDragXCheckBox->setChecked(true);
    this->clickAndDragYCheckBox->setChecked(true);
    return;
    }

  const char *AttributeValue;

  AttributeValue = mrmlPlotChartNode->GetAttribute("LookupTable");
  this->colorNodeComboBox->setCurrentNodeID(AttributeValue);

  AttributeValue = mrmlPlotChartNode->GetAttribute("FontType");
  // After Qt5 migration, the next line can be replaced by this call:
  // d->fontTypeComboBox->setCurrentText(AttributeValue);
  this->fontTypeComboBox->setCurrentIndex(this->fontTypeComboBox->findText(AttributeValue));

  AttributeValue = mrmlPlotChartNode->GetAttribute("TitleFontSize");
  this->titleFontSizeDoubleSpinBox->setValue(atof(AttributeValue));

  AttributeValue = mrmlPlotChartNode->GetAttribute("AxisTitleFontSize");
  this->axisTitleFontSizeDoubleSpinBox->setValue(atof(AttributeValue));

  AttributeValue = mrmlPlotChartNode->GetAttribute("AxisLabelFontSize");
  this->axisLabelFontSizeDoubleSpinBox->setValue(atof(AttributeValue));

  AttributeValue = mrmlPlotChartNode->GetAttribute("ClickAndDragAlongX");
  this->clickAndDragXCheckBox->setChecked(AttributeValue && !strcmp("on", AttributeValue));

  AttributeValue = mrmlPlotChartNode->GetAttribute("ClickAndDragAlongY");
  this->clickAndDragYCheckBox->setChecked(AttributeValue && !strcmp("on", AttributeValue));
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onColorNodeChanged(vtkMRMLNode *node)
{
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }

  vtkMRMLColorNode *mrmlColorNode = vtkMRMLColorNode::SafeDownCast(node);
  if (!mrmlColorNode)
    {
    return;
    }

  mrmlPlotChartNode->SetAttribute("LookupTable", mrmlColorNode->GetID());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::activateClickAndDragX(bool activate)
{
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }

  if (activate)
    {
    mrmlPlotChartNode->SetAttribute("ClickAndDragAlongX", "on");
    }
  else
    {
    mrmlPlotChartNode->SetAttribute("ClickAndDragAlongX", "off");
    }
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::activateClickAndDragY(bool activate)
{
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }

  if (activate)
    {
    mrmlPlotChartNode->SetAttribute("ClickAndDragAlongY", "on");
    }
  else
    {
    mrmlPlotChartNode->SetAttribute("ClickAndDragAlongY", "off");
    }
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onFontTypeChanged(const QString &type)
{
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }

  mrmlPlotChartNode->SetAttribute("FontType", type.toStdString().c_str());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onTitleFontSizeChanged(double size)
{
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }

  mrmlPlotChartNode->SetAttribute("TitleFontSize", (DoubleToString(size).c_str()));
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onAxisTitleFontSizeChanged(double size)
{
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }

  mrmlPlotChartNode->SetAttribute("AxisTitleFontSize", (DoubleToString(size).c_str()));
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onAxisLabelFontSizeChanged(double size)
{
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }

  mrmlPlotChartNode->SetAttribute("AxisLabelFontSize", (DoubleToString(size).c_str()));
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::updateWidgetFromMRMLPlotDataNode()
{
  Q_Q(qMRMLPlotViewInformationWidget);

  q->setEnabled(this->PlotViewNode != 0);
  if (!this->PlotViewNode)
    {
    return;
    }

  QColor color;
  color.setRed(85);
  color.setGreen(170);
  color.setBlue(255);
  color.setAlpha(255);

  if (!this->PlotDataNode)
    {
    this->plotDataNodeComboBox->setCurrentNode(NULL);
    this->InputTableComboBox->setCurrentNode(NULL);
    this->xAxisComboBox->clear();
    this->yAxisComboBox->clear();
    this->plotTypeComboBox->setCurrentIndex(0);
    this->markersStyleComboBox->setCurrentIndex(0);
    this->plotDataColorPickerButton->setColor(color);
    return;
    }

  // Update the plotDataNode ComboBox
  this->plotDataNodeComboBox->setCurrentNode(this->PlotDataNode);

  // Update the TableNode ComboBox
  vtkMRMLTableNode* mrmlTableNode = this->PlotDataNode->GetTableNode();
  this->InputTableComboBox->setCurrentNode(mrmlTableNode);

  // Update the xAxis and yAxis ComboBoxes
  bool xAxisBlockSignals = this->xAxisComboBox->blockSignals(true);
  bool yAxisBlockSignals = this->yAxisComboBox->blockSignals(true);

  this->xAxisComboBox->clear();
  this->yAxisComboBox->clear();
  if (mrmlTableNode)
    {
    if (this->xAxisComboBox->findText("Indexes") == -1)
      {
      this->xAxisComboBox->addItem("Indexes");
      }
    for (int ColumnIndex = 0; ColumnIndex < mrmlTableNode->GetNumberOfColumns(); ColumnIndex++)
      {
      std::string columnName = mrmlTableNode->GetColumnName(ColumnIndex).c_str();
      if (this->xAxisComboBox->findText(columnName.c_str()) == -1)
        {
        this->xAxisComboBox->addItem(columnName.c_str());
        }
      if (this->yAxisComboBox->findText(columnName.c_str()) == -1)
        {
        this->yAxisComboBox->addItem(columnName.c_str());
        }
      }
    }

  std::string xAxisName = this->PlotDataNode->GetXColumnName();
  int xAxisIndex = this->xAxisComboBox->findText(xAxisName.c_str());
  if (xAxisIndex < 0)
    {
    this->xAxisComboBox->addItem(xAxisName.c_str());
    xAxisIndex = this->xAxisComboBox->findText(xAxisName.c_str());
    }
  this->xAxisComboBox->setCurrentIndex(xAxisIndex);

  std::string yAxisName = this->PlotDataNode->GetYColumnName();
  int yAxisIndex = this->yAxisComboBox->findText(yAxisName.c_str());
  if (yAxisIndex < 0)
    {
    this->yAxisComboBox->addItem(yAxisName.c_str());
    yAxisIndex = this->yAxisComboBox->findText(yAxisName.c_str());
    }
  this->yAxisComboBox->setCurrentIndex(yAxisIndex);

  this->xAxisComboBox->blockSignals(xAxisBlockSignals);
  this->yAxisComboBox->blockSignals(yAxisBlockSignals);

  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (mrmlPlotChartNode)
    {
    const char *xAxis = mrmlPlotChartNode->GetAttribute("XAxis");
    if (!strcmp(xAxis, "Custom"))
      {
      this->xAxisComboBox->setEnabled(true);
      }
    else
      {
      this->xAxisComboBox->setEnabled(false);
      }
    }

  // Update the PlotType ComboBox
  const char* plotType = this->PlotDataNode->GetPlotTypeAsString(this->PlotDataNode->GetType());
  // After Qt5 migration, the next line can be replaced by this call:
  // this->plotTypeComboBox->setCurrentText(plotType);
  this->plotTypeComboBox->setCurrentIndex(this->plotTypeComboBox->findText(plotType));

  if (mrmlPlotChartNode)
    {
    const char *type = mrmlPlotChartNode->GetAttribute("Type");
    if (!strcmp(type, "Custom"))
      {
      this->plotTypeComboBox->setEnabled(true);
      }
    else
      {
      this->plotTypeComboBox->setEnabled(false);
      }
    }

  // Update Markers Style
  const char* plotMarkersStyle = this->PlotDataNode->GetMarkersStyleAsString(this->PlotDataNode->GetMarkerStyle());
  // After Qt5 migration, the next line can be replaced by this call:
  // this->markersStyleComboBox->setCurrentText(plotMarkersStyle);
  this->markersStyleComboBox->setCurrentIndex(this->markersStyleComboBox->findText(plotMarkersStyle));

  // Update Markers Size
  this->markersSizeDoubleSpinBox->setValue(this->PlotDataNode->GetMarkerSize());

  // UnEnable Markers Style and Size if the type of the plot is Bar
  vtkPlot *plot = this->PlotDataNode->GetPlot();
  vtkPlotPoints *plotPoints = vtkPlotPoints::SafeDownCast(plot);
  if (plotPoints)
    {
    this->markersStyleComboBox->setEnabled(true);
    this->markersSizeDoubleSpinBox->setEnabled(true);
    }
  else
    {
    this->markersStyleComboBox->setEnabled(false);
    this->markersSizeDoubleSpinBox->setEnabled(false);
    }

  if (mrmlPlotChartNode)
    {
    const char *markers = mrmlPlotChartNode->GetAttribute("Markers");
    if (strcmp(markers, "Custom"))
      {
      this->markersStyleComboBox->setEnabled(false);
      }
    }

  // Update Line Width
  this->lineWidthDoubleSpinBox->setValue(this->PlotDataNode->GetLineWidth());

  // UnEnable Markers Style and Size if the type of the plot is Bar
  vtkPlotLine *plotLine = vtkPlotLine::SafeDownCast(plot);
  if (plotLine)
    {
    this->lineWidthDoubleSpinBox->setEnabled(true);
    }
  else
    {
    this->lineWidthDoubleSpinBox->setEnabled(false);
    }

  // Update PlotDataColorPickerButton
  unsigned char rgba[4];
  this->PlotDataNode->GetPlotColor(rgba);
  color.setRed(static_cast<int> (rgba[0]));
  color.setGreen(static_cast<int> (rgba[1]));
  color.setBlue(static_cast<int> (rgba[2]));
  color.setAlpha(static_cast<int> (rgba[3]));
  this->plotDataColorPickerButton->setColor(color);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onPlotDataNodeChanged(vtkMRMLNode *node)
{
  vtkMRMLPlotDataNode *mrmlPlotDataNode = vtkMRMLPlotDataNode::SafeDownCast(node);

  if (this->PlotDataNode == mrmlPlotDataNode)
    {
    return;
    }

  this->qvtkReconnect(this->PlotDataNode, mrmlPlotDataNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRMLPlotDataNode()));

  this->PlotDataNode = mrmlPlotDataNode;

  this->updateWidgetFromMRMLPlotDataNode();
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onInputTableNodeChanged(vtkMRMLNode *node)
{
  vtkMRMLTableNode *mrmlTableNode = vtkMRMLTableNode::SafeDownCast(node);

  if (!this->PlotDataNode || this->PlotDataNode->GetTableNode() == mrmlTableNode)
    {
    return;
    }

  if (mrmlTableNode)
    {
    this->PlotDataNode->SetAndObserveTableNodeID(mrmlTableNode->GetID());
    }
  else
    {
    this->PlotDataNode->SetAndObserveTableNodeID("NULL");
    }
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onXAxisChanged(const QString &xAxis)
{
  if (!this->PlotDataNode)
    {
    return;
    }

  this->PlotDataNode->SetXColumnName(xAxis.toStdString());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onYAxisChanged(const QString &yAxis)
{
  if (!this->PlotDataNode)
    {
    return;
    }

  this->PlotDataNode->SetYColumnName(yAxis.toStdString());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onPlotTypeChanged(const QString &type)
{
  if (!this->PlotDataNode)
    {
    return;
    }

  this->PlotDataNode->SetType(type.toStdString().c_str());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onMarkersStyleChanged(const QString &style)
{
  if (!this->PlotDataNode)
    {
    return;
    }

  this->PlotDataNode->SetMarkerStyle(this->PlotDataNode->
    GetMarkersStyleFromString(style.toStdString().c_str()));
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onMarkersSizeChanged(double size)
{
  if (!this->PlotDataNode)
    {
    return;
    }

  this->PlotDataNode->SetMarkerSize(size);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onLineWidthChanged(double width)
{
  if (!this->PlotDataNode)
    {
    return;
    }

  this->PlotDataNode->SetLineWidth(width);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onPlotDataColorChanged(const QColor & color)
{
  if (!this->PlotDataNode || !this->PlotDataNode->GetPlot())
    {
    return;
    }

  unsigned char rgba[4];
  rgba[0] = static_cast<unsigned char> (color.red());
  rgba[1] = static_cast<unsigned char> (color.green());
  rgba[2] = static_cast<unsigned char> (color.blue());
  rgba[3] = static_cast<unsigned char> (color.alpha());
  this->PlotDataNode->SetPlotColor(rgba);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewInformationWidgetPrivate::onCopyPlotDataNodeClicked()
{
  Q_Q(const qMRMLPlotViewInformationWidget);

  if (!this->PlotViewNode || !this->PlotDataNode)
    {
    return;
    }

  vtkSmartPointer<vtkMRMLNode> node = vtkSmartPointer<vtkMRMLNode>::Take(
    q->mrmlScene()->CreateNodeByClass("vtkMRMLPlotDataNode"));
  vtkMRMLPlotDataNode *plotDataNodeCopy = vtkMRMLPlotDataNode::SafeDownCast(node);
  plotDataNodeCopy->CopyWithScene(this->PlotDataNode);
  std::string nodeName(this->PlotDataNode->GetName());
  nodeName += "_Copy";
  plotDataNodeCopy->SetName(nodeName.c_str());
  q->mrmlScene()->AddNode(plotDataNodeCopy);

  // Add Reference to the active PlotChartNode
  vtkMRMLPlotChartNode* mrmlPlotChartNode = this->GetPlotChartNodeFromView();
  if (!mrmlPlotChartNode)
    {
    return;
    }
  mrmlPlotChartNode->AddAndObservePlotDataNodeID(plotDataNodeCopy->GetID());
}

// --------------------------------------------------------------------------
// qMRMLPlotViewView methods

// --------------------------------------------------------------------------
qMRMLPlotViewInformationWidget::qMRMLPlotViewInformationWidget(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLPlotViewInformationWidgetPrivate(*this))
{
  Q_D(qMRMLPlotViewInformationWidget);
  d->setupUi(this);
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qMRMLPlotViewInformationWidget::~qMRMLPlotViewInformationWidget()
{
}

//---------------------------------------------------------------------------
vtkMRMLPlotViewNode* qMRMLPlotViewInformationWidget::mrmlPlotViewNode()const
{
  Q_D(const qMRMLPlotViewInformationWidget);
  return d->PlotViewNode;
}

//---------------------------------------------------------------------------
void qMRMLPlotViewInformationWidget::setMRMLPlotViewNode(vtkMRMLNode* newNode)
{
  vtkMRMLPlotViewNode * newPlotViewNode = vtkMRMLPlotViewNode::SafeDownCast(newNode);
  if (!newPlotViewNode)
    {
    return;
    }
  this->setMRMLPlotViewNode(newPlotViewNode);
}

//---------------------------------------------------------------------------
void qMRMLPlotViewInformationWidget::setMRMLPlotViewNode(vtkMRMLPlotViewNode* newPlotViewNode)
{
  Q_D(qMRMLPlotViewInformationWidget);

  if (newPlotViewNode == d->PlotViewNode)
    {
    return;
    }

  d->qvtkReconnect(d->PlotViewNode, newPlotViewNode, vtkCommand::ModifiedEvent,
                   d, SLOT(updateWidgetFromMRMLPlotViewNode()));
  d->qvtkReconnect(d->PlotViewNode, newPlotViewNode, vtkMRMLPlotViewNode::PlotChartNodeChangedEvent,
                   d, SLOT(updateWidgetFromMRMLPlotDataNode()));

  d->PlotViewNode = newPlotViewNode;

  // Update widget state given the new node
  d->updateWidgetFromMRMLPlotViewNode();
}

//---------------------------------------------------------------------------
void qMRMLPlotViewInformationWidget::setViewGroup(int viewGroup)
{
  Q_D(qMRMLPlotViewInformationWidget);

  if (!d->PlotViewNode)
    {
    return;
    }

  d->PlotViewNode->SetViewGroup(viewGroup);
}
