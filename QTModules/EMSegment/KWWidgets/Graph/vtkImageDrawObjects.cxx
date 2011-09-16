/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageDrawObjects.cxx,v $
  Date:      $Date: 2006/02/27 19:21:49 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
#include "vtkImageDrawObjects.h"

#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkMath.h"

#include <stdlib.h>

//------------------------------------------------------------------------------
ObjectList::ObjectList() {
  this->Next           = NULL; 
  memset(this->Position,0,4*sizeof(int));
  this->Thickness = 0;
}

//------------------------------------------------------------------------------
ObjectList::~ObjectList() {
   if (this->Next != NULL) delete this->Next;
} 

//------------------------------------------------------------------------------
int ObjectList::AddObject(int pos[4], vtkFloatingPointType col[3],int type, int thick) {
  // First entry
  if (this->ID == -1) {
    memcpy(this->Color,col,sizeof(vtkFloatingPointType)*3);
    this->ID             = 0;
    this->Type           = type;
    this->Thickness      = thick;
    memcpy(this->Position,pos,sizeof(int)*4);
    return this->ID;
  } 
  
  ObjectList* ListPtr = this;   
  while (ListPtr->Next != NULL)  ListPtr = ListPtr->Next;
  ListPtr->Next = new ObjectList;
  ListPtr->Next->ID = ListPtr->ID + 1;
  ListPtr = ListPtr->Next;
  memcpy(ListPtr->Color,col,sizeof(vtkFloatingPointType)*3);
  memcpy(ListPtr->Position,pos,sizeof(int)*4);
  ListPtr->Type      = type;  
  ListPtr->Thickness = thick;
  return ListPtr->ID;
}

//------------------------------------------------------------------------------
int ObjectList::DeleteObject(int delID) {
  ObjectList* ListPtr = this;
  ObjectList* PrevListPtr = NULL;
  
  while ((ListPtr->ID < delID) && (ListPtr->Next != NULL))  {
    PrevListPtr = ListPtr; 
    ListPtr = ListPtr->Next;
  }
  if ((ListPtr->ID != delID) || (ListPtr->ID < 0)) return 0;
  if (PrevListPtr ==  NULL) {
    // Very first ObjectList member 
    if (ListPtr->Next == NULL) {
      // No more entries
      ListPtr->ID = -1;     
      memset(ListPtr->Color,0,sizeof(vtkFloatingPointType)*3);
      memset(ListPtr->Position,0,sizeof(int)*4);
      ListPtr->Type = 0;  
    } else {
      // Copy the second entry on the first position
      ListPtr->ID                     =  ListPtr->Next->ID;
      memcpy(ListPtr->Color,ListPtr->Next->Color,sizeof(vtkFloatingPointType)*3);
      memcpy(ListPtr->Position,ListPtr->Next->Position,sizeof(int)*4);
      ListPtr->Type                   =  ListPtr->Next->Type;  
      PrevListPtr   =  ListPtr->Next;
      ListPtr->Next                   =  ListPtr->Next->Next;

      // Delete Second entry
      PrevListPtr->Next = NULL;
      delete PrevListPtr;
    }
    return 1;
  } 
  PrevListPtr->Next = ListPtr->Next;
  ListPtr->Next = NULL;
  delete ListPtr;
  return 1; 
}

//------------------------------------------------------------------------------
ObjectList* ObjectList::GetObject(int id) {
  if (id > this->ID) {
    if (this->Next != NULL) return this->Next->GetObject(id);
    return NULL;
  }     
  if ((id < this->ID) || (id < 0)) return NULL;
  return this;
} 

//------------------------------------------------------------------------------
vtkImageDrawObjects* vtkImageDrawObjects::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageDrawObjects");
  if(ret)
    {
    return (vtkImageDrawObjects*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageDrawObjects;
}

//-----------------------------------------------------------------------------
int vtkImageDrawObjects::AddObject(int Xpos0, int Ypos0,int Xpos1, int Ypos1, vtkFloatingPointType color0, vtkFloatingPointType color1,vtkFloatingPointType color2,int type, int radius){
  vtkFloatingPointType color[3];
  int pos[4];
  this->Modified();  
  color[0] = color0; color[1] = color1; color[2] = color2;
  pos[0] = Xpos0; pos[1] = Ypos0; pos[2] = Xpos1; pos[3] = Ypos1; 
  return this->List.AddObject(pos,color,type,radius);  
}

//-----------------------------------------------------------------------------
int vtkImageDrawObjects::DeleteObject(int id) {
  int result = this->List.DeleteObject(id); 
  if (result) this->Modified();
  else vtkErrorMacro("DeleteObject: Object could not delete Object " << id << ", bc it is not part of ObjectList!");
  return result;
}

//-----------------------------------------------------------------------------
void vtkImageDrawObjects::SetObjectColor(int id, vtkFloatingPointType color0, vtkFloatingPointType color1, vtkFloatingPointType color2) {
  vtkFloatingPointType color[3];
  ObjectList *result = this->List.GetObject(id);
  if (result == NULL) {
    vtkErrorMacro("SetObjectColor: Object could not be updated, bc it is not part of ObjectList!");
    return;
  }
  memcpy(color,result->GetColor(),3*sizeof(vtkFloatingPointType)); 
  if ((color[0] != color0) || (color[1] != color1) || (color[2] != color2)) {
    color[0] = color0; color[1] = color1; color[2] = color2;
    result->SetColor(color);
    this->Modified();  
  } 
}

//-----------------------------------------------------------------------------
vtkFloatingPointType* vtkImageDrawObjects::GetObjectColor(int id) {
  ObjectList *result = this->List.GetObject(id);
  if (result == NULL) {
    vtkErrorMacro("GetObjectColor: Object could not retrieve color, bc it is not part of ObjetcList!");
    return NULL;
  }
  return result->GetColor();
}

//-----------------------------------------------------------------------------
void vtkImageDrawObjects::SetObjectPosition(int id, int Xpos0, int Ypos0,int Xpos1, int Ypos1) {
  int pos[4];
  ObjectList *result = this->List.GetObject(id);
  if (result == NULL) {
    vtkErrorMacro("SetObjectPosition: Object could not be updated, bc it is not part of ObjectList!");
    return;
  }
  memcpy(pos,result->GetPosition(),4*sizeof(int)); 
  if ((pos[0] != Xpos0) || (pos[1] != Ypos0) || (pos[2] != Xpos1) || (pos[3] != Ypos1)) {
    pos[0] = Xpos0; pos[1] = Ypos0; pos[2] = Xpos1; pos[3] = Ypos1;
    result->SetPosition(pos);
    this->Modified();  
  } 
}

//-----------------------------------------------------------------------------
int* vtkImageDrawObjects::GetObjectPosition(int id) {
  ObjectList *result = this->List.GetObject(id);
  if (result == NULL) {
    vtkErrorMacro("GetObjectPosition: Could not receive position of object, bc it is not part of ObjectList!");
    return NULL;
  }
  return result->GetPosition();
}

//-----------------------------------------------------------------------------
void vtkImageDrawObjects::SetObjectThickness(int id, int thick) {
  ObjectList *result = this->List.GetObject(id);
  if (result == NULL) {
    vtkErrorMacro("SetObjectThickness: Object could not be updated, bc it is not part of ObjectList!");
    return;
  }
  if (result->GetThickness() != thick) {
    result->SetThickness(thick);
    this->Modified();  
  } 
}

//-----------------------------------------------------------------------------
int vtkImageDrawObjects::GetObjectThickness(int id) {
  ObjectList *result = this->List.GetObject(id);
  if (result == NULL) {
    vtkErrorMacro("GetObjectThickness: Could not receive position of object, bc it is not part of ObjectList!");
    return 0;
  }
  return result->GetThickness();
}

//----------------------------------------------------------------------------
// Make this the output filter 
void vtkImageDrawObjects::ExecuteData(vtkDataObject *)
{
  // 1. Define Variables
  vtkImageData  *outData = this->GetOutput(); 
  vtkIdType           outIncX, outIncY, outIncZ;
  unsigned char *outPtr;

  vtkImageData  *inData  = this->GetInput();
  vtkIdType          inIncX, inIncY, inIncZ;
  unsigned char *inPtr;

  int           *Ext  = inData->GetExtent();  
  ObjectList    *ListPtr = &this->List;
  int           *pos;
  int           border[2];
  int           Xlength, Ylength, outPtrXlength, inPtrXlength,
                SIZE_OF_COPY_BYTES, y;
  unsigned char color[3];

  // 2. Define Output dimenions
  outData->SetWholeExtent(Ext);
  outData->SetExtent(Ext);
  outData->SetScalarType(inData->GetScalarType());
  outData->SetNumberOfScalarComponents(3);
  outData->SetOrigin(inData->GetOrigin());
  outData->SetSpacing(inData->GetSpacing());
  outData->AllocateScalars();

  // 3. Check Input
  if (inData->GetNumberOfScalarComponents() != 3) {
    vtkErrorMacro("Execute: Input has to have 3 Scalar Components");
    return;
  }
  // This is easy to change , but I did not have time 
  if (inData->GetScalarType() != VTK_UNSIGNED_CHAR) {
    vtkErrorMacro("Execute: Input has to be currently unsigned char");
    return;
  }
  // This can be changed too - currently just makes no sense otherwise
  if (Ext[4] != Ext[5]) {
    vtkErrorMacro("Execute: Input has to be two dimensional");
    return;
  }

  // 4. Define Output
  outPtr = (unsigned char *) outData->GetScalarPointerForExtent(Ext);
  outData->GetContinuousIncrements(Ext, outIncX, outIncY, outIncZ);

  inPtr = (unsigned char *) inData->GetScalarPointerForExtent(Ext);
  inData->GetContinuousIncrements(Ext, inIncX, inIncY, inIncZ);

  Xlength = Ext[1] - Ext[0] +1;
  Ylength = Ext[3] - Ext[2] +1;
  
  outPtrXlength = 3 * Xlength +  outIncY;
  inPtrXlength  = 3 * Xlength +  inIncY;
  SIZE_OF_COPY_BYTES = 3 * Xlength * sizeof(unsigned char);

  // 4.a Copy the whole input! 
  // outData->GetPointData()->PassData(inData->GetPointData());
  for (y = 0; y < Ylength; y++) {
    memcpy(outPtr,inPtr,SIZE_OF_COPY_BYTES);
    outPtr += outPtrXlength;
    inPtr  += inPtrXlength;
  } 

  // No objects to draw
  if (ListPtr->GetID() == -1) return;
  // 4.b Draw Objects 
  outPtr = (unsigned char *) outData->GetScalarPointerForExtent(Ext);
  while (ListPtr != NULL) {
    pos = ListPtr->GetPosition();
    border[0] = Xlength - ListPtr->GetThickness() - 1;
    border[1] = Ylength - ListPtr->GetThickness() - 1;

    for (y = 0; y < 4; y++) {
      if (pos[y] < ListPtr->GetThickness()) pos[y] = ListPtr->GetThickness();
      else if (pos[y] > border[y%2]) pos[y] =  border[y%2];
    }
    // Currently only drawing lines
    ConvertColor(ListPtr->GetColor(),color); 
    switch(ListPtr->GetType()) {
    case 0 :  DrawContinousLine(pos[0],pos[1], pos[2], pos[3], color, outPtr,outPtrXlength, ListPtr->GetThickness());
              break;
      default:
      vtkErrorMacro(<< "DrawObjects: Unknown Object Type");
    }
    ListPtr = ListPtr->GetNext();
  }
}

//----------------------------------------------------------------------------
void vtkImageDrawObjects::PrintSelf(ostream& os, vtkIndent indent)
{
  ObjectList      *ListPtr;

  Superclass::PrintSelf(os,indent);
  ListPtr = &(this->List);
  if (ListPtr->GetID() > -1) {
     os << indent << "Objects:  " << endl;
     indent = indent.GetNextIndent();
     while (ListPtr != NULL) {
       os << indent << "ID:       " << ListPtr->GetID() << endl;
       os << indent << "Color:    " << ListPtr->GetColor()[0] << " " <<  ListPtr->GetColor()[1] << " " <<  ListPtr->GetColor()[2] << endl;
       os << indent << "Position: " << ListPtr->GetPosition()[0] << " " <<  ListPtr->GetPosition()[1] << " " <<  ListPtr->GetPosition()[2] << " " <<  ListPtr->GetPosition()[3] << endl;
       os << indent << "Type:     " << ListPtr->GetType() << endl;
       ListPtr = ListPtr->GetNext();
     }
  } else os << indent << "Objects: (NULL)" << endl;
}




