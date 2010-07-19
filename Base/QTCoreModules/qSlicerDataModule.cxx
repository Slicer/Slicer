// SlicerQt includes
#include "qSlicerDataModule.h"
#include "qSlicerDataModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerDataModulePrivate: public ctkPrivate<qSlicerDataModule>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerDataModule);
};

//-----------------------------------------------------------------------------
qSlicerDataModule::qSlicerDataModule(QObject* parentObject)
  :qSlicerAbstractCoreModule(parentObject)
{
  CTK_INIT_PRIVATE(qSlicerDataModule);
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerDataModule::createWidgetRepresentation()
{
  return new qSlicerDataModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerDataModule::createLogic()
{
  return 0;
}

//-----------------------------------------------------------------------------
QString qSlicerDataModule::helpText()const
{
  QString help =
    "The Data Module displays and permits operations on the MRML tree, and "
    "creates and edits transformation hierarchies.<br>"
    "The Load panels exposes options for loading data. Helpful comments can be "
    "opened by clicking on the \"information\" icons in each load panel.<br>"
    "<a href=\"%1/Modules:Data-Documentation-3.6\">%1/Modules:Data-Documentation"
    "-3.6</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerDataModule::acknowledgementText()const
{
  QString about =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC, and the Slicer "
    "Community.<br>"
    "See <a href=\"http://www.slicer.org\">www.slicer.org</a> for details.<br>"
    "The Data module was contributed by Alex Yarmarkovich, Isomics Inc. with "
    "help from others at SPL, BWH (Ron Kikinis, Wendy Plesniak)";
  return about.arg(this->slicerWikiUrl());
}
