/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerMRMLTreeModule.h"

// SlicerQt includes
#include "qSlicerMRMLTreeModuleWidget.h"

//-----------------------------------------------------------------------------
class qSlicerMRMLTreeModulePrivate: public ctkPrivate<qSlicerMRMLTreeModule>
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerMRMLTreeModule, QObject*);

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget * qSlicerMRMLTreeModule::createWidgetRepresentation()
{
  return new qSlicerMRMLTreeModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerMRMLTreeModule::createLogic()
{
  return 0;
}

//-----------------------------------------------------------------------------
QString qSlicerMRMLTreeModule::helpText()const
{
  QString help = "%1";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerMRMLTreeModule::acknowledgementText()const
{
  QString acknowledgement = "";
  return acknowledgement;
}
