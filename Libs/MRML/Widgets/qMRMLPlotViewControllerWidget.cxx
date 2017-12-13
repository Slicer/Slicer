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
#include <QActionGroup>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QHBoxLayout>

// VTK includes
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// CTK includes
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLPlotView.h"
#include "qMRMLPlotViewControllerWidget_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLDoubleArrayNode.h>
#include <vtkMRMLPlotDataNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotViewNode.h>
#include <vtkMRMLSceneViewNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTableNode.h>

// STD include
#include <string>

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLPlotViewControllerWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLPlotViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLPlotViewControllerWidgetPrivate::qMRMLPlotViewControllerWidgetPrivate(
  qMRMLPlotViewControllerWidget& object)
  : Superclass(object)
{
  this->FitToWindowToolButton = 0;

  this->PlotChartNode = 0;
  this->PlotViewNode = 0;
  this->PlotView = 0;

  this->SelectionNode = 0;
}

//---------------------------------------------------------------------------
qMRMLPlotViewControllerWidgetPrivate::~qMRMLPlotViewControllerWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qMRMLPlotViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qMRMLPlotViewControllerWidget::setupUi(this->PopupWidget);

  // Connect PlotChart selector
  this->connect(this->plotChartComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
                SLOT(onPlotChartNodeSelected(vtkMRMLNode*)));

  // Connect Plot selector
  this->connect(this->plotDataComboBox, SIGNAL(checkedNodesChanged()),
                SLOT(onPlotDataNodesSelected()));
  this->connect(this->plotDataComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)),
                SLOT(onPlotDataNodeAdded(vtkMRMLNode*)));
  this->connect(this->plotDataComboBox, SIGNAL(nodeAboutToBeEdited(vtkMRMLNode*)),
                SLOT(onPlotDataNodeEdited(vtkMRMLNode*)));

  // Connect the Plot Type selector
  this->connect(this->plotTypeComboBox, SIGNAL(currentIndexChanged(const QString&)),
                SLOT(onPlotTypeChanged(const QString&)));

  // Connect xAxis comboBox
  this->connect(this->xAxisComboBox, SIGNAL(currentIndexChanged(const QString&)),
                SLOT(onXAxisChanged(const QString&)));

  // Connect Markers comboBox
  this->connect(this->markersComboBox, SIGNAL(currentIndexChanged(const QString&)),
                SLOT(onMarkersChanged(const QString&)));

  // Connect the actions
  QObject::connect(this->actionShow_Grid, SIGNAL(toggled(bool)),
                   q, SLOT(showGrid(bool)));
  QObject::connect(this->actionShow_Legend, SIGNAL(toggled(bool)),
                   q, SLOT(showLegend(bool)));
  QObject::connect(this->actionFit_to_window, SIGNAL(triggered()),
                   q, SLOT(fitPlotToAxes()));

  // Connect the buttons
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
                   this->plotDataComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->plotChartComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::init()
{
  Q_Q(qMRMLPlotViewControllerWidget);

  this->Superclass::init();

  this->ViewLabel->setText(qMRMLPlotViewControllerWidget::tr("P"));
  this->BarLayout->addStretch(1);
  this->setColor(QColor(27, 198, 207));

  this->FitToWindowToolButton = new QToolButton(q);
  this->FitToWindowToolButton->setAutoRaise(true);
  this->FitToWindowToolButton->setDefaultAction(this->actionFit_to_window);
  this->FitToWindowToolButton->setFixedSize(15, 15);
  this->BarLayout->insertWidget(2, this->FitToWindowToolButton);
}


// --------------------------------------------------------------------------
vtkMRMLPlotChartNode* qMRMLPlotViewControllerWidgetPrivate::GetPlotChartNodeFromView()
{
  Q_Q(qMRMLPlotViewControllerWidget);

  if (!this->PlotViewNode || !q->mrmlScene())
    {
    // qDebug() << "No PlotViewNode or no Scene";
    return 0;
    }

  // Get the current PlotChart node
  vtkMRMLPlotChartNode *PlotChartNodeFromViewNode
    = vtkMRMLPlotChartNode::SafeDownCast(q->mrmlScene()->GetNodeByID(this->PlotViewNode->GetPlotChartNodeID()));

  return PlotChartNodeFromViewNode;
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotChartNodeSelected(vtkMRMLNode *node)
{
  Q_Q(qMRMLPlotViewControllerWidget);

  vtkMRMLPlotChartNode *mrmlPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(node);

  if (!this->PlotViewNode || !this->SelectionNode || this->PlotChartNode == mrmlPlotChartNode)
    {
    return;
    }

  this->qvtkReconnect(this->PlotChartNode, mrmlPlotChartNode, vtkCommand::ModifiedEvent,
                      q, SLOT(updateWidgetFromMRML()));

  this->PlotChartNode = mrmlPlotChartNode;
  this->SelectionNode->SetActivePlotChartID(mrmlPlotChartNode ? mrmlPlotChartNode->GetID() : "");
  this->PlotViewNode->SetPlotChartNodeID(mrmlPlotChartNode ? mrmlPlotChartNode->GetID() : NULL);

  q->updateWidgetFromMRML();
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onSelectionNodeModified()
{
  Q_Q(qMRMLPlotViewControllerWidget);

  if (!this->SelectionNode || !q->mrmlScene())
    {
    return;
    }

  vtkMRMLPlotChartNode *mrmlPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(
    q->mrmlScene()->GetNodeByID(this->SelectionNode->GetActivePlotChartID()));

  this->onPlotChartNodeSelected(mrmlPlotChartNode);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotDataNodesSelected()
{
  if (!this->PlotViewNode || !this->PlotChartNode)
    {
    return;
    }

  std::vector<std::string> plotDataNodesIDs;
  this->PlotChartNode->GetPlotIDs(plotDataNodesIDs);

  // loop over arrays in the widget
  for (int idx = 0; idx < this->plotDataComboBox->nodeCount(); idx++)
    {
    vtkMRMLPlotDataNode *dn = vtkMRMLPlotDataNode::SafeDownCast(this->plotDataComboBox->nodeFromIndex(idx));

    bool checked = (this->plotDataComboBox->checkState(dn) == Qt::Checked);

    // is the node in the Plot?
    bool found = false;
    std::vector<std::string>::iterator it = plotDataNodesIDs.begin();
    for (; it != plotDataNodesIDs.end(); ++it)
      {
      if (!strcmp(dn->GetID(), (*it).c_str()))
        {
        if (!checked)
          {
          // plot is not checked but currently in the LayoutPlot, remove it
          // (might want to cache the old name in case user adds it back)
          this->PlotChartNode->RemovePlotDataNodeID((*it).c_str());
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
        this->PlotChartNode->AddAndObservePlotDataNodeID(dn->GetID());
        }
      }
  }
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotDataNodeAdded(vtkMRMLNode *node)
{
  Q_Q(qMRMLPlotViewControllerWidget);

  if (!this->PlotChartNode || !q->mrmlScene())
    {
    return;
    }

  vtkMRMLPlotDataNode *plotDataNode = vtkMRMLPlotDataNode::SafeDownCast(node);

  if (plotDataNode)
    {
    return;
    }

  q->mrmlScene()->AddNode(plotDataNode);

  const char* Type = this->PlotChartNode->GetAttribute("Type");
  if (strcmp("Custom", Type))
    {
    plotDataNode->SetType(plotDataNode->GetPlotTypeFromString(Type));
    }

  const char* XAxis = this->PlotChartNode->GetAttribute("XAxis");
  if (strcmp("Custom", XAxis))
    {
    plotDataNode->SetXColumnName(XAxis);
    }

  const char* Markers = this->PlotChartNode->GetAttribute("Markers");
  if (strcmp("Custom", Markers))
    {
    plotDataNode->SetMarkerStyle(plotDataNode->GetMarkersStyleFromString(Markers));
    }

  // Add the reference of the PlotDataNode in the active PlotChartNode
  this->PlotChartNode->AddAndObservePlotDataNodeID(plotDataNode->GetID());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotDataNodeEdited(vtkMRMLNode *node)
{
  if (node == NULL)
    {
    return;
    }

  QString message = QString("To edit the node %1 : Please navigate to"
                            " the ViewController Module. Additional editing options"
                            " are available under the Advanced menu.").arg(node->GetName());
  qWarning() << Q_FUNC_INFO << ": " << message;
  QMessageBox::warning(NULL, tr("Edit PlotDataNode"), message);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotTypeChanged(const QString &Type)
{
  if (!this->PlotChartNode)
    {
    return;
    }

  this->PlotChartNode->SetAttribute("Type", Type.toStdString().c_str());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onXAxisChanged(const QString &Column)
{
  if (!this->PlotChartNode)
    {
    return;
    }

  this->PlotChartNode->SetAttribute("XAxis", Column.toStdString().c_str());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onMarkersChanged(const QString &str)
{
  if(!this->PlotChartNode)
    {
    return;
    }

  this->PlotChartNode->SetAttribute("Markers", str.toStdString().c_str());
}

// --------------------------------------------------------------------------
// qMRMLPlotViewControllerWidget methods

// --------------------------------------------------------------------------
qMRMLPlotViewControllerWidget::qMRMLPlotViewControllerWidget(QWidget* parentWidget)
  : Superclass(new qMRMLPlotViewControllerWidgetPrivate(*this), parentWidget)
{
  Q_D(qMRMLPlotViewControllerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLPlotViewControllerWidget::~qMRMLPlotViewControllerWidget()
{
  this->setMRMLScene(0);
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setPlotView(qMRMLPlotView* view)
{
  Q_D(qMRMLPlotViewControllerWidget);
  d->PlotView = view;
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setViewLabel(const QString& newViewLabel)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if (d->PlotViewNode)
    {
    logger.error("setViewLabel should be called before setViewNode !");
    return;
    }

  d->PlotViewLabel = newViewLabel;
  d->ViewLabel->setText(d->PlotViewLabel);
}

//---------------------------------------------------------------------------
CTK_GET_CPP(qMRMLPlotViewControllerWidget, QString, viewLabel, PlotViewLabel);


//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setMRMLPlotViewNode(
    vtkMRMLPlotViewNode * viewNode)
{
  Q_D(qMRMLPlotViewControllerWidget);
  this->qvtkReconnect(d->PlotViewNode, viewNode, vtkCommand::ModifiedEvent,
                      this, SLOT(updateWidgetFromMRML()));
  d->PlotViewNode = viewNode;
  this->updateWidgetFromMRML();
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::showGrid(bool show)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  if (show)
    {
    d->PlotChartNode->SetAttribute("ShowGrid", "on");
    }
  else
    {
    d->PlotChartNode->SetAttribute("ShowGrid", "off");
    }
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::showLegend(bool show)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  if (show)
    {
    d->PlotChartNode->SetAttribute("ShowLegend", "on");
    }
  else
    {
    d->PlotChartNode->SetAttribute("ShowLegend", "off");
    }
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::showTitle(bool show)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  if (show)
    {
    d->PlotChartNode->SetAttribute("ShowTitle", "on");
    }
  else
    {
    d->PlotChartNode->SetAttribute("ShowTitle", "off");
  }
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::fitPlotToAxes()
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  d->PlotChartNode->SetAttribute("FitPlotToAxes", "on");
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::showXAxisLabel(bool show)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  if (show)
    {
    d->PlotChartNode->SetAttribute("ShowXAxisLabel", "on");
    }
  else
    {
    d->PlotChartNode->SetAttribute("ShowXAxisLabel", "off");
    }
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::showYAxisLabel(bool show)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  if (show)
    {
    d->PlotChartNode->SetAttribute("ShowYAxisLabel", "on");
    }
  else
    {
    d->PlotChartNode->SetAttribute("ShowYAxisLabel", "off");
    }
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setTitle(const QString &str)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  d->PlotChartNode->SetAttribute("TitleName", str.toLatin1());
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setXAxisLabel(const QString &str)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  d->PlotChartNode->SetAttribute("XAxisLabelName", str.toLatin1());
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setYAxisLabel(const QString &str)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if(!d->PlotChartNode)
    {
    return;
    }

  d->PlotChartNode->SetAttribute("YAxisLabelName", str.toLatin1());
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::editTitle()
{
  Q_D(qMRMLPlotViewControllerWidget);

  if (!d->PlotChartNode)
    {
    return;
    }

  // Bring up a dialog to request a title
  bool ok = false;
  QString newTitle = QInputDialog::getText(
    this, "Edit Title", "Title",
    QLineEdit::Normal, d->PlotChartNode->GetAttribute("TitleName"), &ok);
  if (!ok)
    {
    return;
    }

  // Set the parameter
  this->setTitle(newTitle);
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::editXAxisLabel()
{
  Q_D(qMRMLPlotViewControllerWidget);

  if (!d->PlotChartNode)
    {
    return;
    }

  // Bring up a dialog to request a title
  bool ok = false;
  QString newXLabel = QInputDialog::getText(
    this, "Edit X-axis label", "X-axis label",
    QLineEdit::Normal, d->PlotChartNode->GetAttribute("XAxisLabelName"), &ok);
  if (!ok)
    {
    return;
    }

  // Set the parameter
  this->setXAxisLabel(newXLabel);
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::editYAxisLabel()
{
  Q_D(qMRMLPlotViewControllerWidget);

  if (!d->PlotChartNode)
    {
    return;
    }

  // Bring up a dialog to request a title
  bool ok = false;
  QString newYLabel = QInputDialog::getText(
    this, "Edit Y-axis label", "Y-axis label",
    QLineEdit::Normal, d->PlotChartNode->GetAttribute("YAxisLabelName"), &ok);
  if (!ok)
    {
    return;
    }

  // Set the parameter
  this->setYAxisLabel(newYLabel);
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLPlotViewControllerWidget);

  if (!d->PlotViewNode || !this->mrmlScene())
    {
    return;
    }

  // PlotChartNode selector
  vtkMRMLPlotChartNode* mrmlPlotChartNode = d->GetPlotChartNodeFromView();

  d->plotChartComboBox->setCurrentNode(mrmlPlotChartNode);

  if (!mrmlPlotChartNode)
    {
    // Set the widgets to default states
    int tindex = d->plotTypeComboBox->findText(QString("Custom"));
    d->plotTypeComboBox->setCurrentIndex(tindex);
    tindex = d->xAxisComboBox->findText(QString("Custom"));
    d->xAxisComboBox->setCurrentIndex(tindex);
    tindex = d->markersComboBox->findText(QString("Custom"));
    d->markersComboBox->setCurrentIndex(tindex);
    d->actionShow_Grid->setChecked(true);
    d->actionShow_Legend->setChecked(true);
    d->showTitleCheckBox->setChecked(true);
    d->showXAxisLabelCheckBox->setChecked(true);
    d->showYAxisLabelCheckBox->setChecked(true);
    d->titleLineEdit->setText("");
    d->xAxisLabelLineEdit->setText("");
    d->yAxisLabelLineEdit->setText("");

    bool plotBlockSignals = d->plotDataComboBox->blockSignals(true);
    for (int idx = 0; idx < d->plotDataComboBox->nodeCount(); idx++)
      {
      d->plotDataComboBox->setCheckState(d->plotDataComboBox->nodeFromIndex(idx),
                                         Qt::Unchecked);
      }
    d->plotDataComboBox->blockSignals(plotBlockSignals);

    return;
    }

  int plnWasModifying = mrmlPlotChartNode->StartModify();

  // Plot selector
  std::vector<std::string> plotDataNodesIDs;
  mrmlPlotChartNode->GetPlotIDs(plotDataNodesIDs);
  bool plotBlockSignals = d->plotDataComboBox->blockSignals(true);
  for (int idx = 0; idx < d->plotDataComboBox->nodeCount(); idx++)
    {
    vtkMRMLNode* node = d->plotDataComboBox->nodeFromIndex(idx);
    d->plotDataComboBox->setCheckState(node, Qt::Unchecked);
    }

  bool xAxisComboBoxBlockSignals = d->xAxisComboBox->blockSignals(true);
  QString currentCol = d->xAxisComboBox->itemText(d->xAxisComboBox->currentIndex());
  d->xAxisComboBox->clear();
  d->xAxisComboBox->addItem("Custom");
  d->xAxisComboBox->addItem("Indexes");
  std::vector<std::string>::iterator it = plotDataNodesIDs.begin();
  for (; it != plotDataNodesIDs.end(); ++it)
    {
    vtkMRMLPlotDataNode *plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
      (this->mrmlScene()->GetNodeByID((*it).c_str()));
    if (plotDataNode == NULL)
      {
      continue;
      }
    d->plotDataComboBox->setCheckState(plotDataNode, Qt::Checked);
    vtkMRMLTableNode* mrmlTableNode = plotDataNode->GetTableNode();
    if (mrmlTableNode == NULL)
      {
      continue;
      }
    int numCol = mrmlTableNode->GetNumberOfColumns();
    for (int ii = 0; ii < numCol; ++ii)
      {
      QString ColumnName = QString::fromStdString(mrmlTableNode->GetColumnName(ii));
      if (d->xAxisComboBox->findText(ColumnName) == -1)
        {
        d->xAxisComboBox->addItem(ColumnName);
        }
      }
    }

  // After Qt5 migration, the next line can be replaced by this call:
  // d->xAxisComboBox->setCurrentText(currentCol);
  d->xAxisComboBox->setCurrentIndex(d->xAxisComboBox->findText(currentCol));

  d->xAxisComboBox->blockSignals(xAxisComboBoxBlockSignals);
  d->plotDataComboBox->blockSignals(plotBlockSignals);

  const char *AttributeValue;
  AttributeValue = mrmlPlotChartNode->GetAttribute("ShowGrid");
  d->actionShow_Grid->setChecked(AttributeValue && !strcmp("on", AttributeValue));

  AttributeValue = mrmlPlotChartNode->GetAttribute("ShowLegend");
  d->actionShow_Legend->setChecked(AttributeValue && !strcmp("on", AttributeValue));

  // Titles, axis labels (checkboxes AND text widgets)
  AttributeValue = mrmlPlotChartNode->GetAttribute("ShowTitle");
  d->showTitleCheckBox->setChecked(AttributeValue && !strcmp("on", AttributeValue));
  AttributeValue = mrmlPlotChartNode->GetAttribute("TitleName");
  if (AttributeValue)
    {
    d->titleLineEdit->setText(AttributeValue);
    }
  else
    {
    d->titleLineEdit->clear();
    }

  AttributeValue = mrmlPlotChartNode->GetAttribute("ShowXAxisLabel");
  d->showXAxisLabelCheckBox->setChecked(AttributeValue && !strcmp("on", AttributeValue));
  AttributeValue = mrmlPlotChartNode->GetAttribute("XAxisLabelName");
  if (AttributeValue)
    {
    d->xAxisLabelLineEdit->setText(AttributeValue);
    }
  else
    {
    d->xAxisLabelLineEdit->clear();
    }

  AttributeValue = mrmlPlotChartNode->GetAttribute("ShowYAxisLabel");
  d->showYAxisLabelCheckBox->setChecked(AttributeValue && !strcmp("on", AttributeValue));
  AttributeValue = mrmlPlotChartNode->GetAttribute("YAxisLabelName");
  if (AttributeValue)
    {
    d->yAxisLabelLineEdit->setText(AttributeValue);
    }
  else
    {
    d->yAxisLabelLineEdit->clear();
    }

  // PlotType selector
  const char *type;
  std::string stype("Custom");
  type = mrmlPlotChartNode->GetAttribute("Type");
  if (type == NULL)
    {
    // no type specified, default to "Custom"
    type = stype.c_str();
    }
  if (type)
    {
    QString qtype(type);
    int tindex = d->plotTypeComboBox->findText(qtype);
    if (tindex != -1)
      {
      d->plotTypeComboBox->setCurrentIndex(tindex);
      }
    }

  // Handle Modify events for PlotDataNodes
  std::vector<int> plotDataNodesWasModifying(mrmlPlotChartNode->GetNumberOfPlotDataNodes(), 0);
  it = plotDataNodesIDs.begin();
  for (; it != plotDataNodesIDs.end(); ++it)
    {
    vtkMRMLPlotDataNode *plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
      (this->mrmlScene()->GetNodeByID((*it).c_str()));
    if (!plotDataNode)
      {
      continue;
      }
    int plotDataNodesIndex = std::distance(plotDataNodesIDs.begin(), it);
    plotDataNodesWasModifying[plotDataNodesIndex] = plotDataNode->StartModify();
    }

  // Update selected Type for all PlotDataNode
  if (strcmp(type, "Custom"))
    {
    it = plotDataNodesIDs.begin();
    for (; it != plotDataNodesIDs.end(); ++it)
      {
      vtkMRMLPlotDataNode *plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
        (this->mrmlScene()->GetNodeByID((*it).c_str()));
      if (!plotDataNode)
        {
        continue;
        }
      plotDataNode->SetType(type);
      }
    }

  // XAxis selector
  const char *xAxis;
  stype = "Custom";
  xAxis = mrmlPlotChartNode->GetAttribute("XAxis");
  if (xAxis == NULL)
    {
    // no type specified, default to "Custom"
    xAxis = stype.c_str();
    }
  if (xAxis)
    {
    QString qtype(xAxis);
    int tindex = d->xAxisComboBox->findText(qtype);
    if (tindex != -1)
      {
      d->xAxisComboBox->setCurrentIndex(tindex);
      }
    }

  // Update selected XAxis for all PlotDataNode
  if (strcmp(xAxis, "Custom"))
    {
    it = plotDataNodesIDs.begin();
    for (; it != plotDataNodesIDs.end(); ++it)
      {
      vtkMRMLPlotDataNode *plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
        (this->mrmlScene()->GetNodeByID((*it).c_str()));
      if (!plotDataNode)
        {
        continue;
        }
      plotDataNode->SetXColumnName(xAxis);
      }
    }

  // UnCheck Markers if Type is Bar
  if (!strcmp(type, "Bar"))
    {
    d->markersComboBox->setEnabled(false);
    mrmlPlotChartNode->SetAttribute("Markers", "Custom");
    }
  else
    {
    d->markersComboBox->setEnabled(true);
    }

  // Markers selector
  const char *markers;
  stype = "Custom";
  markers = mrmlPlotChartNode->GetAttribute("Markers");
  if (markers == NULL)
    {
    // no type specified, default to "Custom"
    markers = stype.c_str();
    }
  if (markers)
    {
    QString qtype(markers);
    int tindex = d->markersComboBox->findText(qtype);
    if (tindex != -1)
      {
      d->markersComboBox->setCurrentIndex(tindex);
      }
    }

  // Update selected PlotDataNodes Markers if Type is Line
  if (strcmp("Custom", markers))
    {
    it = plotDataNodesIDs.begin();
    for (; it != plotDataNodesIDs.end(); ++it)
      {
      vtkMRMLPlotDataNode *plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
        (this->mrmlScene()->GetNodeByID((*it).c_str()));
      if (plotDataNode == NULL)
        {
        continue;
        }
      plotDataNode->SetMarkerStyle(plotDataNode->
        GetMarkersStyleFromString(markers));
      }
    }

  // End MRML Modifications
  mrmlPlotChartNode->EndModify(plnWasModifying);

  it = plotDataNodesIDs.begin();
  for (; it != plotDataNodesIDs.end(); ++it)
    {
    vtkMRMLPlotDataNode *plotDataNode = vtkMRMLPlotDataNode::SafeDownCast
      (this->mrmlScene()->GetNodeByID((*it).c_str()));
    if (!plotDataNode)
      {
      continue;
      }
    int plotDataNodesIndex = std::distance(plotDataNodesIDs.begin(), it);
    plotDataNode->EndModify(plotDataNodesWasModifying[plotDataNodesIndex]);
    }

}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setMRMLScene(vtkMRMLScene* newScene)
{
  Q_D(qMRMLPlotViewControllerWidget);

  if (this->mrmlScene() == newScene)
    {
    return;
    }

   d->qvtkReconnect(this->mrmlScene(), newScene, vtkMRMLScene::EndBatchProcessEvent,
                    this, SLOT(updateWidgetFromMRML()));

  // Disable the node selectors as they would fire signal currentIndexChanged(0)
  // meaning that there is no current node anymore. It's not true, it just means
  // that the current node was not in the combo box list menu before.
  bool plotChartBlockSignals = d->plotChartComboBox->blockSignals(true);
  bool plotBlockSignals = d->plotDataComboBox->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->plotChartComboBox->blockSignals(plotChartBlockSignals);
  d->plotDataComboBox->blockSignals(plotBlockSignals);

  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
    this->mrmlScene() ? this->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton") : NULL);

  if (!selectionNode)
    {
    return;
    }

  this->qvtkReconnect(d->SelectionNode, selectionNode, vtkCommand::ModifiedEvent,
                      d, SLOT(onSelectionNodeModified()));

  d->SelectionNode = selectionNode;
  d->onSelectionNodeModified();

  if (this->mrmlScene())
    {
    this->updateWidgetFromMRML();
    }
}
