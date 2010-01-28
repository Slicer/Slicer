/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerTransformsModule.h"

// SlicerQT includes
#include "vtkSlicerTransformLogic.h"
#include "qSlicerTransformsModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerTransformsModulePrivate: public qCTKPrivate<qSlicerTransformsModule>
{
public:
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerTransformsModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerTransformsModule::createWidgetRepresentation()
{
  return new qSlicerTransformsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerTransformsModule::createLogic()
{
  return vtkSlicerTransformLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModule::helpText()const
{
  QString help =
    "The Transforms Module creates and edits transforms. \n"
    "<a href=%1/Modules:Transforms-Documentation-3.4>%1/Modules:Transforms-Documentation-3.4</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsModule::acknowledgementText()const
{
  QString acknowledgement =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. "
    "See <a href=\"http://www.slicer.org\">http://www.slicer.org</a> for details.\n"
    "The Transforms module was contributed by Alex Yarmarkovich, Isomics Inc. with "
    "help from others at SPL, BWH (Ron Kikinis)";
  return acknowledgement;
}
