/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QActionGroup>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QHBoxLayout>

// VTK includes
#include <vtkStringArray.h>

// CTK includes
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLChartView.h"
#include "qMRMLChartViewControllerWidget_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLChartViewNode.h>
#include <vtkMRMLChartNode.h>
#include <vtkMRMLSceneViewNode.h>

// STD include
#include <string>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLChartViewControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLChartViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLChartViewControllerWidgetPrivate::qMRMLChartViewControllerWidgetPrivate(
  qMRMLChartViewControllerWidget& object)
  : Superclass(object)
{
  this->ChartViewNode = nullptr;
  this->ChartView = nullptr;
}

//---------------------------------------------------------------------------
qMRMLChartViewControllerWidgetPrivate::~qMRMLChartViewControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLChartViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qMRMLChartViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qMRMLChartViewControllerWidget::setupUi(this->PopupWidget);

  // Connect Chart selector
  this->connect(this->chartComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onChartNodeSelected(vtkMRMLNode*)));

  // Connect Array selector
  this->connect(this->arrayComboBox, SIGNAL(checkedNodesChanged()),
                SLOT(onArrayNodesSelected()));

  // Connect the Chart Type selector
  this->connect(this->chartTypeComboBox, SIGNAL(activated(const QString&)), SLOT(onChartTypeSelected(const QString&)));

  // Connect the actions
  QObject::connect(this->actionShow_Lines, SIGNAL(toggled(bool)),
                   q, SLOT(showLines(bool)));
  QObject::connect(this->actionShow_Markers, SIGNAL(toggled(bool)),
                   q, SLOT(showMarkers(bool)));
  QObject::connect(this->actionShow_Grid, SIGNAL(toggled(bool)),
                   q, SLOT(showGrid(bool)));
  QObject::connect(this->actionShow_Legend, SIGNAL(toggled(bool)),
                   q, SLOT(showLegend(bool)));

  // Connect the buttons
  this->showLinesToolButton->setDefaultAction(this->actionShow_Lines);
  this->showMarkersToolButton->setDefaultAction(this->actionShow_Markers);
  this->showGridToolButton->setDefaultAction(this->actionShow_Grid);
  this->showLegendToolButton->setDefaultAction(this->actionShow_Legend);

  // Connect the checkboxes
  QObject::connect(this->showTitleCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(showTitle(bool)));
  QObject::connect(this->showXAxisLabelCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(showXAxisLabel(bool)));
  QObject::connect(this->showYAxisLabelCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(showYAxisLabel(bool)));

  // Connect the line edit boxes
  QObject::connect(this->titleLineEdit, SIGNAL(textEdited(const QString&)),
                   q, SLOT(setTitle(const QString&)));
  QObject::connect(this->xAxisLabelLineEdit, SIGNAL(textEdited(const QString&)),
                   q, SLOT(setXAxisLabel(const QString&)));
  QObject::connect(this->yAxisLabelLineEdit, SIGNAL(textEdited(const QString&)),
                   q, SLOT(setYAxisLabel(const QString&)));

  // Connect the edit buttons to work around the issues of the
  // LineEdits not capturing the mouse focus when in ControllerWidget
  QObject::connect(this->editTitleButton, SIGNAL(clicked()),
                   q, SLOT(editTitle()));
  QObject::connect(this->editXAxisLabelButton, SIGNAL(clicked()),
                   q, SLOT(editXAxisLabel()));
  QObject::connect(this->editYAxisLabelButton, SIGNAL(clicked()),
                   q, SLOT(editYAxisLabel()));

  // Connect the scene
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->chartComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->arrayComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
}

//---------------------------------------------------------------------------
void qMRMLChartViewControllerWidgetPrivate::init()
{
  this->Superclass::init();
  this->ViewLabel->setText(qMRMLChartViewControllerWidget::tr("1"));
  this->BarLayout->addStretch(1);
  //this->setColor(QColor("#6e4b7c"));
  this->setColor(QColor("#e1ba3c"));
}


// --------------------------------------------------------------------------
vtkMRMLChartNode* qMRMLChartViewControllerWidgetPrivate::chartNode()
{
  Q_Q(qMRMLChartViewControllerWidget);

  if (!this->ChartViewNode || !q->mrmlScene())
    {
    // qDebug() << "No ChartViewNode or no Scene";
    return nullptr;
    }

  // Get the current chart node
  vtkMRMLChartNode *chartNode
    = vtkMRMLChartNode::SafeDownCast(q->mrmlScene()->GetNodeByID(this->ChartViewNode->GetChartNodeID()));

  return chartNode;
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidgetPrivate::onChartNodeSelected(vtkMRMLNode * node)
{
  Q_Q(qMRMLChartViewControllerWidget);

  if (!this->ChartViewNode)
    {
    return;
    }

  if (this->chartNode() == node)
    {
    return;
    }

  this->qvtkReconnect(this->chartNode(), node, vtkCommand::ModifiedEvent,
                      q, SLOT(updateWidgetFromMRML()));

  this->ChartViewNode->SetChartNodeID(node ? node->GetID() : nullptr);

  q->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidgetPrivate::onArrayNodesSelected()
{
  //Q_Q(qMRMLChartViewControllerWidget);

  if (!this->ChartViewNode)
    {
    return;
    }

  if (!this->chartNode())
    {
    return;
    }

  vtkStringArray *arrayIDs = this->chartNode()->GetArrays();
  vtkStringArray *arrayNames = this->chartNode()->GetArrayNames();

  // loop over arrays in the widget
  for (int idx = 0; idx < this->arrayComboBox->nodeCount(); ++idx)
    {
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->arrayComboBox->nodeFromIndex(idx));

    bool checked = (this->arrayComboBox->checkState(dn) == Qt::Checked);

    // is the node in the chart?
    bool found = false;
    for (int j = 0; j < arrayIDs->GetSize(); ++j)
      {
      if (!strcmp(dn->GetID(), arrayIDs->GetValue(j).c_str()))
        {
        if (!checked)
          {
          // array is not checked but currently in the chart, remove it
          // (might want to cache the old name in case user adds it back)
          this->chartNode()->RemoveArray(arrayNames->GetValue(j).c_str());
          }
        found = true;
        break;
        }
      }
    if (!found)
      {
      if (checked)
        {
        // array is checked but not currently in the chart, add it
        // (need a string for the name, use the GetName() for now).
        this->chartNode()->AddArray(dn->GetName(), dn->GetID());
        }
      }
    }
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidgetPrivate::onChartTypeSelected(const QString& type)
{
  //Q_Q(qMRMLChartViewControllerWidget);

  if (!this->ChartViewNode)
    {
    return;
    }

  if (!this->chartNode())
    {
    return;
    }

  this->chartNode()->SetProperty("default", "type", type.toUtf8());
}

// --------------------------------------------------------------------------
// qMRMLChartViewControllerWidget methods

// --------------------------------------------------------------------------
qMRMLChartViewControllerWidget::qMRMLChartViewControllerWidget(QWidget* parentWidget)
  : Superclass(new qMRMLChartViewControllerWidgetPrivate(*this), parentWidget)
{
  Q_D(qMRMLChartViewControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLChartViewControllerWidget::~qMRMLChartViewControllerWidget()
{
  this->setMRMLScene(nullptr);
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::setChartView(qMRMLChartView* view)
{
  Q_D(qMRMLChartViewControllerWidget);
  d->ChartView = view;
}

//---------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::setViewLabel(const QString& newViewLabel)
{
  Q_D(qMRMLChartViewControllerWidget);

  if (d->ChartViewNode)
    {
    logger.error("setViewLabel should be called before setViewNode !");
    return;
    }

  d->ChartViewLabel = newViewLabel;
  d->ViewLabel->setText(d->ChartViewLabel);
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLChartViewControllerWidget, QString, viewLabel, ChartViewLabel);


// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::setMRMLChartViewNode(
    vtkMRMLChartViewNode * viewNode)
{
  Q_D(qMRMLChartViewControllerWidget);
  this->qvtkReconnect(d->ChartViewNode, viewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));
  d->ChartViewNode = viewNode;
  this->updateWidgetFromMRML();
}

// --------------------------------------------------------------------chart------
void qMRMLChartViewControllerWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLChartViewControllerWidget);

  //qDebug() << "qMRMLChartViewControllerWidget::updateWidgetFromMRML()";

  if (!d->ChartViewNode || !this->mrmlScene())
    {
    return;
    }

  vtkMRMLChartNode *chartNode = d->chartNode();
  if (!chartNode)
    {
    // Set the widgets to default states
    int tindex = d->chartTypeComboBox->findText(QString("Line"));
    d->chartTypeComboBox->setCurrentIndex(tindex);
    d->actionShow_Lines->setChecked(true);
    d->actionShow_Markers->setChecked(false);
    d->actionShow_Grid->setChecked(true);
    d->actionShow_Legend->setChecked(true);
    d->showTitleCheckBox->setChecked(true);
    d->showXAxisLabelCheckBox->setChecked(true);
    d->showYAxisLabelCheckBox->setChecked(true);
    d->titleLineEdit->setText("");
    d->xAxisLabelLineEdit->setText("");
    d->yAxisLabelLineEdit->setText("");
    return;
    }

  // ChartNode selector
  d->chartComboBox->setCurrentNodeID(chartNode->GetID());

  // Array selector
  vtkStringArray *arrayIDs = chartNode->GetArrays();
  bool arrayBlockSignals = d->arrayComboBox->blockSignals(true);
  for (int idx = 0; idx < d->arrayComboBox->nodeCount(); ++idx)
    {
    d->arrayComboBox->setCheckState(d->arrayComboBox->nodeFromIndex(idx),
                                    Qt::Unchecked);
    }
  for (int idx = 0; idx < arrayIDs->GetNumberOfValues(); idx++)
    {
    vtkMRMLDoubleArrayNode *dn = vtkMRMLDoubleArrayNode::SafeDownCast(this->mrmlScene()->GetNodeByID( arrayIDs->GetValue(idx).c_str() ));
    if (dn)
      {
      d->arrayComboBox->setCheckState(dn, Qt::Checked);
      }
    }
  d->arrayComboBox->blockSignals(arrayBlockSignals);

  // ChartType selector
  const char *type;
  std::string stype("Line");
  type = chartNode->GetProperty("default", "type");
  if (!type)
    {
    // no type specified, default to "Line"
    type = stype.c_str();
    }
  if (type)
    {
    QString qtype(type);
    int tindex = d->chartTypeComboBox->findText(qtype);
    if (tindex != -1)
      {
      d->chartTypeComboBox->setCurrentIndex(tindex);
      }
    }

  // Buttons
  const char *propertyValue;
  propertyValue = chartNode->GetProperty("default", "showLines");
  d->actionShow_Lines->setChecked(propertyValue && !strcmp("on", propertyValue));

  propertyValue = chartNode->GetProperty("default", "showMarkers");
  d->actionShow_Markers->setChecked(propertyValue && !strcmp("on", propertyValue));

  propertyValue = chartNode->GetProperty("default", "showGrid");
  d->actionShow_Grid->setChecked(propertyValue && !strcmp("on", propertyValue));

  propertyValue = chartNode->GetProperty("default", "showLegend");
  d->actionShow_Legend->setChecked(propertyValue && !strcmp("on", propertyValue));

  // Based on ChartType, override button checked and enable/disable some controls
  d->showLinesToolButton->setEnabled(true);
  d->showMarkersToolButton->setEnabled(true);
  if (type && !strcmp(type, "Line"))
    {
    d->actionShow_Lines->setChecked(true);
    d->showLinesToolButton->setEnabled(false);
    }
  if (type && !strcmp(type, "Scatter"))
    {
    d->actionShow_Markers->setChecked(true);
    d->showMarkersToolButton->setEnabled(false);
    d->showLinesToolButton->setEnabled(false);
    }
  if (type && !strcmp(type, "Bar"))
    {
    d->actionShow_Lines->setChecked(true);
    d->actionShow_Markers->setChecked(false);
    d->showMarkersToolButton->setEnabled(false);
    d->showLinesToolButton->setEnabled(false);
    }


  // Titles, axis labels (checkboxes AND text widgets)
  propertyValue = chartNode->GetProperty("default", "showTitle");
  d->showTitleCheckBox->setChecked(propertyValue && !strcmp("on", propertyValue));
  propertyValue = chartNode->GetProperty("default", "title");
  if (propertyValue)
    {
    d->titleLineEdit->setText(propertyValue);
    }
  else
    {
    d->titleLineEdit->clear();
    }

  propertyValue = chartNode->GetProperty("default", "showXAxisLabel");
  d->showXAxisLabelCheckBox->setChecked(propertyValue && !strcmp("on", propertyValue));
  propertyValue = chartNode->GetProperty("default", "xAxisLabel");
  if (propertyValue)
    {
    d->xAxisLabelLineEdit->setText(propertyValue);
    }
  else
    {
    d->xAxisLabelLineEdit->clear();
    }

  propertyValue = chartNode->GetProperty("default", "showYAxisLabel");
  d->showYAxisLabelCheckBox->setChecked(propertyValue && !strcmp("on", propertyValue));
  propertyValue = chartNode->GetProperty("default", "yAxisLabel");
  if (propertyValue)
    {
    d->yAxisLabelLineEdit->setText(propertyValue);
    }
  else
    {
    d->yAxisLabelLineEdit->clear();
    }

}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLChartViewControllerWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }

   d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent,
                    this, SLOT(updateWidgetFromMRML()));

  // Disable the node selectors as they would fire signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means
  // that the current node was not in the combo box list menu before
  bool chartBlockSignals = d->chartComboBox->blockSignals(true);
  bool arrayBlockSignals = d->arrayComboBox->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->chartComboBox->blockSignals(chartBlockSignals);
  d->arrayComboBox->blockSignals(arrayBlockSignals);

  if (this->mrmlScene())
    {
    this->updateWidgetFromMRML();
    }
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::showLines(bool show)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "showLines", show ? "on" : "off");

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "showLines");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::showMarkers(bool show)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "showMarkers", show ? "on" : "off");

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "showMarkers");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::showGrid(bool show)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "showGrid", show ? "on" : "off");

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "showGrid");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::showLegend(bool show)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "showLegend", show ? "on" : "off");

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "showLegend");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::showTitle(bool show)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "showTitle", show ? "on" : "off");

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "showTitle");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::showXAxisLabel(bool show)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "showXAxisLabel", show ? "on" : "off");

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "showXAxisLabel");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::showYAxisLabel(bool show)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "showYAxisLabel", show ? "on" : "off");

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "showYAxisLabel");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::setTitle(const QString &str)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "title", str.toUtf8());

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "title");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::setXAxisLabel(const QString &str)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "xAxisLabel", str.toUtf8());

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "xAxisLabel");
}


// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::setYAxisLabel(const QString &str)
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Set the parameter
  chartNode->SetProperty("default", "yAxisLabel", str.toUtf8());

  //qDebug() << "Regetting property: " << chartNode->GetProperty("default", "yAxisLabel");
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::editTitle()
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Bring up a dialog to request a title
  bool ok = false;
  QString newTitle = QInputDialog::getText(
    this, "Edit Title", "Title",
    QLineEdit::Normal, chartNode->GetProperty("default", "title"), &ok);
  if (!ok)
    {
    return;
    }

  // Set the parameter
  this->setTitle(newTitle);
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::editXAxisLabel()
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Bring up a dialog to request a title
  bool ok = false;
  QString newXAxisLabel = QInputDialog::getText(
    this, "Edit X-axis label", "X-axis label",
    QLineEdit::Normal, chartNode->GetProperty("default", "xAxisLabel"), &ok);
  if (!ok)
    {
    return;
    }

  // Set the parameter
  this->setXAxisLabel(newXAxisLabel);
}


// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::editYAxisLabel()
{
  Q_D(qMRMLChartViewControllerWidget);

  vtkMRMLChartNode *chartNode = d->chartNode();

  if (!chartNode)
    {
    return;
    }

  // Bring up a dialog to request a title
  bool ok = false;
  QString newYAxisLabel = QInputDialog::getText(
    this, "Edit Y-axis label", "Y-axis label",
    QLineEdit::Normal, chartNode->GetProperty("default", "yAxisLabel"), &ok);
  if (!ok)
    {
    return;
    }

  // Set the parameter
  this->setYAxisLabel(newYAxisLabel);
}

