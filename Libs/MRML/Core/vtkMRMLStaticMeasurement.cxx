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

// MRML includes
#include "vtkMRMLStaticMeasurement.h"

vtkStandardNewMacro(vtkMRMLStaticMeasurement);

//----------------------------------------------------------------------------
vtkMRMLStaticMeasurement::vtkMRMLStaticMeasurement()
{
  this->LastComputationResult = vtkMRMLMeasurement::OK;
}

//----------------------------------------------------------------------------
vtkMRMLStaticMeasurement::~vtkMRMLStaticMeasurement() = default;

//----------------------------------------------------------------------------
void vtkMRMLStaticMeasurement::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRMLStaticMeasurement::Compute()
{
  // No action is needed because this class is a constant variant of the dynamic
  // vtkMRMLMeasurement. Typically all measurements calculate their own value from
  // its input MRML node. This class is to be able to store static measurements.
  this->LastComputationResult = vtkMRMLMeasurement::OK;
}
