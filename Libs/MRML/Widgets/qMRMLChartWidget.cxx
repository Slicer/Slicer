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
#include "qMRMLChartViewControllerWidget.h"
#include "qMRMLChartView.h"
#include "qMRMLChartWidget.h"

//--------------------------------------------------------------------------
static ctkLogger logger("org.slicer.libs.qmrmlwidgets.qMRMLChartWidget");
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
// qMRMLSliceViewPrivate
class qMRMLChartWidgetPrivate
  : public QObject
{
  Q_DECLARE_PUBLIC(qMRMLChartWidget);
protected:
  qMRMLChartWidget* const q_ptr;
public:
  qMRMLChartWidgetPrivate(qMRMLChartWidget& object);
  ~qMRMLChartWidgetPrivate();

  void init();

  qMRMLChartView*       ChartView;
  qMRMLChartViewControllerWidget* ChartController;
};


//---------------------------------------------------------------------------
qMRMLChartWidgetPrivate::qMRMLChartWidgetPrivate(qMRMLChartWidget& object)
  : q_ptr(&object)
{
  this->ChartView = 0;
  this->ChartController = 0;
}

//---------------------------------------------------------------------------
qMRMLChartWidgetPrivate::~qMRMLChartWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLChartWidgetPrivate::init()
{
  Q_Q(qMRMLChartWidget);

  QVBoxLayout* layout = new QVBoxLayout(q);
  layout->setSpacing(0);
  layout->setContentsMargins(0, 0, 0, 0);

  this->ChartController = new qMRMLChartViewControllerWidget;
  layout->addWidget(this->ChartController);

  this->ChartView = new qMRMLChartView;
  layout->addWidget(this->ChartView);

  this->ChartController->setChartView(this->ChartView);

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ChartView, SLOT(setMRMLScene(vtkMRMLScene*)));
  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ChartController, SLOT(setMRMLScene(vtkMRMLScene*)));
}

// --------------------------------------------------------------------------
// qMRMLChartWidget methods

// --------------------------------------------------------------------------
qMRMLChartWidget::qMRMLChartWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qMRMLChartWidgetPrivate(*this))
{
  Q_D(qMRMLChartWidget);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLChartWidget::~qMRMLChartWidget()
{
  Q_D(qMRMLChartWidget);
  d->ChartView->setMRMLScene(0);
  d->ChartController->setMRMLScene(0);
}


// --------------------------------------------------------------------------
void qMRMLChartWidget::setMRMLChartViewNode(vtkMRMLChartViewNode* newChartViewNode)
{
  Q_D(qMRMLChartWidget);
  d->ChartView->setMRMLChartViewNode(newChartViewNode);
  d->ChartController->setMRMLChartViewNode(newChartViewNode);
}

// --------------------------------------------------------------------------
vtkMRMLChartViewNode* qMRMLChartWidget::mrmlChartViewNode()const
{
  Q_D(const qMRMLChartWidget);
  return d->ChartView->mrmlChartViewNode();
}

// --------------------------------------------------------------------------
qMRMLChartView* qMRMLChartWidget::chartView()const
{
  Q_D(const qMRMLChartWidget);
  return d->ChartView;
}

//---------------------------------------------------------------------------
void qMRMLChartWidget::setViewLabel(const QString& newChartViewLabel)
{
  Q_D(qMRMLChartWidget);
  d->ChartController->setViewLabel(newChartViewLabel);
}

//---------------------------------------------------------------------------
QString qMRMLChartWidget::viewLabel()const
{
  Q_D(const qMRMLChartWidget);
  return d->ChartController->viewLabel();
}

//---------------------------------------------------------------------------
void qMRMLChartWidget::setColorLogic(vtkMRMLColorLogic* colorLogic)
{
  Q_D(const qMRMLChartWidget);
  d->ChartView->setColorLogic(colorLogic);
}

//---------------------------------------------------------------------------
vtkMRMLColorLogic* qMRMLChartWidget::colorLogic()const
{
  Q_D(const qMRMLChartWidget);
  return d->ChartView->colorLogic();
}
