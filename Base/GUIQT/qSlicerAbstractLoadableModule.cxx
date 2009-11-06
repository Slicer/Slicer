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
}

//-----------------------------------------------------------------------------
qSlicerAbstractLoadableModule::~qSlicerAbstractLoadableModule()
{
  if (this->initialized()) { delete this->Internal; }
}

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
QString qSlicerAbstractLoadableModule::aboutText()
{
  qDebug() << "qSlicerAbstractLoadableModule::aboutText - Not implemented"; 
  return QString();
}
