/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLViewNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/
#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLViewNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLViewNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLViewNode");
  if(ret)
    {
    return (vtkMRMLViewNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLViewNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLViewNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLViewNode");
  if(ret)
    {
    return (vtkMRMLViewNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLViewNode;
}


//----------------------------------------------------------------------------
vtkMRMLViewNode::vtkMRMLViewNode()
{
  this->BoxVisible = 1;
  this->AxisLabelsVisible = 1;
  this->FieldOfView = 200;
  this->LetterSize = 0.05;
  this->Spin = 0;
  this->Rock = 0;
  this->SpinDegrees = 2.0;
  this->SpinDirection = vtkMRMLViewNode::Left;
  this->SpinMs = 5;
  this->RockLength = 200;
  this->RockCount = 0;
  this->StereoType = vtkMRMLViewNode::NoStereo;
  this->RenderMode = vtkMRMLViewNode::Perspective;
  //--- Slicer's default light blue color
  this->BackgroundColor[0] = 0.70196;
  this->BackgroundColor[1] = 0.70196;
  this->BackgroundColor[2] = 0.90588;
 }

//----------------------------------------------------------------------------
vtkMRMLViewNode::~vtkMRMLViewNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " fieldOfView=\"" << this->GetFieldOfView() << "\"";
  of << indent << " letterSize=\"" << this->GetLetterSize() << "\"";
  of << indent << " boxVisible=\"" << (this->BoxVisible ? "true" : "false") << "\"";
  of << indent << " axisLabelsVisible=\"" << (this->AxisLabelsVisible ? "true" : "false") << "\"";

  // background color
  of << indent << " backgroundColor=\"" << this->BackgroundColor[0] << " "
     << this->BackgroundColor[1] << " " << this->BackgroundColor[2] << "\"";

  
  // spin or rock?
  of << indent << " spin=\"" << (this->Spin ? "true" : "false") << "\"";
  of << indent << " rock=\"" << (this->Rock ? "true" : "false") << "\"";

  // configure spin
  of << indent << " spinDegrees=\"" << this->GetSpinDegrees() << "\"";
  of << indent << " spinMs=\"" << this->GetSpinMs() << "\"";
  if ( this->GetSpinDirection() == vtkMRMLViewNode::Up )
    {
    of << indent << " spinDirection=\"" << "Up" << "\"";
    }
  else if ( this->GetSpinDirection() == vtkMRMLViewNode::Down )
    {
    of << indent << " spinDirection=\"" << "Down" << "\"";
    }
  else if ( this->GetSpinDirection() == vtkMRMLViewNode::Left )
    {
    of << indent << " spinDirection=\"" << "Left" << "\"";
    }
  else if ( this->GetSpinDirection() == vtkMRMLViewNode::Right )
    {
    of << indent << " spinDirection=\"" << "Right" << "\"";
    }

  // configure rock
  of << indent << " rockLength=\"" << this->GetRockLength() << "\"";
  of << indent << " rockCount=\"" << this->GetRockCount() << "\"";
  
  // configure stereo
  if ( this->GetStereoType() == vtkMRMLViewNode::NoStereo )
    {
    of << indent << " stereoType=\"" << "NoStereo" << "\"";    
    }
  else if ( this->GetStereoType() == vtkMRMLViewNode::RedBlue )
    {
    of << indent << " stereoType=\"" << "RedBlue" << "\"";    
    }
  else if ( this->GetStereoType() == vtkMRMLViewNode::CrystalEyes )
    {
    of << indent << " stereoType=\"" << "CrystalEyes" << "\"";    
    }
  else if ( this->GetStereoType() == vtkMRMLViewNode::Interlaced )
    {
    of << indent << " stereoType=\"" << "Interlaced" << "\"";    
    }

  // configure render mode
  if (this->GetRenderMode() == vtkMRMLViewNode::Perspective )
    {
    of << indent << " renderMode=\"" << "Perspective" << "\"";
    }
  else if ( this->GetRenderMode() == vtkMRMLViewNode::Orthographic )
    {
    of << indent << " renderMode=\"" << "Orthographic" << "\"";
    }


}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "fieldOfView")) 
      {
      std::stringstream ss;
      ss << attValue;
      double fov;
      ss >> fov;
      this->FieldOfView = fov;
      }
    else if (!strcmp(attName, "letterSize")) 
      {
      std::stringstream ss;
      ss << attValue;
      double fov;
      ss >> fov;
      this->LetterSize = fov;
      }

    else if (!strcmp(attName, "backgroundColor"))
      {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->BackgroundColor[0] = val;
      ss << attValue;
      ss >> val;
      this->BackgroundColor[1] = val;
      ss << attValue;
      ss >> val;
      this->BackgroundColor[2] = val;
      }
    
    else if (!strcmp(attName, "boxVisible")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->BoxVisible = 1;
        }
      else
        {
        this->BoxVisible = 0;
        }
      }
    
    else if (!strcmp(attName, "axisLabelsVisible")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->AxisLabelsVisible = 1;
        }
      else
        {
        this->AxisLabelsVisible = 0;
        }
      }

    else if (!strcmp(attName, "stereoType")) 
      {
      if (!strcmp(attValue,"NoStereo")) 
        {
        this->StereoType = vtkMRMLViewNode::NoStereo;
        }
      else if ( !strcmp (attValue, "RedBlue" ))
        {
        this->StereoType = vtkMRMLViewNode::RedBlue;
        }
      else if ( !strcmp (attValue, "CrystalEyes" ))
        {
        this->StereoType = vtkMRMLViewNode::CrystalEyes;
        }
      else if ( !strcmp (attValue, "Interlaced" ))
        {
        this->StereoType = vtkMRMLViewNode::Interlaced;
        }
      
      }

    else if (!strcmp(attName, "rock")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->Rock = 1;
        }
      else
        {
        this->Rock = 0;
        }
      }
    else if (!strcmp(attName, "rockLength" ))
      {
      std::stringstream ss;
      ss << attValue;
      int len;
      ss >> len;
      this->RockLength = len;
      }    
    else if (!strcmp(attName, "rockCount" ))
      {
      std::stringstream ss;
      ss << attValue;
      int count;
      ss >> count;
      this->RockCount = count;
      }    

    else if (!strcmp(attName, "spin")) 
      {
      if (!strcmp(attValue,"true")) 
        {
        this->Spin = 1;
        }
      else
        {
        this->Spin = 0;
        }
      }
    else if (!strcmp(attName, "spinDegrees" ))
      {
      std::stringstream ss;
      ss << attValue;
      int deg;
      ss >> deg;
      this->SpinDegrees = deg;
      }
    else if (!strcmp(attName, "spinMs" ))
      {
      std::stringstream ss;
      ss << attValue;
      int ms;
      ss >> ms;
      this->SpinMs = ms;
      }    

    else if (!strcmp(attName, "spinDirection")) 
      {
      if (!strcmp(attValue,"Left")) 
        {
        this->SpinDirection = vtkMRMLViewNode::Left;
        }
      else if ( !strcmp (attValue, "Right" ))
        {
        this->SpinDirection = vtkMRMLViewNode::Right;
        }
      else if ( !strcmp (attValue, "Up" ))
        {
        this->SpinDirection = vtkMRMLViewNode::Up;
        }
      else if ( !strcmp (attValue, "Down" ))
        {
        this->SpinDirection = vtkMRMLViewNode::Down;
        }
      }

    else if (!strcmp(attName, "renderMode")) 
      {
      if (!strcmp(attValue,"Perspective")) 
        {
        this->RenderMode = vtkMRMLViewNode::Perspective;
        }
      else if ( !strcmp (attValue, "Orthographic" ))
        {
        this->RenderMode = vtkMRMLViewNode::Orthographic;
        }
      }
                

    }
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLViewNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLViewNode *node = (vtkMRMLViewNode *) anode;


  this->SetBoxVisible(node->GetBoxVisible());
  this->SetAxisLabelsVisible(node->GetAxisLabelsVisible());
  this->SetFieldOfView(node->GetFieldOfView());
  this->SetLetterSize(node->GetLetterSize());
  this->SetSpin ( node->GetSpin ( ) );
  this->SetSpinDirection ( node->GetSpinDirection ( ) );
  this->SetSpinMs ( node->GetSpinMs() );
  this->SetSpinDegrees (node->GetSpinDegrees ( ));
  this->SetRock ( node->GetRock ( ) );
  this->SetRockLength ( node->GetRockLength () );
  this->SetRockCount ( node->GetRockCount ( ) );
  this->SetStereoType ( node->GetStereoType ( ) );
  this->SetRenderMode ( node->GetRenderMode() );
  this->SetBackgroundColor ( node->GetBackgroundColor ( ) );
}

//----------------------------------------------------------------------------
void vtkMRMLViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  int idx;
  
  Superclass::PrintSelf(os,indent);

  os << indent << "BoxVisible:        " << this->BoxVisible << "\n";
  os << indent << "AxisLabelsVisible: " << this->AxisLabelsVisible << "\n";
  os << indent << "FieldOfView:       " << this->FieldOfView << "\n";
  os << indent << "LetterSize:       " << this->LetterSize << "\n";

  os << indent << "Spin:       " << this->Spin << "\n";
  os << indent << "SpinDirection:       " << this->SpinDirection << "\n";
  os << indent << "SpinMs:       " << this->SpinMs << "\n";  
  os << indent << "SpinDegrees:       " << this->SpinDegrees << "\n";
  os << indent << "Rock:       " << this->Rock << "\n";
  os << indent << "RockLength:       " << this->RockLength << "\n";
  os << indent << "RockCount:       " << this->RockCount << "\n";
  os << indent << "StereoType:       " << this->StereoType << "\n";
  os << indent << "RenderMode:       " << this->RenderMode << "\n";
  os << indent << "BackgroundColor:       " << this->BackgroundColor[0] << " "
     << this->BackgroundColor[1] << " "
     << this->BackgroundColor[2] <<"\n";
}

