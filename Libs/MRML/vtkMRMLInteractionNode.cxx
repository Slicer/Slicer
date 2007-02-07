#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkMRMLInteractionNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLInteractionNode");
  if(ret)
    {
    return (vtkMRMLInteractionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLInteractionNode;
}

//-----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLInteractionNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLInteractionNode");
  if(ret)
    {
    return (vtkMRMLInteractionNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLInteractionNode;
}


//----------------------------------------------------------------------------
vtkMRMLInteractionNode::vtkMRMLInteractionNode()
{
  this->SingletonTag = "vtkMRMLInteractionNode";

  this->CurrentInteractionMode = vtkMRMLInteractionNode::ViewTransform;
  this->LastInteractionMode = vtkMRMLInteractionNode::ViewTransform;
 }

//----------------------------------------------------------------------------
vtkMRMLInteractionNode::~vtkMRMLInteractionNode()
{
}


//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SetCurrentInteractionMode ( int mode )
{
  switch (mode)
    {
    case vtkMRMLInteractionNode::PickManipulate:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::SelectRegion:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::LassoRegion:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::Place:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::ViewPan:      
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::ViewZoom:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::ViewRotate:      
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::ViewTransform:      
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    default:
      break;
    }
}



//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  // configure stereo
  if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::PickManipulate )
    {
    of << indent << " currentInteractionMode=\"" << "PickManipulate" << "\"";    
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::SelectRegion )
    {
    of << indent << " currentInteractionMode=\"" << "SelectRegion" << "\"";    
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::LassoRegion )
    {
    of << indent << " currentInteractionMode=\"" << "LassoRegion" << "\"";    
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place )
    {
    of << indent << " currentInteractionMode=\"" << "Place" << "\"";    
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewPan )
    {
    of << indent << " currentInteractionMode=\"" << "ViewPan" << "\"";    
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewZoom )
    {
    of << indent << " currentInteractionMode=\"" << "ViewZoom" << "\"";    
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewRotate )
    {
    of << indent << " currentInteractionMode=\"" << "ViewRotate" << "\"";    
    }  
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewTransform )
    {
    of << indent << " currentInteractionMode=\"" << "ViewTransform" << "\"";    
    }  

  if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::PickManipulate )
    {
    of << indent << " lastInteractionMode=\"" << "PickManipulate" << "\"";    
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::SelectRegion )
    {
    of << indent << " lastInteractionMode=\"" << "SelectRegion" << "\"";    
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::LassoRegion )
    {
    of << indent << " lastInteractionMode=\"" << "LassoRegion" << "\"";    
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::Place )
    {
    of << indent << " lastInteractionMode=\"" << "Place" << "\"";    
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::ViewPan )
    {
    of << indent << " lastInteractionMode=\"" << "ViewPan" << "\"";    
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::ViewZoom )
    {
    of << indent << " lastInteractionMode=\"" << "ViewZoom" << "\"";    
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::ViewRotate )
    {
    of << indent << " lastInteractionMode=\"" << "ViewRotate" << "\"";    
    }  
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::ViewTransform )
    {
    of << indent << " lastInteractionMode=\"" << "ViewTransform" << "\"";    
    }  

}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL) 
    {
    attName = *(atts++);
    attValue = *(atts++);


    if (!strcmp(attName, "currentInteractionMode")) 
      {
      if (!strcmp(attValue,"PickManipulate")) 
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::PickManipulate;
        }
      else if ( !strcmp (attValue, "SelectRegion" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::SelectRegion;
        }
      else if ( !strcmp (attValue, "LassoRegion" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::LassoRegion;
        }
      else if ( !strcmp (attValue, "Place" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::Place;
        }
      else if ( !strcmp (attValue, "ViewPan" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::ViewPan;
        }
      else if ( !strcmp (attValue, "ViewZoom" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::ViewZoom;
        }
      else if ( !strcmp (attValue, "ViewRotate" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::ViewRotate;
        }
      else if ( !strcmp (attValue, "ViewTransform" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::ViewTransform;
        }
      }
    else if (!strcmp(attName, "lastInteractionMode"))
      {
      if (!strcmp(attValue,"PickManipulate")) 
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::PickManipulate;
        }
      else if ( !strcmp (attValue, "SelectRegion" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::SelectRegion;
        }
      else if ( !strcmp (attValue, "LassoRegion" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::LassoRegion;
        }
      else if ( !strcmp (attValue, "Place" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::Place;
        }
      else if ( !strcmp (attValue, "ViewPan" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::ViewPan;
        }
      else if ( !strcmp (attValue, "ViewZoom" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::ViewZoom;
        }
      else if ( !strcmp (attValue, "ViewRotate" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::ViewRotate;
        }
      else if ( !strcmp (attValue, "ViewTransform" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::ViewTransform;
        }
      }
    
    }
}




//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLInteractionNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLInteractionNode *node = (vtkMRMLInteractionNode *) anode;

  this->SetCurrentInteractionMode (node->GetCurrentInteractionMode());
  this->SetLastInteractionMode ( node->GetLastInteractionMode());
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);
  os << indent << "CurrentInteractionMode:        " << this->GetInteractionModeAsString(this->CurrentInteractionMode) << "\n";
  os << indent << "LastInteractionMode:        " <<  this->GetInteractionModeAsString(this->LastInteractionMode) << "\n";
}


//---------------------------------------------------------------------------
const char * vtkMRMLInteractionNode::GetInteractionModeAsString(int mode)
{
  if (mode == this->PickManipulate)
    {
    return "PickManipulate";
    }
  if (mode == this->SelectRegion)
    {
    return "SelectRegion";
    }
  if (mode == this->LassoRegion)
    {
    return "LassoRegion";
    }
  if (mode == this->Place)
    {
    return "Place";
    }
  if (mode == this->ViewPan)
    {
    return "ViewPan";
    }
  if (mode == this->ViewZoom)
    {
    return "ViewZoom";
    }
  if (mode == this->ViewRotate)
    {
    return "ViewRotate";
    }
  if (mode == this->ViewTransform)
    {
    return "ViewTransform";
    }
  
  return "(unknown)";
}
