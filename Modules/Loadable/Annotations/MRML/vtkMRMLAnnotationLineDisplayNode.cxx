
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
  this->LabelVisibility = 1;
  this->TickSpacing = 10.0;
  this->MaxTicks = 99;
  this->SliceProjection = (vtkMRMLAnnotationLineDisplayNode::ProjectionOff | 
                           vtkMRMLAnnotationLineDisplayNode::ProjectionDotted |
                           vtkMRMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel);
  
  // Default line color: white
  this->SliceProjectionLineColor[0] = 1.0;
  this->SliceProjectionLineColor[1] = 1.0;
  this->SliceProjectionLineColor[2] = 1.0;
  this->SliceProjectionLineColor[3] = 1.0;

  /// bug 2375: don't show the slice intersection until it's correct
  this->SliceIntersectionVisibility = 0;
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << " lineThickness=\"" << this->LineThickness << "\"";
  of << " labelPosition=\"" << this->LabelPosition << "\"";
  of << " labelVisibility=\"" << (this->LabelVisibility ? "true" : "false") << "\"";
  of << " tickSpacing=\"" << this->TickSpacing << "\"";
  of << " maxTicks=\"" << this->MaxTicks << "\"";
  of << " sliceProjection=\"" << this->SliceProjection << "\"";

  if (this->SliceProjectionLineColor)
    {
    of << indent << " sliceProjectionLineColor=\"" << this->SliceProjectionLineColor[0] << " "
      << this->SliceProjectionLineColor[1] << " "
      << this->SliceProjectionLineColor[2] << "\"";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
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
    else if (!strcmp(attName, "sliceProjectionLineColor"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->SliceProjectionLineColor[0];
      ss >> this->SliceProjectionLineColor[1];
      ss >> this->SliceProjectionLineColor[2];
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
  this->SetSliceProjectionLineColor(node->GetSliceProjectionLineColor());

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
  os << indent << "Slice Projection Line Color : (" 
     << this->SliceProjectionLineColor[0] << ","
     << this->SliceProjectionLineColor[1] << ","
     << this->SliceProjectionLineColor[2] << ","
     << this->SliceProjectionLineColor[3] << ")" << "\n";
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

  backupNode->CopyWithoutModifiedEvent(this);

  this->m_Backup = backupNode;
}

//----------------------------------------------------------------------------
// Restores the backup of this node.
void vtkMRMLAnnotationLineDisplayNode::RestoreBackup()
{
  if (this->m_Backup)
    {
    this->CopyWithSingleModifiedEvent(this->m_Backup);
    }
  else
    {
    vtkErrorMacro("RestoreBackup - could not get the attached backup")
    }
}
