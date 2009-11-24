#include "qSlicerMeasurementsModule.h"

#include "ui_qSlicerMeasurementsModule.h"

#include <QtPlugin>
#include <QDebug>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMeasurementsModule, qSlicerMeasurementsModule);

//-----------------------------------------------------------------------------
class qSlicerMeasurementsModule::qInternal : public Ui::qSlicerMeasurementsModule
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerMeasurementsModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerMeasurementsModule);

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModule::setup()
{
  this->Superclass::setup();
  Q_ASSERT(this->Internal != 0);

  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}
