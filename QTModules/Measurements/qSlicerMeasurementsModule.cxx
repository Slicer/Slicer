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
qSlicerMeasurementsModule::qSlicerMeasurementsModule(QWidget *parent) : Superclass(parent)
{
}

//-----------------------------------------------------------------------------
qSlicerMeasurementsModule::~qSlicerMeasurementsModule()
{
  if (this->initialized()) { delete this->Internal; }
}

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
