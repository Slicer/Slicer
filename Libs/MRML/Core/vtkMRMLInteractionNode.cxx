
// MRML includes
#include "vtkMRMLInteractionNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLInteractionNode);


//----------------------------------------------------------------------------
vtkMRMLInteractionNode::vtkMRMLInteractionNode()
{
  this->HideFromEditors = 1;

  this->SetSingletonTag("Singleton");

  this->CurrentInteractionMode = vtkMRMLInteractionNode::ViewTransform;
  this->LastInteractionMode = vtkMRMLInteractionNode::ViewTransform;
  this->PlaceModePersistence = 0;
  this->TransformModePersistence = 1;
  this->EnableFiberEdit = 0;
}

//----------------------------------------------------------------------------
vtkMRMLInteractionNode::~vtkMRMLInteractionNode()
{
}

//----------------------------------------------------------------------------
int vtkMRMLInteractionNode::GetInteractionModeByString ( const char * modeString )
{
  if (modeString == NULL)
    {
    return (-1);
    }

  if ( !strcmp (modeString, "Place" ))
    {
    return ( vtkMRMLInteractionNode::Place);
    }
  else if ( !strcmp (modeString, "ViewTransform" ))
    {
    return ( vtkMRMLInteractionNode::ViewTransform);
    }
  else if ( !strcmp (modeString, "Transform" ))
    {
    return ( vtkMRMLInteractionNode::ViewTransform);
    }
//  else if ( !strcmp (modeString, "SelectRegion" ))
//    {
//    return ( vtkMRMLInteractionNode::SelectRegion);
//    }
//  else if ( !strcmp (modeString, "LassoRegion" ))
//    {
//    return ( vtkMRMLInteractionNode::LassoRegion);
//    }
  else
    {
    return (-1);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::NormalizeAllMouseModes()
{
  this->PlaceModePersistence = 0;
  this->TransformModePersistence = 1;
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SetPlaceModePersistence ( int val )
{
  if (this->PlaceModePersistence != val)
    {
    this->PlaceModePersistence = val;
    this->InvokeEvent(this->InteractionModePersistenceChangedEvent, NULL);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SetTransformModePersistence ( int val )
{
  if (this->TransformModePersistence != val)
    {
    this->TransformModePersistence = val;
    this->InvokeEvent(this->InteractionModePersistenceChangedEvent, NULL);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SetLastInteractionMode ( int mode )
{
  if (this->LastInteractionMode != mode)
    {
    this->LastInteractionMode = mode;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SetCurrentInteractionMode ( int mode )
{
  if ( this->CurrentInteractionMode == mode )
    {
    return;
    }
  switch (mode)
    {
    case vtkMRMLInteractionNode::Place:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::ViewTransform:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
    case vtkMRMLInteractionNode::Select:
      this->CurrentInteractionMode = mode;
      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
      break;
//    case vtkMRMLInteractionNode::LassoRegion:
//      this->CurrentInteractionMode = mode;
//      this->InvokeEvent(this->InteractionModeChangedEvent, NULL);
//      break;
    default:
      break;
    }
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place )
    {
    of << indent << " currentInteractionMode=\"" << "Place" << "\"";
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewTransform )
    {
    of << indent << " currentInteractionMode=\"" << "ViewTransform" << "\"";
    }
//  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::SelectRegion )
//    {
//    of << indent << " currentInteractionMode=\"" << "SelectRegion" << "\"";
//    }
//  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::LassoRegion )
//    {
//    of << indent << " currentInteractionMode=\"" << "LassoRegion" << "\"";
//    }

  of << indent << " placeModePersistence=\"" << (this->PlaceModePersistence ? "true" : "false") << "\"";

  if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::Place )
    {
    of << indent << " lastInteractionMode=\"" << "Place" << "\"";
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::ViewTransform )
    {
    of << indent << " lastInteractionMode=\"" << "ViewTransform" << "\"";
    }
//  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::SelectRegion )
//    {
//    of << indent << " lastInteractionMode=\"" << "SelectRegion" << "\"";
//    }
//  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::LassoRegion )
//    {
//    of << indent << " lastInteractionMode=\"" << "LassoRegion" << "\"";
//    }

}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "currentInteractionMode"))
      {
      if ( !strcmp (attValue, "Place" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::Place;
        }
      else if ( !strcmp (attValue, "ViewTransform" ))
        {
        this->CurrentInteractionMode = vtkMRMLInteractionNode::ViewTransform;
        }
//      else if ( !strcmp (attValue, "SelectRegion" ))
//        {
//        this->CurrentInteractionMode = vtkMRMLInteractionNode::SelectRegion;
//        }
//      else if ( !strcmp (attValue, "LassoRegion" ))
//        {
//        this->CurrentInteractionMode = vtkMRMLInteractionNode::LassoRegion;
//        }
      }
    else if (!strcmp(attName, "placeModePersistence"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->PlaceModePersistence = 1;
        }
      else
        {
        this->PlaceModePersistence = 0;
        }
      }
    else if (!strcmp(attName, "lastInteractionMode"))
      {
      if ( !strcmp (attValue, "Place" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::Place;
        }
      else if ( !strcmp (attValue, "ViewTransform" ))
        {
        this->LastInteractionMode = vtkMRMLInteractionNode::ViewTransform;
        }
//      else if ( !strcmp (attValue, "SelectRegion" ))
//        {
//        this->LastInteractionMode = vtkMRMLInteractionNode::SelectRegion;
//        }
//      else if ( !strcmp (attValue, "LassoRegion" ))
//        {
//        this->LastInteractionMode = vtkMRMLInteractionNode::LassoRegion;
//        }
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLInteractionNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLInteractionNode *node = (vtkMRMLInteractionNode *) anode;

  this->SetCurrentInteractionMode (node->GetCurrentInteractionMode());
  this->SetPlaceModePersistence (node->GetPlaceModePersistence());

  this->SetLastInteractionMode ( node->GetLastInteractionMode());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  os << indent << "CurrentInteractionMode:        " << this->GetInteractionModeAsString(this->CurrentInteractionMode) << "\n";
  os << indent << "LastInteractionMode:        " <<  this->GetInteractionModeAsString(this->LastInteractionMode) << "\n";

  os << indent << "PlaceModePersistence: " << this->GetPlaceModePersistence() << "\n";
  os << indent << "TransformModePersistence: " << this->GetTransformModePersistence() << "\n";
}

//---------------------------------------------------------------------------
const char * vtkMRMLInteractionNode::GetInteractionModeAsString(int mode)
{
  if (mode == this->Place)
    {
    return "Place";
    }
  else if (mode == this->ViewTransform)
    {
    return "ViewTransform";
    }
  //  else if (mode == this->SelectRegion)
  //    {
  //    return "SelectRegion";
  //    }
  //  else if (mode == this->LassoRegion)
  //    {
  //    return "LassoRegion";
  //    }
  else
    {
    return "(unknown)";
    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SwitchToPersistentPlaceMode()
{
  if (this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
      this->GetPlaceModePersistence() == 1)
    {
    return;
    }
  this->NormalizeAllMouseModes();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  this->SetPlaceModePersistence(1);
  this->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SwitchToSinglePlaceMode()
{
  if (this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
      this->GetPlaceModePersistence() == 0)
    {
    return;
    }
  this->NormalizeAllMouseModes();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  this->SetPlaceModePersistence(0);
  this->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SwitchToViewTransformMode()
{
  if (this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewTransform &&
      this->GetTransformModePersistence() == 1)
    {
    return;
    }
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  // only set transform mode persistence, keep the state of the pick and place
  // mode persistence
  this->SetTransformModePersistence(1);
  this->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
}
