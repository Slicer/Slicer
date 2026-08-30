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

#include "vtkMRMLLayerDMObjectEventObserverScripted.h"

#include "vtkMRMLLayerDMPythonUtil.h"
#include <vtkObjectFactory.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMObjectEventObserverScripted);

//-----------------------------------------------------------------------------
vtkMRMLLayerDMObjectEventObserverScripted::vtkMRMLLayerDMObjectEventObserverScripted()
  : Object(nullptr)
{
  this->SetUpdateCallback(
    [this](vtkObject* node, unsigned long eventId, void* callData) -> void
    {
      if (!vtkMRMLLayerDMPythonUtil::IsValidPythonContext())
      {
        return;
      }

      vtkPythonScopeGilEnsurer gilEnsurer;
      if (auto result = vtkMRMLLayerDMPythonUtil::CallPythonObject(this->Object, vtkMRMLLayerDMPythonUtil::ToPyArgs(node, eventId, callData)))
      {
        Py_DECREF(result);
      }
      else
      {
        auto errorMsg = std::string(__func__) + ": Failed to call : " + vtkMRMLLayerDMPythonUtil::GetObjectStr(this->Object) + ":";
        vtkMRMLLayerDMPythonUtil::PrintErrorTraceback(this, errorMsg);
      }
    });
}

//-----------------------------------------------------------------------------
vtkMRMLLayerDMObjectEventObserverScripted::~vtkMRMLLayerDMObjectEventObserverScripted()
{
  vtkMRMLLayerDMPythonUtil::DeletePythonObject(&this->Object);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserverScripted::SetPythonCallback(PyObject* object)
{
  vtkMRMLLayerDMPythonUtil::SetPythonObject(&this->Object, object);
}

//-----------------------------------------------------------------------------
PyObject* vtkMRMLLayerDMObjectEventObserverScripted::CastCallData(PyObject* object, int vtkType)
{
  return vtkMRMLLayerDMPythonUtil::CastCallData(object, vtkType);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMObjectEventObserverScripted::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Python object: " << (this->Object ? "set" : "(none)") << std::endl;
}
