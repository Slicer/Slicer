// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerEMSegmentModuleWidget.h"
#include "ui_qSlicerEMSegmentModule.h"

//-----------------------------------------------------------------------------
class qSlicerEMSegmentModuleWidgetPrivate: public ctkPrivate<qSlicerEMSegmentModuleWidget>,
                                         public Ui_qSlicerEMSegmentModule
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerEMSegmentModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerEMSegmentModuleWidget::setup()
{
  CTK_D(qSlicerEMSegmentModuleWidget);
  d->setupUi(this);

}

//-----------------------------------------------------------------------------
QAction* qSlicerEMSegmentModuleWidget::showModuleAction()
{
  return new QAction(QIcon(":/Icons/EMSegment.png"), tr("Show EMSegment module"), this);
}

