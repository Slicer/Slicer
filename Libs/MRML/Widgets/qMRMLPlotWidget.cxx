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
#include <QDebug>
#include <QFileInfo>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QToolButton>

// CTK includes
#include <ctkPopupWidget.h>

// MRML includes
#include <vtkMRMLPlotViewNode.h>

// qMRML includes
#include "qMRMLPlotViewControllerWidget.h"
#include "qMRMLPlotView.h"
#include "qMRMLPlotWidget.h"

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate
class qMRMLPlotWidgetPrivate : public QObject
{
  Q_DECLARE_PUBLIC(qMRMLPlotWidget);

protected:
  qMRMLPlotWidget* const q_ptr;

public:
  qMRMLPlotWidgetPrivate(qMRMLPlotWidget& object);
  ~qMRMLPlotWidgetPrivate() override;

  void init();

  qMRMLPlotView* PlotView;
  qMRMLPlotViewControllerWidget* PlotController;
};

//---------------------------------------------------------------------------
qMRMLPlotWidgetPrivate::qMRMLPlotWidgetPrivate(qMRMLPlotWidget& object)
  : q_ptr(&object)
{
  this->PlotView = nullptr;
  this->PlotController = nullptr;
}

//---------------------------------------------------------------------------
qMRMLPlotWidgetPrivate::~qMRMLPlotWidgetPrivate() = default;

//---------------------------------------------------------------------------
void qMRMLPlotWidgetPrivate::init()
{
  Q_Q(qMRMLPlotWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  this->PlotController = new qMRMLPlotViewControllerWidget;
  layout->addWidget(this->PlotController);

  this->PlotView = new qMRMLPlotView;
  this->PlotView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(this->PlotView);

  this->PlotController->setPlotView(this->PlotView);

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->PlotView, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)), this->PlotController, SLOT(setMRMLScene(vtkMRMLScene*)));
}

// --------------------------------------------------------------------------
// qMRMLPlotWidget methods

// --------------------------------------------------------------------------
qMRMLPlotWidget::qMRMLPlotWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLPlotWidgetPrivate(*this))
{
  Q_D(qMRMLPlotWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLPlotWidget::~qMRMLPlotWidget()
{
  Q_D(qMRMLPlotWidget);
  d->PlotView->setMRMLScene(nullptr);
  d->PlotController->setMRMLScene(nullptr);
}

// --------------------------------------------------------------------------
void qMRMLPlotWidget::setMRMLPlotViewNode(vtkMRMLPlotViewNode* newPlotViewNode)
{
  Q_D(qMRMLPlotWidget);
  d->PlotView->setMRMLPlotViewNode(newPlotViewNode);
  d->PlotController->setMRMLPlotViewNode(newPlotViewNode);
}

//--------------------------------------------------------------------------
void qMRMLPlotWidget::setMRMLAbstractViewNode(vtkMRMLAbstractViewNode* newViewNode)
{
  vtkMRMLPlotViewNode* plotViewNode = vtkMRMLPlotViewNode::SafeDownCast(newViewNode);
  if (newViewNode && !plotViewNode)
  {
    qWarning() << Q_FUNC_INFO << " failed: Invalid view node type " << newViewNode->GetClassName()
               << ". Expected node type: vtkMRMLPlotViewNode";
  }
  this->setMRMLPlotViewNode(plotViewNode);
}

//--------------------------------------------------------------------------
vtkMRMLPlotViewNode* qMRMLPlotWidget::mrmlPlotViewNode() const
{
  Q_D(const qMRMLPlotWidget);
  return d->PlotView->mrmlPlotViewNode();
}

//--------------------------------------------------------------------------
vtkMRMLAbstractViewNode* qMRMLPlotWidget::mrmlAbstractViewNode() const
{
  return this->mrmlPlotViewNode();
}

//--------------------------------------------------------------------------
qMRMLPlotView* qMRMLPlotWidget::plotView() const
{
  Q_D(const qMRMLPlotWidget);
  return d->PlotView;
}

//--------------------------------------------------------------------------
QWidget* qMRMLPlotWidget::viewWidget() const
{
  return this->plotView();
}

//--------------------------------------------------------------------------
qMRMLPlotViewControllerWidget* qMRMLPlotWidget::plotController() const
{
  Q_D(const qMRMLPlotWidget);
  return d->PlotController;
}

//---------------------------------------------------------------------------
qMRMLViewControllerBar* qMRMLPlotWidget::controllerWidget() const
{
  return this->plotController();
}
