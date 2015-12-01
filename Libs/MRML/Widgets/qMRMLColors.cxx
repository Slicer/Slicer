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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes

// qMRML includes
#include "qMRMLColors.h"

// VTK includes
#include <vtkColor.h>
#include "vtkMRMLColors.h"
#include <vtkVersion.h>

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceRed()
{
  return qMRMLColors::fromVTKColor(vtkMRMLColors::sliceRed());
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceGreen()
{
  return qMRMLColors::fromVTKColor(vtkMRMLColors::sliceGreen());
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceYellow()
{
  return qMRMLColors::fromVTKColor(vtkMRMLColors::sliceYellow());
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceOrange()
{
  return qMRMLColors::fromVTKColor(vtkMRMLColors::sliceOrange());
}

//------------------------------------------------------------------------------
QColor qMRMLColors::threeDViewBlue()
{
  return qMRMLColors::fromVTKColor(vtkMRMLColors::threeDViewBlue());
}

//------------------------------------------------------------------------------
QColor qMRMLColors::sliceGray()
{
  return qMRMLColors::fromVTKColor(vtkMRMLColors::sliceGray());
}

//------------------------------------------------------------------------------
QColor qMRMLColors::fromVTKColor(const vtkColor3d& vtkColor)
{
  return QColor::fromRgb(vtkColor.GetRed(), vtkColor.GetGreen(), vtkColor.GetBlue());
}
