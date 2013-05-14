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
  bool keyPressed = false;
  char *keySym = this->GetInteractor()->GetKeySym();
  if (keySym && strcmp(keySym, "d") == 0)
    {
    keyPressed = true;
    }

  if (eventid == vtkCommand::LeftButtonReleaseEvent && keyPressed)
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

      vtkIdType cellID = -1;

      if (modelDisplayableManager->Pick(x,yNew) &&
          strcmp(modelDisplayableManager->GetPickedNodeID(),"") != 0)
        {

        cellID = modelDisplayableManager->GetPickedCellID();

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

      if (displayNode)
        {
        this->DeletePickedFiber(displayNode, cellID);
        this->GetInteractionNode()->SetCurrentInteractionMode(vtkMRMLInteractionNode::ViewTransform);
        }

      // reset pick tolerance
      modelDisplayableManager->SetPickTolerance(pickTolerance);
      }
    }

  if (keyPressed)
    {
    this->GetInteractor()->SetKeySym(0);
    }

  this->PassThroughInteractorStyleEvent(eventid);

  return;
}

//---------------------------------------------------------------------------
void vtkMRMLTractographyDisplayDisplayableManager::DeletePickedFiber(vtkMRMLFiberBundleDisplayNode* displayNode,
                                                                     vtkIdType pickedCell)
{
  if (!displayNode)
  {
    return;
  }
  vtkMRMLFiberBundleNode *fiberBundleNode = vtkMRMLFiberBundleNode::SafeDownCast(
    displayNode->GetDisplayableNode());

  if (!fiberBundleNode)
  {
    return;
  }

  vtkMRMLFiberBundleLineDisplayNode *lineDisplayNode = vtkMRMLFiberBundleLineDisplayNode::SafeDownCast(displayNode);
  vtkMRMLFiberBundleTubeDisplayNode *tubeDisplayNode = vtkMRMLFiberBundleTubeDisplayNode::SafeDownCast(displayNode);
  vtkMRMLFiberBundleGlyphDisplayNode *glyphDisplayNode = vtkMRMLFiberBundleGlyphDisplayNode::SafeDownCast(displayNode);

  vtkIdType cellID = -1;
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

  if (cellID >=0)
    {
    cellID = fiberBundleNode->GetUnShuffledFiberID(cellID);

    int shuffleIDs = fiberBundleNode->GetEnableShuffleIDs();

    fiberBundleNode->SetEnableShuffleIDs(0);

    vtkPolyData *polyData = vtkPolyData::New();
    polyData->DeepCopy(fiberBundleNode->GetPolyData());
    polyData->DeleteCell(cellID);
    polyData->RemoveDeletedCells();
    fiberBundleNode->SetAndObservePolyData(polyData);
    polyData->Delete();

    fiberBundleNode->SetEnableShuffleIDs(shuffleIDs);

    /** just updating polydata does not work for some reason
    vtkPolyData *polyData = fiberBundleNode->GetPolyData();
    polyData->DeleteCell(cellID);
    polyData->RemoveDeletedCells();
    polyData->Modified();
    tubeDisplayNode->InvokeEvent(vtkCommand::ModifiedEvent);
    //tubeDisplayNode->SetInputPolyData(polyData);
    ***/
    }

}
