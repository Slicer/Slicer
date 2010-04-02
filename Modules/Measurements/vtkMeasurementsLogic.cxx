#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkIntArray.h"

#include "vtkMeasurementsLogic.h"
#include "vtkMeasurements.h"

#include "vtkMRMLScene.h"

// for making a ruler between fids
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLMeasurementsRulerNode.h"

//----------------------------------------------------------------------------
vtkMeasurementsLogic* vtkMeasurementsLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMeasurementsLogic");
  if(ret)
    {
      return (vtkMeasurementsLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMeasurementsLogic;
}


//----------------------------------------------------------------------------
vtkMeasurementsLogic::vtkMeasurementsLogic()
{
}



//----------------------------------------------------------------------------
vtkMeasurementsLogic::~vtkMeasurementsLogic()
{ 
  this->SetAndObserveMRMLScene ( NULL );
}


//----------------------------------------------------------------------------
void vtkMeasurementsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);  
}


//----------------------------------------------------------------------------
int vtkMeasurementsLogic::NewRulerBetweenFiducials(const char *rulerName)
{
  /// Make a new ruler node, popping up a rename dialogue first, placing it
  /// between the last two fiducials added on the currently selected list,
  /// then remove those two fiducials. If the list name is not
  /// 'MeasurementsRuler', use that in the list annotation string
  if (this->GetMRMLScene() == NULL)
    {
    vtkErrorMacro("NewRulerBetweenFiducials: no mrml scene, no idea which fiduical list to use!");
    return 0;
    }

  // get the selection node
  vtkMRMLSelectionNode *selnode = NULL;
  
  selnode = vtkMRMLSelectionNode::SafeDownCast (
            this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLSelectionNode"));

  if (selnode == NULL)
    {
    vtkErrorMacro("NewRulerBetweenFiducials: no selection node in mrml scene, no idea which fiduical list to use!");
    return 0;
    }
  if (selnode->GetActiveFiducialListID() == NULL)
    {
    vtkErrorMacro("NewRulerBetweenFiducials: no active fiducial list in the selection node, no idea which fiduical list to use!");
    return 0;
    }

  // get the active fiducial list
  vtkMRMLFiducialListNode *activeFiducialList = NULL;
  activeFiducialList = vtkMRMLFiducialListNode::SafeDownCast(this->MRMLScene->GetNodeByID(selnode->GetActiveFiducialListID()));
  if (activeFiducialList == NULL)
    {
    vtkErrorMacro("NewRulerBetweenFiducials: no valid active fiducial list in the selection node, no idea which fiduical list to use!");
    return 0;
    }

  int numFids = activeFiducialList->GetNumberOfFiducials();

  if (numFids < 2)
    {
    vtkErrorMacro("NewRulerBetweenFiducials: only have " << numFids << " fidicuals in list " << activeFiducialList->GetName() << ", need at least 2 to make a ruler from the last two in the list");
    return 0;
    }
  float *p1 = activeFiducialList->GetNthFiducialXYZ(numFids-1);
  float *p2 = activeFiducialList->GetNthFiducialXYZ(numFids-2);

  if (p1 == NULL)
    {
    vtkErrorMacro("NewRulerBetweenFiducials: have an invalid point position for fid number " << numFids-1);
    return 0;
    }
  if (p2 == NULL)
    {
    vtkErrorMacro("NewRulerBetweenFiducials: have an invalid point position for fid number " << numFids-2);
    return 0;
    }

  // okay, now can make a new ruler node
  vtkMRMLMeasurementsRulerNode *ruler = vtkMRMLMeasurementsRulerNode::New();
  std::string name;
  if (rulerName == NULL)
    {
    name = std::string("Ruler: ");
    }
  else if (strcmp(rulerName,"") == 0)
    {
    name = std::string("");
    }
  else
    {
    name = std::string(rulerName) + std::string(": ");
    ruler->SetName(rulerName);
    }      
  
  std::string annotFormat = name + std::string("%.1f mm");
  ruler->SetDistanceAnnotationFormat(annotFormat.c_str());
  double pos1[3] = {p1[0], p1[1], p1[2]};
  double pos2[3] = {p2[0], p2[1], p2[2]};
  ruler->SetPosition1(pos1);
  ruler->SetPosition2(pos2);
  this->GetMRMLScene()->AddNode(ruler);
  ruler->Delete();

  // delete the two fids that were used
  activeFiducialList->RemoveFiducial(numFids-1);
  activeFiducialList->RemoveFiducial(numFids-2);

  return 1;
}
