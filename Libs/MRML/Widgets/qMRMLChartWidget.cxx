

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
}

// --------------------------------------------------------------------------
void qMRMLChartWidget::addDisplayableManager(const QString& vtkNotUsed(dManager))
{
  //Q_D(qMRMLChartWidget);

  // Charts do not use DisplayableManager (yet)
  //d->ChartView->addDisplayableManager(dManager);
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
