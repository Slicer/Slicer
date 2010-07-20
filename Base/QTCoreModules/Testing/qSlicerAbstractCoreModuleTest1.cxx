/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageErode.h,v $
  Date:      $Date: 2006/01/06 17:56:40 $
  Version:   $Revision: 1.17 $

=========================================================================auto=*/

#include "qSlicerAbstractCoreModule.h"

#include <stdlib.h>
#include <iostream>

class ACoreModule: public qSlicerAbstractCoreModule
{
public:
  virtual QString title()const
  {
    return "A title \n\t#$%^&*";
  }
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation()
  {
    return 0;
  }
  vtkSlicerLogic* createLogic()
  {
    return 0;
  }
};


int qSlicerAbstractCoreModuleTest1(int, char * [] )
{
  qSlicerAbstractCoreModule* module = new ACoreModule;
  delete module;

  return EXIT_SUCCESS;
}
