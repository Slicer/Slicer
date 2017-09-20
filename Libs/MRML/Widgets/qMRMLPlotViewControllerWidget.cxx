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
#include <QHBoxLayout>

// VTK includes
#include <vtkCollection.h>
#include <vtkStringArray.h>

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
#include <vtkSmartPointer.h>

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
  this->PlotChartNode = NULL;
  this->PlotViewNode = NULL;
  this->PlotView = NULL;
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

  // Connect the Plot Type selector
  this->connect(this->plotTypeComboBox, SIGNAL(currentIndexChanged(const QString&)),
                SLOT(onPlotTypeSelected(const QString&)));

  // Connect the actions
  QObject::connect(this->actionShow_Grid, SIGNAL(toggled(bool)),
                   q, SLOT(showGrid(bool)));
  QObject::connect(this->actionShow_Legend, SIGNAL(toggled(bool)),
                   q, SLOT(showLegend(bool)));

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
  this->Superclass::init();
  this->ViewLabel->setText(qMRMLPlotViewControllerWidget::tr("P"));
  this->BarLayout->addStretch(1);
  this->setColor(QColor(27, 198, 207));
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
void qMRMLPlotViewControllerWidgetPrivate::onPlotChartNodeSelected(vtkMRMLNode * node)
{
  Q_Q(qMRMLPlotViewControllerWidget);

  vtkMRMLPlotChartNode *mrmlPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(node);

  if (!this->PlotViewNode || this->PlotChartNode == mrmlPlotChartNode)
    {
    return;
    }

  this->qvtkReconnect(this->PlotChartNode, mrmlPlotChartNode, vtkCommand::ModifiedEvent,
                      q, SLOT(updateWidgetFromMRML()));

  this->PlotChartNode = mrmlPlotChartNode;

  this->PlotViewNode->SetPlotChartNodeID(mrmlPlotChartNode ? mrmlPlotChartNode->GetID() : NULL);

  q->updateWidgetFromMRML();

  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
    q->mrmlScene() ? q->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton") : NULL);

  if (!selectionNode)
    {
    qWarning() << "qMRMLPlotViewController::onPlotChartNodeSelected: invalid selection Node";
    return;
    }

  selectionNode->SetActivePlotChartID(mrmlPlotChartNode->GetID());
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotDataNodesSelected()
{
  Q_Q(qMRMLPlotViewControllerWidget);

  if (!this->PlotViewNode)
    {
    return;
    }

  if (!this->PlotChartNode)
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
void qMRMLPlotViewControllerWidgetPrivate::onPlotTypeSelected(const QString &Type)
{
  Q_Q(qMRMLPlotViewControllerWidget);
  if (!this->PlotChartNode || !q->mrmlScene())
    {
    return;
    }

  this->PlotChartNode->SetPlotType(Type.toStdString().c_str());
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

  vtkMRMLPlotChartNode* mrmlPlotChartNode = d->GetPlotChartNodeFromView();
  // PlotChartNode selector
  d->plotChartComboBox->setCurrentNode(mrmlPlotChartNode);

  if (!mrmlPlotChartNode)
    {
    // Set the widgets to default states
    int tindex = d->plotTypeComboBox->findText(QString("Line"));
    d->plotTypeComboBox->setCurrentIndex(tindex);
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

  // Plot selector
  std::vector<std::string> plotDataNodesIDs;
  mrmlPlotChartNode->GetPlotIDs(plotDataNodesIDs);
  bool plotBlockSignals = d->plotDataComboBox->blockSignals(true);
  for (int idx = 0; idx < d->plotDataComboBox->nodeCount(); idx++)
    {
    vtkMRMLNode* node = d->plotDataComboBox->nodeFromIndex(idx);
    d->plotDataComboBox->setCheckState(node, Qt::Unchecked);
    }
  std::vector<std::string>::iterator it = plotDataNodesIDs.begin();
  for (; it != plotDataNodesIDs.end(); ++it)
    {
    vtkMRMLPlotDataNode *dn = vtkMRMLPlotDataNode::SafeDownCast
      (this->mrmlScene()->GetNodeByID((*it).c_str()));
    if (dn)
      {
      d->plotDataComboBox->setCheckState(dn, Qt::Checked);
      }
    }
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
  std::string stype("Line");
  type = mrmlPlotChartNode->GetAttribute("Type");
  if (!type)
    {
    // no type specified, default to "Line"
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
  // that the current node was not in the combo box list menu before
  bool plotChartBlockSignals = d->plotChartComboBox->blockSignals(true);
  bool plotBlockSignals = d->plotDataComboBox->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->plotChartComboBox->blockSignals(plotChartBlockSignals);
  d->plotDataComboBox->blockSignals(plotBlockSignals);

  if (this->mrmlScene())
    {
    this->updateWidgetFromMRML();
    }
}
