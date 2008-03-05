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
#include "vtkMRMLVolumeNode.h"
#include "vtkMatrix4x4.h"
#include <vtkSlicerDaemonWin32Header.h>


class VTK_SLICERDAEMON_EXPORT vtkTclHelper : public vtkObject
{
public:
    void PrintSelf(ostream& os, vtkIndent indent);
    vtkTypeRevisionMacro(vtkTclHelper, vtkObject);

    static vtkTclHelper *New();

    // Description
    // hook for script to tell this instance what interpreter to use
    // - call it with the tag returned from AddObserver on this instance
    void SetInterpFromCommand(unsigned long tag);

    vtkSetObjectMacro(ImageData, vtkImageData);
    vtkGetObjectMacro(ImageData, vtkImageData);
    vtkSetObjectMacro(VolumeNode, vtkMRMLVolumeNode);
    vtkGetObjectMacro(VolumeNode, vtkMRMLVolumeNode);
    vtkSetObjectMacro(MeasurementFrame, vtkMatrix4x4);
    vtkGetObjectMacro(MeasurementFrame, vtkMatrix4x4);

    void GetRASToVTKMatrix(vtkMatrix4x4 *RASToVTK);
    void SendImageDataScalars(char *sockname);
    void SendImageDataTensors(char *sockname);
    void SendImageDataTensors_UndoSlicerTransform(char *sockname);
    void ReceiveImageDataScalars(char *sockname);
    void ReceiveImageDataTensors(char *sockname);
    void ReceiveImageDataTensors_DoSlicerTransform(char *sockname);
    const char *Execute (char *Command);


protected:
    vtkTclHelper();
    ~vtkTclHelper(); 
    
    vtkMRMLVolumeNode *VolumeNode;
    vtkImageData *ImageData;    
    vtkMatrix4x4 *MeasurementFrame;    
    Tcl_Interp *Interp;           /* Tcl interpreter */
};


#endif
