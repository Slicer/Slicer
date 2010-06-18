#include "qSlicerWelcomeModule.h"

// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include "qSlicerWelcomeModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWelcomeModule, qSlicerWelcomeModule);

//-----------------------------------------------------------------------------
class qSlicerWelcomeModulePrivate: public ctkPrivate<qSlicerWelcomeModule>
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerWelcomeModule, QObject*);

QString qSlicerWelcomeModule::helpText()const
{
  return QString();
}

QString qSlicerWelcomeModule::acknowledgementText()const
{
  return "This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC and the Slicer Community. See <a>http://www.slicer.org</a> for details. We would also like to express our sincere thanks to members of the Slicer User Community who have helped us to design the contents of this Welcome Module, and whose feedback continues to improve functionality, usability and Slicer user experience\n.";
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerWelcomeModule::createWidgetRepresentation()
{
  return new qSlicerWelcomeModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerWelcomeModule::createLogic()
{
  return 0;
}
