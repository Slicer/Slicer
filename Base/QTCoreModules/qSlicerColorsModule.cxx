// SlicerQt includes
#include "qSlicerColorsModule.h"
#include "qSlicerColorsModuleWidget.h"

// Slicer Logic includes
#include "vtkSlicerColorLogic.h"

//-----------------------------------------------------------------------------
class qSlicerColorsModulePrivate: public ctkPrivate<qSlicerColorsModule>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerColorsModule);
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerColorsModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerColorsModule::createWidgetRepresentation()
{
  return new qSlicerColorsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerColorsModule::createLogic()
{
  return vtkSlicerColorLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerColorsModule::helpText()const
{
  QString help =
    "The <b>Colors Module</b> manages color look up tables.<br>"
    "For more information see <a>http://www.slicer.org/slicerWiki/index.php/"
    "Modules:Colors-Documentation-3.6</a><br>"
    "Tables are used by mappers to translate between an integer and a colour "
    "value for display of models and volumes.<br>Slicer supports three kinds "
    "of tables:<br>"
    "1. Continuous scales, like the greyscale table.<br>"
    "2. Parametric tables, defined by an equation, such as the FMRIPA table.<br>"
    "3. Discrete tables, such as those read in from a file.<br><br>"
    "You can specify a directory from which to read color files using the "
    "View -> Application Settings window, Module Settings frame, in the "
    "User defined color file paths section. The color file format is a plain "
    "text file with the .txt or .ctbl extension. Each line in the file has:<br>"
    "label\tname\tR\tG\tB\tA\nlabel is an integer, name a string, and RGBA are "
    "0-255.<br><br>Users are only allowed to edit User type tables. Use the "
    "Edit frame to create a new color table (you can copy from an existing "
    "one), and save it to a file using the File, Save interface.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerColorsModule::acknowledgementText()const
{
  QString about =
    "This module was developed by Nicole Aucoin, SPL, BWH (Ron Kikinis). "
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community. See %1 for details.";
  return about.arg(this->slicerWikiUrl());
}
