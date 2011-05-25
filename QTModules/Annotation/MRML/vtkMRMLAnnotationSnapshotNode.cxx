#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkMRMLAnnotationSnapshotStorageNode.h"
#include "vtkBitArray.h"
#include "vtkDataSetAttributes.h"
#include "vtkPointData.h"
#include <vtkPNGWriter.h>
#include <vtkPNGReader.h>
#include <vtkSmartPointer.h>
#include "vtkStringArray.h"

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode::vtkMRMLAnnotationSnapshotNode()
{
  this->ScreenShot = NULL;
}

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode::~vtkMRMLAnnotationSnapshotNode()
{
  if (this->ScreenShot)
    {
    this->ScreenShot->Delete();
    this->ScreenShot = NULL;
    }
}

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode* vtkMRMLAnnotationSnapshotNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationSnapshotNode");
  if(ret)
    {
    return (vtkMRMLAnnotationSnapshotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationSnapshotNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationSnapshotNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationSnapshotNode");
  if(ret)
    {
    return (vtkMRMLAnnotationSnapshotNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationSnapshotNode;
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  of << indent << " screenshotType=\"" << this->GetScreenShotType() << "\"";

  vtkStdString description = this->GetSnapshotDescription();
  vtksys::SystemTools::ReplaceString(description,"\n","[br]");

  of << indent << " snapshotDescription=\"" << description << "\"";

}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotNode::ReadXMLAttributes(const char** atts)
{

  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "screenshotType"))
      {
      std::stringstream ss;
      ss << attValue;
      int screenshotType;
      ss >> screenshotType;
      this->SetScreenShotType(screenshotType);
      }
    else if(!strcmp(attName, "snapshotDescription"))
      {
      std::stringstream ss;
      ss << attValue;
      vtkStdString sceneViewDescription;
      ss >> sceneViewDescription;

      vtksys::SystemTools::ReplaceString(sceneViewDescription,"[br]","\n");

      this->SetSnapshotDescription(sceneViewDescription);
      }
    }
  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLAnnotationSnapshotNode::CreateDefaultStorageNode()
{
  return vtkMRMLAnnotationSnapshotStorageNode::New();
}
