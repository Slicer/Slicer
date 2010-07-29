/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageErode.h,v $
  Date:      $Date: 2006/01/06 17:56:40 $
  Version:   $Revision: 1.17 $

=========================================================================auto=*/

#include "qSlicerAbstractModule.h"

#include <stdlib.h>

class AModule: public qSlicerAbstractModule
{
public:
  virtual QString title()const { return "A Title";}
  virtual qSlicerAbstractModuleRepresentation* createWidgetRepresentation()
  {
    return 0;
  }

  virtual vtkSlicerLogic* createLogic()
  {
    return 0;
  }
protected:
  virtual void setup () {}
};

int qSlicerAbstractModuleTest1(int, char * [] )
{
  AModule module;

  return EXIT_SUCCESS;
}

