#include "qSlicerVolumesModule.h" 

#include "ui_qSlicerVolumesModule.h" 

#include <QDebug>

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
  this->Internal = new qInternal;
  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerVolumesModule::~qSlicerVolumesModule()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesModule::moduleTitle()
{
  return "Volumes"; 
}
