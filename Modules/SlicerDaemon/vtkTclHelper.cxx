/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTclHelper.cxx,v $
  Date:      $Date: 2006/01/06 17:58:00 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/

/* 
 * vtkTclHelper allows access to Tcl and vtk routines in the same class
 * inspired by vtkTkRenderWidget and similar classes.
 */


#include <stdlib.h>

#include "vtkTclHelper.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkTclUtil.h"

vtkCxxRevisionMacro(vtkTclHelper, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkTclHelper);


vtkTclHelper::vtkTclHelper()
{
    this->Interp = NULL;
    this->ImageData = NULL;
}


vtkTclHelper::~vtkTclHelper() 
{ 
}


void 
vtkTclHelper::SetInterpFromCommand(unsigned long tag)
{
    vtkCommand *c = this->GetCommand(tag);
    vtkTclCommand *tc = (vtkTclCommand *) c;

    this->Interp = tc->Interp;
}

void 
vtkTclHelper::SendImageDataScalars(char *sockname)
{
    int mode;
    Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);

    if ( ! (mode & TCL_WRITABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not writable\n");
        return;
    }

    if ( this->ImageData == NULL )
    {   vtkErrorMacro ("Image Data is NULL");
        return;
    }

    int dims[3];
    this->ImageData->GetDimensions(dims);
    int bytes = this->ImageData->GetScalarSize() * 
                    this->ImageData->GetNumberOfScalarComponents() * 
                        dims[0] * dims[1] * dims[2];

    int written = Tcl_WriteRaw(channel, (char *) this->ImageData->GetScalarPointer(), bytes);
    Tcl_Flush(channel);

    if ( written != bytes )
    {   vtkErrorMacro ("Only wrote " << written << " but expected to write " << bytes << "\n");
        return;
    }
}

void 
vtkTclHelper::ReceiveImageDataScalars(char *sockname)
{
    int mode;
    Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);

    if ( ! (mode & TCL_READABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not readable" << "\n");
        return;
    }

    if ( this->ImageData == NULL )
    {   vtkErrorMacro ("Image Data is NULL");
        return;
    }

    int dims[3];
    this->ImageData->GetDimensions(dims);
    int bytes = this->ImageData->GetScalarSize() * 
                    this->ImageData->GetNumberOfScalarComponents() * 
                        dims[0] * dims[1] * dims[2];

    int read = Tcl_Read(channel, (char *) this->ImageData->GetScalarPointer(), bytes);

    if ( read != bytes )
    {   vtkErrorMacro ("Only read " << read << " but expected to read " << bytes << "\n");
        return;
    }
}

const char *
vtkTclHelper::Execute (char *Command)
{
  int res;
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION <= 2
  res = Tcl_GlobalEval(this->Interp, this->Command);
#else
  res = Tcl_EvalEx(this->Interp, Command, -1, TCL_EVAL_GLOBAL);
#endif  

  return Tcl_GetStringResult (this->Interp);
}


void vtkTclHelper::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent << "Interp: " << this->Interp << "\n";
    os << indent << "ImageData: " << this->ImageData << "\n";

}

