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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#include "vtkOrientedImageData.h"

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>
#include <vtkMathUtilities.h>

// STD includes
#include <algorithm>

vtkStandardNewMacro(vtkOrientedImageData);

//----------------------------------------------------------------------------
vtkOrientedImageData::vtkOrientedImageData()
{
  int i=0,j=0;
  for(i=0; i<3; i++)
    {
    for(j=0; j<3; j++)
      {
      this->Directions[i][j] = (i == j) ? 1.0 : 0.0;
      }
    }
}

//----------------------------------------------------------------------------
vtkOrientedImageData::~vtkOrientedImageData() = default;

//----------------------------------------------------------------------------
void vtkOrientedImageData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << "Directions:\n";

  int i=0,j=0;
  for(i=0; i<3; i++)
    {
    for(j=0; j<3; j++)
      {
      os << indent << " " << this->Directions[i][j];
      }
      os << indent << "\n";
    }
  os << "\n";
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::ShallowCopy(vtkDataObject *dataObject)
{
  // Copy orientation information
  this->CopyDirections(dataObject);

  // Do superclass (image, origin, spacing)
  this->vtkImageData::ShallowCopy(dataObject);
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::DeepCopy(vtkDataObject *dataObject)
{
  // Copy orientation information
  this->CopyDirections(dataObject);

  // Do superclass (image, origin, spacing)
  this->vtkImageData::DeepCopy(dataObject);
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::CopyDirections(vtkDataObject *dataObject)
{
  vtkOrientedImageData *orientedImageData = vtkOrientedImageData::SafeDownCast(dataObject);
  if (orientedImageData != nullptr)
    {
    double dirs[3][3] = {{0.0, 0.0, 0.0},
                         {0.0, 0.0, 0.0},
                         {0.0, 0.0, 0.0}};
    orientedImageData->GetDirections(dirs);
    this->SetDirections(dirs);
    }
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::SetDirections(double dirs[3][3])
{
  bool isModified = false;
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      if (!vtkMathUtilities::FuzzyCompare<double>(this->Directions[i][j], dirs[i][j]))
        {
        this->Directions[i][j] = dirs[i][j];
        isModified = true;
        }
      }
    }
  if (isModified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::SetDirections(double ir, double jr, double kr,
                                              double ia, double ja, double ka,
                                              double is, double js, double ks)
{
  double dirs[3][3] = {{ir, jr, kr},
                       {ia, ja, ka},
                       {is, js, ks}};
  this->SetDirections(dirs);
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::GetDirections(double dirs[3][3])
{
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      dirs[i][j] = this->Directions[i][j];
      }
    }
}

//----------------------------------------------------------------------------
double vtkOrientedImageData::GetMinSpacing()
{
  if (this->GetSpacing() == nullptr)
    {
    return 0;
    }
  double minSpace = this->GetSpacing()[0];
  for(int i = 1; i < 3; ++i)
    {
    minSpace = std::min(this->GetSpacing()[i], minSpace);
    }
  return minSpace;
}

//----------------------------------------------------------------------------
double vtkOrientedImageData::GetMaxSpacing()
{
  if (this->GetSpacing() == nullptr)
    {
    return 0;
    }
  double maxSpace = this->GetSpacing()[0];
  for(int i = 1; i < 3; ++i)
    {
    maxSpace = std::max(this->GetSpacing()[i], maxSpace);
    }
  return maxSpace;
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::SetImageToWorldMatrix(vtkMatrix4x4* argMat)
{
  if (argMat == nullptr)
    {
    return;
    }
  vtkNew<vtkMatrix4x4> mat;
  mat->DeepCopy(argMat);
  bool isModified = false;

  // normalize direction vectors
  int col=0;
  for (col=0; col<3; col++)
    {
    double len=0.0;
    int row=0;
    for (row=0; row<3; row++)
      {
      len += mat->GetElement(row, col) * mat->GetElement(row, col);
      }
    len = sqrt(len);

    // Set spacing
    if (!vtkMathUtilities::FuzzyCompare<double>(this->Spacing[col], len))
      {
      this->Spacing[col] = len;
      isModified = true;
      }

    for (row=0; row<3; row++)
      {
      mat->SetElement(row, col,  mat->GetElement(row, col)/len);
      }
    }

  for (int row=0; row<3; row++)
    {
    for (int col=0; col<3; col++)
      {
      if (!vtkMathUtilities::FuzzyCompare<double>(this->Directions[row][col], mat->GetElement(row, col)))
        {
        this->Directions[row][col] = mat->GetElement(row, col);
        isModified = true;
        }
      }

      // Set origin
      if (!vtkMathUtilities::FuzzyCompare<double>(this->Origin[row], mat->GetElement(row, 3)))
        {
        this->Origin[row] = mat->GetElement(row, 3);
        isModified = true;
        }
    }

  // Only one Modified event
  if (isModified)
    {
    this->Modified();
    }
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::SetGeometryFromImageToWorldMatrix(vtkMatrix4x4* argMat)
{
  this->SetImageToWorldMatrix(argMat);
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::GetImageToWorldMatrix(vtkMatrix4x4* mat)
{
  if (mat == nullptr)
    {
    return;
    }

  // this is the full matrix including the spacing and origin
  mat->Identity();
  int row=0,col=0;
  for (row=0; row<3; row++)
    {
    for (col=0; col<3; col++)
      {
      mat->SetElement(row, col, this->Spacing[col] * Directions[row][col]);
      }
    mat->SetElement(row, 3, this->Origin[row]);
    }
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::GetWorldToImageMatrix(vtkMatrix4x4* mat)
{
  this->GetImageToWorldMatrix(mat);
  mat->Invert();
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::SetDirectionMatrix(vtkMatrix4x4* ijkToRASDirectionMatrix)
{
  double dirs[3][3] = {{0.0, 0.0, 0.0},
                       {0.0, 0.0, 0.0},
                       {0.0, 0.0, 0.0}};
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      dirs[i][j] = ijkToRASDirectionMatrix->Element[i][j];
      }
    }
  this->SetDirections(dirs);
}

//----------------------------------------------------------------------------
void vtkOrientedImageData::GetDirectionMatrix(vtkMatrix4x4* ijkToRASDirectionMatrix)
{
  double dirs[3][3] = {{0.0, 0.0, 0.0},
                       {0.0, 0.0, 0.0},
                       {0.0, 0.0, 0.0}};
  this->GetDirections(dirs);
  for (int i=0; i<3; i++)
    {
    for (int j=0; j<3; j++)
      {
      ijkToRASDirectionMatrix->Element[i][j] = dirs[i][j];
      }
    }
}

//---------------------------------------------------------------------------
// (Xmin, Xmax, Ymin, Ymax, Zmin, Zmax)
//---------------------------------------------------------------------------
void vtkOrientedImageData::ComputeBounds()
{
  if ( this->GetMTime() <= this->ComputeTime )
    {
    return;
    }

  // Sanity check
  const int* extent = this->Extent;
  if ( extent[0] > extent[1] ||
       extent[2] > extent[3] ||
       extent[4] > extent[5] )
    {
    // Image is empty, indicated by uninitialized bounds
    vtkMath::UninitializeBounds(this->Bounds);
    return;
    }

  // Compute oriented image corners
  vtkNew<vtkMatrix4x4> geometryMatrix;
  this->GetImageToWorldMatrix(geometryMatrix.GetPointer());

  vtkBoundingBox boundingBox;
  for (int xSide=0; xSide<2; ++xSide)
    {
    for (int ySide=0; ySide<2; ++ySide)
      {
      for (int zSide=0; zSide<2; ++zSide)
        {
        // Get corner point. Loop variables are either 0 or 1, so coordinate is
        // either low or high extent bound along that axis
        double cornerPointIJK[4] = {
          static_cast<double>(extent[xSide]),
          static_cast<double>(extent[2+ySide]),
          static_cast<double>(extent[4+zSide]),
          1.0 };

        // Use voxel corner as boundary, not voxel center:
        cornerPointIJK[0] += (xSide == 0 ? -0.5 : 0.5);
        cornerPointIJK[1] += (ySide == 0 ? -0.5 : 0.5);
        cornerPointIJK[2] += (zSide == 0 ? -0.5 : 0.5);

        // Transform IJK coordinate to get the world coordinate
        double cornerPointWorld[4] = {0.0,0.0,0.0,0.0};
        geometryMatrix->MultiplyPoint(cornerPointIJK, cornerPointWorld);

        // Determine bounds based on current corner point
        boundingBox.AddPoint(cornerPointWorld);
        }
      }
    }

  boundingBox.GetBounds(this->Bounds);
  this->ComputeTime.Modified();
}

//---------------------------------------------------------------------------
bool vtkOrientedImageData::IsEmpty()
{
  // Empty if extent is uninitialized or otherwise invalid
  if (this->Extent[0] > this->Extent[1] || this->Extent[2] > this->Extent[3] || this->Extent[4] > this->Extent[5])
    {
    // empty
    return true;
    }
  return false;
}
