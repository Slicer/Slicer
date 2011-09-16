/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageDrawObjects.h,v $
  Date:      $Date: 2006/02/27 19:21:49 $
  Version:   $Revision: 1.9 $

=========================================================================auto=*/
// .NAME vtkImageDrawObjects - Abstract Filter used in slicer to plot graphs
// .SECTION Description
// vtkImageDrawObjects is 
//

#ifndef __vtkImageDrawObjects_h
#define __vtkImageDrawObjects_h

#include "vtkImageToImageFilter.h"
#include "vtkImageGraph.h" // For GraphList
#include "vtkEMSegmentStep.h"

//BTX

// Description:
// Type of Objects:
//  0 = a line 
class  VTK_EMSEGMENT_EXPORT ObjectList : public GraphList {
public:
   ObjectList* GetNext() {return this->Next;} 
   int AddObject(int pos[4],vtkFloatingPointType col[3],int type, int thick);  
   int DeleteObject(int delID);

   void SetPosition(int pos[4]) {memcpy(this->Position,pos,4*sizeof(int));}
   int* GetPosition() {return this->Position;}  

   void SetThickness(int thick) {this->Thickness = thick;}
   int GetThickness() {return this->Thickness;}  
 
   ObjectList* GetObject(int id); 

   ~ObjectList();
   ObjectList();
protected:
   int Position[4];
   int Thickness;
   ObjectList* Next; 
}; 
//ETX

class vtkDataObject;
class  VTK_EMSEGMENT_EXPORT vtkImageDrawObjects : public vtkImageToImageFilter
{ 
public:
  static vtkImageDrawObjects *New();
  vtkTypeMacro(vtkImageDrawObjects,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add an object to the graph. It returns the ID of the data entry back
  int AddObject(int Xpos0, int Ypos0,int Xpos1, int Ypos1, vtkFloatingPointType color0, vtkFloatingPointType color1,vtkFloatingPointType color2,int type, int thick);

  // Description:
  // Deletes an object from the list -> if successfull returns 1 - otherwise 0; 
  int DeleteObject(int id);

  // Description: 
  // Set the color of an object 
  void SetObjectColor(int id, vtkFloatingPointType color0, vtkFloatingPointType color1,vtkFloatingPointType color2);
  vtkFloatingPointType* GetObjectColor(int id);

  // Description: 
  // Set/Get the position of an object 
  void SetObjectPosition(int id, int Xpos0, int Ypos0,int Xpos1, int Ypos1);
  int* GetObjectPosition(int id);

  // Description: 
  // Set/Get the position of an object 
  void SetObjectThickness(int id, int thick);
  int GetObjectThickness(int id);

protected:
  vtkImageDrawObjects() {};
  ~vtkImageDrawObjects() {};

  //void ExecuteInformation(vtkImageData *inData, vtkImageData *outData);
  // void ExecuteInformation() {this->vtkImageToImageFilter::ExecuteInformation(); };
  void ExecuteData(vtkDataObject *data);

  ObjectList List; 
private:
  vtkImageDrawObjects(const vtkImageDrawObjects&);  // Not implemented.
  void operator=(const vtkImageDrawObjects&);  // Not implemented.
};

#endif

  
