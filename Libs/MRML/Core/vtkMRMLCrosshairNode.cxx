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
  this->CrosshairBehavior = vtkMRMLCrosshairNode::OffsetJumpSlice;
  this->CrosshairThickness = vtkMRMLCrosshairNode::Fine;
  this->CrosshairRAS[0] = this->CrosshairRAS[1] = this->CrosshairRAS[2] = 0.0;
  this->LightBoxPane = 0;
  this->SetSingletonTag("default");

  this->CursorPositionRAS[0] = this->CursorPositionRAS[1] = this->CursorPositionRAS[2] = 0.0;
  this->CursorPositionRASValid = false;

  this->CursorPositionXYZ[0] = this->CursorPositionXYZ[1] = this->CursorPositionXYZ[2] = 0.0;
  this->CursorSliceNode = nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLCrosshairNode::~vtkMRMLCrosshairNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  if ( this->CrosshairMode == vtkMRMLCrosshairNode::NoCrosshair )
    {
    of << " crosshairMode=\"" << "NoCrosshair" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowBasic )
    {
    of << " crosshairMode=\"" << "ShowBasic" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowIntersection )
    {
    of << " crosshairMode=\"" << "ShowIntersection" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowHashmarks )
    {
    of << " crosshairMode=\"" << "ShowHashmarks" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowAll )
    {
    of << " crosshairMode=\"" << "ShowAll" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowSmallBasic )
    {
    of << " crosshairMode=\"" << "ShowSmallBasic" << "\"";
    }
  else if ( this->CrosshairMode == vtkMRMLCrosshairNode::ShowSmallIntersection )
    {
    of << " crosshairMode=\"" << "ShowSmallIntersection" << "\"";
    }

  if ( this->CrosshairBehavior == vtkMRMLCrosshairNode::OffsetJumpSlice )
    {
    of << " crosshairBehavior=\"" << "OffsetJumpSlice" << "\"";
    }
  else if (this->CrosshairBehavior == vtkMRMLCrosshairNode::CenteredJumpSlice)
    {
    of << " crosshairBehavior=\"" << "CenteredJumpSlice" << "\"";
    }
  else if (this->CrosshairBehavior == vtkMRMLCrosshairNode::NoAction)
    {
    of << " crosshairBehavior=\"" << "NoAction" << "\"";
    }

  if ( this->CrosshairThickness == vtkMRMLCrosshairNode::Fine )
    {
    of << " crosshairThickness=\"" << "Fine" << "\"";
    }
  else if ( this->CrosshairThickness == vtkMRMLCrosshairNode::Medium )
    {
    of << " crosshairThickness=\"" << "Medium" << "\"";
    }
  else if ( this->CrosshairThickness == vtkMRMLCrosshairNode::Thick )
    {
    of << " crosshairThickness=\"" << "Thick" << "\"";
    }

  of <<  " crosshairRAS=\"" << this->CrosshairRAS[0] << " "
     << this->CrosshairRAS[1] << " " << this->CrosshairRAS[2] << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
    else if (!strcmp (attName, "crosshairBehavior" ))
      {
      if ( !strcmp (attValue, "OffsetJumpSlice")
        || !strcmp(attValue, "JumpSlice")
        || !strcmp(attValue, "Normal"))
        {
        this->SetCrosshairBehavior(vtkMRMLCrosshairNode::OffsetJumpSlice);
        }
      if (!strcmp (attValue, "CenteredJumpSlice"))
        {
        this->SetCrosshairBehavior(vtkMRMLCrosshairNode::CenteredJumpSlice);
        }
      else if (!strcmp (attValue, "NoAction"))
        {
        this->SetCrosshairBehavior(vtkMRMLCrosshairNode::NoAction);
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
void vtkMRMLCrosshairNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCrosshairNode* node = vtkMRMLCrosshairNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  this->SetCrosshairMode ( node->GetCrosshairMode() );
  this->SetCrosshairBehavior (node->GetCrosshairBehavior());
  this->SetCrosshairThickness (node->GetCrosshairThickness());
  this->SetCrosshairRAS(node->GetCrosshairRAS());
}

//----------------------------------------------------------------------------
void vtkMRMLCrosshairNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "CrosshairMode: " << this->CrosshairMode << "\n";
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
  this->CursorSliceNode=nullptr; // slice position is not available
  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
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

  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
}

//---------------------------------------------------------------------------
void vtkMRMLCrosshairNode::SetCursorPositionInvalid()
{
  this->CursorPositionRASValid = false;
  this->CursorSliceNode = nullptr;
  this->InvokeEvent(vtkMRMLCrosshairNode::CursorPositionModifiedEvent, nullptr);
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
