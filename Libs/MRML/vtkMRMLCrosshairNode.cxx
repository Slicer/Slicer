/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCrosshairNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLCrosshairNode.h"

//------------------------------------------------------------------------------
vtkMRMLCrosshairNode* vtkMRMLCrosshairNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCrosshairNode");
  if(ret)
    {
    return (vtkMRMLCrosshairNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCrosshairNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLCrosshairNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCrosshairNode");
  if(ret)
    {
    return (vtkMRMLCrosshairNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCrosshairNode;
}

//----------------------------------------------------------------------------
vtkMRMLCrosshairNode::vtkMRMLCrosshairNode()
{
  this->CrosshairMode = vtkMRMLCrosshairNode::NoCrosshair;
  this->CrosshairBehavior = vtkMRMLCrosshairNode::Normal;
  this->CrosshairThickness = vtkMRMLCrosshairNode::Fine;
  this->Navigation = 1;
  this->CrosshairRAS[0] = this->CrosshairRAS[1] = this->CrosshairRAS[2] = 0.0;
  this->SetSingletonTag("default");
}

//----------------------------------------------------------------------------
vtkMRMLCrosshairNode::~vtkMRMLCrosshairNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if ( this->CrosshairMode == vtkMRMLCrosshairNode::NoCrosshair )
    {
    of << indent << " crosshairMode=\"" << "NoCrosshair" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowBasic )
    {
    of << indent << " crosshairMode=\"" << "ShowBasic" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowIntersection )
    {
    of << indent << " crosshairMode=\"" << "ShowIntersection" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowHashmarks )
    {
    of << indent << " crosshairMode=\"" << "ShowHashmarks" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowAll )
    {
    of << indent << " crosshairMode=\"" << "ShowAll" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowSmallBasic )
    {
    of << indent << " crosshairMode=\"" << "ShowSmallBasic" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowSmallIntersection )
    {
    of << indent << " crosshairMode=\"" << "ShowSmallIntersection" << "\"";
    }
  
  of << indent << " navigation=\"" << (this->Navigation ? "true" : "false") << "\"";

  if ( this->CrosshairBehavior == vtkMRMLCrosshairNode::JumpSlice )
    {
    of << indent << " crosshairBehavior=\"" << "JumpSlice" << "\"";
    }
  else if ( this->CrosshairBehavior == vtkMRMLCrosshairNode::Normal )
    {
    of << indent << " crosshairBehavior=\"" << "Normal" << "\"";
    }

  if ( this->CrosshairThickness == vtkMRMLCrosshairNode::Fine )
    {
    of << indent << " crosshairThickness=\"" << "Fine" << "\"";
    }
  else if ( this->CrosshairThickness == vtkMRMLCrosshairNode::Medium )
    {
    of << indent << " crosshairThickness=\"" << "Medium" << "\"";
    }
  else if ( this->CrosshairThickness == vtkMRMLCrosshairNode::Thick )
    {
    of << indent << " crosshairThickness=\"" << "Thick" << "\"";
    }

  of << indent <<  " crosshairRAS=\"" << this->CrosshairRAS[0] << " "
     << this->CrosshairRAS[1] << " " << this->CrosshairRAS[2] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
  if (!strcmp(attName, "CrosshairName")) 
      {
      this->SetCrosshairName( attValue );
      }
    else if(!strcmp (attName, "crosshairMode" ))
      {
      if (!strcmp (attValue, "NoCrosshair"))
        {
        this->SetCrosshairMode (vtkMRMLCrosshairNode::NoCrosshair);
        }
      else if (!strcmp (attValue, "ShowBasic"))
        {
        this->SetCrosshairMode (vtkMRMLCrosshairNode::ShowBasic);
        }
      else if (!strcmp (attValue, "ShowIntersection"))
        {
        this->SetCrosshairMode (vtkMRMLCrosshairNode::ShowIntersection);
        }      
      else if (!strcmp (attValue, "ShowHashmarks"))
        {
        this->SetCrosshairMode ( vtkMRMLCrosshairNode::ShowHashmarks);
        }
      else if (!strcmp (attValue, "ShowAll"))
        {
        this->SetCrosshairMode (vtkMRMLCrosshairNode::ShowAll);
        }
      else if (!strcmp (attValue, "ShowSmallBasic"))
        {
        this->SetCrosshairMode (vtkMRMLCrosshairNode::ShowSmallBasic);
        }
      else if (!strcmp (attValue, "ShowSmallIntersection"))
        {
        this->SetCrosshairMode (vtkMRMLCrosshairNode::ShowSmallIntersection);
        }
      }
    else if (!strcmp (attName, "navigation" ))
      {
      if ( !strcmp (attValue, "true"))
        {
        this->NavigationOn();
        }
      else
        {
        this->NavigationOff();
        }
      }
    else if (!strcmp (attName, "crosshairBehavior" ))
      {
      if ( !strcmp (attValue, "JumpSlice"))
        {
        this->SetCrosshairBehavior ( vtkMRMLCrosshairNode::JumpSlice);
        }
      if ( !strcmp (attValue, "Normal"))
        {
        this->SetCrosshairBehavior ( vtkMRMLCrosshairNode::Normal);
        }
      }
    else if(!strcmp (attName, "crosshairThickness" ))
      {
      if (!strcmp (attValue, "Fine"))
        {
        this->SetCrosshairThickness (vtkMRMLCrosshairNode::Fine);
        }
      else if (!strcmp (attValue, "Medium"))
        {
        this->SetCrosshairThickness (vtkMRMLCrosshairNode::Medium);
        }
      else if (!strcmp (attValue, "Thick"))
        {
        this->SetCrosshairThickness (vtkMRMLCrosshairNode::Thick);
        }
      }
    else if (!strcmp(attName, "crosshairRAS"))
      {
      std::stringstream ss;
      double val;
      ss << attValue;
      int i;
      for (i=0; i<3; i++) 
        {
        ss >> val;
        this->CrosshairRAS[i] = val;
        }
      }
    }
  
  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLCrosshairNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLCrosshairNode *node = vtkMRMLCrosshairNode::SafeDownCast(anode);

  this->SetCrosshairMode ( node->GetCrosshairMode() );
  this->SetCrosshairBehavior (node->GetCrosshairBehavior());
  this->SetCrosshairThickness (node->GetCrosshairThickness());
  this->SetCrosshairRAS(node->GetCrosshairRAS());
  this->SetNavigation(node->GetNavigation());

  this->EndModify(disabledModify);

}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "CrosshairMode: " << this->CrosshairMode << "\n";
  os << indent << "Navigation: " << (this->Navigation ? "true" : "false") << "\n";
  os << indent << "CrosshairBehavior: " << this->CrosshairBehavior << "\n";
  os << indent << "CrosshairThickness: " << this->CrosshairThickness << "\n";
  os << indent << "CrosshairRAS: \n";
  for (unsigned int idx = 0; idx < 3; ++idx) 
    {
    os << indent << indent<< " " << this->CrosshairRAS[idx];
    }
  os << "\n";

}

// End
