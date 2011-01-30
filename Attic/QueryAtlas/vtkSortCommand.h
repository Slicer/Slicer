/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSortCommand.h,v $
  Date:      $Date: 2006/05/26 19:59:44 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*===========================================================

Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#ifndef __vtkSortCommand_h
#define __vtkSortCommand_h


#include "vtkObject.h"
#include "vtkCommand.h"
#include "vtkSorter.h"

#include <vtkQueryAtlasConfigure.h>


// strange issue /w vtkCommand not being a vtkObject - so we use vtkSorter to do all work except Execute()


class VTK_QUERYATLAS_EXPORT vtkSortCommand : public vtkCommand
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    //vtkTypeRevisionMacro(vtkSortCommand, vtkCommand);
    //vtkTypeRevisionMacro(vtkSortCommand, vtkObjectBase);

    static vtkSortCommand *New();

    // these macros won't work with a vtkCommand
    //vtkGetObjectMacro(Sorter, vtkSorter);  
    //vtkSetObjectMacro(Sorter, vtkSorter);  

    void SetSorter(vtkSorter *s);

protected:
    vtkSortCommand();
    ~vtkSortCommand(); 

    vtkSorter *Sorter;
//BTX
    void vtkSortCommand::Execute(vtkObject *caller, unsigned long eventId, void *callData);
//ETX
};

#endif
