
// Qt includes
#include <QActionGroup>
#include <QDebug>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QHBoxLayout>

// CTK includes
#include <ctkButtonGroup.h>
#include <ctkLogger.h>
#include <ctkPopupWidget.h>
#include <ctkSignalMapper.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLNodeFactory.h"
#include "qMRMLSceneViewMenu.h"
#include "qMRMLChartView.h"
#include "qMRMLChartViewControllerWidget_p.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLChartViewNode.h>
#include <vtkMRMLSceneViewNode.h>

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
  this->ChartViewNode = 0;
  this->ChartView = 0;
  this->MRMLChartNode = 0;
}

//---------------------------------------------------------------------------
qMRMLChartViewControllerWidgetPrivate::~qMRMLChartViewControllerWidgetPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLChartViewControllerWidgetPrivate::setupPopupUi()
{
  Q_Q(qMRMLChartViewControllerWidget);

  this->Superclass::setupPopupUi();
  this->PopupWidget->setAlignment(Qt::AlignBottom | Qt::AlignLeft);
  this->Ui_qMRMLChartViewControllerWidget::setupUi(this->PopupWidget);

  // Connect Chart selector
  // this->connect(this->ChartComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)),
  //               SLOT(onChartNodeSelected(vtkMRMLNode*)));

  // when the user select an entry already selected, we want to
  // synchronize with the linked slice logics as they mighy not have
  // the same entry selected 
  // this->connect(this->ChartComboBox, SIGNAL(nodeActivated(vtkMRMLNode*)),
  //              SLOT(onChartNodeSelected(vtkMRMLNode*)));

  QObject::connect(q, SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                   this->ChartComboBox, SLOT(setMRMLScene(vtkMRMLScene*)));
  
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

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidget::updateWidgetFromMRML()
{
  Q_D(qMRMLChartViewControllerWidget);

  if (!d->ChartViewNode)
    {
    return;
    }
}

// --------------------------------------------------------------------------
void qMRMLChartViewControllerWidgetPrivate::onChartNodeSelected(vtkMRMLNode * node)
{
  Q_Q(qMRMLChartViewControllerWidget);

  if (!this->MRMLChartNode)
    {
    return;
    }

  this->ChartViewNode->SetChartNodeID(node ? node->GetID() : 0);
}
