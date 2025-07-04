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
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/
/**
 * @class   vtkImplicitInvertableBoolean
 * @brief   implicit function consisting of boolean combinations of implicit functions, with invert option
 *
 * This class is a subclass of vtkImplicitBoolean that adds an option to invert the result of the boolean operation.
 *
 * For clipping functions, it is necessary to be able to invert the result of implicit functions. That ability is
 * supported with vtkImplicitSum, however vtkImplicitSum doesn't work for capping purposes, since we need to be able to
 * retrieve the plane functions from the clipping functions, however the internal functions are not accessible in
 * vtkImplicitSum. This class is a workaround for that issue.
 *
 * In the future the changes in this class may be moved to vtkAddon or VTK, either by adding the invert option to
 * vtkImplicitBoolean or by making the internal vtkImplicitFunctionCollection accessible in vtkImplicitSum.
 */

#ifndef vtkImplicitInvertableBoolean_h
#define vtkImplicitInvertableBoolean_h

// VTK includes
#include "vtkImplicitBoolean.h"

// MRML includes
#include "vtkMRML.h"

class VTK_MRML_EXPORT vtkImplicitInvertableBoolean : public vtkImplicitBoolean
{
public:
  vtkTypeMacro(vtkImplicitInvertableBoolean, vtkImplicitBoolean);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  static vtkImplicitInvertableBoolean* New();

  //@{
  /**
   * Evaluate boolean combinations of implicit function using current operator.
   */
  using vtkImplicitBoolean::EvaluateFunction;
  double EvaluateFunction(double x[3]) override;
  //@}

  /**
   * Evaluate gradient of boolean combination.
   */
  void EvaluateGradient(double x[3], double g[3]) override;

  //@{
  /**
   * Specify if the resulting function should be inverted
   */
  vtkSetMacro(Invert, bool);
  vtkGetMacro(Invert, bool);
  vtkBooleanMacro(Invert, bool);
  //@}

protected:
  vtkImplicitInvertableBoolean();
  ~vtkImplicitInvertableBoolean() override;

  bool Invert{ false };

private:
  vtkImplicitInvertableBoolean(const vtkImplicitInvertableBoolean&) = delete;
  void operator=(const vtkImplicitInvertableBoolean&) = delete;
};
#endif
