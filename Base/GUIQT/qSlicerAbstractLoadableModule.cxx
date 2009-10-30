#include "qSlicerAbstractLoadableModule.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerAbstractLoadableModule::qInternal
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerAbstractLoadableModule::qSlicerAbstractLoadableModule(QWidget *parent)
 :Superclass(parent)
{
  this->Internal = new qInternal;
}

//-----------------------------------------------------------------------------
qSlicerAbstractLoadableModule::~qSlicerAbstractLoadableModule()
{
  delete this->Internal; 
}

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::dumpObjectInfo()
{
  this->Superclass::dumpObjectInfo(); 
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::helpText()
{
  qDebug() << "qSlicerAbstractLoadableModule::helpText - Not implemented"; 
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::aboutText()
{
  qDebug() << "qSlicerAbstractLoadableModule::aboutText - Not implemented"; 
}
