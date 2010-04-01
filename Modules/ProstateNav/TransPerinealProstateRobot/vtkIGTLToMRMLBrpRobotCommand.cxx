/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include <vtksys/SystemTools.hxx>

#include "vtkObjectFactory.h"
#include "vtkIGTLToMRMLLinearTransform.h"

#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLBrpRobotCommandNode.h"
#include "igtlHeaderMessage.h"
#include "vtkIGTLToMRMLBrpRobotCommand.h"

vtkStandardNewMacro(vtkIGTLToMRMLBrpRobotCommand);
vtkCxxRevisionMacro(vtkIGTLToMRMLBrpRobotCommand, "$Revision: 1.0 $");

//---------------------------------------------------------------------------
// Following commands are supported by this class
//
//   1. START_UP Internally calls the INITIALIZE command.
//   2. SET_Z_FRAME Stores the Z-Frame.
//   3. TARGETING Just checks the Z-Frame values.
//   4. MOVE_TO Repeat this command for each target.
//   5. MANUAL Changes to Manual insertion. Switch back to automatic by
//      calling TARGETING.
//   6. INSERT Automatic insertion. Not yet implemented on the hardware.
//   7. BIOPSY Takes the biopsy automatically, see INSERT.
//   8. HOME Ex. after manual insertion to change the biopsy gun.
//   9. EMERGENCY If something is wrong. To get out of this state START_UP
//      needs to be called again.
//  10. GET_COORDINATE Call this as often as needed to get the latest
//      position of the robot.
//  11. GET_STATUS Will respond with OK or other status - again, called often. 
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkIGTLToMRMLBrpRobotCommand::vtkIGTLToMRMLBrpRobotCommand()
{
  this->IGTLNames.clear();

  // Slicer -> Robot
  /*
  this->IGTLNames.push_back(std::string("START_UP"));
  this->IGTLNames.push_back(std::string("PLANNING"));
  this->IGTLNames.push_back(std::string("CALIBRATION"));
  this->IGTLNames.push_back(std::string("TARGETING"));
  this->IGTLNames.push_back(std::string("MANUAL"));
  this->IGTLNames.push_back(std::string("EMERGENCY"));
  this->IGTLNames.push_back(std::string("INITIALIZE"));
  this->IGTLNames.push_back(std::string("SET_Z_FRAME"));

  this->IGTLNames.push_back(std::string("INITIALIZE"));
  this->IGTLNames.push_back(std::string("SET_Z_FRAME"));
  this->IGTLNames.push_back(std::string("MOVE_TO"));
  this->IGTLNames.push_back(std::string("INSERT"));
  this->IGTLNames.push_back(std::string("BIOPSY"));
  this->IGTLNames.push_back(std::string("HOME"));
  this->IGTLNames.push_back(std::string("STOP"));
  this->IGTLNames.push_back(std::string("E_STOP"));
  this->IGTLNames.push_back(std::string("SET_JOINT"));
  this->IGTLNames.push_back(std::string("SET_VELOCITY"));
  this->IGTLNames.push_back(std::string("SET_VOLTAGE_OUTPUT"));
  this->IGTLNames.push_back(std::string("LOCK"));
  this->IGTLNames.push_back(std::string("UNLOCK"));
  this->IGTLNames.push_back(std::string("LOCK_AXIS"));
  this->IGTLNames.push_back(std::string("UNLOCK_AXIS"));
  this->IGTLNames.push_back(std::string("LIMIT_VELOCITY"));
  this->IGTLNames.push_back(std::string("GET_COORDINATE"));
  this->IGTLNames.push_back(std::string("GET_STATUS"));
  this->IGTLNames.push_back(std::string("GET_BIOPSY"));
  this->IGTLNames.push_back(std::string("GET_NR_JOINT"));
  this->IGTLNames.push_back(std::string("GET_JOINT"));
  this->IGTLNames.push_back(std::string("GET_VELOCITY"));
  this->IGTLNames.push_back(std::string("GET_VOLTAGE_OUTPUT"));
  this->IGTLNames.push_back(std::string("GET_JOINT_TRACK"));
  this->IGTLNames.push_back(std::string("GET_BRAKE"));
  this->IGTLNames.push_back(std::string("GET_MOVING"));
  */

  // Robot -> Slicer
  //STATUS
  /*
  this->IGTLNames.push_back(std::string("BRAKES"));
  this->IGTLNames.push_back(std::string("DONE_MOVING"));
  this->IGTLNames.push_back(std::string("COORDINATES"));
  this->IGTLNames.push_back(std::string("JOINTS"));
  this->IGTLNames.push_back(std::string("JOINT_POS"));
  this->IGTLNames.push_back(std::string("JOINT_VEL"));
  this->IGTLNames.push_back(std::string("JOINT_TRACK"));
  this->IGTLNames.push_back(std::string("AIR_PRESSURE"));
  */

}


//---------------------------------------------------------------------------
vtkIGTLToMRMLBrpRobotCommand::~vtkIGTLToMRMLBrpRobotCommand()
{
}


//---------------------------------------------------------------------------
void vtkIGTLToMRMLBrpRobotCommand::PrintSelf(ostream& os, vtkIndent indent)
{
}


//---------------------------------------------------------------------------
vtkMRMLNode* vtkIGTLToMRMLBrpRobotCommand::CreateNewNode(vtkMRMLScene* scene, const char* name)
{
  vtkMRMLBrpRobotCommandNode* commandNode;

  commandNode = vtkMRMLBrpRobotCommandNode::New();
  commandNode->SetName(name);
  commandNode->SetDescription("Created by Prosotate module");

  scene->AddNode(commandNode);  

  return commandNode;
}


//---------------------------------------------------------------------------
vtkIntArray* vtkIGTLToMRMLBrpRobotCommand::GetNodeEvents()
{
  vtkIntArray* events;

  events = vtkIntArray::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);

  return events;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLBrpRobotCommand::IGTLToMRML(igtl::MessageBase::Pointer buffer, vtkMRMLNode* node)
{
  return 1;
}


//---------------------------------------------------------------------------
int vtkIGTLToMRMLBrpRobotCommand::MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg)
{

  if (mrmlNode && event == vtkCommand::ModifiedEvent)
    {
    vtkMRMLBrpRobotCommandNode* commandNode 
      = vtkMRMLBrpRobotCommandNode::SafeDownCast(mrmlNode);
    std::string command = commandNode->PopOutgoingCommand();
    
    std::cerr << "MRMLToIGTL   -- " << command << std::endl;

    if (strcmp(command.c_str(), "MOVE_TO") == 0)
      {
      if (!commandNode->GetScene())
        {
        return 0;
        }
      vtkMRMLNode* node = commandNode->GetScene()->GetNodeByID(commandNode->GetTargetTransformNodeID());
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
      if (transformNode)
        {
        vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
        if (this->OutMoveToMsg.IsNull())
          {
          this->OutMoveToMsg = igtl::MoveToMessage::New();
          this->OutMoveToMsg->SetDeviceName(mrmlNode->GetName());
          }

        igtl::Matrix4x4 igtlmatrix;
        
        igtlmatrix[0][0]  = matrix->GetElement(0, 0);
        igtlmatrix[1][0]  = matrix->GetElement(1, 0);
        igtlmatrix[2][0]  = matrix->GetElement(2, 0);
        igtlmatrix[3][0]  = matrix->GetElement(3, 0);
        
        igtlmatrix[0][1]  = matrix->GetElement(0, 1);
        igtlmatrix[1][1]  = matrix->GetElement(1, 1);
        igtlmatrix[2][1]  = matrix->GetElement(2, 1);
        igtlmatrix[3][1]  = matrix->GetElement(3, 1);
        
        igtlmatrix[0][2]  = matrix->GetElement(0, 2);
        igtlmatrix[1][2]  = matrix->GetElement(1, 2);
        igtlmatrix[2][2]  = matrix->GetElement(2, 2);
        igtlmatrix[3][2]  = matrix->GetElement(3, 2);
        
        igtlmatrix[0][3]  = matrix->GetElement(0, 3);
        igtlmatrix[1][3]  = matrix->GetElement(1, 3);
        igtlmatrix[2][3]  = matrix->GetElement(2, 3);
        igtlmatrix[3][3]  = matrix->GetElement(3, 3);
        
        float position[3];
        float quaternion[4];
        
        position[0] = igtlmatrix[0][3];
        position[1] = igtlmatrix[1][3];
        position[2] = igtlmatrix[2][3];

        igtl::MatrixToQuaternion(igtlmatrix, quaternion);

        //this->OutMoveToMsg->SetMatrix(igtlmatrix);
        this->OutMoveToMsg->SetPosition(position);
        this->OutMoveToMsg->SetQuaternion(quaternion);
        this->OutMoveToMsg->Pack();
        
        *size = this->OutMoveToMsg->GetPackSize();
        *igtlMsg = (void*)this->OutMoveToMsg->GetPackPointer();
        return 1;
        
        }
      else
        {
        return 0;
        }
      }
    else if (strcmp(command.c_str(), "SET_Z_FRAME") == 0)
      {
      if (!commandNode->GetScene())
        {
        return 0;
        }
      std::cerr << "int vtkIGTLToMRMLBrpRobotCommand::MRMLToIGTL(unsigned long event, vtkMRMLNode* mrmlNode, int* size, void** igtlMsg)" << std::endl;
      vtkMRMLNode* node = commandNode->GetScene()->GetNodeByID(commandNode->GetZFrameTransformNodeID());
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
      if (transformNode)
        {
        vtkMatrix4x4* matrix = transformNode->GetMatrixTransformToParent();
        if (this->OutSetZFrameMsg.IsNull())
          {
          this->OutSetZFrameMsg = igtl::SetZFrameMessage::New();
          this->OutSetZFrameMsg->SetDeviceName(mrmlNode->GetName());
          }
        
        igtl::Matrix4x4 igtlmatrix;
        
        igtlmatrix[0][0]  = matrix->GetElement(0, 0);
        igtlmatrix[1][0]  = matrix->GetElement(1, 0);
        igtlmatrix[2][0]  = matrix->GetElement(2, 0);
        igtlmatrix[3][0]  = matrix->GetElement(3, 0);
        
        igtlmatrix[0][1]  = matrix->GetElement(0, 1);
        igtlmatrix[1][1]  = matrix->GetElement(1, 1);
        igtlmatrix[2][1]  = matrix->GetElement(2, 1);
        igtlmatrix[3][1]  = matrix->GetElement(3, 1);
        
        igtlmatrix[0][2]  = matrix->GetElement(0, 2);
        igtlmatrix[1][2]  = matrix->GetElement(1, 2);
        igtlmatrix[2][2]  = matrix->GetElement(2, 2);
        igtlmatrix[3][2]  = matrix->GetElement(3, 2);
        
        igtlmatrix[0][3]  = matrix->GetElement(0, 3);
        igtlmatrix[1][3]  = matrix->GetElement(1, 3);
        igtlmatrix[2][3]  = matrix->GetElement(2, 3);
        igtlmatrix[3][3]  = matrix->GetElement(3, 3);
        
        float position[3];
        float quaternion[4];
        
        position[0] = igtlmatrix[0][3];
        position[1] = igtlmatrix[1][3];
        position[2] = igtlmatrix[2][3];
        igtl::MatrixToQuaternion(igtlmatrix, quaternion);
        
        //this->OutMoveToMsg->SetMatrix(igtlmatrix);
        this->OutSetZFrameMsg->SetPosition(position);
        this->OutSetZFrameMsg->SetQuaternion(quaternion);
        this->OutSetZFrameMsg->Pack();

        std::cerr << "SIZE: " << this->OutSetZFrameMsg->GetPackSize() << std::endl;
        
        *size = this->OutSetZFrameMsg->GetPackSize();
        *igtlMsg = (void*)this->OutSetZFrameMsg->GetPackPointer();

        return 1;
        
        }
      }
    else if (strcmp(command.c_str(), "INSERT") == 0)
      {
      //return 1;
      }
    else
      {
      if (this->OutgoingMsg.IsNull())
        {
        this->OutgoingMsg = igtl::HeaderMessage::New();
        this->OutgoingMsg->SetDeviceName(mrmlNode->GetName());
        }
      if (!this->OutgoingMsg->SetDeviceType(command.c_str()))
        {
        *size = 0;
        return 0;
        }
      this->OutgoingMsg->Pack();
      *size = this->OutgoingMsg->GetPackSize();
      *igtlMsg = (void*)this->OutgoingMsg->GetPackPointer();
      return 1;
      }
    }
  
  return 0;

}

