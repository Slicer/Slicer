#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationSnapshotNode.h"
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

}

//------------------------------------------------------------------------------
vtkMRMLAnnotationSnapshotNode::~vtkMRMLAnnotationSnapshotNode()
{

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

  of << indent << " screenshotType=\"" << this->GetScreenshotType() << "\"";

  vtkStdString description = this->GetSnapshotDescription();
  vtksys::SystemTools::ReplaceString(description,"\n","[br]");

  of << indent << " snapshotDescription=\"" << description << "\"";

  if (this->GetScreenshot())
    {
    // create the directory 'AnnotationScreenshots'
    vtkStdString screenCapturePath;
    screenCapturePath += this->GetScene()->GetRootDirectory();
    screenCapturePath += "/";
    screenCapturePath += "AnnotationScreenshots/";

    vtksys::SystemTools::MakeDirectory(vtksys::SystemTools::ConvertToOutputPath(screenCapturePath.c_str()).c_str());

    // write out the associated screencapture
    vtkSmartPointer<vtkPNGWriter> pngWriter = vtkSmartPointer<vtkPNGWriter>::New();
    pngWriter->SetInput(this->GetScreenshot());

    vtkStdString screenCaptureFilename;
    screenCaptureFilename += screenCapturePath;
    screenCaptureFilename += this->GetID();
    screenCaptureFilename += ".png";

    pngWriter->SetFileName(vtksys::SystemTools::ConvertToOutputPath(screenCaptureFilename.c_str()).c_str());
    pngWriter->Write();
    }

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
      this->SetScreenshotType(screenshotType);
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

  // now read the screenCapture
  vtkStdString screenCapturePath;
  screenCapturePath += this->GetScene()->GetRootDirectory();
  screenCapturePath += "/";
  screenCapturePath += "AnnotationScreenshots/";

  vtkStdString screenCaptureFilename;
  screenCaptureFilename += screenCapturePath;
  screenCaptureFilename += this->GetID();
  screenCaptureFilename += ".png";


  if (vtksys::SystemTools::FileExists(vtksys::SystemTools::ConvertToOutputPath(screenCaptureFilename.c_str()).c_str(),true))
    {

    vtkSmartPointer<vtkPNGReader> pngReader = vtkSmartPointer<vtkPNGReader>::New();
    pngReader->SetFileName(vtksys::SystemTools::ConvertToOutputPath(screenCaptureFilename.c_str()).c_str());
    pngReader->Update();

    vtkImageData* imageData = vtkImageData::New();
    imageData->DeepCopy(pngReader->GetOutput());

    this->SetScreenshot(imageData);
    this->GetScreenshot()->SetSpacing(1.0, 1.0, 1.0);
    this->GetScreenshot()->SetOrigin(0.0, 0.0, 0.0);
    this->GetScreenshot()->SetScalarType(VTK_UNSIGNED_CHAR);
    }


  this->EndModify(disabledModify);
}

