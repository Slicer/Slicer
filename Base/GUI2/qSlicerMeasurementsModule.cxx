#include "qSlicerMeasurementsModule.h" 

#include "ui_qSlicerMeasurementsModule.h" 

#include <QVector>
#include <QButtonGroup>
#include <QDebug>

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
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerMeasurementsModule::~qSlicerMeasurementsModule()
{
  delete this->Internal; 
}

