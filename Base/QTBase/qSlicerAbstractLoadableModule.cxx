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
qSlicerCxxInternalConstructor1Macro(qSlicerAbstractLoadableModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerAbstractLoadableModule);

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::initializer()
{
  Q_ASSERT(this->Internal != 0);
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
