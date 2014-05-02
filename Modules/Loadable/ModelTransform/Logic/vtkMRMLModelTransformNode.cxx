#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLModelTransformNode.h"

//------------------------------------------------------------------------------

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLModelTransformNode);

//----------------------------------------------------------------------------
vtkMRMLModelTransformNode::vtkMRMLModelTransformNode()
{
  this->HideFromEditors = true;
  this->InputModelID = NULL;
  this->OutputModelID = NULL;
  this->TransformNodeID = NULL;
  this->NewModelName = NULL;
  this->TransformNormals = 1;
}

//----------------------------------------------------------------------------
vtkMRMLModelTransformNode::~vtkMRMLModelTransformNode()
{
  this->SetInputModelID ( NULL );
  this->SetOutputModelID ( NULL );
  this->SetTransformNodeID ( NULL );
  this->SetNewModelName ( NULL );
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLModelTransformNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLModelTransformNode *node = vtkMRMLModelTransformNode::SafeDownCast(anode);
  if (node)
  {
    this->SetInputModelID(node->GetInputModelID());
    this->SetOutputModelID(node->GetOutputModelID());
    this->SetTransformNodeID(node->GetTransformNodeID());
    this->SetNewModelName(node->GetNewModelName());
    this->SetTransformNormals(node->GetTransformNormals());
  }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLModelTransformNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //TODO
  vtkMRMLNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkMRMLModelTransformNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->InputModelID != NULL)
    {
    of << indent << " inputModelRef=\"" << this->InputModelID << "\"";
    }
  if (this->TransformNodeID != NULL)
    {
    of << indent << " transformNodeRef=\"" << this->TransformNodeID << "\"";
    }
  if (this->OutputModelID != NULL)
    {
    of << indent << " outputModelRef=\"" << this->OutputModelID << "\"";
    }
  if (this->NewModelName != NULL)
    {
    of << indent << " newModelName=\"" << this->NewModelName << "\"";
    }

  of << indent << " transformNormals=\"" << this->TransformNormals << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLModelTransformNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "inputModelRef"))
      {
      this->SetInputModelID(attValue);
      }
    else if (!strcmp(attName, "transformNodeRef"))
      {
      this->SetTransformNodeID(attValue);
      }
    else if (!strcmp(attName, "outputModelRef"))
      {
      this->SetOutputModelID(attValue);
      }
    else if (!strcmp(attName, "newModelName"))
      {
      this->SetNewModelName(attValue);
      }
    else if (!strcmp(attName, "transformNormals"))
      {
      std::stringstream ss;
      ss << attValue;
      ss >> TransformNormals;
      }
    }

  this->EndModify(disabledModify);
}
