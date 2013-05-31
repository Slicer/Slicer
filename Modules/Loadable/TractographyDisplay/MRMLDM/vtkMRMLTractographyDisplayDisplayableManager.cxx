/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include "vtkMRMLTractographyDisplayDisplayableManager.h"


// MRML includes
#include "vtkEventBroker.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"

#include <vtkMRMLModelDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>

// VTK includes

#include "vtkInteractorStyle.h"
#include <vtkNew.h>
#include "vtkObjectFactory.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkPolyData.h"
#include "vtkPointData.h"

// ITKSys includes
//#include <itksys/SystemTools.hxx>
//#include <itksys/Directory.hxx>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLTractographyDisplayDisplayableManager);
vtkCxxRevisionMacro (vtkMRMLTractographyDisplayDisplayableManager, "$Revision: 1.0 $");

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
vtkMRMLTractographyDisplayDisplayableManager::vtkMRMLTractographyDisplayDisplayableManager()
{
  this->EnableFiberEdit = 0;
  this->SelectedFiberBundleNode = 0;

  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeftButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeftButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::RightButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::RightButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MiddleButtonPressEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MiddleButtonReleaseEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MouseWheelBackwardEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::MouseWheelForwardEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::EnterEvent);
  this->RemoveInteractorStyleObservableEvent(vtkCommand::LeaveEvent);
  this->AddInteractorStyleObservableEvent(vtkCommand::KeyPressEvent);
}

//---------------------------------------------------------------------------
vtkMRMLTractographyDisplayDisplayableManager::~vtkMRMLTractographyDisplayDisplayableManager()
{
}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::PrintSelf(std::ostream &os, vtkIndent indent)
{
  os<<indent<<"Print logic"<<endl;
}

//---------------------------------------------------------------------------
int vtkMRMLTractographyDisplayDisplayableManager::ActiveInteractionModes()
{
  return vtkMRMLInteractionNode::ViewTransform;
}


//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::OnInteractorStyleEvent(int eventid)
{
  //if (eventid == vtkCommand::LeftButtonReleaseEvent && keyPressed)
  if (this->GetEnableFiberEdit() &&
      eventid == vtkCommand::KeyPressEvent && 
      (this->GetInteractor()->GetKeyCode() == 'd' ||
       this->GetInteractor()->GetKeyCode() == 'x' ||
       this->GetInteractor()->GetKeyCode() == 's') )
    {
    double x = this->GetInteractor()->GetEventPosition()[0];
    double y = this->GetInteractor()->GetEventPosition()[1];

    double windowWidth = this->GetInteractor()->GetRenderWindow()->GetSize()[0];
    double windowHeight = this->GetInteractor()->GetRenderWindow()->GetSize()[1];

    if (x < windowWidth && y < windowHeight)
      {
      // it's a 3D displayable manager and the click could have been on a node
      double yNew = windowHeight - y - 1;
      vtkMRMLFiberBundleDisplayNode *displayNode = NULL;

      vtkMRMLModelDisplayableManager *modelDisplayableManager =
              vtkMRMLModelDisplayableManager::SafeDownCast(
              this->GetMRMLDisplayableManagerGroup()->GetDisplayableManagerByClassName(
                "vtkMRMLModelDisplayableManager"));

      double pickTolerance = modelDisplayableManager->GetPickTolerance();
      modelDisplayableManager->SetPickTolerance(0.001);

      vtkIdType pickedCellID = -1;

      if (modelDisplayableManager->Pick(x,yNew) &&
          strcmp(modelDisplayableManager->GetPickedNodeID(),"") != 0)
        {

        pickedCellID = modelDisplayableManager->GetPickedCellID();

        // find the node id, the picked node name is probably the display node
        const char *pickedNodeID = modelDisplayableManager->GetPickedNodeID();

        vtkMRMLNode *mrmlNode = this->GetMRMLScene()->GetNodeByID(pickedNodeID);
        if (mrmlNode)
          {
          displayNode = vtkMRMLFiberBundleDisplayNode::SafeDownCast(mrmlNode);
          }
        else
          {
          vtkDebugMacro("couldn't find a mrml node with ID " << pickedNodeID);
          }
        }

      // reset pick tolerance
      modelDisplayableManager->SetPickTolerance(pickTolerance);

      if (this->GetInteractor()->GetKeyCode() == 'd')
        {
        std::vector<vtkIdType> cellIDs;
        if (this->SelectedCells.empty())
          {
          // delete picked fiber
          if (displayNode)
            {
            vtkIdType cellID = -1;
            vtkMRMLFiberBundleNode *fiberBundleNode = this->GetPickedFiber(displayNode, pickedCellID, cellID);
            if (fiberBundleNode && cellID > -1)
              {
              cellIDs.push_back(cellID);
              this->DeletePickedFibers(fiberBundleNode, cellIDs);
              }
            }
          }
        else
          {
          // delete all selected fibers
          this->DeleteSelectedFibers();
          this->SelectedFiberBundleNode = NULL;
          this->SelectedCells.clear();
          }
        }
      else if (this->GetInteractor()->GetKeyCode() == 's')
        {
        // add fiber to selection
        vtkIdType cellID = -1;
        vtkMRMLFiberBundleNode *fiberBundleNode = this->GetPickedFiber(displayNode, pickedCellID, cellID);
        if (fiberBundleNode != this->SelectedFiberBundleNode)
          {
          this->ClearSelectedFibers();
          }
        if (fiberBundleNode && cellID > -1)
          {
          this->SelectedFiberBundleNode = fiberBundleNode;
          std::vector<vtkIdType> cellIDs;
          cellIDs.push_back(cellID);
          this->SelectPickedFibers(fiberBundleNode, cellIDs);
          }
        }
      else if (this->GetInteractor()->GetKeyCode() == 'x')
        {
        // unselect all selected fibers
        this->ClearSelectedFibers();
        }

      }

    this->GetInteractionNode()->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
    }

  this->PassThroughInteractorStyleEvent(eventid);

  return;
}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::ClearSelectedFibers()
{
  // unselect all selected fibers
  std::vector<vtkIdType> cellIDs;
  std::map <vtkIdType, std::vector<double> >::iterator it;
  for(it = this->SelectedCells.begin(); it != this->SelectedCells.end(); it++)
    {
    cellIDs.push_back(it->first);
    }
  this->SelectPickedFibers(this->SelectedFiberBundleNode, cellIDs);
  this->SelectedFiberBundleNode = NULL;
  this->SelectedCells.clear();
}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::DeleteSelectedFibers()
{
  std::vector<vtkIdType> cellIDs;
  std::map <vtkIdType, std::vector<double> >::iterator it;
  for (it = this->SelectedCells.begin(); it != this->SelectedCells.end(); it++)
    {
    cellIDs.push_back(it->first);
    }
  this->DeletePickedFibers(this->SelectedFiberBundleNode, cellIDs);
  // reset selection
}
 
//---------------------------------------------------------------------------
vtkMRMLFiberBundleNode*
vtkMRMLTractographyDisplayDisplayableManager::GetPickedFiber(vtkMRMLFiberBundleDisplayNode* displayNode,
                                                             vtkIdType pickedCell, vtkIdType &cellID)
{
  cellID = -1;

  vtkMRMLFiberBundleNode *fiberBundleNode = NULL;

  if (!displayNode)
  {
    return fiberBundleNode;
  }

  vtkMRMLFiberBundleLineDisplayNode *lineDisplayNode = vtkMRMLFiberBundleLineDisplayNode::SafeDownCast(displayNode);
  vtkMRMLFiberBundleTubeDisplayNode *tubeDisplayNode = vtkMRMLFiberBundleTubeDisplayNode::SafeDownCast(displayNode);
  vtkMRMLFiberBundleGlyphDisplayNode *glyphDisplayNode = vtkMRMLFiberBundleGlyphDisplayNode::SafeDownCast(displayNode);

  if (tubeDisplayNode)
    {
    int numSides = tubeDisplayNode->GetTubeNumberOfSides();
    cellID = pickedCell/numSides;
    }
  else if (lineDisplayNode)
    {
    cellID = pickedCell;
    }
  else if (glyphDisplayNode)
    {
    // NOT IMPLEMENTED YET
    }

  if(cellID >= 0)
    {            
    fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(displayNode->GetDisplayableNode());
    cellID = fiberBundleNode->GetUnShuffledFiberID(cellID);
    }

  return fiberBundleNode;
}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::DeletePickedFibers(vtkMRMLFiberBundleNode *fiberBundleNode,
                                                                     std::vector<vtkIdType> &cellIDs)
{
  if (!fiberBundleNode)
  {
    return;
  }

  int shuffleIDs = fiberBundleNode->GetEnableShuffleIDs();
  fiberBundleNode->SetEnableShuffleIDs(0);

  vtkPolyData *polyData = vtkPolyData::New();
  polyData->DeepCopy(fiberBundleNode->GetPolyData());
  for (unsigned int i=0; i<cellIDs.size(); i++)
    {
    if (cellIDs[i] >= 0)
      {
      polyData->DeleteCell(cellIDs[i]);
      }
    }
  polyData->RemoveDeletedCells();
  fiberBundleNode->SetAndObservePolyData(polyData);
  polyData->Delete();

  fiberBundleNode->SetEnableShuffleIDs(shuffleIDs);

}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::SelectPickedFibers(vtkMRMLFiberBundleNode *fiberBundleNode,
                                                                     std::vector<vtkIdType> &cellIDs)
{
  if (!fiberBundleNode)
    {
    return;
    }

  int shuffleIDs = fiberBundleNode->GetEnableShuffleIDs();
  fiberBundleNode->SetEnableShuffleIDs(0);

  // copy polydata
  vtkIdType npts;
  vtkIdType* pts;
  vtkPolyData *polyData = vtkPolyData::New();
  polyData->DeepCopy(fiberBundleNode->GetPolyData());
  if (polyData->GetPointData() == NULL || polyData->GetPointData()->GetArray(0) == NULL)
    {
    return;
    }

  // get selection color(value) from the range
  vtkDataArray *pointScalars = polyData->GetPointData()->GetArray(0);
  double *range = pointScalars->GetRange();
  float selectValue = range[1];
  int ncomponents = pointScalars->GetNumberOfComponents();

  // Loop overs cells

  for (unsigned int i=0; i<cellIDs.size(); i++)
    {
    if (cellIDs[i] < 0)
      {
      continue;
      }

    polyData->GetCellPoints(cellIDs[i], npts, pts);

    std::map <vtkIdType, std::vector<double> >::iterator it = this->SelectedCells.find(cellIDs[i]);
    std::vector<double> values;

    if (it != this->SelectedCells.end())
      {
      // if cell was already selected, unselect
      // reset scalar values
      values = it->second;
      for (int p=0; p<npts; p++)
        {
        for (int c=0; c<ncomponents; c++)
          {
          pointScalars->SetComponent(pts[p], c, values[p*ncomponents+c]);
          }
        }
      this->SelectedCells.erase(it);
      }
    else
      {
      // select cell, store scalar values and replace them with 'selectValue'
      for (int p=0; p<npts; p++)
        {
        for (int c=0; c<ncomponents; c++)
          {
          values.push_back(pointScalars->GetComponent(p, c));
          pointScalars->SetComponent(pts[p], c, selectValue);
          }
        }
      this->SelectedCells[cellIDs[i]] = values;
      }
    }//for (unsigned int i=0; i<cellIDs.size(); i++)

  fiberBundleNode->SetAndObservePolyData(polyData);
  polyData->Delete();

  fiberBundleNode->SetEnableShuffleIDs(shuffleIDs);

}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{

  vtkMRMLInteractionNode *interactionNode = NULL;

  if (newScene)
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast(
          newScene->GetNthNodeByClass(0, "vtkMRMLInteractionNode"));
    }
  if (interactionNode)
    {
    if (newScene)
      {
      vtkObserveMRMLNodeMacro(interactionNode);
      }
    else
      {
      vtkUnObserveMRMLNodeMacro(interactionNode);
      }
    }

  Superclass::SetMRMLSceneInternal(newScene);
}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager
::ProcessMRMLNodesEvents(vtkObject *caller,unsigned long event,void *callData)
{

  vtkMRMLInteractionNode * interactionNode = vtkMRMLInteractionNode::SafeDownCast(caller);
  if (interactionNode && event == vtkCommand::ModifiedEvent)
  {
    this->SetEnableFiberEdit(interactionNode->GetEnableFiberEdit());
  }
}
