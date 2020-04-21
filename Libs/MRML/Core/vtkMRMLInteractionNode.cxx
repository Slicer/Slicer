
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
vtkMRMLInteractionNode::~vtkMRMLInteractionNode() = default;

//----------------------------------------------------------------------------
int vtkMRMLInteractionNode::GetInteractionModeByString ( const char * modeString )
{
  if (modeString == nullptr)
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
    this->InvokeEvent(this->InteractionModePersistenceChangedEvent, nullptr);
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SetTransformModePersistence ( int val )
{
  if (this->TransformModePersistence != val)
    {
    this->TransformModePersistence = val;
    this->InvokeEvent(this->InteractionModePersistenceChangedEvent, nullptr);
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
  if (this->CurrentInteractionMode == mode)
    {
    return;
    }
  bool wasPlacing = (this->CurrentInteractionMode == vtkMRMLInteractionNode::Place);
  this->CurrentInteractionMode = mode;
  this->Modified();
  this->InvokeCustomModifiedEvent(vtkMRMLInteractionNode::InteractionModeChangedEvent);
  if (wasPlacing)
    {
    this->InvokeCustomModifiedEvent(vtkMRMLInteractionNode::EndPlacementEvent);
    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

  if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place )
    {
    of << " currentInteractionMode=\"" << "Place" << "\"";
    }
  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewTransform )
    {
    of << " currentInteractionMode=\"" << "ViewTransform" << "\"";
    }
//  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::SelectRegion )
//    {
//    of << " currentInteractionMode=\"" << "SelectRegion" << "\"";
//    }
//  else if ( this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::LassoRegion )
//    {
//    of << " currentInteractionMode=\"" << "LassoRegion" << "\"";
//    }

  of << " placeModePersistence=\"" << (this->PlaceModePersistence ? "true" : "false") << "\"";

  if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::Place )
    {
    of << " lastInteractionMode=\"" << "Place" << "\"";
    }
  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::ViewTransform )
    {
    of << " lastInteractionMode=\"" << "ViewTransform" << "\"";
    }
//  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::SelectRegion )
//    {
//    of << " lastInteractionMode=\"" << "SelectRegion" << "\"";
//    }
//  else if ( this->GetLastInteractionMode() == vtkMRMLInteractionNode::LassoRegion )
//    {
//    of << " lastInteractionMode=\"" << "LassoRegion" << "\"";
//    }
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
  int disabledModify = this->StartModify();
  this->NormalizeAllMouseModes();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  this->SetPlaceModePersistence(1);
  this->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SwitchToSinglePlaceMode()
{
  if (this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::Place &&
      this->GetPlaceModePersistence() == 0)
    {
    return;
    }
  int disabledModify = this->StartModify();
  this->NormalizeAllMouseModes();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  this->SetPlaceModePersistence(0);
  this->SetCurrentInteractionMode(vtkMRMLInteractionNode::Place);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::SwitchToViewTransformMode()
{
  if (this->GetCurrentInteractionMode() == vtkMRMLInteractionNode::ViewTransform &&
      this->GetTransformModePersistence() == 1)
    {
    return;
    }
  int disabledModify = this->StartModify();
  this->SetLastInteractionMode(this->GetCurrentInteractionMode());
  // only set transform mode persistence, keep the state of the pick and place
  // mode persistence
  this->SetTransformModePersistence(1);
  this->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLInteractionNode::EditNode(vtkMRMLNode* node)
{
  // Observers in qSlicerCoreApplication listen for this event
  this->InvokeEvent(EditNodeEvent, node);
}
