/*==============================================================================

  Copyright (c) The Intervention Centre
  Oslo University Hospital, Oslo, Norway. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Rafael Palomar (The Intervention Centre,
  Oslo University Hospital) and was supported by The Research Council of Norway
  through the ALive project (grant nr. 311393).

==============================================================================*/

#ifndef __vtkslicerlivermarkupslogic_h_
#define __vtkslicerlivermarkupslogic_h_

#include <vtkSlicerMarkupsLogic.h>

#include "vtkSlicerPluggableMarkupsTestModuleLogicExport.h"

class VTK_SLICER_PLUGGABLEMARKUPSTEST_MODULE_LOGIC_EXPORT vtkSlicerPluggableMarkupsTestLogic:
  public vtkSlicerMarkupsLogic
{
public:
  static vtkSlicerPluggableMarkupsTestLogic* New();
  vtkTypeMacro(vtkSlicerPluggableMarkupsTestLogic, vtkSlicerMarkupsLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkSlicerPluggableMarkupsTestLogic();
  ~vtkSlicerPluggableMarkupsTestLogic() override;

  void ObserveMRMLScene() override;
  void RegisterNodes() override;

  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;

private:
  vtkSlicerPluggableMarkupsTestLogic(const vtkSlicerPluggableMarkupsTestLogic&) = delete;
  void operator=(const vtkSlicerPluggableMarkupsTestLogic&) = delete;
};

#endif // __vtkslicerlivermarkupslogic_h_
