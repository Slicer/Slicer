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
#include <ctkLogger.h>
#include <ctkPopupWidget.h>

// qMRML includes
#include "qMRMLPlotViewControllerWidget.h"
#include "qMRMLPlotView.h"
#include "qMRMLPlotWidget.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLPlotWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate
class qMRMLPlotWidgetPrivate
  : public QObject
{
  Q_DECLARE_PUBLIC(qMRMLPlotWidget);
protected:
  qMRMLPlotWidget* const q_ptr;
public:
  qMRMLPlotWidgetPrivate(qMRMLPlotWidget& object);
  ~qMRMLPlotWidgetPrivate();

  void init();

  qMRMLPlotView*       PlotView;
  qMRMLPlotViewControllerWidget* PlotController;
};


//---------------------------------------------------------------------------
qMRMLPlotWidgetPrivate::qMRMLPlotWidgetPrivate(qMRMLPlotWidget& object)
  : q_ptr(&object)
{
  this->PlotView = 0;
  this->PlotController = 0;
}

//---------------------------------------------------------------------------
qMRMLPlotWidgetPrivate::~qMRMLPlotWidgetPrivate()
{
}

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
  layout->addWidget(this->PlotView);

  this->PlotController->setPlotView(this->PlotView);

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->PlotView, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->PlotController, SLOT(setMRMLScene(vtkMRMLScene*)));
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
  d->PlotView->setMRMLScene(0);
  d->PlotController->setMRMLScene(0);
}


// --------------------------------------------------------------------------
void qMRMLPlotWidget::setMRMLPlotViewNode(vtkMRMLPlotViewNode* newPlotViewNode)
{
  Q_D(qMRMLPlotWidget);
  d->PlotView->setMRMLPlotViewNode(newPlotViewNode);
  d->PlotController->setMRMLPlotViewNode(newPlotViewNode);
}

// --------------------------------------------------------------------------
vtkMRMLPlotViewNode* qMRMLPlotWidget::mrmlPlotViewNode()const
{
  Q_D(const qMRMLPlotWidget);
  return d->PlotView->mrmlPlotViewNode();
}

// --------------------------------------------------------------------------
qMRMLPlotView* qMRMLPlotWidget::plotView()const
{
  Q_D(const qMRMLPlotWidget);
  return d->PlotView;
}

//---------------------------------------------------------------------------
void qMRMLPlotWidget::setViewLabel(const QString& newPlotViewLabel)
{
  Q_D(qMRMLPlotWidget);
  d->PlotController->setViewLabel(newPlotViewLabel);
}

//---------------------------------------------------------------------------
QString qMRMLPlotWidget::viewLabel()const
{
  Q_D(const qMRMLPlotWidget);
  return d->PlotController->viewLabel();
}

//---------------------------------------------------------------------------
void qMRMLPlotWidget::setColorLogic(vtkMRMLColorLogic* colorLogic)
{
  Q_D(const qMRMLPlotWidget);
  d->PlotView->setColorLogic(colorLogic);
}

//---------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLPlotWidget::colorLogic()const
{
  Q_D(const qMRMLPlotWidget);
  return d->PlotView->colorLogic();
}
