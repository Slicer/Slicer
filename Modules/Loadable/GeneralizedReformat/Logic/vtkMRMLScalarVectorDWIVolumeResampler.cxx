/*==============================================================================

  Program: 3D Slicer

  Copyright(c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// GeneralizedReformat includes
#include "vtkMRMLScalarVectorDWIVolumeResampler.h"

// Slicer includes
#include <vtkSlicerCLIModuleLogic.h>

// GeneralizedReformat Logic includes
#include <vtkMRMLAbstractVolumeResampler.h>

// MRML includes
#include <vtkMRMLCommandLineModuleNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVolumeNode.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLScalarVectorDWIVolumeResampler);

//----------------------------------------------------------------------------
void vtkMRMLScalarVectorDWIVolumeResampler::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLScalarVectorDWIVolumeResampler::Resample(vtkMRMLVolumeNode* inputVolume,
                                                     vtkMRMLVolumeNode* outputVolume,
                                                     vtkMRMLTransformNode* resamplingTransform,
                                                     vtkMRMLVolumeNode* referenceVolume,
                                                     int interpolationType,
                                                     const ResamplingParameters& resamplingParameter)
{
  // A helper RAII class to ensure a vtkMRMLNode is removed from its scene
  // when the enclosing scope is exited, regardless of how it is exited.
  struct vtkMRMLNodeCleanup
  {
  public:
    vtkMRMLNodeCleanup(vtkMRMLScene* scene, vtkMRMLNode* node)
      : Scene(scene)
      , Node(node)
    {}
    ~vtkMRMLNodeCleanup()
    {
      if (this->Scene != nullptr)
      {
        this->Scene->RemoveNode(this->Node);
      }
    }

  private:
    vtkMRMLScene* Scene{ nullptr };
    vtkMRMLNode* Node{ nullptr };
  };

  vtkMRMLApplicationLogic* appLogic = this->GetMRMLApplicationLogic();
  if (appLogic == nullptr)
  {
    vtkErrorMacro("Resample: MRML application logic is not available");
    return false;
  }

  vtkSlicerCLIModuleLogic* cliLogic =
    vtkSlicerCLIModuleLogic::SafeDownCast(appLogic->GetModuleLogic("ResampleScalarVectorDWIVolume"));
  if (cliLogic == nullptr)
  {
    vtkErrorMacro("Resample: ResampleScalarVectorDWIVolume module logic is not available");
    return false;
  }

  // PERF: Revisit if creating and removing the node impacts performances.
  vtkMRMLCommandLineModuleNode* cmdNode = cliLogic->CreateNodeInScene();
  if (cmdNode == nullptr)
  {
    vtkErrorMacro("Resample: Failed to create ResampleScalarVectorDWIVolume node");
    return false;
  }

  vtkMRMLNodeCleanup nodeCleanup(cliLogic->GetMRMLScene(), cmdNode);

  if (inputVolume == nullptr)
  {
    vtkErrorMacro("Resample: Input volume node is not set");
    return false;
  }
  cmdNode->SetParameterAsString("inputVolume", inputVolume->GetID());

  if (outputVolume == nullptr)
  {
    vtkErrorMacro("Resample: Output volume node is not set");
    return false;
  }
  cmdNode->SetParameterAsString("outputVolume", outputVolume->GetID());

  if (resamplingTransform != nullptr)
  {
    cmdNode->SetParameterAsString("transformationFile", resamplingTransform->GetID());
  }
  if (referenceVolume != nullptr)
  {
    cmdNode->SetParameterAsString("referenceVolume", referenceVolume->GetID());
  }

  switch (interpolationType)
  {
    case vtkMRMLAbstractVolumeResampler::InterpolationTypeNearestNeighbor:
      cmdNode->SetParameterAsString("interpolationType", "nn");
      break;
    case vtkMRMLAbstractVolumeResampler::InterpolationTypeLinear:
      cmdNode->SetParameterAsString("interpolationType", "linear");
      break;
    case vtkMRMLAbstractVolumeResampler::InterpolationTypeWindowedSinc:
      cmdNode->SetParameterAsString("interpolationType", "ws");
      break;
    case vtkMRMLAbstractVolumeResampler::InterpolationTypeBSpline:
      cmdNode->SetParameterAsString("interpolationType", "bs");
      break;
    case vtkMRMLAbstractVolumeResampler::InterpolationTypeUndefined:
    default:
      break;
  }

  if (!resamplingParameter.empty())
  {
    vtkErrorMacro("Resample: resamplingParameter is not currently supported");
    return false;
  }

  cliLogic->ApplyAndWait(cmdNode, /* updateDisplay= */ false);

  bool success = cmdNode->GetStatus() == vtkMRMLCommandLineModuleNode::Completed;

  if (!success)
  {
    vtkErrorMacro("Resample: " << cmdNode->GetErrorText());
  }

  return success;
}
