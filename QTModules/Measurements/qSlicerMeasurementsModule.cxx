#include "qSlicerMeasurementsModule.h" 

#include "ui_qSlicerMeasurementsModule.h" 

#include <QtPlugin>
#include <QDebug>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMeasurementsModule, qSlicerMeasurementsModule)

//-----------------------------------------------------------------------------
class qSlicerMeasurementsModule::qInternal : public Ui::qSlicerMeasurementsModule
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerWidgetCxxZeroInitConstructorMacro(qSlicerMeasurementsModule);
qSlicerCxxDestructorMacro(qSlicerMeasurementsModule);

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModule::initializer()
{
  this->Superclass::initializer();
  
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerMeasurementsModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}
