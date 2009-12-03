#include "qSlicerVolumesModule.h"
#include "ui_qSlicerVolumesModule.h"

// QT includes
#include <QtPlugin>
#include <QDebug>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumesModule, qSlicerVolumesModule);

//-----------------------------------------------------------------------------
struct qSlicerVolumesModulePrivate: public qCTKPrivate<qSlicerVolumesModule>,
                                    public Ui_qSlicerVolumesModule
{
  qSlicerVolumesModulePrivate()
    {
    }
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerVolumesModule, QWidget*);

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::setup()
{
  this->Superclass::setup();
  QCTK_D(qSlicerVolumesModule);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}

//-----------------------------------------------------------------------------
QAction* qSlicerVolumesModule::showModuleAction()
{
  return new QAction(QIcon(":/Icons/Volumes.png"), tr("Show Volumes module"), this);
}

//-----------------------------------------------------------------------------
bool qSlicerVolumesModule::isShowModuleActionVisibleByDefault()
{
  return true;
}
