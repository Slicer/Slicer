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
void qSlicerAbstractLoadableModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo(); 
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::helpText()
{
  qDebug() << "qSlicerAbstractLoadableModule::helpText - Not implemented"; 
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::aboutText()
{
  qDebug() << "qSlicerAbstractLoadableModule::aboutText - Not implemented"; 
  return QString();
}
