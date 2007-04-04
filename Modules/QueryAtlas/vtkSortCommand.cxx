/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSortCommand.cxx,v $
  Date:      $Date: 2006/05/26 19:59:43 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*
Authors: Michael McKenna, David Small, Steve Pieper.
    Small Design Firm, Inc.,
    in association with Isomics, Inc.,
    and Partners Health Care.  MA, USA.
    June, 2004

===========================================================*/


#include "vtkSortCommand.h"
#include "vtkObject.h"
#include "vtkRenderer.h"
#include "vtkActorCollection.h"
//#include "vtkRendererCollection.h"
#include "vtkActor.h"
//#include "vtkRenderWindow.h"
#include "vtkMapper.h"
#include "vtkCamera.h"
#include "vtkMath.h"
#include "vtkFollower.h"

#include "vtkCard.h"
#include "vtkTextureText.h"


// NOTE - we can't use the standard New macro, since this is not a vtkObject, but a vtkObjectBase


vtkSortCommand *vtkSortCommand::New() {
    return new vtkSortCommand();
}


vtkSortCommand::vtkSortCommand()
{
    this->Sorter = NULL;
}


vtkSortCommand::~vtkSortCommand() { 
}


void vtkSortCommand::SetSorter(vtkSorter *s) {
    this->Sorter = s;
    vtkRenderer *r = s->GetRenderer();
    if (r == NULL) return;
    r->AddObserver(vtkCommand::StartEvent, this);
}


void vtkSortCommand::Execute(vtkObject *caller, unsigned long eventId, void *callData) {
    //printf("vtkSortCommand::Execute - caller = ");  caller->Print(cout);  flush(cout);

    if (this->Sorter == NULL) return;

    if (!caller->IsA("vtkRenderer")) {
        //vtkErrorMacro("vtkSortCommand::Execute: Error - caller is not a renderer.\n");
        return;
    }

    vtkRenderer *r = (vtkRenderer *)caller;
    if (r == NULL) {
        //vtkErrorMacro("vtkSortCommand::Execute: NO Renderer.\n");
        return;
    }

    this->Sorter->DepthSort();
}



void vtkSortCommand::PrintSelf(ostream& os, vtkIndent indent)
{
//  this->Superclass::PrintSelf(os,indent);

  /*  TODO - do print
  os << indent << "X Resolution: " << this->XResolution << "\n";
  os << indent << "Y Resolution: " << this->YResolution << "\n";
...
*/

}
