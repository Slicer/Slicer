/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkProjectMarkupsCurvePointsFilter.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkInformationVector.h"

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkProjectMarkupsCurvePointsFilter);

//---------------------------------------------------------------------------
int vtkProjectMarkupsCurvePointsFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    }
  else
    {
    vtkErrorMacro("Cannot set input info for port " << port);
    return 0;
    }
  return 1;
}

//---------------------------------------------------------------------------
int vtkProjectMarkupsCurvePointsFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData* inputPolyData = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!inputPolyData)
    {
    return 1;
    }

  vtkPoints* inputPoints = inputPolyData->GetPoints();
  if (!inputPoints)
    {
    return 1;
    }

  if (!this->Parent)
    {
    return 1;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* outputPolyData = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!outputPolyData)
    {
    return 1;
    }
  outputPolyData->DeepCopy(inputPolyData);
  vtkNew<vtkPoints> outputPoints;

  //if we have a surface node, project, otherwise the copy was enough
  if (this->Parent->GetSurfaceNode())
    {
    if(!this->Parent->ProjectPointsToSurface(this->Parent->GetSurfaceNode(), 1, inputPoints, outputPoints))
      {
      return 0;
      }
    outputPolyData->SetPoints(outputPoints);
    }

  outputPolyData->Squeeze();
  return 1;
}
