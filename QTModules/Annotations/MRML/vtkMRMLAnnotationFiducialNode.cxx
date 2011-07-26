#include <sstream>

#include "vtkObjectFactory.h"
#include "vtkMRMLAnnotationFiducialNode.h"
#include <vtkPolyData.h>

//------------------------------------------------------------------------------
vtkMRMLAnnotationFiducialNode* vtkMRMLAnnotationFiducialNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationFiducialNode");
  if(ret)
    {
    return (vtkMRMLAnnotationFiducialNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationFiducialNode;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAnnotationFiducialNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLAnnotationFiducialNode");
  if(ret)
    {
    return (vtkMRMLAnnotationFiducialNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLAnnotationFiducialNode;
}


//---------------------------------------------------------------------------
int vtkMRMLAnnotationFiducialNode::SetFiducial(const char* label, double newControl[3],int selectedFlag, int visibleFlag)
{
  if (!this->SetFiducialCoordinates(newControl))
    {
      return 0;
    }

  if (label) 
    {
      this->SetFiducialLabel(label);
    }
  else 
    {
      this->SetTextFromID();      
    }

  this->SetSelected(selectedFlag);
  this->SetVisible(visibleFlag);

  return 1;
}

//---------------------------------------------------------------------------
bool vtkMRMLAnnotationFiducialNode::GetFiducialCoordinates(double coord[3])
{
  coord[0] = coord[1] = coord[2] = 0.0;
  if (this->PolyData && this->PolyData->GetPoints()) 
    {
    this->PolyData->GetPoint(0, coord);
    return true;
    }
  return false;
}

//---------------------------------------------------------------------------
void vtkMRMLAnnotationFiducialNode::SetTextFromID()
{
  std::string idLabel = this->GetID();
  std::string textLabel;
  
  if (this->NumberingScheme == vtkMRMLAnnotationControlPointsNode::UseID)
    {
      textLabel = idLabel;
    }
  else
    {
    size_t pos = idLabel.find_last_not_of("0123456789");
    std::string strippedID = idLabel.substr(0, pos+1);
    std::stringstream ss;
    ss << strippedID;

    if (this->NumberingScheme == vtkMRMLAnnotationControlPointsNode::UseIndex)
      {
    // use the fid's index
    ss << '0';
      }
    else if (this->NumberingScheme == vtkMRMLAnnotationControlPointsNode::UsePrevious)
      {
    vtkErrorMacro("Currently option vtkMRMLAnnotationControlPointsNode::UsePrevious is not installed");
    return;
      }
      // use the number from the previous fiducial
      // int lastNumber = 0;
      // if (id > 0)
      //   {
      //       std::string previousLabel = this->GetText(id-1);
      //   size_t prevpos = previousLabel.find_last_not_of("0123456789");
      //   std::string suffixPreviousLabel = previousLabel.substr(prevpos+1, std::string::npos);
      //   lastNumber = atoi(suffixPreviousLabel.c_str());
      //   lastNumber++;
      //   }
      // ss << lastNumber;
      // }
    textLabel = ss.str();
    }
  this->SetFiducialLabel(textLabel.c_str());
}

