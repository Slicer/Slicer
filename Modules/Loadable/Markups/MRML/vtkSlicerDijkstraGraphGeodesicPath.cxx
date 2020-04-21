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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

// Markups MRML includes
#include "vtkSlicerDijkstraGraphGeodesicPath.h"

// VTK includes
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerDijkstraGraphGeodesicPath);

//------------------------------------------------------------------------------
vtkSlicerDijkstraGraphGeodesicPath::vtkSlicerDijkstraGraphGeodesicPath()
{
  this->UseScalarWeights = true;
  this->PreviousUseScalarWeights = this->UseScalarWeights;
  this->CostFunctionType = COST_FUNCTION_TYPE_DISTANCE;
  this->PreviousCostFunctionType = this->CostFunctionType;
}

//------------------------------------------------------------------------------
vtkSlicerDijkstraGraphGeodesicPath::~vtkSlicerDijkstraGraphGeodesicPath() = default;

//------------------------------------------------------------------------------
void vtkSlicerDijkstraGraphGeodesicPath::PrintSelf(std::ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "CostFunction: " << this->GetCostFunctionTypeAsString(this->CostFunctionType) << std::endl;
}

//------------------------------------------------------------------------------
const char* vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeAsString(int costFunction)
{
  switch (costFunction)
    {
    case COST_FUNCTION_TYPE_DISTANCE:
      {
      return "distance";
      }
    case COST_FUNCTION_TYPE_ADDITIVE:
      {
      return "additive";
      }
    case COST_FUNCTION_TYPE_MULTIPLICATIVE:
      {
      return "multiplicative";
      }
    case COST_FUNCTION_TYPE_INVERSE_SQUARED:
      {
      return "inverseSquared";
      }
    default:
      {
      return "";
      }
    }
}

//------------------------------------------------------------------------------
int vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeFromString(const char* costFunctionType)
{
  if (costFunctionType == nullptr)
    {
    // invalid name
    vtkGenericWarningMacro("Invalid sorting method name");
    return -1;
    }
  for (int i = 0; i < vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_LAST; i++)
    {
    if (strcmp(costFunctionType, vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  vtkGenericWarningMacro("Unknown cost function type: " << costFunctionType);
  return -1;
}

//----------------------------------------------------------------------------
int vtkSlicerDijkstraGraphGeodesicPath::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  /// Reimplemented to rebuild the adjacency info if either CostFunctionType or UseScalarWeights are changed.

  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkPolyData* input = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!input)
    {
    return 0;
    }

  vtkPolyData* output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!output)
    {
    return 0;
    }

  if (this->AdjacencyBuildTime.GetMTime() < input->GetMTime() ||
      this->CostFunctionType != this->PreviousCostFunctionType ||
      static_cast<bool>(this->UseScalarWeights) != this->PreviousUseScalarWeights)
    {
    this->Initialize(input);
    }
  else
    {
    this->Reset();
    }
  this->PreviousUseScalarWeights = this->UseScalarWeights;
  this->PreviousCostFunctionType= this->CostFunctionType;

  if (this->NumberOfVertices == 0)
    {
    return 0;
    }

  this->ShortestPath(input, this->StartVertex, this->EndVertex);
  this->TraceShortestPath(input, output, this->StartVertex, this->EndVertex);
  return 1;
}

//------------------------------------------------------------------------------
double vtkSlicerDijkstraGraphGeodesicPath::CalculateStaticEdgeCost(vtkDataSet* inData, vtkIdType u, vtkIdType v)
{
  // Parent implementation is inverse squared
  if (this->CostFunctionType == COST_FUNCTION_TYPE_INVERSE_SQUARED)
    {
    return Superclass::CalculateStaticEdgeCost(inData, u, v);
    }

  double p1[3];
  inData->GetPoint(u,p1);
  double p2[3];
  inData->GetPoint(v,p2);

  double distance = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
  double cost = distance;
  if (this->UseScalarWeights && this->CostFunctionType != COST_FUNCTION_TYPE_DISTANCE)
    {
    double scalarV = 0.0;
    // Note this edge cost is not symmetric!
    if (inData->GetPointData())
      {
      vtkFloatArray* scalars = vtkFloatArray::SafeDownCast(inData->GetPointData()->GetScalars());
      if (scalars)
        {
        scalarV = static_cast<double>(scalars->GetValue(v));
        }
      }

    switch (this->CostFunctionType)
      {
      case COST_FUNCTION_TYPE_ADDITIVE:
        cost += scalarV;
        break;
      case COST_FUNCTION_TYPE_MULTIPLICATIVE:
        cost *= scalarV;
      default:
        break;
      }
    }
  return cost;
}
