/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransRectalProstateRobotNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLTransRectalProstateRobotNode.h"

#include "vtkObjectFactory.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScene.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "vtkMRMLModelNode.h"
#include "vtkCylinderSource.h"
#include "vtkTubeFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkAppendPolyData.h"
#include "vtkTriangleFilter.h"
#include "vtkSmartPointer.h"
#include "vtkMath.h"
#include "vtkContourFilter.h"
#include "vtkPointData.h"
#include "vtkAppendPolyData.h"
#include "vtkSphereSource.h"
#include "vtkGlyph3D.h"
#include "vtkPoints.h"
#include "vtkPointSource.h"

#include "vtkProstateNavTargetDescriptor.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTransRectalProstateRobotNode);

//----------------------------------------------------------------------------
vtkMRMLTransRectalProstateRobotNode::vtkMRMLTransRectalProstateRobotNode()
{
  this->CalibrationAlgo=vtkSmartPointer<vtkTransRectalFiducialCalibrationAlgo>::New();
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    this->CalibrationMarkerValid[i]=false;
    this->ModelMarkers[i]=vtkPolyData::New();
  } 
  this->ModelAxes=vtkPolyData::New();
  this->ModelProbe=vtkPolyData::New();
  this->ModelNeedle=vtkPolyData::New();

  this->ModelAxesVisible=false;
  
  ResetCalibrationData();
  this->RobotModelNodeID=NULL;
  this->RobotModelNode=NULL;
}

//----------------------------------------------------------------------------
vtkMRMLTransRectalProstateRobotNode::~vtkMRMLTransRectalProstateRobotNode()
{
  SetAndObserveRobotModelNodeID(NULL);

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    if (this->ModelMarkers[i]!=NULL)
    {
      this->ModelMarkers[i]->Delete();
      this->ModelMarkers[i]=NULL;
    }
  }
  if (this->ModelAxes!=NULL)
  {
    this->ModelAxes->Delete();
    this->ModelAxes=NULL;
  }
  if (this->ModelProbe!=NULL)
  {
    this->ModelProbe->Delete();
    this->ModelProbe=NULL;
  }
  if (this->ModelNeedle!=NULL)
  {
    this->ModelNeedle->Delete();
    this->ModelNeedle=NULL;
  }
}


//----------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

}

//----------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLTransRectalProstateRobotNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
  vtkMRMLTransRectalProstateRobotNode *node = (vtkMRMLTransRectalProstateRobotNode *) anode;

  this->SetRobotModelNodeID(node->RobotModelNodeID);

  //int type = node->GetType();
  
}

//----------------------------------------------------------------------------
int vtkMRMLTransRectalProstateRobotNode::Init(vtkSlicerApplication* app)
{ 
  this->Superclass::Init(app);

  // Robot model
  // This part should be moved to Robot Display Node.
  if (GetRobotModelNode()==NULL)
  {
    const char* nodeID = AddRobotModel("TransrectalProstateRobot");
    vtkMRMLModelNode*  modelNode = vtkMRMLModelNode::SafeDownCast(this->Scene->GetNodeByID(nodeID));
    if (modelNode)
      {
      vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
      displayNode->SetVisibility(0);
      modelNode->Modified();
      this->Scene->Modified();
      //modelNode->SetAndObserveTransformNodeID(GetZFrameTransformNodeID());
      SetAndObserveRobotModelNodeID(nodeID);
      }
  }

  return 1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTransRectalProstateRobotNode::AddRobotModel(const char* nodeName)
{
  
  vtkSmartPointer<vtkMRMLModelNode> robotModel = vtkSmartPointer<vtkMRMLModelNode>::New();
  vtkSmartPointer<vtkMRMLModelDisplayNode> robotDisp  = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();

  this->Scene->AddNode(robotDisp);
  this->Scene->AddNode(robotModel);

  robotDisp->SetScene(this->Scene);
  robotModel->SetName(nodeName);
  robotModel->SetScene(this->Scene);
  robotModel->SetAndObserveDisplayNodeID(robotDisp->GetID());
  robotModel->SetHideFromEditors(0);

  double color[3];
  color[0] = 0.5;
  color[1] = 0.5;
  color[2] = 1.0;
  robotDisp->SetColor(color);
  robotDisp->SetOpacity(0.5);

  return robotModel->GetID(); // model is added to the scene, so the GetID string remains valid

}

//----------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::SetAndObserveRobotModelNodeID(const char *nodeId)
{
  vtkSetAndObserveMRMLObjectMacro(this->RobotModelNode, NULL);
  this->SetRobotModelNodeID(nodeId);
  vtkMRMLModelNode *tnode = this->GetRobotModelNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->RobotModelNode, tnode, events);
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLTransRectalProstateRobotNode::GetRobotModelNode()
{
  if (this->GetScene() && this->RobotModelNodeID != NULL )
    {    
    return vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetNodeByID(this->RobotModelNodeID));
    }
  return NULL;
}


void vtkMRMLTransRectalProstateRobotNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
}


//----------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
bool vtkMRMLTransRectalProstateRobotNode::FindTargetingParams(vtkProstateNavTargetDescriptor *targetDesc)
{
  // this is used for coverage area computation (IsOutsideReach means that the target is outside the robot's coverage area)
  // :TODO: update this for arbitrary target and calib volume transform
  return this->CalibrationAlgo->FindTargetingParams(targetDesc);
}


std::string vtkMRMLTransRectalProstateRobotNode::GetTargetInfoText(vtkProstateNavTargetDescriptor *targetDesc)
{
  bool validTargeting=FindTargetingParams(targetDesc);

  std::ostrstream os;    
  os << targetDesc->GetName()<<std::endl;
  if (validTargeting)
  {
    if (targetDesc->GetCalibrationFoRStr().compare(targetDesc->GetTargetingFoRStr())!=0)
    {
      os << "Warning: frame of reference id mismatch"<<std::endl;
    }
    if (targetDesc->GetIsOutsideReach())
    {
      os << "Warning: the target is not reachable"<<std::endl;
    }    
    os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(1);
    os << "Depth: "<<targetDesc->GetDepthCM()<<" cm"<<std::endl;
    os << "Device rotation: "<<targetDesc->GetAxisRotation()<<" deg"<<std::endl;
    os << "Needle angle: "<<targetDesc->GetNeedleAngle()<<" deg"<<std::endl;
  }
  os << "Needle type: "<<targetDesc->GetNeedleTypeString()<<std::endl;
  os << std::setiosflags(ios::fixed | ios::showpoint) << std::setprecision(1);
  os << targetDesc->GetRASLocationString().c_str()<<std::endl;
  os << std::ends;
  std::string result=os.str();
  os.rdbuf()->freeze();
  return result;
}

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::SetCalibrationMarker(unsigned int markerNr, double markerRAS[3])
{
  if (markerNr<0 || markerNr>=CALIB_MARKER_COUNT)
    {
    vtkErrorMacro("SetCalibrationMarker: Invalid calibration marker index "<<markerNr);
    return;
    }

  this->CalibrationMarkerPositions[markerNr][0]=markerRAS[0];
  this->CalibrationMarkerPositions[markerNr][1]=markerRAS[1];
  this->CalibrationMarkerPositions[markerNr][2]=markerRAS[2];
  this->CalibrationMarkerValid[markerNr]=true;
}
//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::GetCalibrationMarker(unsigned int markerNr, double &r, double &a, double &s, bool &valid)
{
  if (markerNr<0 || markerNr>=4)
    {
    vtkErrorMacro("GetCalibrationMarker: Invalid calibration marker index "<<markerNr);
    return;
    }  
  r=this->CalibrationMarkerPositions[markerNr][0];
  a=this->CalibrationMarkerPositions[markerNr][1];
  s=this->CalibrationMarkerPositions[markerNr][2];
  valid=this->CalibrationMarkerValid[markerNr];
}
//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::RemoveAllCalibrationMarkers()
{
  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    this->CalibrationMarkerValid[i]=false;
  }
}

//------------------------------------------------------------------------------
bool vtkMRMLTransRectalProstateRobotNode::SegmentRegisterMarkers(vtkMRMLScalarVolumeNode *calibVol, double thresh[4], double fidDimsMm[3], double radiusMm, bool bUseRadius, double initialAngle, std::string &resultDetails, bool enableAutomaticCenterpointAdjustment)
{
  TRProstateBiopsyCalibrationFromImageInput in;

  for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
  {
    bool valid=false;
    GetCalibrationMarker(i, in.MarkerInitialPositions[i][0], in.MarkerInitialPositions[i][1], in.MarkerInitialPositions[i][2], valid); 
    if (!valid)
    {
      vtkErrorMacro("SegmentRegisterMarkers marker "<<i+1<<" is undefined");
      resultDetails="Not all calibration markers are defined";
      return false;
    }
    in.MarkerSegmentationThreshold[i]=thresh[i];
  }
  for (int i=0; i<3; i++)
  {
    in.MarkerDimensionsMm[i]=fidDimsMm[i];
  }
  in.MarkerRadiusMm=radiusMm;
  in.RobotInitialAngle=initialAngle;
  
  if (calibVol==0)
  {
    vtkErrorMacro("SegmentRegisterMarkers CalibrationVolume is invalid");
    resultDetails="Calibration volume is invalid";
    return false;
  }
  vtkSmartPointer<vtkMatrix4x4> ijkToRAS = vtkSmartPointer<vtkMatrix4x4>::New(); 
  calibVol->GetIJKToRASMatrix(ijkToRAS);

  in.VolumeIJKToRASMatrix=ijkToRAS;
  in.VolumeImageData=calibVol->GetImageData();

  // get frame of reference uid
  const itk::MetaDataDictionary &volDictionary = calibVol->GetMetaDataDictionary();
  std::string tagValue; 
  tagValue.clear();
  itk::ExposeMetaData<std::string>( volDictionary, "0020|0052", tagValue );   
  in.FoR=tagValue;

  this->CalibrationAlgo->SetEnableMarkerCenterpointAdjustment(enableAutomaticCenterpointAdjustment);

  TRProstateBiopsyCalibrationFromImageOutput res;
  bool success=true;
  if (!this->CalibrationAlgo->CalibrateFromImage(in, res))
  {
    // calibration failed

    ResetCalibrationData();

    std::ostrstream os;
    // if a marker not found, then make the error report more precise
    for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
    {      
      if (!res.MarkerFound[i])
      {
        os << "Marker "<<i+1<<" cannot be detected. ";
        success=false;
      }
    } 
    os << "Calibration failed." << std::ends;
    resultDetails=os.str();
    os.rdbuf()->freeze();  
    this->Modified();
  }
  else
  {    
    // calibration is successful

    /*commented out to keep the original marker guesses
    // update manually set marker positions with the result of the marker detection
    for (unsigned int i=0; i<CALIB_MARKER_COUNT; i++)
    {      
      SetCalibrationMarker(i, res.MarkerPositions[i]);
      in.MarkerSegmentationThreshold[i]=thresh[i];
    }
    */

    const TRProstateBiopsyCalibrationData calibData=this->CalibrationAlgo->GetCalibrationData();
    SetCalibrationData(calibData);    

    resultDetails="Calibration is successfully completed.";    
    success=true;  
    this->Modified();
  }  

  UpdateModelAxes();
  UpdateModelProbe();
  UpdateModelMarkers();
  UpdateModelNeedle(NULL);
  UpdateModel();

  return success;
}

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::ResetCalibrationData()
{
  this->CalibrationData.AxesDistance = -1;
  this->CalibrationData.AxesAngleDegrees = 0;
  this->CalibrationData.RobotRegistrationAngleDegrees = 0;
  for (int i=0; i<3; i++)
  {
    this->CalibrationData.I1[i]=0.0;
    this->CalibrationData.I2[i]=0.0;
    this->CalibrationData.v1[i]=0.0;
    this->CalibrationData.v2[i]=0.0;
  }
  this->CalibrationData.CalibrationValid=false;

  Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::UpdateModel()
{
  vtkMRMLModelNode* modelNode=GetRobotModelNode();
  if (modelNode==NULL)
  {
    return;
  }
  vtkMRMLModelDisplayNode* displayNode = vtkMRMLModelDisplayNode::SafeDownCast(modelNode->GetDisplayNode());
  if (displayNode==NULL)
  {
    return;
  }      

  // Merge all into a single polydata
  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  bool hasPoints=false;

  // Probe
  if (this->ModelProbe->GetNumberOfPoints()>0)
  {
    appender->AddInput(this->ModelProbe);
    hasPoints=true;
  }
  for (unsigned int markerId=0; markerId<2 && markerId<CALIB_MARKER_COUNT; markerId++)
  {
    if (this->ModelMarkers[markerId]->GetNumberOfPoints()>0)
    {
      appender->AddInput(this->ModelMarkers[markerId]);
      hasPoints=true;
    }
  }

  // Calibration axes (and calibration needle)
  if (this->ModelAxesVisible && this->ModelAxes->GetNumberOfPoints()>0)
  {
    if (this->ModelAxes->GetNumberOfPoints()>0)
    {
      appender->AddInput(this->ModelAxes);
      hasPoints=true;
    }
    for (unsigned int markerId=2; markerId<CALIB_MARKER_COUNT; markerId++)
    {
      if (this->ModelMarkers[markerId]->GetNumberOfPoints()>0)
      {
        appender->AddInput(this->ModelMarkers[markerId]);
        hasPoints=true;
      }
    }
  }

  // Targeting needle
  if (this->ModelNeedle->GetNumberOfPoints()>0)
  {
    appender->AddInput(this->ModelNeedle);
    hasPoints=true;
  }

  // If there were no points in the polydata at all, then we would get a VTK warning.
  // Add a dummy point to avoid warning in this case.
  if (!hasPoints)
  {
    vtkSmartPointer<vtkPointSource> point=vtkSmartPointer<vtkPointSource>::New();
    point->SetNumberOfPoints(1);
    appender->AddInputConnection(point->GetOutputPort());
  }

  vtkSmartPointer<vtkTriangleFilter> cleaner=vtkSmartPointer<vtkTriangleFilter>::New();
  cleaner->SetInputConnection(appender->GetOutputPort());
  cleaner->Update();
  
  modelNode->SetAndObservePolyData(cleaner->GetOutput());
  
  displayNode->SetPolyData(modelNode->GetPolyData());

  if (displayNode->GetPolyData() != NULL) 
  {
    displayNode->GetPolyData()->Modified();
  }
  displayNode->SetModifiedSinceRead(1); 
  
  displayNode->SetVisibility(1);

}


//------------------------------------------------------------------------------
bool vtkMRMLTransRectalProstateRobotNode::ShowRobotAtTarget(vtkProstateNavTargetDescriptor *targetDesc)
{
  UpdateModelNeedle(targetDesc);
  UpdateModel();
  return true;
}

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::UpdateModelNeedle(vtkProstateNavTargetDescriptor *targetDesc)
{
  if (targetDesc==NULL)
  {
    this->ModelNeedle->Reset();
    return;
  }

  // get RAS points of start and end point of needle
  // for the 3D viewer, the RAS coodinates are the world coordinates!!
  // this makes things simpler
  // render the needle as a thin pipe

  // start point is the target RAS
  double targetRAS[3];
  targetDesc->GetRASLocation(targetRAS); 

  double targetHingeRAS[3];
  targetDesc->GetHingePosition(targetHingeRAS);

  double needleVector[3];
  needleVector[0] = targetRAS[0] - targetHingeRAS[0];
  needleVector[1] = targetRAS[1] - targetHingeRAS[1];
  needleVector[2] = targetRAS[2] - targetHingeRAS[2];
  vtkMath::Normalize(needleVector);

  double overshoot = targetDesc->GetNeedleOvershoot();
  double needleLength = targetDesc->GetNeedleLength();

  double needleEndRAS[3];
  needleEndRAS[0] = targetRAS[0] + overshoot*needleVector[0];
  needleEndRAS[1] = targetRAS[1] + overshoot*needleVector[1];
  needleEndRAS[2] = targetRAS[2] + overshoot*needleVector[2];

  double needleStartRAS[3];
  needleStartRAS[0] = needleEndRAS[0] - needleLength*needleVector[0];
  needleStartRAS[1] = needleEndRAS[1] - needleLength*needleVector[1];
  needleStartRAS[2] = needleEndRAS[2] - needleLength*needleVector[2];

  // merge all polydata by this appender
  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  vtkSmartPointer<vtkLineSource> NeedleTrajectoryLine=vtkSmartPointer<vtkLineSource>::New();
  NeedleTrajectoryLine->SetResolution(100); 
  NeedleTrajectoryLine->SetPoint1(needleEndRAS);
  NeedleTrajectoryLine->SetPoint2(needleStartRAS);

  vtkSmartPointer<vtkTubeFilter> NeedleTrajectoryTube=vtkSmartPointer<vtkTubeFilter>::New();
  NeedleTrajectoryTube->SetInputConnection(NeedleTrajectoryLine->GetOutputPort());
  NeedleTrajectoryTube->SetRadius(1.0);
  NeedleTrajectoryTube->SetNumberOfSides(16);
  NeedleTrajectoryTube->CappingOn();

  appender->AddInputConnection(NeedleTrajectoryTube->GetOutputPort());  

  // a thinner tube representing the needle overshoot (when the needle is triggered it extends to -overshoot distance from the needle tip)
  if (overshoot<0)
  {
    double needleEndRAS[3];
    needleEndRAS[0] = targetRAS[0];
    needleEndRAS[1] = targetRAS[1];
    needleEndRAS[2] = targetRAS[2];

    double needleStartRAS[3];
    needleStartRAS[0] = targetRAS[0] + overshoot*needleVector[0];
    needleStartRAS[1] = targetRAS[1] + overshoot*needleVector[1];
    needleStartRAS[2] = targetRAS[2] + overshoot*needleVector[2];

    vtkSmartPointer<vtkLineSource> needleOvershootLine=vtkSmartPointer<vtkLineSource>::New();
    needleOvershootLine->SetResolution(100); 
    needleOvershootLine->SetPoint1(needleEndRAS);
    needleOvershootLine->SetPoint2(needleStartRAS);

    vtkSmartPointer<vtkTubeFilter> needleOvershootTube=vtkSmartPointer<vtkTubeFilter>::New();
    needleOvershootTube->SetInputConnection(needleOvershootLine->GetOutputPort());
    needleOvershootTube->SetRadius(0.2);
    needleOvershootTube->SetNumberOfSides(8);
    needleOvershootTube->CappingOn();

    appender->AddInputConnection(needleOvershootTube->GetOutputPort());
  }

  // Save result
  appender->Update();
  this->ModelNeedle->DeepCopy(appender->GetOutput());

}

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::UpdateModelProbe()
{
  this->ModelProbe->Reset();

  if (!this->CalibrationData.CalibrationValid)
  {
    // if there is no calibration, we cannot show where is the probe
    return;
  }
  // update robot base position
  vtkSmartPointer<vtkMatrix4x4> baseTransform=vtkSmartPointer<vtkMatrix4x4>::New();
  if (!GetRobotBaseTransform(baseTransform))
  {
    // no calibration
    return;
  }

  // merge all polydata by this appender
  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  double point1Probe[4]={-100,0,0 ,1}; // probe tip point
  double point2Probe[4]={400,0,0 ,1}; // probe base point
  
  double point1Ras[4]={0,0,0 ,1};
  double point2Ras[4]={0,0,0 ,1};

  baseTransform->MultiplyPoint(point1Probe, point1Ras);
  baseTransform->MultiplyPoint(point2Probe, point2Ras);

  vtkSmartPointer<vtkLineSource> probeLine=vtkSmartPointer<vtkLineSource>::New();
  probeLine->SetResolution(100);
  probeLine->SetPoint1(point1Ras);
  probeLine->SetPoint2(point2Ras);

  vtkSmartPointer<vtkTubeFilter> probeTube=vtkSmartPointer<vtkTubeFilter>::New();
  probeTube->SetInputConnection(probeLine->GetOutputPort());
  //probeTube->SetRadius(14.5);
  probeTube->SetRadius(13.0); // TODO: read this from a model descriptor
  probeTube->SetNumberOfSides(20);
  probeTube->CappingOn();
  
  appender->AddInputConnection(probeTube->GetOutputPort());  

  /*
  vtkSmartPointer<vtkTubeFilter> probeCenterlineTube=vtkSmartPointer<vtkTubeFilter>::New();
  probeCenterlineTube->SetInputConnection(probeLine->GetOutputPort());
  probeCenterlineTube->SetRadius(0.5);
  probeCenterlineTube->SetNumberOfSides(8);
  probeCenterlineTube->CappingOn();

  appender->AddInputConnection(probeCenterlineTube->GetOutputPort());
  */

  appender->Update();
  this->ModelProbe->DeepCopy(appender->GetOutput());
}


//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::UpdateModelAxes()
{
  this->ModelAxes->Reset();

  if (!this->CalibrationData.CalibrationValid)
  {
    return;
  }

  // merge all polydata by this appender
  vtkSmartPointer<vtkAppendPolyData> appender = vtkSmartPointer<vtkAppendPolyData>::New();

  // form the axis 1 line
  // set up the line actors
  vtkSmartPointer<vtkLineSource> axis1Line = vtkSmartPointer<vtkLineSource>::New();  
  axis1Line->SetResolution(100); 
  
  double needle1[3];
  double needle2[3];
  for (int i=0; i<3; i++)
  {
    needle1[i]=this->CalibrationData.I2[i]-100*this->CalibrationData.v2[i];
    needle2[i]=this->CalibrationData.I2[i]+200*this->CalibrationData.v2[i];
  }

  axis1Line->SetPoint1(needle1);
  axis1Line->SetPoint2(needle2);

  appender->AddInputConnection(axis1Line->GetOutputPort());

  // 2nd axis line

  // form the axis 2 line
  // set up the line actors
  vtkSmartPointer<vtkLineSource> axis2Line = vtkSmartPointer<vtkLineSource>::New();  
  axis2Line->SetResolution(100); 

  double robotaxis1[3];
  double robotaxis2[3];
  for (int i=0; i<3; i++)
  {
    robotaxis1[i]=this->CalibrationData.I1[i]-100*this->CalibrationData.v1[i];
    robotaxis2[i]=this->CalibrationData.I1[i]+200*this->CalibrationData.v1[i];
  }

  axis2Line->SetPoint1(robotaxis1);
  axis2Line->SetPoint2(robotaxis2);

  appender->AddInputConnection(axis2Line->GetOutputPort());

  /*
  // Detected marker centerpoints

  vtkPoints *points = vtkPoints::New();
   
  this->CalibrationAlgo->GetAxisCenterpoints(points, 0);
  this->CalibrationAlgo->GetAxisCenterpoints(points, 1);

  vtkPolyData *pointspoly = vtkPolyData::New();
  pointspoly->SetPoints(points);
  
  vtkSphereSource *glyph = vtkSphereSource::New();
  vtkGlyph3D *glypher = vtkGlyph3D::New();
  glypher->SetInput(pointspoly);
  glypher->SetSourceConnection(glyph->GetOutputPort());
  glypher->SetScaleFactor(0.25);

  appender->AddInputConnection(glypher->GetOutputPort());
  */

  appender->Update();
  this->ModelAxes->DeepCopy(appender->GetOutput());
}

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::UpdateModelMarkers()
{
  // Extract marker surfaces for display
  for (unsigned int markerId=0; markerId<CALIB_MARKER_COUNT; markerId++)
  {      
    vtkImageData *imagedata=this->CalibrationAlgo->GetCalibMarkerPreProcOutput(markerId);
    vtkSmartPointer<vtkContourFilter> objectSurfaceExtractor=vtkSmartPointer<vtkContourFilter>::New();
    objectSurfaceExtractor->SetInput(imagedata);

    double* imgIntensityRange = imagedata->GetPointData()->GetScalars()->GetRange();
    double imgIntensityUnit = (imgIntensityRange[1]-imgIntensityRange[0])* 0.01; // 1 unit is 1 percent of the intensity range    
    objectSurfaceExtractor->SetValue(0, imgIntensityUnit);

    vtkTransformPolyDataFilter *polyTrans = vtkTransformPolyDataFilter::New();
    polyTrans->SetInputConnection(objectSurfaceExtractor->GetOutputPort());
    vtkSmartPointer<vtkTransform> ijkToRASTransform=vtkSmartPointer<vtkTransform>::New();

    vtkMatrix4x4* ijkToRAS=this->CalibrationAlgo->GetCalibMarkerPreProcOutputIJKToRAS();

    ijkToRASTransform->SetMatrix(ijkToRAS);
    polyTrans->SetTransform(ijkToRASTransform);
    polyTrans->Update();

    this->ModelMarkers[markerId]->DeepCopy(polyTrans->GetOutput());
  }
}


//------------------------------------------------------------------------------
bool vtkMRMLTransRectalProstateRobotNode::GetRobotBaseTransform(vtkMatrix4x4* transform)
{
  if (!this->CalibrationData.CalibrationValid)
  {
    // no claibration robot position is unknown
    return false;
  }

  // TODO: fix this, this is just a dummy implementation, it does not take into account current encoder position

  double v1norm[3]={this->CalibrationData.v1[0], this->CalibrationData.v1[1], this->CalibrationData.v1[2]};         
  vtkMath::Normalize(v1norm);
  double v2norm[3]={this->CalibrationData.v2[0], this->CalibrationData.v2[1], this->CalibrationData.v2[2]};         
  vtkMath::Normalize(v2norm);

  double x[3]={v1norm[0],v1norm[1],v1norm[2]};
  double y[3]={0,0,0};
  vtkMath::Cross(x, v2norm, y);
  double z[3]={0,0,0};
  vtkMath::Cross(x, y, z);

  // orientation
  transform->SetElement(0,0, x[0]);
  transform->SetElement(1,0, x[1]);
  transform->SetElement(2,0, x[2]);

  transform->SetElement(0,1, y[0]);
  transform->SetElement(1,1, y[1]);
  transform->SetElement(2,1, y[2]);

  transform->SetElement(0,2, z[0]);
  transform->SetElement(1,2, z[1]);
  transform->SetElement(2,2, z[2]);

  // position
  transform->SetElement(0,3, this->CalibrationData.I1[0]);
  transform->SetElement(1,3, this->CalibrationData.I1[1]);
  transform->SetElement(2,3, this->CalibrationData.I1[2]);
  
  return true;
} 

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::SetCalibrationData(const TRProstateBiopsyCalibrationData& calibData) 
{ 
  this->CalibrationData=calibData; 
  UpdateModelAxes();
  UpdateModelProbe();
  UpdateModelMarkers();
  UpdateModelNeedle(NULL);
  UpdateModel();
}

//------------------------------------------------------------------------------
void vtkMRMLTransRectalProstateRobotNode::SetModelAxesVisible(bool visible)
{ 
  if (this->ModelAxesVisible==visible)
  {
    // no change
    return;
  }
  this->ModelAxesVisible=visible;
  UpdateModel(); // :TODO: use standard vtkSetMacro and update automatically when the update event is triggered
}
