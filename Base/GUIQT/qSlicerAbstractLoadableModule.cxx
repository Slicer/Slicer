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
qSlicerWidgetCxxZeroInitConstructorMacro(qSlicerAbstractLoadableModule);
qSlicerCxxDestructorMacro(qSlicerAbstractLoadableModule);

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::initializer()
{
  this->Internal = new qInternal;
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
QString qSlicerAbstractLoadableModule::acknowledgementText()
{
  qDebug() << "qSlicerAbstractLoadableModule::acknowledgementText - Not implemented"; 
  return QString();
}
