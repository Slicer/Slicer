/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkProstateNavLogic.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerFiducialsGUI.h"
#include "vtkSlicerColorLogic.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"

#include "vtkMRMLProstateNavManagerNode.h"
#include "vtkMRMLRobotNode.h"

#include "vtkKWRadioButton.h"

// for DICOM read
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkGDCMImageIO.h"
#include "itkSpatialOrientationAdapter.h"

#include "vtkMRMLBrpRobotCommandNode.h"

#include "vtkProstateNavGUI.h"

#include "ProstateNavMath.h"

const int COVERAGE_MAP_SIZE_MM=500;
const int COVERAGE_MAP_RESOLUTION_MM=5;

vtkCxxRevisionMacro(vtkProstateNavLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkProstateNavLogic);

//---------------------------------------------------------------------------
vtkProstateNavLogic::vtkProstateNavLogic()
{
  // Timer Handling
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkProstateNavLogic::DataCallback);

  this->TimerOn = 0;
}


//---------------------------------------------------------------------------
vtkProstateNavLogic::~vtkProstateNavLogic()
{
  if (this->DataCallbackCommand)
  {
    this->DataCallbackCommand->Delete();
  }
}

//---------------------------------------------------------------------------
void vtkProstateNavLogic::PrintSelf(ostream& os, vtkIndent indent)
{
    this->vtkObject::PrintSelf(os, indent);

    os << indent << "vtkProstateNavLogic:             " << this->GetClassName() << "\n";

}

//---------------------------------------------------------------------------
void vtkProstateNavLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
    vtkProstateNavLogic *self = reinterpret_cast<vtkProstateNavLogic *>(clientData);
    vtkDebugWithObjectMacro(self, "In vtkProstateNavLogic DataCallback");
    self->UpdateAll();
}

//---------------------------------------------------------------------------
void vtkProstateNavLogic::UpdateAll()
{

}


int vtkProstateNavLogic::Enter()
{      
  vtkKWTkUtilities::CreateTimerHandler(this->GetGUI()->GetApplication(), 200, this, "TimerHandler");
  return 1;
}

//----------------------------------------------------------------------------
void vtkProstateNavLogic::TimerHandler()
{
  if (this->TimerOn)
    {
      if (GUI!=NULL)
      {
        if (GUI->GetProstateNavManager()!=NULL)
        {
          if (GUI->GetProstateNavManager()->GetRobotNode()!=NULL)
          {
            this->GUI->GetProstateNavManager()->GetRobotNode()->OnTimer();
          }
        }
      }      
    }
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::RobotStop()
{

  std::cerr << "vtkProstateNavLogic::RobotStop()" << std::endl;
  return 1;

}


//---------------------------------------------------------------------------
int vtkProstateNavLogic::RobotMoveTo(float px, float py, float pz,
                                     float nx, float ny, float nz,
                                     float tx, float ty, float tz)
{

  std::cerr << "vtkProstateNavLogic::RobotMoveTo()" << std::endl;
  return 1;
}


//---------------------------------------------------------------------------
int vtkProstateNavLogic::RobotMoveTo(float position[3], float orientation[3])
{
  std::cerr << "vtkProstateNavLogic::RobotMoveTo()" << std::endl;

  return 1;
}

//---------------------------------------------------------------------------
int  vtkProstateNavLogic::RobotMoveTo()
{
  return this->GUI->GetProstateNavManager()->GetRobotNode()->MoveTo(this->GUI->GetProstateNavManager()->GetRobotNode()->GetTargetTransformNodeID());  
}
//---------------------------------------------------------------------------
int vtkProstateNavLogic::ScanStart()
{

  return 1;
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::ScanPause()
{
  return 1;
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::ScanStop()
{

  return 1;
}

//---------------------------------------------------------------------------
void vtkProstateNavLogic::SetSliceViewFromVolume(vtkMRMLVolumeNode *volumeNode)
{
  if (!volumeNode)
    {
    return;
    }

  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> permutationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  vtkSmartPointer<vtkMatrix4x4> rotationMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  volumeNode->GetIJKToRASDirectionMatrix(matrix);
  vtkMRMLTransformNode *transformNode = volumeNode->GetParentTransformNode();
  if ( transformNode )
    {
    vtkSmartPointer<vtkMatrix4x4> rasToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
    transformNode->GetMatrixTransformToWorld(rasToRAS);
    vtkMatrix4x4::Multiply4x4 (rasToRAS, matrix, matrix);
    }


  //slicerCerr("matrix");
  //slicerCerr("   " << matrix->GetElement(0,0) <<
//             "   " << matrix->GetElement(0,1) <<
  //           "   " << matrix->GetElement(0,2));
  //slicerCerr("   " << matrix->GetElement(1,0) <<
    //         "   " << matrix->GetElement(1,1) <<
    //         "   " << matrix->GetElement(1,2));
  //slicerCerr("   " << matrix->GetElement(2,0) <<
   //          "   " << matrix->GetElement(2,1) <<
    //         "   " << matrix->GetElement(2,2));

  int permutation[3];
  int flip[3];
  ProstateNavMath::ComputePermutationFromOrientation(matrix, permutation, flip);

  //slicerCerr("permutation " << permutation[0] << " " <<
//             permutation[1] << " " << permutation[2]);
  //slicerCerr("flip " << flip[0] << " " <<
  //           flip[1] << " " << flip[2]);

  permutationMatrix->SetElement(0,0,0);
  permutationMatrix->SetElement(1,1,0);
  permutationMatrix->SetElement(2,2,0);

  permutationMatrix->SetElement(0, permutation[0],
                     (flip[permutation[0]] ? -1 : 1));
  permutationMatrix->SetElement(1, permutation[1],
                     (flip[permutation[1]] ? -1 : 1));
  permutationMatrix->SetElement(2, permutation[2],
                     (flip[permutation[2]] ? -1 : 1));

  //slicerCerr("permutationMatrix");
  //slicerCerr("   " << permutationMatrix->GetElement(0,0) <<
//             "   " << permutationMatrix->GetElement(0,1) <<
  //           "   " << permutationMatrix->GetElement(0,2));
  //slicerCerr("   " << permutationMatrix->GetElement(1,0) <<
    //         "   " << permutationMatrix->GetElement(1,1) <<
      //       "   " << permutationMatrix->GetElement(1,2));
  //slicerCerr("   " << permutationMatrix->GetElement(2,0) <<
        //     "   " << permutationMatrix->GetElement(2,1) <<
          //   "   " << permutationMatrix->GetElement(2,2));

  permutationMatrix->Invert();
  vtkMatrix4x4::Multiply4x4(matrix, permutationMatrix, rotationMatrix); 

  vtkSlicerApplicationLogic *appLogic = this->GetGUI()->GetApplicationLogic();

  
  // Set the slice views to match the volume slice orientation
  for (int i = 0; i < 3; i++)
    {
    static const char *panes[3] = { "Red", "Yellow", "Green" };

    vtkMatrix4x4 *newMatrix = vtkMatrix4x4::New();

    vtkMRMLSliceLogic *slice = appLogic->GetSliceLogic(
      const_cast<char *>(panes[i]));
    
    vtkMRMLSliceNode *sliceNode = slice->GetSliceNode();

    // Need to find window center and rotate around that

    // Default matrix orientation for slice
    newMatrix->SetElement(0, 0, 0.0);
    newMatrix->SetElement(1, 1, 0.0);
    newMatrix->SetElement(2, 2, 0.0);
    if (i == 0)
      {
      newMatrix->SetElement(0, 0, -1.0);
      newMatrix->SetElement(1, 1, 1.0);
      newMatrix->SetElement(2, 2, 1.0);
      }
    else if (i == 1)
      {
      newMatrix->SetElement(1, 0, -1.0);
      newMatrix->SetElement(2, 1, 1.0);
      newMatrix->SetElement(0, 2, 1.0);
      }
    else if (i == 2)
      {
      newMatrix->SetElement(0, 0, -1.0);
      newMatrix->SetElement(2, 1, 1.0);
      newMatrix->SetElement(1, 2, 1.0);
      }

    // Next, set the orientation to match the volume
    sliceNode->SetOrientationToReformat();
    vtkMatrix4x4::Multiply4x4(rotationMatrix, newMatrix, newMatrix);
    sliceNode->SetSliceToRAS(newMatrix);
    sliceNode->UpdateMatrices();
    newMatrix->Delete();
    }

}
//---------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *vtkProstateNavLogic::AddVolumeToScene(const char *fileName, VolumeType volumeType/*=VOL_GENERIC*/)
{
  if (fileName==0)
  {
    vtkErrorMacro("AddVolumeToScene: invalid filename");
    return 0;
  }

  vtksys_stl::string volumeNameString = vtksys::SystemTools::GetFilenameName(fileName);
  vtkMRMLScalarVolumeNode *volumeNode = this->AddArchetypeVolume(fileName, volumeNameString.c_str());

  if (volumeNode==NULL)
  {
    vtkErrorMacro("Error adding volume to the scene");
    return NULL;
  }

  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Error adding volume to the scene, manager is invalid");
    return NULL;
  }

  this->SetSliceViewFromVolume(volumeNode);

  switch (volumeType)
  {
  case VOL_CALIBRATION:
    manager->SetCalibrationVolumeNodeID(volumeNode->GetID());
    break;
  case VOL_TARGETING:
    manager->SetTargetingVolumeNodeID(volumeNode->GetID());
    break;
  case VOL_VERIFICATION:
    manager->SetVerificationVolumeNodeID(volumeNode->GetID());
    break;
  default:
    vtkErrorMacro("AddVolumeToScene: unknown volume type: " << volumeType);
  }
  
  volumeNode->Modified();
  this->Modified();

  return volumeNode;
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::SelectVolumeInScene(vtkMRMLScalarVolumeNode* volumeNode, VolumeType volumeType)
{
  if (volumeNode==0)
  {
    vtkErrorMacro("SelectVolumeInScene: invalid volume");
    return 0;
  }

  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Error adding volume to the scene, manager is invalid");
    return 0;
  }

  this->SetSliceViewFromVolume(volumeNode);

  this->GetGUI()->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID( volumeNode->GetID() );
  this->GetGUI()->GetApplicationLogic()->PropagateVolumeSelection();

  switch (volumeType)
  {
  case VOL_CALIBRATION:
    manager->SetCalibrationVolumeNodeID(volumeNode->GetID());
    break;
  case VOL_TARGETING:
    manager->SetTargetingVolumeNodeID(volumeNode->GetID());
    break;
  case VOL_VERIFICATION:
    manager->SetVerificationVolumeNodeID(volumeNode->GetID());
    break;
  default:
    vtkErrorMacro("AddVolumeToScene: unknown volume type: " << volumeType);
  }
  
  volumeNode->Modified();
  this->Modified();

  return 1;
}

//---------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *vtkProstateNavLogic::AddArchetypeVolume(const char* fileName, const char *volumeName)
{
  // Set up storageNode
  vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode> storageNode = vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode>::New(); 
  storageNode->SetFileName(fileName);
  // check to see if can read this type of file
  if (storageNode->SupportedFileType(fileName) == 0)
    {
    vtkErrorMacro("AddArchetypeVolume: can't read this kind of file: " << fileName);
    return 0;
    }
  storageNode->SetCenterImage(false);
  storageNode->SetSingleFile(false);
  storageNode->SetUseOrientationFromFile(true);

  // Set up scalarNode
  vtkSmartPointer<vtkMRMLScalarVolumeNode> scalarNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  scalarNode->SetName(volumeName);
  scalarNode->SetLabelMap(false);

  // Set up displayNode
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> displayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();   
  displayNode->SetAutoWindowLevel(false);
  displayNode->SetInterpolate(true);  
  vtkSmartPointer<vtkSlicerColorLogic> colorLogic = vtkSmartPointer<vtkSlicerColorLogic>::New(); 
  displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
  
  // Add nodes to scene
  vtkDebugMacro("LoadArchetypeVolume: adding storage node to the scene");
  storageNode->SetScene(this->GetMRMLScene());
  this->GetMRMLScene()->AddNode(storageNode);
  vtkDebugMacro("LoadArchetypeVolume: adding display node to the scene");
  displayNode->SetScene(this->GetMRMLScene());
  this->GetMRMLScene()->AddNode(displayNode);
  vtkDebugMacro("LoadArchetypeVolume: adding scalar node to the scene");
  scalarNode->SetScene(this->GetMRMLScene());
  this->GetMRMLScene()->AddNode(scalarNode);
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
  scalarNode->SetAndObserveDisplayNodeID(displayNode->GetID());
  
  // Read the volume into the node
  vtkDebugMacro("AddArchetypeVolume: about to read data into scalar node " << scalarNode->GetName());
  storageNode->AddObserver(vtkCommand::ProgressEvent, this->LogicCallbackCommand);
  if (this->GetDebug())
    {
    storageNode->DebugOn();
    }
  storageNode->ReadData(scalarNode);
  vtkDebugMacro("AddArchetypeVolume: finished reading data into scalarNode");
  storageNode->RemoveObservers(vtkCommand::ProgressEvent, this->LogicCallbackCommand);
 
  return scalarNode;
}

//--------------------------------------------------------------------------------------
std::string vtkProstateNavLogic::GetFoRStrFromVolumeNodeID(const char* volNodeID)
{  
  vtkMRMLScalarVolumeNode *volNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(volNodeID));  
  if (volNode==NULL)
  {
    vtkErrorMacro("Cannot get FoR, VolumeNode is undefined");
    return std::string("");
  }

  // remaining information to be had from the meta data dictionary     
  const itk::MetaDataDictionary &volDictionary = volNode->GetMetaDataDictionary();
  std::string tagValue; 

  // frame of reference uid
  tagValue.clear(); itk::ExposeMetaData<std::string>( volDictionary, "0020|0052", tagValue );
  
  return tagValue;
}

// Recreate, update coverage volume
// Add the volume and a display node to the scene
// return 0 if failed
//----------------------------------------------------------------------------
int vtkProstateNavLogic::ShowCoverage(bool show) 
{

  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Error showing coverage, manager is invalid");
    return 0;
  }
  
  // always delete it first, because we recreate if it is needed
  // TODO: if the coverage area is not changed (e.g., calibration is not changed) then don't delete and recreate the volume, just show/hide it once it is created
  vtkMRMLScalarVolumeNode* coverageVolumeNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(manager->GetCoverageVolumeNodeID()));
  if (coverageVolumeNode!=NULL)
  {
    DeleteCoverageVolume();
  }

  if (!show)
  {
    // we don't need to show the volume, so we can quit now
    return 1;
  }

  // Save original slice location
  vtkSlicerApplicationGUI *applicationGUI = this->GUI->GetApplicationGUI();
  double oldSliceSetting[3];
  oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceOffset());
  oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceOffset());
  oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceOffset());

  if (CreateCoverageVolume()==0)
  {
    return 0;
  }
  
  coverageVolumeNode=vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(manager->GetCoverageVolumeNodeID()));
  if (coverageVolumeNode==NULL)
  {
    // the volume node should have been created by now
    return 0;
  }

  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelOpacity(0.6);

  // Select coverage volume node id as label
  const char* coverageVolNodeID=manager->GetCoverageVolumeNodeID();
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(coverageVolNodeID);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(coverageVolNodeID);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->GetSliceCompositeNode()->SetLabelVolumeID(coverageVolNodeID);

  // Restore to original slice location
  applicationGUI->GetMainSliceGUI("Red")->GetLogic()->SetSliceOffset(oldSliceSetting[0]);
  applicationGUI->GetMainSliceGUI("Yellow")->GetLogic()->SetSliceOffset(oldSliceSetting[1]);
  applicationGUI->GetMainSliceGUI("Green")->GetLogic()->SetSliceOffset(oldSliceSetting[2]);

  return 1;
}

// Create and initialize coverage volume (stored in the main MRML node)
// return 0 if failed
int vtkProstateNavLogic::CreateCoverageVolume()
{
  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Error creating coverage volume, manager is invalid");
    return 0;
  }

  //vtkMRMLVolumeNode* baseVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(manager->GetCalibrationVolumeNodeID()));
  vtkMRMLVolumeNode* baseVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(manager->GetTargetingVolumeNodeID()));
  if (!baseVolumeNode) 
  {
    // volume node is already created
    return 0;
  }

  // Create volume node (as copy of calibration volume)
  vtkSmartPointer<vtkMRMLScalarVolumeNode> coverageVolumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();    
  int modifiedSinceRead = baseVolumeNode->GetModifiedSinceRead();
  coverageVolumeNode->CopyWithScene(baseVolumeNode);
  coverageVolumeNode->SetName(ROBOT_COVERAGE_AREA_NODE_NAME);

  // Create image data
  vtkSmartPointer<vtkImageData> coverageLabelMapImage=vtkSmartPointer<vtkImageData>::New();
  int dim=COVERAGE_MAP_SIZE_MM/COVERAGE_MAP_RESOLUTION_MM;
  coverageLabelMapImage->SetDimensions(dim, dim, dim);    
  coverageLabelMapImage->SetWholeExtent(0,dim-1,0,dim-1,0,dim-1);
  coverageLabelMapImage->SetScalarType(VTK_SHORT);
  coverageLabelMapImage->AllocateScalars();
  coverageVolumeNode->SetAndObserveImageData(coverageLabelMapImage);
  
  // Get the calibration volume centerpoint in RAS coordinates
  double rasCenterPoint[4]={0,0,0,1}; // centerpoint position in RAS coorindates
  {   
    int extent[6];
    baseVolumeNode->GetImageData()->GetWholeExtent(extent);

    double ijkCenterPoint[4]={0,0,0,1}; // centerpoint position in IJK coorindates
    ijkCenterPoint[0]=(extent[0]+extent[1])/2;
    ijkCenterPoint[1]=(extent[2]+extent[3])/2;
    ijkCenterPoint[2]=(extent[4]+extent[5])/2;

    vtkSmartPointer<vtkMatrix4x4> ijkToRas=vtkSmartPointer<vtkMatrix4x4>::New();
    baseVolumeNode->GetIJKToRASMatrix(ijkToRas);
    vtkMRMLTransformNode *transformNode = baseVolumeNode->GetParentTransformNode();
    if ( transformNode )
      {
      vtkSmartPointer<vtkMatrix4x4> rasToRAS = vtkSmartPointer<vtkMatrix4x4>::New();
      transformNode->GetMatrixTransformToWorld(rasToRAS);
      vtkMatrix4x4::Multiply4x4 (rasToRAS, ijkToRas, ijkToRas);
      }

    ijkToRas->MultiplyPoint(ijkCenterPoint, rasCenterPoint);
  }
   
  // Set coverage volume size and position
  coverageVolumeNode->SetOrigin(rasCenterPoint[0]-COVERAGE_MAP_SIZE_MM/2, rasCenterPoint[1]-COVERAGE_MAP_SIZE_MM/2, rasCenterPoint[2]-COVERAGE_MAP_SIZE_MM/2);
  coverageVolumeNode->SetSpacing(COVERAGE_MAP_RESOLUTION_MM,COVERAGE_MAP_RESOLUTION_MM,COVERAGE_MAP_RESOLUTION_MM);
  vtkSmartPointer<vtkMatrix4x4> ijkToRasDirectionMatrix=vtkSmartPointer<vtkMatrix4x4>::New();
  ijkToRasDirectionMatrix->Identity();
  coverageVolumeNode->SetIJKToRASDirectionMatrix(ijkToRasDirectionMatrix);
  
  coverageVolumeNode->SetAndObserveStorageNodeID(NULL);
  coverageVolumeNode->SetModifiedSinceRead(1);
  coverageVolumeNode->SetLabelMap(1);

  // Create display node
  vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode> coverageDisplayNode  = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
  coverageDisplayNode->SetAndObserveColorNodeID ("vtkMRMLColorTableNodeLabels"); // set the display node to have a label map lookup table
  this->GetMRMLScene()->AddNode(coverageDisplayNode);
  
  // link the display node with the volume node
  coverageVolumeNode->SetAndObserveDisplayNodeID( coverageDisplayNode->GetID() );  

  // add the label volume to the scene
  this->GetMRMLScene()->AddNode(coverageVolumeNode);

  manager->SetCoverageVolumeNodeID(coverageVolumeNode->GetID());   // Add ref to main MRML node

  // Restore modifiedSinceRead value since copy cause Modify on image data.
  baseVolumeNode->SetModifiedSinceRead(modifiedSinceRead);

  return UpdateCoverageVolumeImage();
}

// Update the coverage volume label map image
// return 0 if failed
int vtkProstateNavLogic::UpdateCoverageVolumeImage()
{
  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Error updating coverage volume, manager is invalid");
    return 0;
  }
  vtkMRMLVolumeNode* coverageVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(manager->GetCoverageVolumeNodeID()));
  if (coverageVolumeNode==NULL)
  {
    vtkWarningMacro("CoverageMapUpdate failed, the map is not initialized");
    return 0;
  }
  vtkImageData *coverageImage=coverageVolumeNode->GetImageData();
  if (coverageImage==NULL)
  {
    vtkWarningMacro("CoverageMapUpdate failed, the map is not initialized");
    return 0;
  }

//  std::string needleType = this->NeedleTypeMenuList->GetWidget()->GetValue();
  
  //double *origin=coverageImage->GetOrigin();
  //double *spacing=coverageImage->GetSpacing();

  double rasPoint[4]={0,0,0,1};
  double ijkPoint[4]={0,0,0,1};
  int extent[6];
  coverageImage->GetWholeExtent(extent);

  vtkSmartPointer<vtkMatrix4x4> ijkToRas=vtkSmartPointer<vtkMatrix4x4>::New();
  coverageVolumeNode->GetIJKToRASMatrix(ijkToRas);

  vtkSmartPointer<vtkProstateNavTargetDescriptor> targetDesc=vtkSmartPointer<vtkProstateNavTargetDescriptor>::New();
  targetDesc->SetNeedleType(manager->GetNeedleType(manager->GetCurrentNeedleIndex()), 
    manager->GetNeedleLength(manager->GetCurrentNeedleIndex()), 
    manager->GetNeedleOvershoot(manager->GetCurrentNeedleIndex()));
  double needleLength=manager->GetNeedleLength(manager->GetCurrentNeedleIndex());

  std::string FoR = this->GetFoRStrFromVolumeNodeID(manager->GetTargetingVolumeNodeID());
  targetDesc->SetTargetingFoRStr(FoR);
  

  float value=0;  
  for (int z=extent[4]; z<=extent[5]; z++)
  {
    ijkPoint[2]=z;
    for (int y=extent[2]; y<=extent[3]; y++)
    {
      ijkPoint[1]=y;
      for (int x=extent[0]; x<=extent[1]; x++)
      {         
        ijkPoint[0]=x;           
        ijkToRas->MultiplyPoint(ijkPoint, rasPoint);
        
        value=0;

        // it is not a boundary voxel
        // (we leave a black boundary around the image to ensure that
        // contouring of the coverage area results in a closed surface)
        if (z!=extent[4] && z!=extent[5] && 
          y!=extent[2] && y!=extent[3] &&
          x!=extent[0] && x!=extent[1])
        {
          targetDesc->SetRASLocation(rasPoint[0], rasPoint[1], rasPoint[2]);
          
          if(manager->FindTargetingParams(targetDesc))
          {
            if (!targetDesc->GetIsOutsideReach())
            {
              // inside reach
              if (targetDesc->GetDepthCM()<=needleLength)
              {
                // needle is long enough
                value=1;
              }
            }
          }
        }

        coverageImage->SetScalarComponentFromFloat(x, y, z, 0, value);
      }
    }
  }

  coverageImage->Update();
  coverageVolumeNode->Modified();
  
  return 1;
}

// Remove coverage volume and display node from the scene and from the main MRML node
void vtkProstateNavLogic::DeleteCoverageVolume() 
{
  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Error deleting coverage volume, manager is invalid");
    return;
  }
  vtkMRMLVolumeNode* coverageVolumeNode = vtkMRMLVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(manager->GetCoverageVolumeNodeID()));
  if (coverageVolumeNode==NULL)
  {
    vtkWarningMacro("Error deleting coverage volume, coverage volume node is not found");
    return;
  }
  
  // delete volume node
  manager->SetCoverageVolumeNodeID(NULL);

  // delete image data
  coverageVolumeNode->SetAndObserveImageData(NULL);

  // remove node from scene
  vtkMRMLScene *scene=this->GetMRMLScene();
  if (scene!=NULL)
  { 
    scene->RemoveNode(coverageVolumeNode);
  }
  
}

void vtkProstateNavLogic::UpdateTargetListFromMRML()
{
  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Error updating targetlist from mrml, manager is invalid");
    return;
  }
  vtkMRMLFiducialListNode* fidNode=manager->GetTargetPlanListNode();
  if (fidNode==NULL)
  {
    vtkErrorMacro("Error updating targetlist from mrml, fiducial node is invalid");
    return;
  }

  // True if we modified fiducials at all (typically the label has to be changed)
  bool fidNodeModified=false;
  // If we modified fiducials, then do it in one step, with Start/EndModify. For that we need to remember the previous state.
  int fidNodeModifyOld=0;

  for (int i=0; i<manager->GetTotalNumberOfTargets(); i++)
  {
    vtkProstateNavTargetDescriptor *t=manager->GetTargetDescriptorAtIndex(i);
    if (fidNode->GetFiducialIndex(t->GetFiducialID())<0)
    {
      // fiducial not found, need to delete it
      manager->RemoveTargetDescriptorAtIndex(i);
      i--; // repeat check on the i-th element
    }
  }

  for (int i=0; i<fidNode->GetNumberOfFiducials(); i++)
  {
    int targetIndex=GetTargetIndexFromFiducialID(fidNode->GetNthFiducialID(i));
    if (targetIndex<0)
    {

      if (!fidNodeModified)
      {
        fidNodeModified=true;
        fidNodeModifyOld=fidNode->StartModify();
      }

      // New fiducial, create associated target
      vtkSmartPointer<vtkProstateNavTargetDescriptor> targetDesc=vtkSmartPointer<vtkProstateNavTargetDescriptor>::New();      
      
      targetDesc->SetFiducialID(fidNode->GetNthFiducialID(i));

      int needleIndex=manager->GetCurrentNeedleIndex();
      vtkMRMLProstateNavManagerNode::NeedleDescriptorStruct needleDesc;
      if (!manager->GetNeedle(needleIndex, needleDesc))
      {
        vtkErrorMacro("Failed to get info for needle "<<needleIndex);
      }
      targetDesc->SetNeedleType(needleDesc.Description, needleDesc.NeedleLength, needleDesc.NeedleOvershoot);

      needleDesc.LastTargetId++;
      if (!manager->SetNeedle(needleIndex, needleDesc))
      {
        vtkErrorMacro("Failed to set info for needle "<<needleIndex);
      }

      std::ostrstream strvalue;
      strvalue << needleDesc.NeedleName << needleDesc.LastTargetId << std::ends;        
      fidNode->SetNthFiducialLabelText(i,strvalue.str());
      strvalue.rdbuf()->freeze(0);     

      std::string FoR = this->GetFoRStrFromVolumeNodeID(manager->GetTargetingVolumeNodeID());
      targetDesc->SetTargetingFoRStr(FoR);

      manager->AddTargetDescriptor(targetDesc);
    }

    targetIndex=GetTargetIndexFromFiducialID(fidNode->GetNthFiducialID(i));
    if (targetIndex>=0)
    {
      // Update fiducial
      vtkProstateNavTargetDescriptor* targetDesc = manager->GetTargetDescriptorAtIndex(targetIndex);
      if (targetDesc!=NULL)
      {
        float *rasLocation=fidNode->GetNthFiducialXYZ(i);
        targetDesc->SetRASLocation(rasLocation[0], rasLocation[1], rasLocation[2]);

        float *rasOrientation=fidNode->GetNthFiducialOrientation(i);
        targetDesc->SetRASOrientation(rasOrientation[0], rasOrientation[1], rasOrientation[2], rasOrientation[3]);

        targetDesc->SetName(fidNode->GetNthFiducialLabelText(i));

        // :TODO: update needle,  etc. parameters ?

        // calculate targeting parameters for active needle, store in a target descriptor
        if(!manager->FindTargetingParams(targetDesc))
          {
          vtkErrorMacro("Error finding targeting parameters");
          }        
      }
      else
      {
        vtkErrorMacro("Invalid target descriptor");
      }
    }
    else
    {
      vtkErrorMacro("Invalid Fiducial ID");
    }
  }

  if (fidNodeModified)
  {
    fidNode->EndModify(fidNodeModifyOld);
    // StartModify/EndModify discarded vtkMRMLFiducialListNode::FiducialModifiedEvent-s, so we have to resubmit them now
    fidNode->InvokeEvent(vtkMRMLFiducialListNode::FiducialModifiedEvent, NULL);
  }
}

//----------------------------------------------------------------------------
int vtkProstateNavLogic::GetTargetIndexFromFiducialID(const char* fiducialID)
{
  if (fiducialID==NULL)
  {
    vtkWarningMacro("Fiducial ID is invalid");
    return -1;
  }
  vtkMRMLProstateNavManagerNode* manager=this->GUI->GetProstateNavManager();
  if (manager==NULL)
  {
    vtkErrorMacro("Manager is invalid");
    return -1;
  }
  for (int i=0; i<manager->GetTotalNumberOfTargets(); i++)
  {
    vtkProstateNavTargetDescriptor *t=manager->GetTargetDescriptorAtIndex(i);
    if (t->GetFiducialID().compare(fiducialID)==0)
    {
      // found the target corresponding to the fiducialID
      return i;
    }
  }
  return -1;
}

//----------------------------------------------------------------------------
int vtkProstateNavLogic::SetMouseInteractionMode(int mode)
{  
  if (GetApplicationLogic()==NULL)
  {
   vtkErrorMacro("Application logic is invalid");
    return 0;
  }
  if (GetApplicationLogic()->GetMRMLScene()==NULL)
  {
    vtkErrorMacro("Scene is invalid");
    return 0;
  }
  vtkMRMLInteractionNode *interactionNode = vtkMRMLInteractionNode::SafeDownCast(GetApplicationLogic()->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
  if (interactionNode==NULL)
  {
    vtkErrorMacro("Interaction node is invalid");
    return 0;
  }
  
  if (this->GetGUI()==NULL)
  {
    vtkErrorMacro("GUI is invalid");
    return 0;
  }  
  vtkSlicerApplication* app=vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  if (app==NULL)
  {
    vtkErrorMacro("Application is invalid");
    return 0;
  }
  vtkSlicerApplicationGUI* appGUI = app->GetApplicationGUI();
  if (appGUI==NULL)
  {
    vtkErrorMacro("Application GUI is invalid");
    return 0;
  }
  vtkSlicerToolbarGUI *tGUI = appGUI->GetApplicationToolbar();
  if (tGUI==NULL)
  {
    vtkErrorMacro("Application toolbar GUI is invalid");
    return 0;
  }

  // Set logic state
  interactionNode->SetCurrentInteractionMode(mode); 

  // Set pick/place state to persistent (stay in the staet after picking/placing a fiducial)
  if (mode==vtkMRMLInteractionNode::Place)
  {
    interactionNode->SetPlaceModePersistence(1);
  }
  else if (mode==vtkMRMLInteractionNode::PickManipulate)
  {
    interactionNode->SetPickModePersistence(1);
  }
  
  return 1;
}

//----------------------------------------------------------------------------
int vtkProstateNavLogic::SetCurrentFiducialList(vtkMRMLFiducialListNode* fidNode)
{
  if (fidNode==NULL)
  {
    vtkErrorMacro("Fiducial node is invalid");
    return 0;
  }

  if (this->GetGUI()==NULL)
  {
    vtkErrorMacro("GUI is invalid");
    return 0;
  }  
  vtkSlicerApplication* app=vtkSlicerApplication::SafeDownCast(this->GetGUI()->GetApplication());
  if (app==NULL)
  {
    vtkErrorMacro("Application is invalid");
    return 0;
  }

  vtkSlicerFiducialsGUI* fidGUI = vtkSlicerFiducialsGUI::SafeDownCast ( app->GetModuleGUIByName ("Fiducials"));
  if (fidGUI==NULL)
  {
    vtkErrorMacro("Fiducial GUI is invalid");
    return 0;
  }
  
  // Activate target fiducials in the Fiducial GUI
  fidGUI->Enter();
  fidGUI->SetFiducialListNodeID(fidNode->GetID());
  
  return 1;
}
