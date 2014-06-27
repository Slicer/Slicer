/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLCrosshairNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCrosshairNode.h"

// VTK includes
#include <vtkMatrix4x4.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLCrosshairNode);

//----------------------------------------------------------------------------
vtkMRMLCrosshairNode::vtkMRMLCrosshairNode()
{
  this->HideFromEditors = 1;

  this->CrosshairMode = vtkMRMLCrosshairNode::NoCrosshair;
  this->CrosshairBehavior = vtkMRMLCrosshairNode::Normal;
  this->CrosshairThickness = vtkMRMLCrosshairNode::Fine;
  this->Navigation = 0;
  this->CrosshairRAS[0] = this->CrosshairRAS[1] = this->CrosshairRAS[2] = 0.0;
  this->LightBoxPane = 0;
  this->SetSingletonTag("default");

  this->CursorPositionRAS[0] = this->CursorPositionRAS[1] = this->CursorPositionRAS[2] = 0.0;
  this->CursorPositionRASValid = false;

  this->CursorPositionXYZ[0] = this->CursorPositionXYZ[1] = this->CursorPositionXYZ[2] = 0.0;
  this->CursorSliceNode = NULL;
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

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCrosshairRAS(double ras[3], int id)
{
  bool modified = false;

  if (this->LightBoxPane != id)
    {
    modified = true;
    }

  if (this->CrosshairRAS[0] != ras[0]
      || this->CrosshairRAS[1] != ras[1]
      || this->CrosshairRAS[2] != ras[2])
    {
    modified = true;
    }

  this->CrosshairRAS[0] = ras[0];
  this->CrosshairRAS[1] = ras[1];
  this->CrosshairRAS[2] = ras[2];
  this->LightBoxPane = id;

  if (modified)
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCursorPositionRAS(double ras[3])
{
  this->CursorPositionRAS[0]=ras[0];
  this->CursorPositionRAS[1]=ras[1];
  this->CursorPositionRAS[2]=ras[2];
  this->CursorPositionRASValid=true;
  this->CursorSliceNode=NULL; // slice position is not available
  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, NULL);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCursorPositionXYZ(double xyz[3], vtkMRMLSliceNode *sliceNode)
{
  this->CursorPositionXYZ[0]=xyz[0];
  this->CursorPositionXYZ[1]=xyz[1];
  this->CursorPositionXYZ[2]=xyz[2];
  this->CursorSliceNode=sliceNode;

  // Cursor position in the slice viewer defines the RAS position, so update that as well
  if (this->CursorSliceNode)
    {
    double xyzw[4] = {xyz[0], xyz[1], xyz[2], 1.0 };
    double rasw[4] = {0.0, 0.0, 0.0, 1.0};
    sliceNode->GetXYToRAS()->MultiplyPoint(xyzw, rasw);
    this->CursorPositionRAS[0]=rasw[0]/rasw[3];
    this->CursorPositionRAS[1]=rasw[1]/rasw[3];
    this->CursorPositionRAS[2]=rasw[2]/rasw[3];
    this->CursorPositionRASValid=true;
    }

  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, NULL);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCursorPositionInvalid()
{
  this->CursorPositionRASValid = false;
  this->CursorSliceNode = NULL;
  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, NULL);
}

//---------------------------------------------------------------------------
bool vtkMRMLCrosshairNode::GetCursorPositionRAS(double ras[3])
{
  ras[0]=this->CursorPositionRAS[0];
  ras[1]=this->CursorPositionRAS[1];
  ras[2]=this->CursorPositionRAS[2];
  return this->CursorPositionRASValid;
}

//---------------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLCrosshairNode::GetCursorPositionXYZ(double xyz[3])
{
  xyz[0]=this->CursorPositionXYZ[0];
  xyz[1]=this->CursorPositionXYZ[1];
  xyz[2]=this->CursorPositionXYZ[2];
  return this->CursorSliceNode;
}

// End
