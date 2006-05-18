/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTclHelper.h,v $
  Date:      $Date: 2006/01/06 17:58:01 $
  Version:   $Revision: 1.4 $

=========================================================================auto=*/
// .NAME vtkTclHelper - a bridge between tcl and vtk
// -- this class allows binary data to be read from a tcl channel 
//    and put into a vtk data structure (vtkImageData for now)

// .SECTION Description

// .SECTION See Also
// vtkTkRenderWidget 


#ifndef __vtkTclHelper_h
#define __vtkTclHelper_h

#include "vtkTcl.h"
#include "vtkObject.h"
#include "vtkImageData.h"

#include <vtkSlicerDaemonWin32Header.h>


class VTK_SLICERDAEMON_EXPORT vtkTclHelper : public vtkObject
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkTclHelper, vtkObject);

    static vtkTclHelper *New();

    void SetInterpFromCommand(unsigned long tag);

    vtkSetObjectMacro(ImageData, vtkImageData);
    vtkGetObjectMacro(ImageData, vtkImageData);

    void SendImageDataScalars(char *sockname);
    void ReceiveImageDataScalars(char *sockname);
    const char *Execute (char *Command);


protected:
    vtkTclHelper();
    ~vtkTclHelper(); 

    vtkImageData *ImageData;           
    Tcl_Interp *Interp;           /* Tcl interpreter */
};


#endif
