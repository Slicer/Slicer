#include "qSlicerAbstractLoadableModule.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerAbstractLoadableModulePrivate: public qCTKPrivate<qSlicerAbstractLoadableModule>
{
  qSlicerAbstractLoadableModulePrivate()
    {
    this->Name = "NA";
    }
  QString Name;
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractLoadableModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::setup()
{
  Q_ASSERT(qctk_d() != 0);
}

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractLoadableModule, QString, name, Name);
QCTK_SET_CXX(qSlicerAbstractLoadableModule, const QString&, setName, Name);

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
