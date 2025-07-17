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
#include "vtkImplicitInvertableBoolean.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkImplicitInvertableBoolean);

//----------------------------------------------------------------------------
vtkImplicitInvertableBoolean::vtkImplicitInvertableBoolean() = default;

//----------------------------------------------------------------------------
vtkImplicitInvertableBoolean::~vtkImplicitInvertableBoolean() = default;

//----------------------------------------------------------------------------
double vtkImplicitInvertableBoolean::EvaluateFunction(double x[3])
{
  const double value = Superclass::EvaluateFunction(x);
  return this->Invert ? -value : value;
}

//----------------------------------------------------------------------------
void vtkImplicitInvertableBoolean::EvaluateGradient(double x[3], double g[3])
{
  Superclass::EvaluateGradient(x, g);
  if (this->Invert)
  {
    g[0] = -g[0];
    g[1] = -g[1];
    g[2] = -g[2];
  }
}

//----------------------------------------------------------------------------
void vtkImplicitInvertableBoolean::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Invert:\n";
  os << indent << (this->Invert ? "True" : "False") << "\n";
}
