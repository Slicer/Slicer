/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelNode.cxx,v $
  Date:      $Date: 2006/03/03 22:26:39 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLModelNode.h"

//------------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLModelNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelNode");
  if(ret)
  {
    return (vtkMRMLModelNode*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLModelNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLModelNode");
  if(ret)
    {
      return (vtkMRMLModelNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLModelNode;
}


//----------------------------------------------------------------------------
vtkMRMLModelNode::vtkMRMLModelNode()
{

  PolyData = NULL;

  // Strings
  this->Color = NULL;

  // Numbers
  this->Opacity = 1.0;
  this->Visibility = 1;
  this->Clipping = 0;
  this->BackfaceCulling = 1;
  this->ScalarVisibility = 0;
  this->VectorVisibility = 0;
  this->TensorVisibility = 0;
  
  // Arrays
  this->ScalarRange[0] = 0;
  this->ScalarRange[1] = 100;


  // Scalars
  this->LUTName = -1;
}

//----------------------------------------------------------------------------
vtkMRMLModelNode::~vtkMRMLModelNode()
{
  if (this->Color) {
    delete [] this->Color;
    this->Color = NULL;
  }
  if (this->PolyData) {
    this->PolyData->Delete();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLModelNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  vtkIndent i1(nIndent);

  of << i1 << "<Model";

  // Strings
  of << " id='" << this->ID << "'";
  if (this->Name && strcmp(this->Name, "")) 
  {
    of << " name='" << this->Name << "'";
  }
  if (this->Color && strcmp(this->Color, "")) 
  {
    of << " color='" << this->Color << "'";
  }
  if (this->Description && strcmp(this->Description, "")) 
  {
    of << " description='" << this->Description << "'";
  }

  //if (this->LUTName && strcmp(this->LUTName,""))
  if (this->LUTName != -1)
  {
      of << " lutName='" << this->LUTName << "'";
  }
  
  // Numbers
  if (this->Opacity != 1.0)
  {
    of << " opacity='" << this->Opacity << "'";
  }
  if (this->Visibility != 1)
  {
    of << " visibility='" << (this->Visibility ? "true" : "false") << "'";
  }
  if (this->Clipping != 0)
  {
    of << " clipping='" << (this->Clipping ? "true" : "false") << "'";
  }
  if (this->BackfaceCulling != 1)
  {
    of << " backfaceCulling='" << (this->BackfaceCulling ? "true" : "false") << "'";
  }
  if (this->ScalarVisibility != 0)
  {
    of << " scalarVisibility='" << (this->ScalarVisibility ? "true" : "false") << "'";
  }

  // Arrays
  if (this->ScalarRange[0] != 0 || this->ScalarRange[1] != 100)
  {
    of << " scalarRange='" << this->ScalarRange[0] << " "
       << this->ScalarRange[1] << "'";
  }
  of << "></Model>\n";;
}

//----------------------------------------------------------------------------
void vtkMRMLModelNode::ReadXMLAttributes(const char** atts)
{

  vtkMRMLNode::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "Color")) {
      this->SetColor(attValue);
    }
    else if (!strcmp(attName, "ScalarRange")) {
      std::stringstream ss;
      ss << attValue;
      ss >> ScalarRange[0];
      ss >> ScalarRange[1];
    }
    else if (!strcmp(attName, "LUTName")) {
      std::stringstream ss;
      ss << attValue;
      ss >> LUTName;
    }
    else if (!strcmp(attName, "Opacity")) {
      std::stringstream ss;
      ss << attValue;
      ss >> Opacity;
    }
    else if (!strcmp(attName, "Visibility")) {
      std::stringstream ss;
      ss << attValue;
      ss >> Visibility;
    }
    else if (!strcmp(attName, "BackfaceCulling")) {
      std::stringstream ss;
      ss << attValue;
      ss >> BackfaceCulling;
    }
    else if (!strcmp(attName, "ScalarVisibility")) {
      std::stringstream ss;
      ss << attValue;
      ss >> ScalarVisibility;
    }
    else if (!strcmp(attName, "VectorVisibility")) {
      std::stringstream ss;
      ss << attValue;
      ss >> VectorVisibility;
    }
    else if (!strcmp(attName, "TensorVisibility")) {
      std::stringstream ss;
      ss << attValue;
      ss >> TensorVisibility;
    }
  }  
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLModelNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLNode::Copy(anode);
  vtkMRMLModelNode *node = (vtkMRMLModelNode *) anode;

  // Strings

  this->SetColor(node->Color);

  // Vectors
  this->SetScalarRange(node->ScalarRange);
  
  // Numbers
  this->SetOpacity(node->Opacity);
  this->SetVisibility(node->Visibility);
  this->SetScalarVisibility(node->ScalarVisibility);
  this->SetBackfaceCulling(node->BackfaceCulling);
  this->SetClipping(node->Clipping);
  this->SetPolyData(node->PolyData);

}

//----------------------------------------------------------------------------
void vtkMRMLModelNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  vtkMRMLNode::PrintSelf(os,indent);

  os << indent << "Color: " <<
    (this->Color ? this->Color : "(none)") << "\n";

  os << indent << "Opacity:           " << this->Opacity << "\n";
  os << indent << "Visibility:        " << this->Visibility << "\n";
  os << indent << "ScalarVisibility:  " << this->ScalarVisibility << "\n";
  os << indent << "BackfaceCulling:   " << this->BackfaceCulling << "\n";
  os << indent << "Clipping:          " << this->Clipping << "\n";

  os << "ScalarRange:\n";
  for (idx = 0; idx < 2; ++idx)
  {
    os << indent << ", " << this->ScalarRange[idx];
  }
  os << "\nPoly Data:\n";
  if (this->PolyData) {
    this->PolyData->PrintSelf(os, indent.GetNextIndent());
  }

}
