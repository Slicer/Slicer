/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware SAS

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thibault Pelletier, Kitware SAS,
  and was partially funded by ANR grants ANR-22-CE45-0034 and ANR-18-RHUS-005.

==============================================================================*/

#include "vtkMRMLLayerDMPipelineScriptedCreator.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipeline.h"
#include "vtkMRMLLayerDMPythonUtil.h"

// Slicer includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLNode.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkPythonUtil.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMPipelineScriptedCreator);

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipelineScriptedCreator::vtkMRMLLayerDMPipelineScriptedCreator()
  : Object(nullptr)
{
  this->SetCallback(
    [this](vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) -> vtkSmartPointer<vtkMRMLLayerDMPipeline>
    {
      if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
      {
        return nullptr;
      }

      vtkPythonScopeGilEnsurer gilEnsurer;
      PyObject* result = vtkMRMLLayerDMPythonUtil::CallPythonObject(
        this->Object, vtkMRMLLayerDMPythonUtil::ToPyArgs({ vtkMRMLLayerDMPythonUtil::ToPyObject(viewNode), vtkMRMLLayerDMPythonUtil::ToPyObject(node) }));
      if (!result)
      {
        auto errorMsg = std::string(__func__) + ": Failed to call : " + vtkMRMLLayerDMPythonUtil::GetObjectStr(this->Object) + ":";
        vtkMRMLLayerDMPythonUtil::PrintErrorTraceback(this, errorMsg);
        return nullptr;
      }
      return vtkMRMLLayerDMPipeline::SafeDownCast(vtkPythonUtil::GetPointerFromObject(result, "vtkMRMLLayerDMPipeline"));
    });
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMPipelineScriptedCreator::~vtkMRMLLayerDMPipelineScriptedCreator()
{
  vtkMRMLLayerDMPythonUtil::DeletePythonObject(&this->Object);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineScriptedCreator::SetPythonCallback(PyObject* object)
{
  vtkMRMLLayerDMPythonUtil::SetPythonObject(&this->Object, object);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineScriptedCreator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Python object: " << (this->Object ? "set" : "(none)") << std::endl;
}
