// MRML includes
#include "vtkMRMLAnnotationSnapshotNode.h"
#include "vtkMRMLAnnotationSnapshotStorageNode.h"
#include "vtkMRMLScene.h"

// VTKsys includes
#include <vtksys/SystemTools.hxx>

// VTK includes
#include <vtkImageData.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode::vtkMRMLAnnotationSnapshotNode()
{
  this->ScreenShot = nullptr;
  this->ScaleFactor = 1.0;
}

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode::~vtkMRMLAnnotationSnapshotNode()
{
  if (this->ScreenShot)
    {
    this->ScreenShot->Delete();
    this->ScreenShot = nullptr;
    }
}

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLAnnotationSnapshotNode);


//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  of << " screenshotType=\"" << this->GetScreenShotType() << "\"";

  vtkStdString description = this->GetSnapshotDescription();
  vtksys::SystemTools::ReplaceString(description,"\n","[br]");

  of << " snapshotDescription=\"" << description << "\"";

  of << " scaleFactor=\"" << this->GetScaleFactor() << "\"";
}


//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotNode::ReadXMLAttributes(const char** atts)
{

  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
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
    else if (!strcmp(attName, "scaleFactor"))
      {
      std::stringstream ss;
      ss << attValue;
      double scaleFactor;
      ss >> scaleFactor;
      this->SetScaleFactor(scaleFactor);
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
  vtkMRMLScene* scene = this->GetScene();
  if (scene == nullptr)
    {
    vtkErrorMacro("CreateDefaultStorageNode failed: scene is invalid");
    return nullptr;
    }
  return vtkMRMLStorageNode::SafeDownCast(
    scene->CreateNodeByClass("vtkMRMLAnnotationSnapshotStorageNode"));
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotNode::SetSnapshotDescription(const vtkStdString& newDescription)
{
  if (this->SnapshotDescription == newDescription)
    {
    return;
    }
  this->SnapshotDescription = newDescription;
  this->StorableModifiedTime.Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotNode::SetScreenShot(vtkImageData* newScreenShot)
{
  this->StorableModifiedTime.Modified();
  vtkSetObjectBodyMacro(ScreenShot, vtkImageData, newScreenShot);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationSnapshotNode::SetScreenShotType(int newScreenShotType)
{
  if (this->ScreenShotType == newScreenShotType)
    {
    return;
    }
  this->ScreenShotType = newScreenShotType;
  this->StorableModifiedTime.Modified();
  this->Modified();
}
