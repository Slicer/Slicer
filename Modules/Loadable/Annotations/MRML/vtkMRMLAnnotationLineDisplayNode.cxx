
// MRMLAnnotation includes
#include "vtkMRMLAnnotationLineDisplayNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationLineDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLAnnotationLineDisplayNode::vtkMRMLAnnotationLineDisplayNode()
{
  this->LineThickness = 1.0;
  this->LabelPosition = 0.2;
  this->LabelVisibility = 0;
  this->TickSpacing = 10.0;
  this->MaxTicks = 99;
  this->SliceProjection = (vtkMRMLAnnotationDisplayNode::ProjectionOff |
                           vtkMRMLAnnotationLineDisplayNode::ProjectionDashed |
                           vtkMRMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel |
                           vtkMRMLAnnotationLineDisplayNode::ProjectionThickerOnTop |
                           vtkMRMLAnnotationLineDisplayNode::ProjectionUseRulerColor);

  this->UnderLineThickness = 1.0;
  this->OverLineThickness = 3.0;
  /// bug 2375: don't show the slice intersection until it's correct
  this->Visibility2D = 0;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  of << " lineThickness=\"" << this->LineThickness << "\"";
  of << " labelPosition=\"" << this->LabelPosition << "\"";
  of << " labelVisibility=\"" << (this->LabelVisibility ? "true" : "false") << "\"";
  of << " tickSpacing=\"" << this->TickSpacing << "\"";
  of << " maxTicks=\"" << this->MaxTicks << "\"";
  of << " sliceProjection=\"" << this->SliceProjection << "\"";

  of << " projectedColor=\"" << this->ProjectedColor[0] << " "
     << this->ProjectedColor[1] << " "
     << this->ProjectedColor[2] << "\"";

  of << " projectedOpacity=\"" << this->ProjectedOpacity << "\"";
  of << " underLineThickness=\"" << this->UnderLineThickness << "\"";
  of << " overLineThickness=\"" << this->OverLineThickness << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);

      if (!strcmp(attName, "lineThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LineThickness;
      }
      else if (!strcmp(attName, "labelPosition"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->LabelPosition;
        }
      else if (!strcmp(attName, "labelVisibility"))
        {
        if (!strcmp(attValue,"true"))
          {
          this->LabelVisibility = 1;
          }
        else
          {
          this->LabelVisibility = 0;
          }
        }
      else if (!strcmp(attName, "tickSpacing"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->TickSpacing;
        }
      else if (!strcmp(attName, "maxTicks"))
        {
        std::stringstream ss;
        ss << attValue;
        ss >> this->MaxTicks;
        }
    else if (!strcmp(attName, "sliceProjection"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SliceProjection;
      }
    else if (!strcmp(attName, "projectedColor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ProjectedColor[0];
      ss >> this->ProjectedColor[1];
      ss >> this->ProjectedColor[2];
      }
    else if (!strcmp(attName, "projectedOpacity"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->ProjectedOpacity;
      }
    else if (!strcmp(attName, "underLineThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->UnderLineThickness;
      }
    else if (!strcmp(attName, "overLineThickness"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->OverLineThickness;
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLAnnotationLineDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLAnnotationLineDisplayNode *node = (vtkMRMLAnnotationLineDisplayNode *) anode;
  this->SetLineThickness(node->LineThickness);
  this->SetLabelPosition(node->LabelPosition);
  this->SetLabelVisibility(node->LabelVisibility);
  this->SetTickSpacing(node->TickSpacing);
  this->SetMaxTicks(node->MaxTicks);
  this->SetSliceProjection(node->SliceProjection);
  this->SetProjectedColor(node->GetProjectedColor());
  this->SetProjectedOpacity(node->GetProjectedOpacity());
  this->SetUnderLineThickness(node->GetUnderLineThickness());
  this->SetOverLineThickness(node->GetOverLineThickness());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "Line Thickness   : " << this->LineThickness << "\n";
  os << indent << "Label Position   : " << this->LabelPosition << "\n";
  os << indent << "Label Visibility : " << (this->LabelVisibility ? "true" : "false") << "\n";
  os << indent << "Tick Spacing     : " << this->TickSpacing << "\n";
  os << indent << "Max Ticks        : " << this->MaxTicks << "\n";
  os << indent << "Slice Projection : " << this->SliceProjection << "\n";
  os << indent << "Projected Color : ("
     << this->ProjectedColor[0] << ","
     << this->ProjectedColor[1] << ","
     << this->ProjectedColor[2] << ")" << "\n";
  os << indent << "Projected Opacity: " << this->ProjectedOpacity << "\n";
  os << indent << "Under Line Thickness: " << this->UnderLineThickness << "\n";
  os << indent << "Over Line Thickness: " << this->OverLineThickness << "\n";
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::SetLineThickness(double thickness)
{
    if (this->LineThickness == thickness)
      {
      return;
      }
    vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting LineThickness to " << thickness);
    this->LineThickness = thickness;

    //if (!this->GetDisableModifiedEvent())
    //  {
      // invoke a display modified event
      // KP this->InvokeEvent(vtkMRMLAnnotationLineDisplayNode::DisplayModifiedEvent);
    //  }
  this->Modified();
}

//----------------------------------------------------------------------------
// Create a backup of this node and store it with the node.
void vtkMRMLAnnotationLineDisplayNode::CreateBackup()
{
  vtkMRMLAnnotationLineDisplayNode * backupNode = vtkMRMLAnnotationLineDisplayNode::New();

  int oldMode = backupNode->GetDisableModifiedEvent();
  backupNode->DisableModifiedEventOn();
  backupNode->Copy(this);
  backupNode->SetDisableModifiedEvent(oldMode);

  this->m_Backup = backupNode;
}

//----------------------------------------------------------------------------
// Restores the backup of this node.
void vtkMRMLAnnotationLineDisplayNode::RestoreBackup()
{
  if (this->m_Backup)
    {
    MRMLNodeModifyBlocker blocker(this);
    this->Copy(this->m_Backup);
    }
  else
    {
    vtkErrorMacro("RestoreBackup - could not get the attached backup");
    }
}
