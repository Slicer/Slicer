#include "qSlicerVolumesModule.h" 

#include "ui_qSlicerVolumesModule.h" 

#include <QtPlugin>
#include <QDebug>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumesModule, qSlicerVolumesModule)

//-----------------------------------------------------------------------------
class qSlicerVolumesModule::qInternal: public Ui::qSlicerVolumesModule
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerVolumesModule::qSlicerVolumesModule(QWidget *parent) : Superclass(parent)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesModule::~qSlicerVolumesModule()
{
  if (this->initialized()) { delete this->Internal; }
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::initializer()
{
  this->Superclass::initializer();
  
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}
