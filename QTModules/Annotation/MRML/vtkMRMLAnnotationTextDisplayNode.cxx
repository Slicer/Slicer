#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayNode*
vtkMRMLAnnotationTextDisplayNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAnnotationTextDisplayNode");
  if (ret)
    {
      return (vtkMRMLAnnotationTextDisplayNode*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationTextDisplayNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode*
vtkMRMLAnnotationTextDisplayNode::CreateNodeInstance()
{
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRMLAnnotationTextDisplayNode");
  if (ret)
    {
      return (vtkMRMLAnnotationTextDisplayNode*) ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationTextDisplayNode;
}

//----------------------------------------------------------------------------
vtkMRMLAnnotationTextDisplayNode::vtkMRMLAnnotationTextDisplayNode()
{
  this->TextScale = 4.5;

  this->UseLineWrap = 0;
  this->MaxCharactersPerLine = 20;

  this->ShowArrowHead = 1;  
  this->ShowBorder = 1;
  this->ShowLeader = 1;
  this->UseThreeDimensionalLeader = 1;
  this->LeaderGlyphSize = 0.025;
  this->MaximumLeaderGlyphSize = 20;
  this->Padding = 3;
  this->AttachEdgeOnly = 0;
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << " textScale=\"" << this->TextScale << "\"";
  of << " useLineWrap=\"" << (this->UseLineWrap ? "true" : "false") << "\"";
  of << " maxCharactersPerLine=\"" << this->MaxCharactersPerLine<< "\"";
  of << " showBorder=\"" << (this->ShowBorder ? "true" : "false") << "\"";
  of << " showLeader=\"" << (this->ShowLeader ? "true" : "false") << "\"";
  of << " showArrowHead=\"" << (this->ShowArrowHead ? "true" : "false") << "\"";
  of << " useThreeDimensionalLeader=\"" << (this->UseThreeDimensionalLeader ? "true" : "false") << "\"";
  of << " leaderGlyphSize=\"" << this->LeaderGlyphSize << "\"";
  of << " maximumLeaderGlyphSize=\"" << this->MaximumLeaderGlyphSize << "\"";
  of << " padding=\"" << this->Padding << "\"";
  of << " attachEdgeOnly=\"" << (this->AttachEdgeOnly ? "true" : "false") << "\"";
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "textScale"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->TextScale;
      }
    else if (!strcmp(attName, "useLineWrap"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->UseLineWrap = 1;
        }
      else
        {
        this->UseLineWrap = 0;
        }
      }
    else if (!strcmp(attName, "maxCharactersPerLine"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MaxCharactersPerLine;
      }
    else if (!strcmp(attName, "showBorder"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ShowBorder = 1;
        }
      else
        {
        this->ShowBorder = 0;
        }
      }
    else if (!strcmp(attName, "showLeader"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ShowLeader = 1;
        }
      else
        {
        this->ShowLeader = 0;
        }
      }
    else if (!strcmp(attName, "showArrowHead"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->ShowArrowHead = 1;
        }
      else
        {
        this->ShowArrowHead = 0;
        }
      }
    else if (!strcmp(attName, "useThreeDimensionalLeader"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->UseThreeDimensionalLeader = 1;
        }
      else
        {
        this->UseThreeDimensionalLeader = 0;
        }
      }
    else if (!strcmp(attName, "leaderGlyphSize"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->LeaderGlyphSize;
      }
    else if (!strcmp(attName, "maximumLeaderGlyphSize"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->MaximumLeaderGlyphSize;
      }
    else if (!strcmp(attName, "padding"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> this->Padding;
      }
    else if (!strcmp(attName, "attachEdgeOnly"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->AttachEdgeOnly = 1;
        }
      else
        {
        this->AttachEdgeOnly = 0;
        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void
vtkMRMLAnnotationTextDisplayNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLAnnotationTextDisplayNode *node =
      (vtkMRMLAnnotationTextDisplayNode *) anode;

  this->SetTextScale(node->TextScale);
  this->SetUseLineWrap(node->UseLineWrap);
  this->SetMaxCharactersPerLine(node->MaxCharactersPerLine);
  this->SetShowBorder(node->ShowBorder);
  this->SetShowLeader(node->ShowLeader);
  this->SetShowArrowHead(node->ShowArrowHead);
  this->SetUseThreeDimensionalLeader(node->UseThreeDimensionalLeader);
  this->SetLeaderGlyphSize(node->LeaderGlyphSize);
  this->SetMaximumLeaderGlyphSize(node->MaximumLeaderGlyphSize);
  this->SetPadding(node->Padding);
  this->SetAttachEdgeOnly(node->AttachEdgeOnly);

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Text scale: (";
  os << this->TextScale << ")\n";

  os << indent << "UseLineWrap: " << (this->UseLineWrap ? "true" : "false") << std::endl;
  os << indent << "MaxCharactersPerLine: " << this->MaxCharactersPerLine << std::endl;

  os << indent << "ShowBorder: " << (this->ShowBorder ? "true" : "false") << std::endl;
  os << indent << "ShowLeader: " << (this->ShowLeader ? "true" : "false") << std::endl;
  os << indent << "ShowArrowHead: " << (this->ShowArrowHead ? "true" : "false") << std::endl;
  os << indent << "UseThreeDimensionalLeader" << (this->UseThreeDimensionalLeader ? "true" : "false") << std::endl;
  os << indent << "LeaderGlyphSize: " << this->LeaderGlyphSize << std::endl;
  os << indent << "MaximumLeaderGlyphSize: " << this->MaximumLeaderGlyphSize << std::endl;
  os << indent << "Padding: " << this->Padding << std::endl;
  os << indent << "AttachEdgeOnly: " << (this->AttachEdgeOnly ? "true" : "false") << std::endl;

}

//---------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::ProcessMRMLEvents(vtkObject *caller,
    unsigned long event, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//-----------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void
vtkMRMLAnnotationTextDisplayNode::SetTextScale(double scale)
{

  //vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting TextScale to " << scale);
  this->TextScale = scale;

  if (!this->GetDisableModifiedEvent())
    {
      // invoke a display modified event
      // this->InvokeEvent(vtkMRMLAnnotationTextDisplayNode::DisplayModifiedEvent);
      this->Modified();
    }
  //this->InvokeEvent(vtkCommand::ModifiedEvent);
  this->ModifiedSinceReadOn();
}



//----------------------------------------------------------------------------
// Create a backup of this node and store it with the node.
void vtkMRMLAnnotationTextDisplayNode::CreateBackup()
{

  vtkMRMLAnnotationTextDisplayNode * backupNode = vtkMRMLAnnotationTextDisplayNode::New();

  backupNode->CopyWithoutModifiedEvent(this);

  this->m_Backup = backupNode;

}

//----------------------------------------------------------------------------
// Restores the backup of this node.
void vtkMRMLAnnotationTextDisplayNode::RestoreBackup()
{

  if (this->m_Backup)
    {
    this->CopyWithSingleModifiedEvent(this->m_Backup);
    }
  else
    {
    vtkErrorMacro("RestoreBackup - could not get the attached backup:" << this->GetID())
    }

}

