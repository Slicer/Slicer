// SlicerLogic includes
#include "vtkSlicerSceneViewLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSceneViewNode.h>

// VTK includes
#include <vtkSmartPointer.h>

// STD includes
#include <string>
#include <iostream>
#include <sstream>

// Convenient macro
#define VTK_CREATE(type, name) \
  vtkSmartPointer<type> name = vtkSmartPointer<type>::New()

//-----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerSceneViewLogic, "$Revision: 1.0$")
vtkStandardNewMacro(vtkSlicerSceneViewLogic)

//-----------------------------------------------------------------------------
// vtkSlicerSceneViewLogic methods
//-----------------------------------------------------------------------------
vtkSlicerSceneViewLogic::vtkSlicerSceneViewLogic()
{

}

//-----------------------------------------------------------------------------
vtkSlicerSceneViewLogic::~vtkSlicerSceneViewLogic()
{
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//-----------------------------------------------------------------------------
void vtkSlicerSceneViewLogic::ProcessMRMLEvents(
  vtkObject* vtkNotUsed(caller), unsigned long vtkNotUsed(event), void * vtkNotUsed(callData))
{

}

void vtkSlicerSceneViewLogic::RegisterNodes()
{

  if(!this->GetMRMLScene())
    {
    return;
    }


  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::New();
  this->GetMRMLScene()->RegisterNodeClass(viewNode);
  viewNode->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewLogic::CreateSceneView(const char* name, const char* description, int screenshotType, vtkImageData* screenshot)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  if (!screenshot)
    {
    vtkErrorMacro("CreateSceneView: No screenshot was set.")
    return;
    }

  vtkStdString nameString = vtkStdString(name);

  vtkMRMLSceneViewNode * newSceneViewNode = vtkMRMLSceneViewNode::New();
  newSceneViewNode->SetScene(this->GetMRMLScene());
  if (strcmp(nameString,""))
    {
    // a name was specified
    newSceneViewNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    newSceneViewNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("SceneView"));
    }

  vtkStdString descriptionString = vtkStdString(description);

  newSceneViewNode->SetSceneViewDescription(descriptionString);
  newSceneViewNode->SetScreenshotType(screenshotType);
  newSceneViewNode->SetScreenshot(screenshot);
  newSceneViewNode->StoreScene();
  //newSceneViewNode->HideFromEditorsOff();
  this->GetMRMLScene()->AddNode(newSceneViewNode);

}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewLogic::ModifySceneView(vtkStdString id, const char* name, const char* description, int screenshotType, vtkImageData* screenshot)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  if (!screenshot)
    {
    vtkErrorMacro("ModifySceneView: No screenshot was set.")
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id.c_str()));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewName: Could not get sceneView node!")
    return;
    }
  vtkStdString nameString = vtkStdString(name);

  if (strcmp(nameString,""))
    {
    // a name was specified
    viewNode->SetName(nameString.c_str());
    }
  else
    {
    // if no name is specified, generate a new unique one
    viewNode->SetName(this->GetMRMLScene()->GetUniqueNameByString("SceneView"));
    }

  vtkStdString descriptionString = vtkStdString(description);
  viewNode->SetSceneViewDescription(descriptionString);
  viewNode->SetScreenshotType(screenshotType);
  viewNode->SetScreenshot(screenshot);

  // TODO why two events?
  viewNode->Modified();
  viewNode->GetScene()->InvokeEvent(vtkCommand::ModifiedEvent, viewNode);

}

//---------------------------------------------------------------------------
vtkStdString vtkSlicerSceneViewLogic::GetSceneViewName(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return 0;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewName: Could not get sceneView node!")
    return 0;
    }

  return vtkStdString(viewNode->GetName());
}

//---------------------------------------------------------------------------
vtkStdString vtkSlicerSceneViewLogic::GetSceneViewDescription(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return 0;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewDescription: Could not get sceneView node!")
    return 0;
    }

  return viewNode->GetSceneViewDescription();
}

//---------------------------------------------------------------------------
int vtkSlicerSceneViewLogic::GetSceneViewScreenshotType(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return -1;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshotType: Could not get sceneView node!")
    return -1;
    }

  return viewNode->GetScreenshotType();
}

//---------------------------------------------------------------------------
vtkImageData* vtkSlicerSceneViewLogic::GetSceneViewScreenshot(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return 0;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshot: Could not get sceneView node!")
    return 0;
    }

  return viewNode->GetScreenshot();
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewLogic::RestoreSceneView(const char* id)
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshot: Could not get sceneView node!")
    return;
    }

  this->GetMRMLScene()->SaveStateForUndo();
  viewNode->RestoreScene();
}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewLogic::MoveSceneViewUp(const char* id)
{
  if (!id)
    {
    return;
    }

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshot: Could not get sceneView node!")
    return;
    }

  int numberOfSceneViews = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");

  // this is the buffer
  vtkMRMLSceneViewNode* bufferNode = 0;

  // this is the current node used during the loop
  vtkMRMLSceneViewNode* vNode = 0;

  for(int i=0; i<numberOfSceneViews; ++i)
    {

    vNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLSceneViewNode"));

    // we buffer the previous node
    bufferNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(i-1,"vtkMRMLSceneViewNode"));;

    if (!strcmp(vNode->GetID(),viewNode->GetID()))
      {
      // this is the selected node

      // jump out of the loop
      break;
      }

    }

  if (!bufferNode)
    {
    // there is no node before the selected one, so we jump out
    return;
    }

  // now we copy the current node
  vtkMRMLSceneViewNode* copyNode = vtkMRMLSceneViewNode::New();
  copyNode->CopyWithoutModifiedEvent(vNode);

  // ..and delete the current one
  this->GetMRMLScene()->RemoveNode(vNode);

  // ..and insert the copy before our bufferNode
  this->GetMRMLScene()->InsertBeforeNode(bufferNode,copyNode);

}

//---------------------------------------------------------------------------
void vtkSlicerSceneViewLogic::MoveSceneViewDown(const char* id)
{
  if (!id)
    {
    return;
    }

  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("No scene set.")
    return;
    }

  vtkMRMLSceneViewNode* viewNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(id));

  if (!viewNode)
    {
    vtkErrorMacro("GetSceneViewScreenshot: Could not get sceneView node!")
    return;
    }

  int numberOfSceneViews = this->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLSceneViewNode");

  // this is the buffer
  vtkMRMLSceneViewNode* bufferNode = 0;

  // this is the current node used during the loop
  vtkMRMLSceneViewNode* vNode = 0;

  for(int i=0; i<numberOfSceneViews; ++i)
    {

    vNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLSceneViewNode"));

    // we buffer the next node
    bufferNode = vtkMRMLSceneViewNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(i+1,"vtkMRMLSceneViewNode"));;

    if (!strcmp(vNode->GetID(),viewNode->GetID()))
      {
      // this is the selected node

      // jump out of the loop
      break;
      }


    }

  if (!bufferNode)
    {
    // there is no node after the selected one, so we jump out
    return;
    }

  // now we copy the current node
  vtkMRMLSceneViewNode* copyNode = vtkMRMLSceneViewNode::New();
  copyNode->CopyWithoutModifiedEvent(bufferNode);

  // ..and delete the buffered one
  this->GetMRMLScene()->RemoveNode(bufferNode);

  // ..and insert the copy before our current node
  this->GetMRMLScene()->InsertBeforeNode(vNode,copyNode);

}
