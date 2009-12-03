#include "qSlicerMeasurementsModule.h"
#include "ui_qSlicerMeasurementsModule.h"

// QT includes
#include <QtPlugin>
#include <QDebug>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMeasurementsModule, qSlicerMeasurementsModule);

//-----------------------------------------------------------------------------
struct qSlicerMeasurementsModulePrivate: public qCTKPrivate<qSlicerMeasurementsModule>,
                                         public Ui_qSlicerMeasurementsModule
{
  qSlicerMeasurementsModulePrivate()
    {
    }
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMeasurementsModule, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModule::setup()
{
  this->Superclass::setup();
  QCTK_D(qSlicerMeasurementsModule);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QAction* qSlicerMeasurementsModule::showModuleAction()
{
  return new QAction(QIcon(":/Icons/Measurements.png"), tr("Show Measurements module"), this);
}

//-----------------------------------------------------------------------------
bool qSlicerMeasurementsModule::showModuleActionVisibleByDefault()
{
  return true;
}
