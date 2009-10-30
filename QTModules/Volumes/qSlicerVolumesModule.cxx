#include "qSlicerVolumesModule.h" 

#include "ui_qSlicerVolumesModule.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
qSlicerGetModuleTitleDefinitionMacro(qSlicerVolumesModule, "Volumes"); 

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
void qSlicerVolumesModule::dumpObjectInfo()
{
  this->Superclass::dumpObjectInfo();
}
