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

#include "vtkMRMLMarkupsClosedCurveNode.h"

// MRML includes
#include "vtkCurveGenerator.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsClosedCurveNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode::vtkMRMLMarkupsClosedCurveNode()
{
  this->CurveClosed = true;
  this->CurveGenerator->SetCurveIsLoop(true);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode::~vtkMRMLMarkupsClosedCurveNode() = default;
