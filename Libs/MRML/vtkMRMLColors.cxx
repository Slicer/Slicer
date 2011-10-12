/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Michael Jeulin-L, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// vtkMRML includes
#include "vtkMRMLColors.h"

// VTK includes
#include <vtkColor.h>

//------------------------------------------------------------------------------
vtkColor3d vtkMRMLColors::sliceRed()
{
  // 243, 74, 51 -> #F34A33
  vtkColor3d flamingo = vtkColor3d(0.952941176471, 0.290196078431, 0.2);
  return flamingo;
}

//------------------------------------------------------------------------------
vtkColor3d vtkMRMLColors::sliceGreen()
{
  // 110, 176, 75 -> #6EB04B
  vtkColor3d chelseaCucumber = vtkColor3d(0.43137254902, 0.690196078431, 0.294117647059);
  return chelseaCucumber;
}

//------------------------------------------------------------------------------
vtkColor3d vtkMRMLColors::sliceYellow()
{
  // 237, 213, 76 -> #EDD54C
  vtkColor3d ronchy = vtkColor3d(0.929411764706, 0.835294117647, 0.298039215686);
  return ronchy;
}

//------------------------------------------------------------------------------
vtkColor3d vtkMRMLColors::sliceOrange()
{
  // 225, 112, 18 -> #E17012
  vtkColor3d tango = vtkColor3d(0.882352941176, 0.439215686275, 0.0705882352941);
  return tango;
}

//------------------------------------------------------------------------------
vtkColor3d vtkMRMLColors::threeDViewBlue()
{
  // 116, 131, 233 -> #7483E9
  vtkColor3d blue = vtkColor3d(116, 131, 233);
  return blue;
}

//------------------------------------------------------------------------------
vtkColor3d vtkMRMLColors::sliceGray()
{
  // #8C8C8C
  vtkColor3d gray = vtkColor3d(140, 140, 140);
  return gray;
}

//------------------------------------------------------------------------------
void vtkMRMLColors::toRGBColor(const vtkColor3d& vtkColor, double rgbColor[3])
{
  rgbColor[0] = vtkColor.Red();
  rgbColor[1] = vtkColor.Green();
  rgbColor[2] = vtkColor.Blue();
}
