#include "qSlicerVolumesModule.h"
#include "ui_qSlicerVolumesModule.h"

// QT includes
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
qSlicerCxxInternalConstructor1Macro(qSlicerVolumesModule, QWidget*);
qSlicerCxxDestructorMacro(qSlicerVolumesModule);

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::setup()
{
  this->Superclass::setup();
  Q_ASSERT(this->Internal != 0);

  this->Internal->setupUi(this);
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo();
}
