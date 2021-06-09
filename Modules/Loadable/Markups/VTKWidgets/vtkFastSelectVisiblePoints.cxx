/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was centerally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

#include "vtkFastSelectVisiblePoints.h"

#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkDataSet.h"
#include <vtkFloatArray.h>
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"

vtkStandardNewMacro(vtkFastSelectVisiblePoints);

//----------------------------------------------------------------------------
vtkFastSelectVisiblePoints::vtkFastSelectVisiblePoints()
{
  this->ZBuffer = nullptr;
}

//----------------------------------------------------------------------------
vtkFastSelectVisiblePoints::~vtkFastSelectVisiblePoints() = default;

//----------------------------------------------------------------------------
void vtkFastSelectVisiblePoints::ResetZBuffer()
{
  this->ZBuffer = nullptr;
}

//----------------------------------------------------------------------------
void vtkFastSelectVisiblePoints::UpdateZBuffer()
{
  this->ResetZBuffer();
  float* zPtr = this->Initialize(true);

  this->ZBuffer = vtkSmartPointer<vtkFloatArray>::New();
  vtkIdType size = (this->InternalSelection[1] - this->InternalSelection[0] + 1) * (this->InternalSelection[3] - this->InternalSelection[2] + 1);
  this->ZBuffer->SetArray(zPtr, size, 0);
}

//----------------------------------------------------------------------------
int vtkFastSelectVisiblePoints::RequestData(vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet* input = vtkDataSet::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkPolyData* output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkIdType ptId, cellId;
  int visible;
  vtkPointData* inPD = input->GetPointData();
  vtkPointData* outPD = output->GetPointData();
  vtkIdType numPts = input->GetNumberOfPoints();
  double x[4];

  // Nothing to extract if there are no points in the data set.
  if (numPts < 1)
    {
    return 1;
    }

  if (this->Renderer == nullptr)
    {
    vtkErrorMacro(<< "Renderer must be set");
    return 0;
    }

  if (!this->Renderer->GetRenderWindow())
    {
    vtkErrorMacro("No render window -- can't get window size to query z buffer.");
    return 0;
    }

  // This will trigger if you do something like ResetCamera before the Renderer or
  // RenderWindow have allocated their appropriate system resources (like creating
  // an OpenGL context)." Resource allocation must occur before we can use the Z
  // buffer.
  if (this->Renderer->GetRenderWindow()->GetNeverRendered())
    {
    vtkDebugMacro("RenderWindow not initialized -- aborting update.");
    return 1;
    }

  vtkCamera* cam = this->Renderer->GetActiveCamera();
  if (!cam)
    {
    return 1;
    }

  vtkPoints* outPts = vtkPoints::New();
  outPts->Allocate(numPts / 2 + 1);
  outPD->CopyAllocate(inPD);

  vtkCellArray* outputVertices = vtkCellArray::New();
  output->SetVerts(outputVertices);
  outputVertices->Delete();

  if (!this->ZBuffer)
    {
    this->UpdateZBuffer();
    }
  else
    {
    this->Initialize(false);
    }

  int abort = 0;
  vtkIdType progressInterval = numPts / 20 + 1;
  x[3] = 1.0;
  for (cellId = (-1), ptId = 0; ptId < numPts && !abort; ptId++)
    {
    // perform conversion
    input->GetPoint(ptId, x);

    if (!(ptId % progressInterval))
      {
      this->UpdateProgress(static_cast<double>(ptId) / numPts);
      abort = this->GetAbortExecute();
      }

    visible = IsPointOccluded(x, this->ZBuffer->GetPointer(0));

    if ((visible && !this->SelectInvisible) || (!visible && this->SelectInvisible))
      {
      cellId = outPts->InsertNextPoint(x);
      output->InsertNextCell(VTK_VERTEX, 1, &cellId);
      outPD->CopyData(inPD, ptId, cellId);
      }
    } // for all points

  output->SetPoints(outPts);
  outPts->Delete();
  output->Squeeze();

  vtkDebugMacro(<< "Selected " << cellId + 1 << " out of " << numPts << " original points");

  return 1;
}

//----------------------------------------------------------------------------
void vtkFastSelectVisiblePoints::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
