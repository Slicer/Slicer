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
#include "vtkMRMLMarkupsFiducialDisplayNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STL includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsFiducialDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialDisplayNode::vtkMRMLMarkupsFiducialDisplayNode()
{
  // Markups display node settings
  this->PropertiesLabelVisibility = false;
  this->PointLabelsVisibility = true;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialDisplayNode::~vtkMRMLMarkupsFiducialDisplayNode() = default;
