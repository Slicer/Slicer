
#include "vtkObjectFactory.h"

#include "vtkMRMLPETProceduralColorNode.h"

#include "vtkColorTransferFunction.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPETProceduralColorNode);


//----------------------------------------------------------------------------
vtkMRMLPETProceduralColorNode::vtkMRMLPETProceduralColorNode()
{

  // all this is done in the superclass...
  //this->Name = nullptr;
  //this->SetName("");
  //this->FileName = nullptr;

  //this->ColorTransferFunction = nullptr;
  //this->ColorTransferFunction = vtkColorTransferFunction::New();
}

//----------------------------------------------------------------------------
vtkMRMLPETProceduralColorNode::~vtkMRMLPETProceduralColorNode()
{
  if (this->ColorTransferFunction)
    {
    this->ColorTransferFunction->Delete();
    this->ColorTransferFunction = nullptr;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  Superclass::WriteXML(of, nIndent);

}

//----------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::ReadXMLAttributes(const char** atts)
{

  Superclass::ReadXMLAttributes(atts);

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLPETProceduralColorNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  //vtkMRMLPETProceduralColorNode *node = (vtkMRMLPETProceduralColorNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os,indent);
  if (this->ColorTransferFunction != nullptr)
    {
    os << indent << "ColorTransferFunction:" << endl;
    this->ColorTransferFunction->PrintSelf(os, indent.GetNextIndent());
    }
}

//-----------------------------------------------------------

void vtkMRMLPETProceduralColorNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetTypeToHeat()
{
  this->SetType(this->PETheat);
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetTypeToRainbow()
{
  this->SetType(this->PETrainbow);
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetTypeToMIP()
{
  this->SetType(this->PETMIP);
}

//---------------------------------------------------------------------------
const char *vtkMRMLPETProceduralColorNode::GetTypeAsString()
{
  if (this->Type == this->PETheat)
    {
    return "PET-Heat";
    }
  else if (this->Type == this->PETrainbow)
    {
    return "PET-Rainbow";
    }
  else if ( this->Type == this->PETMIP )
    {
    return "PET-MaximumIntensityProjection";
    }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetType(int type)
{

  this->Type = type;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type << " = " << this->GetTypeAsString());

  // is it created yet?
  if (this->ColorTransferFunction == nullptr)
    {
    this->ColorTransferFunction = vtkColorTransferFunction::New();
    }

  // clear it out
  this->ColorTransferFunction->RemoveAllPoints();
  this->ColorTransferFunction->SetColorSpaceToRGB();

  // Set up the custom colours here for this type
  if (this->Type == this->PETheat)
    {
    this->SetDescription("Useful for displaying colorized PET data.");
    if (this->ColorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType heat: error adding point 0");
      }
    if (this->ColorTransferFunction->AddRGBPoint(35, 0.36, 0.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType heat: error adding point 35");
      }
    if (this->ColorTransferFunction->AddRGBPoint(38, 0.5, 0.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType heat: error adding point 38.0");
      }
    if (this->ColorTransferFunction->AddRGBPoint(50, 0.7, 0.1, 0.0) == -1)
      {
      vtkErrorMacro("SetType heat: error adding point 50");
      }
    if (this->ColorTransferFunction->AddRGBPoint(70, 0.7, 0.5, 0.0) == -1)
      {
      vtkErrorMacro("SetType heat: error adding point 70");
      }
    if (this->ColorTransferFunction->AddRGBPoint(87, 1.0, 1.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType heat: error adding point 87");
      }
    if (this->ColorTransferFunction->AddRGBPoint(255, 1.0, 1.0, 1.0) == -1)
      {
      vtkErrorMacro("SetType heat: error adding point 255");
      }
    }
  else if (this->Type == this->PETrainbow)
    {
    this->SetDescription("Useful for displaying colorized PET data.");
    if (this->ColorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType rainbowT: error adding point 0");
      }
    if (this->ColorTransferFunction->AddRGBPoint(35, 0.0, 0.0, 0.3) == -1)
      {
      vtkErrorMacro("SetType rainbowT: error adding point 35");
      }
    if (this->ColorTransferFunction->AddRGBPoint(38, 0.2, 0.0, 0.3) == -1)
      {
      vtkErrorMacro("SetType rainbowT: error adding point 38");
      }
    if (this->ColorTransferFunction->AddRGBPoint(50, 0.5, 0.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType rainbowT: error adding point 50");
      }
    if (this->ColorTransferFunction->AddRGBPoint(70, 0.7, 0.5, 0.0) == -1)
      {
      vtkErrorMacro("SetType rainbowT: error adding point 70");
      }
    if (this->ColorTransferFunction->AddRGBPoint(87, 1.0, 1.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType rainbowT: error adding point 87");
      }
    if (this->ColorTransferFunction->AddRGBPoint(255, 1.0, 1.0, 1.0) == -1)
      {
      vtkErrorMacro("SetType rainbowT: error adding point 255");      }
    }
  else if (this->Type == this->PETMIP)
    {
    this->SetDescription("Useful for displaying inverse grey PET data.");
    if (this->ColorTransferFunction->AddRGBPoint(0, 1.0, 1.0, 1.0) == -1)
      {
      vtkErrorMacro("SetType MIP: error adding point 0");
      }
    if (this->ColorTransferFunction->AddRGBPoint(255, 0.0, 0.0, 0.0) == -1)
      {
      vtkErrorMacro("SetType MIP: error adding point 255");      }
    }

  // build it

  // invoke a modified event
  this->Modified();

  // invoke a type  modified event
  this->InvokeEvent(vtkMRMLProceduralColorNode::TypeModifiedEvent);
}
