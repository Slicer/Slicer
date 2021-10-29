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
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QToolButton>

// VTK includes
#include <vtkCollection.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <vtkPlotLine.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

// CTK includes
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLPlotView.h"
#include "qMRMLPlotViewControllerWidget_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLPlotSeriesNode.h>
#include <vtkMRMLPlotChartNode.h>
#include <vtkMRMLPlotViewNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTableNode.h>

// STD include
#include <string>

//--------------------------------------------------------------------------
// qMRMLPlotViewControllerWidgetPrivate methods

//---------------------------------------------------------------------------
qMRMLPlotViewControllerWidgetPrivate::qMRMLPlotViewControllerWidgetPrivate(
  qMRMLPlotViewControllerWidget& object)
  : Superclass(object)
{
  this->FitToWindowToolButton = nullptr;

  this->PlotChartNode = nullptr;
  this->PlotView = nullptr;
}

//---------------------------------------------------------------------------
qMRMLPlotViewControllerWidgetPrivate::~qMRMLPlotViewControllerWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qMRMLPlotViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qMRMLPlotViewControllerWidget::setupUi(this->PopupWidget);

  this->connect(this->plotChartComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), SLOT(onPlotChartNodeSelected(vtkMRMLNode*)));

  this->connect(this->plotSeriesComboBox, SIGNAL(checkedNodesChanged()), SLOT(onPlotSeriesNodesSelected()));
  this->connect(this->plotSeriesComboBox, SIGNAL(nodeAddedByUser(vtkMRMLNode*)), SLOT(onPlotSeriesNodeAdded(vtkMRMLNode*)));

  this->connect(this->plotTypeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onPlotTypeChanged(int)));
  this->connect(this->interactionModeComboBox, SIGNAL(currentIndexChanged(int)), SLOT(onInteractionModeChanged(int)));
  QObject::connect(this->actionFit_to_window, SIGNAL(triggered()), q, SLOT(fitPlotToAxes()));

  QObject::connect(this->exportPushButton, SIGNAL(clicked()), q, SLOT(onExportButton()));

  // Connect the scene
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->plotChartComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->plotSeriesComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
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

  if (!q->mrmlPlotViewNode() || !q->mrmlScene())
    {
    // qDebug() << "No PlotViewNode or no Scene";
    return nullptr;
    }

  // Get the current PlotChart node
  vtkMRMLPlotChartNode *PlotChartNodeFromViewNode
    = vtkMRMLPlotChartNode::SafeDownCast(q->mrmlScene()->GetNodeByID(q->mrmlPlotViewNode()->GetPlotChartNodeID()));

  return PlotChartNodeFromViewNode;
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotChartNodeSelected(vtkMRMLNode *node)
{
  Q_Q(qMRMLPlotViewControllerWidget);

  vtkMRMLPlotChartNode *mrmlPlotChartNode = vtkMRMLPlotChartNode::SafeDownCast(node);

  if (!q->mrmlPlotViewNode() || this->PlotChartNode == mrmlPlotChartNode)
    {
    return;
    }

  q->mrmlPlotViewNode()->SetPlotChartNodeID(mrmlPlotChartNode ? mrmlPlotChartNode->GetID() : nullptr);

  vtkMRMLSelectionNode* selectionNode = vtkMRMLSelectionNode::SafeDownCast(
    q->mrmlScene() ? q->mrmlScene()->GetNodeByID("vtkMRMLSelectionNodeSingleton") : nullptr);
  if (selectionNode)
    {
    selectionNode->SetActivePlotChartID(mrmlPlotChartNode ? mrmlPlotChartNode->GetID() : "");
    }

  q->updateWidgetFromMRML();
}


// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotSeriesNodesSelected()
{
  Q_Q(qMRMLPlotViewControllerWidget);
  if (!q->mrmlPlotViewNode() || !this->PlotChartNode)
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
void qMRMLPlotViewControllerWidgetPrivate::onPlotSeriesNodeAdded(vtkMRMLNode *node)
{
  Q_Q(qMRMLPlotViewControllerWidget);
  if (!this->PlotChartNode || !q->mrmlScene())
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
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onPlotTypeChanged(int plotType)
{
  if (!this->PlotChartNode)
    {
    return;
    }
  this->PlotChartNode->SetPropertyToAllPlotSeriesNodes(vtkMRMLPlotChartNode::PlotType,
    vtkMRMLPlotSeriesNode::GetPlotTypeAsString(plotType));
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidgetPrivate::onInteractionModeChanged(int interactionMode)
{
  Q_Q(qMRMLPlotViewControllerWidget);
  if (!q->mrmlPlotViewNode())
    {
    return;
    }
  q->mrmlPlotViewNode()->SetInteractionMode(interactionMode);
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
  this->setMRMLScene(nullptr);
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
  if (!this->mrmlPlotViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return;
    }
  this->mrmlPlotViewNode()->SetLayoutLabel(newViewLabel.toUtf8());
}

//---------------------------------------------------------------------------
QString qMRMLPlotViewControllerWidget::viewLabel()const
{
  if (!this->mrmlPlotViewNode())
    {
    qCritical() << Q_FUNC_INFO << " failed: must set view node first";
    return QString();
    }
  return this->mrmlPlotViewNode()->GetLayoutLabel();
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::setMRMLPlotViewNode(
    vtkMRMLPlotViewNode * viewNode)
{
  Q_D(qMRMLPlotViewControllerWidget);
  this->setMRMLViewNode(viewNode);
}

//---------------------------------------------------------------------------
 vtkMRMLPlotViewNode* qMRMLPlotViewControllerWidget::mrmlPlotViewNode()const
{
  Q_D(const qMRMLPlotViewControllerWidget);
  return vtkMRMLPlotViewNode::SafeDownCast(this->mrmlViewNode());
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::fitPlotToAxes()
{
  Q_D(qMRMLPlotViewControllerWidget);
  if(!d->PlotView)
    {
    return;
    }
  d->PlotView->fitToContent();
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::onExportButton()
{
  Q_D(qMRMLPlotViewControllerWidget);
  if(!d->PlotView || !this->mrmlPlotViewNode())
    {
    return;
    }

  vtkMRMLPlotChartNode* mrmlPlotChartNode = d->GetPlotChartNodeFromView();

  QString name;
  if (mrmlPlotChartNode)
    {
    name = mrmlPlotChartNode->GetName();
    }

  QString fileName = QFileDialog::getSaveFileName(this, tr("Save as SVG"),
    name, tr("Scalable Vector Graphics (*.svg)"));
  if (!fileName.isEmpty())
    {
    d->PlotView->saveAsSVG(fileName);
    }
}

//---------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLPlotViewControllerWidget);

  if (!this->mrmlPlotViewNode() || !this->mrmlScene())
    {
    return;
    }

  d->ViewLabel->setText(this->mrmlPlotViewNode()->GetLayoutLabel());

  // PlotChartNode selector
  vtkMRMLPlotChartNode* mrmlPlotChartNode = d->GetPlotChartNodeFromView();

  if (mrmlPlotChartNode != d->PlotChartNode)
    {
    this->qvtkReconnect(d->PlotChartNode, mrmlPlotChartNode, vtkCommand::ModifiedEvent,
      this, SLOT(updateWidgetFromMRML()));
    d->PlotChartNode = mrmlPlotChartNode;
    }

  bool wasBlocked = d->plotChartComboBox->blockSignals(true);
  d->plotChartComboBox->setCurrentNode(mrmlPlotChartNode);
  d->plotChartComboBox->blockSignals(wasBlocked);

  d->plotTypeComboBox->setEnabled(mrmlPlotChartNode != nullptr);
  d->plotSeriesComboBox->setEnabled(mrmlPlotChartNode != nullptr);

  if (!mrmlPlotChartNode)
    {
    // Set the widgets to default states
    bool wasBlocked = d->plotTypeComboBox->blockSignals(true);
    d->plotTypeComboBox->setCurrentIndex(-1);
    d->plotTypeComboBox->blockSignals(wasBlocked);

    bool plotBlockSignals = d->plotSeriesComboBox->blockSignals(true);
    for (int idx = 0; idx < d->plotSeriesComboBox->nodeCount(); idx++)
      {
      d->plotSeriesComboBox->setCheckState(d->plotSeriesComboBox->nodeFromIndex(idx), Qt::Unchecked);
      }
    d->plotSeriesComboBox->blockSignals(plotBlockSignals);
    return;
    }

  // Plot series selector
  bool plotBlockSignals = d->plotSeriesComboBox->blockSignals(true);
  for (int idx = 0; idx < d->plotSeriesComboBox->nodeCount(); idx++)
    {
    vtkMRMLNode* plotSeriesNode = d->plotSeriesComboBox->nodeFromIndex(idx);
    Qt::CheckState checkState = Qt::Unchecked;
    if (plotSeriesNode && mrmlPlotChartNode->HasPlotSeriesNodeID(plotSeriesNode->GetID()))
      {
      checkState = Qt::Checked;
      }
    d->plotSeriesComboBox->setCheckState(plotSeriesNode, checkState);
    }
  d->plotSeriesComboBox->blockSignals(plotBlockSignals);

  d->actionShow_Grid->setChecked(mrmlPlotChartNode->GetGridVisibility());
  d->actionShow_Legend->setChecked(mrmlPlotChartNode->GetLegendVisibility());

  wasBlocked = d->interactionModeComboBox->blockSignals(true);
  d->interactionModeComboBox->setCurrentIndex(this->mrmlPlotViewNode()->GetInteractionMode());
  d->interactionModeComboBox->blockSignals(wasBlocked);

  // Show plot type if they are the same in all selected plot nodes.
  wasBlocked = d->plotTypeComboBox->blockSignals(true);
  std::string plotType;
  if (mrmlPlotChartNode->GetPropertyFromAllPlotSeriesNodes(vtkMRMLPlotChartNode::PlotType, plotType))
    {
    d->plotTypeComboBox->setCurrentIndex(vtkMRMLPlotSeriesNode::GetPlotTypeFromString(plotType.c_str()));
    }
  else
    {
    d->plotTypeComboBox->setCurrentIndex(-1);
    }
  d->plotTypeComboBox->blockSignals(wasBlocked);
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
  bool plotBlockSignals = d->plotSeriesComboBox->blockSignals(true);

  this->Superclass::setMRMLScene(newScene);

  d->plotChartComboBox->blockSignals(plotChartBlockSignals);
  d->plotSeriesComboBox->blockSignals(plotBlockSignals);

  if (this->mrmlScene())
    {
    this->updateWidgetFromMRML();
    }
}

// --------------------------------------------------------------------------
void qMRMLPlotViewControllerWidget::updateWidgetFromMRMLView()
{
  Superclass::updateWidgetFromMRMLView();
  this->updateWidgetFromMRML();
}
