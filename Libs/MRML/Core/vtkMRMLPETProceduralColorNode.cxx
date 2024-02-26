
#include "vtkObjectFactory.h"

#include "vtkMRMLPETProceduralColorNode.h"

#include "vtkColorTransferFunction.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLPETProceduralColorNode);

//----------------------------------------------------------------------------
vtkMRMLPETProceduralColorNode::vtkMRMLPETProceduralColorNode()
{

  // all this is done in the superclass...
  // this->Name = nullptr;
  // this->SetName("");
  // this->FileName = nullptr;

  // this->ColorTransferFunction = nullptr;
  // this->ColorTransferFunction = vtkColorTransferFunction::New();
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
void vtkMRMLPETProceduralColorNode::Copy(vtkMRMLNode* anode)
{
  Superclass::Copy(anode);
  // vtkMRMLPETProceduralColorNode *node = (vtkMRMLPETProceduralColorNode *) anode;
}

//----------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::PrintSelf(ostream& os, vtkIndent indent)
{

  Superclass::PrintSelf(os, indent);
  if (this->ColorTransferFunction != nullptr)
  {
    os << indent << "ColorTransferFunction:" << endl;
    this->ColorTransferFunction->PrintSelf(os, indent.GetNextIndent());
  }
}

//-----------------------------------------------------------

void vtkMRMLPETProceduralColorNode::UpdateScene(vtkMRMLScene* scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
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
void vtkMRMLPETProceduralColorNode::SetTypeToRainbow2()
{
  this->SetType(this->PETrainbow);
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetTypeToDICOM()
{
  this->SetType(this->PETDICOM);
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetTypeToHotMetalBlue()
{
  this->SetType(this->PEThotMetalBlue);
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetTypeToMIP()
{
  this->SetType(this->PETMIP);
}

//---------------------------------------------------------------------------
const char* vtkMRMLPETProceduralColorNode::GetTypeAsString()
{
  if (this->Type == this->PETheat)
  {
    return "PET-Heat";
  }
  else if (this->Type == this->PETrainbow)
  {
    return "PET-Rainbow";
  }
  else if (this->Type == this->PETrainbow2)
  {
    return "PET-Rainbow2";
  }
  else if (this->Type == this->PETMIP)
  {
    return "PET-MaximumIntensityProjection";
  }
  else if (this->Type == this->PETDICOM)
  {
    return "PET-DICOM";
  }
  else if (this->Type == this->PEThotMetalBlue)
  {
    return "PET-HotMetalBlue";
  }
  return "(unknown)";
}

//---------------------------------------------------------------------------
void vtkMRMLPETProceduralColorNode::SetType(int type)
{

  this->Type = type;

  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Type to " << type << " = "
                << this->GetTypeAsString());

  // is it created yet?
  if (this->ColorTransferFunction == nullptr)
  {
    this->ColorTransferFunction = vtkColorTransferFunction::New();
  }

  // clear it out
  this->ColorTransferFunction->RemoveAllPoints();
  this->ColorTransferFunction->SetColorSpaceToRGB();

  // Set up the custom colors here for this type
  if (this->Type == this->PETheat)
  {
    this->SetDescription("Useful for displaying colorized PET data.");
    this->ColorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(35, 0.36, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(38, 0.5, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(50, 0.7, 0.1, 0.0);
    this->ColorTransferFunction->AddRGBPoint(70, 0.7, 0.5, 0.0);
    this->ColorTransferFunction->AddRGBPoint(87, 1.0, 1.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(255, 1.0, 1.0, 1.0);
  }
  else if (this->Type == this->PETrainbow)
  {
    this->SetDescription("Useful for displaying colorized PET data.");
    this->ColorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(35, 0.0, 0.0, 0.3);
    this->ColorTransferFunction->AddRGBPoint(38, 0.2, 0.0, 0.3);
    this->ColorTransferFunction->AddRGBPoint(50, 0.5, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(70, 0.7, 0.5, 0.0);
    this->ColorTransferFunction->AddRGBPoint(87, 1.0, 1.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(255, 1.0, 1.0, 1.0);
  }
  else if (this->Type == this->PETrainbow2)
  {
    this->SetDescription("Useful for displaying colorized PET data. Based on Rainbow.lut at "
                         "https://sourceforge.net/projects/bifijiplugins/files/extraLUT/");
    this->ColorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(32, 0.250980392, 0, 0.501960784);
    this->ColorTransferFunction->AddRGBPoint(64, 0.0, 0.0, 1.0);
    this->ColorTransferFunction->AddRGBPoint(96, 0.0, 1.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(160, 1.0, 1.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(192, 1.0, 0.752941176, 0.0);
    this->ColorTransferFunction->AddRGBPoint(255, 1.0, 0.011764706, 0.0);
  }
  else if (this->Type == this->PETMIP)
  {
    this->SetDescription("Useful for displaying inverse grey PET data.");
    this->ColorTransferFunction->AddRGBPoint(0, 1.0, 1.0, 1.0);
    this->ColorTransferFunction->AddRGBPoint(255, 0.0, 0.0, 0.0);
  }
  else if (this->Type == this->PETDICOM)
  {
    this->SetDescription("DICOM PET Color Palette (Part 6, B.1.2)");
    this->ColorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(63, 0.0, 0.501960784, 0.490196078);
    this->ColorTransferFunction->AddRGBPoint(128, 0.501960784, 0.0, 1.0);
    this->ColorTransferFunction->AddRGBPoint(192, 1.0, 0.501960784, 0.0);
    this->ColorTransferFunction->AddRGBPoint(255, 1.0, 1.0, 1.0);
  }
  else if (this->Type == this->PEThotMetalBlue)
  {
    this->SetDescription("DICOM Hot Metal Blue Color Palette (Part 6, B.1.3)");
    this->ColorTransferFunction->AddRGBPoint(0, 0.0, 0.0, 0.0);
    this->ColorTransferFunction->AddRGBPoint(85, 0.0, 0.0, 0.654901961);
    this->ColorTransferFunction->AddRGBPoint(102, 0.196078431, 0.0, 0.784313725);
    this->ColorTransferFunction->AddRGBPoint(119, 0.352941176, 0.0, 0.588235294);
    this->ColorTransferFunction->AddRGBPoint(136, 0.549019608, 0.125490196, 0.196078431);
    this->ColorTransferFunction->AddRGBPoint(153, 0.745098039, 0.250980392, 0.0);
    this->ColorTransferFunction->AddRGBPoint(170, 1.0, 0.376470588, 0.0);
    this->ColorTransferFunction->AddRGBPoint(255, 1.0, 1.0, 1.0);
  }

  // build it

  // invoke a modified event
  this->Modified();

  // invoke a type  modified event
  this->InvokeEvent(vtkMRMLProceduralColorNode::TypeModifiedEvent);
}
