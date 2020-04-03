/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women\"s Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLSliceCompositeNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include "vtkObjectFactory.h"

// STD includes
#include <sstream>

vtkCxxSetReferenceStringMacro(vtkMRMLSliceCompositeNode, BackgroundVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSliceCompositeNode, ForegroundVolumeID);
vtkCxxSetReferenceStringMacro(vtkMRMLSliceCompositeNode, LabelVolumeID);

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSliceCompositeNode);

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::vtkMRMLSliceCompositeNode()
{
  this->HideFromEditors = 1;

  this->BackgroundVolumeID = nullptr;
  this->ForegroundVolumeID = nullptr;
  this->LabelVolumeID = nullptr;
  this->Compositing = 0;
  this->ForegroundOpacity = 0.0; // start by showing only the background volume
  this->LabelOpacity = 1.0; // Show the label if there is one
  this->LinkedControl = 0;
  this->FiducialVisibility = 1;
  this->FiducialLabelVisibility = 1;
  this->AnnotationSpace = vtkMRMLSliceCompositeNode::IJKAndRAS;
  this->AnnotationMode = vtkMRMLSliceCompositeNode::All;
  this->SliceIntersectionVisibility = 0;
  this->DoPropagateVolumeSelection = true;
  this->Interacting = 0;
  this->InteractionFlags = 0;
  this->HotLinkedControl = 0;
  this->InteractionFlagsModifier = (unsigned int) -1;
}

//----------------------------------------------------------------------------
vtkMRMLSliceCompositeNode::~vtkMRMLSliceCompositeNode()
{
  if (this->BackgroundVolumeID)
    {
    this->SetBackgroundVolumeID(nullptr);
    }
  if (this->ForegroundVolumeID)
    {
    this->SetForegroundVolumeID(nullptr);
    }
  if (this->LabelVolumeID)
    {
    this->SetLabelVolumeID(nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " backgroundVolumeID=\"" <<
   (this->BackgroundVolumeID ? this->BackgroundVolumeID : "") << "\"";
  of << " foregroundVolumeID=\"" <<
   (this->ForegroundVolumeID ? this->ForegroundVolumeID : "") << "\"";
  of << " labelVolumeID=\"" <<
   (this->LabelVolumeID ? this->LabelVolumeID : "") << "\"";

  of << " compositing=\"" << this->Compositing << "\"";
  of << " foregroundOpacity=\"" << this->ForegroundOpacity << "\"";
  of << " labelOpacity=\"" << this->LabelOpacity << "\"";
  of << " linkedControl=\"" << this->LinkedControl << "\"";
  of << " fiducialVisibility=\"" << this->FiducialVisibility << "\"";
  of << " fiducialLabelVisibility=\"" << this->FiducialLabelVisibility << "\"";
  of << " sliceIntersectionVisibility=\"" << this->SliceIntersectionVisibility << "\"";
  if (this->GetLayoutName() != nullptr)
    {
    of << " layoutName=\"" << this->GetLayoutName() << "\"";
    }

  if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::XYZ)
    {
    of << " annotationSpace=\"" << "xyz" << "\"";
    }
  else if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::IJK)
    {
    of << " annotationSpace=\"" << "ijk" << "\"";
    }
  else if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::RAS)
    {
    of << " annotationSpace=\"" << "RAS" << "\"";
    }
  else if ( this->AnnotationSpace == vtkMRMLSliceCompositeNode::IJKAndRAS)
    {
    of << " annotationSpace=\"" << "IJKAndRAS" << "\"";
    }

  if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::NoAnnotation )
    {
    of << " annotationMode=\"" << "NoAnnotation" << "\"";
    }
  else if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::All )
    {
    of << " annotationMode=\"" << "All" << "\"";
    }
  if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::LabelValuesOnly )
    {
    of << " annotationMode=\"" << "LabelValuesOnly" << "\"";
    }
  if ( this->AnnotationMode == vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly )
    {
    of << " annotationMode=\"" << "LabelAndVoxelValuesOnly" << "\"";
    }
  of << " doPropagateVolumeSelection=\"" << (int)this->DoPropagateVolumeSelection << "\"";
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteracting(int interacting)
{
  // Don't call Modified()
  this->Interacting = interacting;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteractionFlags(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlags = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetInteractionFlagsModifier(unsigned int flags)
{
  // Don't call Modified()
  this->InteractionFlagsModifier = flags;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::ResetInteractionFlagsModifier()
{
  // Don't call Modified()
  this->InteractionFlagsModifier = (unsigned int) -1;
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::SetSceneReferences()
{
   Superclass::SetSceneReferences();
   this->Scene->AddReferencedNodeID(this->BackgroundVolumeID, this);
   this->Scene->AddReferencedNodeID(this->ForegroundVolumeID, this);
   this->Scene->AddReferencedNodeID(this->LabelVolumeID, this);
}

//-----------------------------------------------------------
void vtkMRMLSliceCompositeNode::UpdateReferences()
{
   Superclass::UpdateReferences();

  if (this->BackgroundVolumeID != nullptr && this->Scene->GetNodeByID(this->BackgroundVolumeID) == nullptr)
    {
    this->SetBackgroundVolumeID(nullptr);
    }
  if (this->ForegroundVolumeID != nullptr && this->Scene->GetNodeByID(this->ForegroundVolumeID) == nullptr)
    {
    this->SetForegroundVolumeID(nullptr);
    }
  if (this->LabelVolumeID != nullptr && this->Scene->GetNodeByID(this->LabelVolumeID) == nullptr)
    {
    this->SetLabelVolumeID(nullptr);
    }


}
//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->BackgroundVolumeID && !strcmp(oldID, this->BackgroundVolumeID))
    {
    this->SetBackgroundVolumeID(newID);
    }
  if (this->ForegroundVolumeID && !strcmp(oldID, this->ForegroundVolumeID))
    {
    this->SetForegroundVolumeID(newID);
    }
  if (this->LabelVolumeID && !strcmp(oldID, this->LabelVolumeID))
    {
    this->SetLabelVolumeID(newID);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "backgroundVolumeID"))
      {
      if (attValue && *attValue == '\0')
        {
        this->SetBackgroundVolumeID(nullptr);
        }
      else
        {
        this->SetBackgroundVolumeID(attValue);
        //this->Scene->AddReferencedNodeID(this->BackgroundVolumeID, this);
        }
      }
    else if (!strcmp(attName, "foregroundVolumeID"))
      {
      if (attValue && *attValue == '\0')
        {
        this->SetForegroundVolumeID(nullptr);
        }
      else
        {
        this->SetForegroundVolumeID(attValue);
        //this->Scene->AddReferencedNodeID(this->ForegroundVolumeID, this);
        }
      }
    else if (!strcmp(attName, "labelVolumeID"))
      {
      if (attValue && *attValue == '\0')
        {
        this->SetLabelVolumeID(nullptr);
        }
      else
        {
        this->SetLabelVolumeID(attValue);
        //this->Scene->AddReferencedNodeID(this->LabelVolumeID, this);
        }
      }
    else if (!strcmp(attName, "compositing"))
      {
      this->SetCompositing( atoi(attValue) );
      }
    else if (!strcmp(attName, "foregroundOpacity"))
      {
      this->SetForegroundOpacity( atof(attValue) );
      }
    else if (!strcmp(attName, "labelOpacity"))
      {
      this->SetLabelOpacity( atof(attValue) );
      }
    else if (!strcmp(attName, "linkedControl"))
      {
      this->SetLinkedControl( atoi(attValue) );
      }
    else if (!strcmp(attName, "hotLinkedControl"))
      {
      this->SetHotLinkedControl( atoi(attValue) );
      }
    else if (!strcmp(attName, "fiducialVisibility"))
      {
      this->SetFiducialVisibility( atoi(attValue) );
      }
    else if (!strcmp(attName, "fiducialLabelVisibility"))
      {
      this->SetFiducialLabelVisibility( atoi(attValue) );
      }
    else if (!strcmp(attName, "sliceIntersectionVisibility"))
      {
      this->SetSliceIntersectionVisibility( atoi(attValue) );
      }
   else if (!strcmp(attName, "layoutName"))
      {
      this->SetLayoutName( attValue );
      }

    else if(!strcmp (attName, "annotationSpace" ))
      {
      if (!strcmp (attValue, "xyz"))
        {
        this->SetAnnotationSpace (vtkMRMLSliceCompositeNode::XYZ);
        }
      else if (!strcmp (attValue, "ijk"))
        {
        this->SetAnnotationSpace (vtkMRMLSliceCompositeNode::IJK);
        }
      else if (!strcmp (attValue, "RAS"))
        {
        this->SetAnnotationSpace  (vtkMRMLSliceCompositeNode::RAS);
        }
      else if (!strcmp (attValue, "IJKAndRAS"))
        {
        this->SetAnnotationSpace  (vtkMRMLSliceCompositeNode::IJKAndRAS);
        }
      }
    else if(!strcmp (attName, "annotationMode" ))
      {
      if (!strcmp (attValue, "NoAnnotation"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::NoAnnotation);
        }
      else if (!strcmp (attValue, "All"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::All);
        }
      else if (!strcmp (attValue, "LabelValuesOnly"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::LabelValuesOnly);
        }
      else if (!strcmp (attValue, "LabelAndVoxelValuesOnly"))
        {
        this->SetAnnotationMode (vtkMRMLSliceCompositeNode::LabelAndVoxelValuesOnly);
        }
      }
    else if(!strcmp (attName, "doPropagateVolumeSelection" ))
      {
      this->SetDoPropagateVolumeSelection(atoi(attValue)?true:false);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLSliceCompositeNode *node = vtkMRMLSliceCompositeNode::SafeDownCast(anode);

  this->SetBackgroundVolumeID(node->GetBackgroundVolumeID());
  this->SetForegroundVolumeID(node->GetForegroundVolumeID());
  this->SetLabelVolumeID(node->GetLabelVolumeID());
  this->SetCompositing(node->GetCompositing());
  this->SetForegroundOpacity(node->GetForegroundOpacity());
  this->SetLabelOpacity(node->GetLabelOpacity());
  this->SetLinkedControl (node->GetLinkedControl());
  this->SetHotLinkedControl (node->GetHotLinkedControl());
  this->SetFiducialVisibility ( node->GetFiducialVisibility ( ) );
  this->SetFiducialLabelVisibility ( node->GetFiducialLabelVisibility ( ) );
  this->SetAnnotationSpace ( node->GetAnnotationSpace() );
  this->SetAnnotationMode ( node->GetAnnotationMode() );
  this->SetDoPropagateVolumeSelection (node->GetDoPropagateVolumeSelection());
}

//----------------------------------------------------------------------------
void vtkMRMLSliceCompositeNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "BackgroundVolumeID: " <<
   (this->BackgroundVolumeID ? this->BackgroundVolumeID : "(none)") << "\n";
  os << indent << "ForegroundVolumeID: " <<
   (this->ForegroundVolumeID ? this->ForegroundVolumeID : "(none)") << "\n";
  os << indent << "LabelVolumeID: " <<
    (this->LabelVolumeID ? this->LabelVolumeID : "(none)") << "\n";
  os << indent << "Compositing: " << this->Compositing << "\n";
  os << indent << "ForegroundOpacity: " << this->ForegroundOpacity << "\n";
  os << indent << "LabelOpacity: " << this->LabelOpacity << "\n";
  os << indent << "LinkedControl: " << this->LinkedControl << "\n";
  os << indent << "HotLinkedControl: " << this->HotLinkedControl << "\n";
  os << indent << "FiducialVisibility: " << this->FiducialVisibility << "\n";
  os << indent << "FiducialLabelVisibility: " << this->FiducialLabelVisibility << "\n";
  os << indent << "SliceIntersectionVisibility: " << this->SliceIntersectionVisibility << "\n";
  os << indent << "AnnotationSpace: " << this->AnnotationSpace << "\n";
  os << indent << "AnnotationMode: " << this->AnnotationMode << "\n";
  os << indent << "DoPropagateVolumeSelection: " << this->DoPropagateVolumeSelection << "\n";
  os << indent << "Interacting: " <<
    (this->Interacting ? "on" : "off") << "\n";
}

// End
