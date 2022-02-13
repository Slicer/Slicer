/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkMRMLSliceIntersectionInteractionRepresentationHelper.h"


#include <deque>
#define _USE_MATH_DEFINES
#include <math.h>

#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceCompositeNode.h"

#include "vtkActor2D.h"
#include "vtkArcSource.h"
#include "vtkAppendPolyData.h"
#include "vtkAssemblyPath.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCellArray.h"
#include "vtkCommand.h"
#include "vtkConeSource.h"
#include "vtkCoordinate.h"
#include "vtkCursor2D.h"
#include "vtkCylinderSource.h"
#include "vtkGlyph2D.h"
#include "vtkInteractorObserver.h"
#include "vtkLeaderActor2D.h"
#include "vtkLine.h"
#include "vtkLineSource.h"
#include "vtkMath.h"
#include "vtkMatrix3x3.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkPoints.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkProperty2D.h"
#include "vtkPlane.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTextMapper.h"
#include "vtkTextProperty.h"
#include "vtkTubeFilter.h"
#include "vtkWindow.h"

// MRML includes
#include <vtkMRMLInteractionEventData.h>

// Handles
static const double SLICEOFFSET_HANDLE_DEFAULT_POSITION[3] = { 0.0,0.0,0.0 };
static const double SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[3] = { 0.0,1.0,0.0 };

vtkStandardNewMacro(vtkMRMLSliceIntersectionInteractionRepresentationHelper);

//----------------------------------------------------------------------
vtkMRMLSliceIntersectionInteractionRepresentationHelper::vtkMRMLSliceIntersectionInteractionRepresentationHelper()
{
}

//----------------------------------------------------------------------
vtkMRMLSliceIntersectionInteractionRepresentationHelper::~vtkMRMLSliceIntersectionInteractionRepresentationHelper()
{
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentationHelper::PrintSelf(ostream & os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentationHelper::GetSliceViewBoundariesXY(vtkMRMLSliceNode* sliceNode, double* sliceViewBounds)
{
  // Get FOV of current slice node in mm
  char* sliceNodeName = sliceNode->GetName();
  double sliceFOVMm[3] = { 0.0,0.0,0.0 };
  sliceNode->GetFieldOfView(sliceFOVMm);

  // Get XYToRAS and RASToXY transform matrices
  vtkMatrix4x4* currentXYToRAS = sliceNode->GetXYToRAS();
  vtkNew<vtkMatrix4x4> currentRASToXY;
  vtkMatrix4x4::Invert(currentXYToRAS, currentRASToXY);

  // Get slice view axes in RAS
  double sliceOrigin[4] = { 0.0,0.0,0.0,1.0 };
  double slicePointAxisX[4] = { 100.0,0.0,0.0,1.0 };
  double slicePointAxisY[4] = { 0.0,100.0,0.0,1.0 };
  currentXYToRAS->MultiplyPoint(sliceOrigin, sliceOrigin);
  currentXYToRAS->MultiplyPoint(slicePointAxisX, slicePointAxisX);
  currentXYToRAS->MultiplyPoint(slicePointAxisY, slicePointAxisY);
  double sliceAxisX[3] = { slicePointAxisX[0] - sliceOrigin[0],
                           slicePointAxisX[1] - sliceOrigin[1],
                           slicePointAxisX[2] - sliceOrigin[2] };
  double sliceAxisY[3] = { slicePointAxisY[0] - sliceOrigin[0],
                           slicePointAxisY[1] - sliceOrigin[1],
                           slicePointAxisY[2] - sliceOrigin[2] };
  vtkMath::Normalize(sliceAxisX);
  vtkMath::Normalize(sliceAxisY);

  // Calculate corners of FOV in RAS coordinate system
  double bottomLeftCornerRAS[4] = { 0.0,0.0,0.0,1.0 };
  double topLeftCornerRAS[4] = { 0.0,0.0,0.0,1.0 };
  double bottomRightCornerRAS[4] = { 0.0,0.0,0.0,1.0 };
  double topRightCornerRAS[4] = { 0.0,0.0,0.0,1.0 };

  // Get slice view corners RAS
  bottomLeftCornerRAS[0] = sliceOrigin[0];
  bottomLeftCornerRAS[1] = sliceOrigin[1];
  bottomLeftCornerRAS[2] = sliceOrigin[2];
  topLeftCornerRAS[0] = sliceOrigin[0] + sliceAxisY[0] * sliceFOVMm[1];
  topLeftCornerRAS[1] = sliceOrigin[1] + sliceAxisY[1] * sliceFOVMm[1];
  topLeftCornerRAS[2] = sliceOrigin[2] + sliceAxisY[2] * sliceFOVMm[1];
  bottomRightCornerRAS[0] = sliceOrigin[0] + sliceAxisX[0] * sliceFOVMm[0];
  bottomRightCornerRAS[1] = sliceOrigin[1] + sliceAxisX[1] * sliceFOVMm[0];
  bottomRightCornerRAS[2] = sliceOrigin[2] + sliceAxisX[2] * sliceFOVMm[0];
  topRightCornerRAS[0] = sliceOrigin[0] + sliceAxisY[0] * sliceFOVMm[1] + sliceAxisX[0] * sliceFOVMm[0];
  topRightCornerRAS[1] = sliceOrigin[1] + sliceAxisY[1] * sliceFOVMm[1] + sliceAxisX[1] * sliceFOVMm[0];
  topRightCornerRAS[2] = sliceOrigin[2] + sliceAxisY[2] * sliceFOVMm[1] + sliceAxisX[2] * sliceFOVMm[0];

  // Calculate corners of FOV in XY coordinate system
  double bottomLeftCornerXY[4] = { 0.0,0.0,0.0,1.0 };
  double topLeftCornerXY[4] = { 0.0,0.0,0.0,1.0 };
  double bottomRightCornerXY[4] = { 0.0,0.0,0.0,1.0 };
  double topRightCornerXY[4] = { 0.0,0.0,0.0,1.0 };
  currentRASToXY->MultiplyPoint(bottomLeftCornerRAS, bottomLeftCornerXY);
  currentRASToXY->MultiplyPoint(topLeftCornerRAS, topLeftCornerXY);
  currentRASToXY->MultiplyPoint(bottomRightCornerRAS, bottomRightCornerXY);
  currentRASToXY->MultiplyPoint(topRightCornerRAS, topRightCornerXY);

  // Get slice view range XY
  sliceViewBounds[0] = bottomLeftCornerXY[0]; // Min value of X
  sliceViewBounds[1] = topRightCornerXY[0]; // Max value of X
  sliceViewBounds[2] = bottomLeftCornerXY[1]; //  Min value of Y
  sliceViewBounds[3] = topRightCornerXY[1]; //  Max value of Y
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentationHelper::GetIntersectionWithSliceViewBoundaries(double* pointA, double* pointB,
      double* sliceViewBounds, double* intersectionPoint)
{
    // Get line equation -> y = slope * x + intercept
    double xA = pointA[0];
    double yA = pointA[1];
    double xB = pointB[0];
    double yB = pointB[1];
    double dx, dy, slope, intercept;
    dx = xB - xA;
    dy = yB - yA;
    slope = dy / dx;
    intercept = yA - slope * xA;

    // Get line bounding box
    double lineBounds[4] = { std::min(xA, xB), std::max(xA, xB), std::min(yA, yB), std::max(yA, yB) };

    // Slice view bounds
    double xMin = sliceViewBounds[0];
    double xMax = sliceViewBounds[1];
    double yMin = sliceViewBounds[2];
    double yMax = sliceViewBounds[3];

    // Get intersection point using line equation
    double x0, y0;
    if ((xMin > lineBounds[0]) && (xMin < lineBounds[1]))
      {
      y0 = slope * xMin + intercept;
      if ((y0 > yMin) && (y0 < yMax))
        {
        intersectionPoint[0] = xMin;
        intersectionPoint[1] = y0;
        return;
        }
      }
    if ((xMax > lineBounds[0]) && (xMax < lineBounds[1]))
      {
      y0 = slope * xMax + intercept;
      if ((y0 > yMin) && (y0 < yMax))
        {
        intersectionPoint[0] = xMax;
        intersectionPoint[1] = y0;
        return;
        }
      }
    if ((yMin > lineBounds[2]) && (yMin < lineBounds[3]))
      {
      if (std::isfinite(slope)) // check if slope is finite
        {
        x0 = (yMin - intercept)/slope;
        if ((x0 > xMin) && (x0 < xMax))
          {
          intersectionPoint[0] = x0;
          intersectionPoint[1] = yMin;
          return;
          }
        }
      else // infinite slope = vertical line
        {
          intersectionPoint[0] = lineBounds[0]; // or lineBounds[1] (if the line is vertical, then both points A and B have the same value of X)
          intersectionPoint[1] = yMin;
          return;
        }
      }
    if ((yMax > lineBounds[2]) && (yMax < lineBounds[3]))
      {
      if (std::isfinite(slope)) // check if slope is finite
        {
        x0 = (yMax - intercept)/slope;
        if ((x0 > xMin) && (x0 < xMax))
          {
          intersectionPoint[0] = x0;
          intersectionPoint[1] = yMax;
          return;
          }
        }
      else // infinite slope = vertical line
        {
          intersectionPoint[0] = lineBounds[0]; // or lineBounds[1] (if the line is vertical, then both points A and B have the same value of X)
          intersectionPoint[1] = yMax;
          return;
        }
      }
    return;
}

//---------------------------------------------------------------------------
int vtkMRMLSliceIntersectionInteractionRepresentationHelper::IntersectWithFinitePlane(double n[3], double o[3],
  double pOrigin[3], double px[3], double py[3],
  double x0[3], double x1[3])
{
  // Since we are dealing with convex shapes, if there is an intersection a
  // single line is produced as output. So all this is necessary is to
  // intersect the four bounding lines of the finite line and find the two
  // intersection points.
  int numInts = 0;
  double t, * x = x0;
  double xr0[3], xr1[3];

  // First line
  xr0[0] = pOrigin[0];
  xr0[1] = pOrigin[1];
  xr0[2] = pOrigin[2];
  xr1[0] = px[0];
  xr1[1] = px[1];
  xr1[2] = px[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }

  // Second line
  xr1[0] = py[0];
  xr1[1] = py[1];
  xr1[2] = py[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // Third line
  xr0[0] = -pOrigin[0] + px[0] + py[0];
  xr0[1] = -pOrigin[1] + px[1] + py[1];
  xr0[2] = -pOrigin[2] + px[2] + py[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    x = x1;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // Fourth and last line
  xr1[0] = px[0];
  xr1[1] = px[1];
  xr1[2] = px[2];
  if (vtkPlane::IntersectWithLine(xr0, xr1, n, o, t, x))
    {
    numInts++;
    }
  if (numInts == 2)
    {
    return 1;
    }

  // No intersection has occurred, or a single degenerate point
  return 0;
}

//----------------------------------------------------------------------
int vtkMRMLSliceIntersectionInteractionRepresentationHelper::GetLineTipsFromIntersectingSliceNode(vtkMRMLSliceNode* intersectingSliceNode,
    vtkMatrix4x4* intersectingXYToXY, double intersectionOuterLineTip1[3], double intersectionOuterLineTip2[3])
{
  // Define current slice plane
  double slicePlaneNormal[3] = { 0.,0.,1. };
  double slicePlaneOrigin[3] = { 0., 0., 0. };

  // Define slice size dimensions
  int* intersectingSliceSizeDimensions = intersectingSliceNode->GetDimensions();
  double intersectingPlaneOrigin[4] = { 0, 0, 0, 1 };
  double intersectingPlaneX[4] = { double(intersectingSliceSizeDimensions[0]), 0., 0., 1. };
  double intersectingPlaneY[4] = { 0., double(intersectingSliceSizeDimensions[1]), 0., 1. };
  intersectingXYToXY->MultiplyPoint(intersectingPlaneOrigin, intersectingPlaneOrigin);
  intersectingXYToXY->MultiplyPoint(intersectingPlaneX, intersectingPlaneX);
  intersectingXYToXY->MultiplyPoint(intersectingPlaneY, intersectingPlaneY);

  // Compute intersection
  int intersectionFound = this->IntersectWithFinitePlane(slicePlaneNormal, slicePlaneOrigin,
    intersectingPlaneOrigin, intersectingPlaneX, intersectingPlaneY, intersectionOuterLineTip1, intersectionOuterLineTip2);

  return intersectionFound;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentationHelper::ComputeHandleToWorldTransformMatrix(double handlePosition[2], double handleOrientation[2],
  vtkMatrix4x4* handleToWorldTransformMatrix)
{
  // Reset handle to world transform
  handleToWorldTransformMatrix->Identity();

  // Get rotation matrix
  double handleOrientationDefault[2] = { SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[0],
                                         SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION [1]};
  this->RotationMatrixFromVectors(handleOrientationDefault, handleOrientation, handleToWorldTransformMatrix);

  // Add translation to matrix
  double handleTranslation[2] = { handlePosition[0] - SLICEOFFSET_HANDLE_DEFAULT_POSITION[0],
                                  handlePosition[1] - SLICEOFFSET_HANDLE_DEFAULT_POSITION[1]};
  handleToWorldTransformMatrix->SetElement(0, 3, handleTranslation[0]); // Translation X
  handleToWorldTransformMatrix->SetElement(1, 3, handleTranslation[1]); // Translation Y
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentationHelper::RotationMatrixFromVectors(double vector1[2], double vector2[2], vtkMatrix4x4* rotationMatrixHom)
{
  // 3D vectors
  double vector1_3D[3] = { vector1[0], vector1[1], 0.0};
  double vector2_3D[3] = { vector2[0], vector2[1], 0.0 };

  // Normalize input vectos
  vtkMath::Normalize(vector1_3D);
  vtkMath::Normalize(vector2_3D);

  // Cross and dot products
  double v[3];
  vtkMath::Cross(vector1_3D, vector2_3D, v);
  double c = vtkMath::Dot(vector1_3D, vector2_3D);
  double s = vtkMath::Norm(v);

  // Compute rotation matrix
  if (s == 0.0) // If vectors are aligned (i.e., cross product = 0)
    {
    if (c > 0.0) // Same direction
      {
      rotationMatrixHom->Identity();
      }
    else // Opposite direction
      {
      vtkNew<vtkTransform> transform;
      transform->RotateZ(180); // invert direction
      rotationMatrixHom->SetElement(0, 0, transform->GetMatrix()->GetElement(0, 0));
      rotationMatrixHom->SetElement(0, 1, transform->GetMatrix()->GetElement(0, 1));
      rotationMatrixHom->SetElement(0, 2, transform->GetMatrix()->GetElement(0, 2));
      rotationMatrixHom->SetElement(0, 3, 0.0);
      rotationMatrixHom->SetElement(1, 0, transform->GetMatrix()->GetElement(1, 0));
      rotationMatrixHom->SetElement(1, 1, transform->GetMatrix()->GetElement(1, 1));
      rotationMatrixHom->SetElement(1, 2, transform->GetMatrix()->GetElement(1, 2));
      rotationMatrixHom->SetElement(1, 3, 0.0);
      rotationMatrixHom->SetElement(2, 0, transform->GetMatrix()->GetElement(2, 0));
      rotationMatrixHom->SetElement(2, 1, transform->GetMatrix()->GetElement(2, 1));
      rotationMatrixHom->SetElement(2, 2, transform->GetMatrix()->GetElement(2, 2));
      rotationMatrixHom->SetElement(2, 3, 0.0);
      rotationMatrixHom->SetElement(3, 0, 0.0);
      rotationMatrixHom->SetElement(3, 1, 0.0);
      rotationMatrixHom->SetElement(3, 2, 0.0);
      rotationMatrixHom->SetElement(3, 3, 1.0);
      }
    }
  else // If vectors are not aligned
    {
    vtkNew<vtkMatrix3x3> rotationMatrix;
    vtkNew<vtkMatrix3x3> identityMatrix;
    vtkNew<vtkMatrix3x3> kmat;
    kmat->SetElement(0, 0, 0.0);
    kmat->SetElement(0, 1, -v[2]);
    kmat->SetElement(0, 2, v[1]);
    kmat->SetElement(1, 0, v[2]);
    kmat->SetElement(1, 1, 0.0);
    kmat->SetElement(1, 2, -v[0]);
    kmat->SetElement(2, 0, -v[1]);
    kmat->SetElement(2, 1, v[0]);
    kmat->SetElement(2, 2, 0.0);
    vtkNew<vtkMatrix3x3> kmat2;
    vtkMatrix3x3::Multiply3x3(kmat, kmat, kmat2);
    vtkNew<vtkMatrix3x3> kmat2x;
    rotationMatrix->SetElement(0, 0, identityMatrix->GetElement(0, 0) + kmat->GetElement(0, 0) + kmat2->GetElement(0, 0) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(0, 1, identityMatrix->GetElement(0, 1) + kmat->GetElement(0, 1) + kmat2->GetElement(0, 1) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(0, 2, identityMatrix->GetElement(0, 2) + kmat->GetElement(0, 2) + kmat2->GetElement(0, 2) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(1, 0, identityMatrix->GetElement(1, 0) + kmat->GetElement(1, 0) + kmat2->GetElement(1, 0) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(1, 1, identityMatrix->GetElement(1, 1) + kmat->GetElement(1, 1) + kmat2->GetElement(1, 1) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(1, 2, identityMatrix->GetElement(1, 2) + kmat->GetElement(1, 2) + kmat2->GetElement(1, 2) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(2, 0, identityMatrix->GetElement(2, 0) + kmat->GetElement(2, 0) + kmat2->GetElement(2, 0) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(2, 1, identityMatrix->GetElement(2, 1) + kmat->GetElement(2, 1) + kmat2->GetElement(2, 1) * ((1 - c) / (pow(s, 2.0))));
    rotationMatrix->SetElement(2, 2, identityMatrix->GetElement(2, 2) + kmat->GetElement(2, 2) + kmat2->GetElement(2, 2) * ((1 - c) / (pow(s, 2.0))));

    // Convert to 4x4 matrix
    rotationMatrixHom->SetElement(0, 0, rotationMatrix->GetElement(0, 0));
    rotationMatrixHom->SetElement(0, 1, rotationMatrix->GetElement(0, 1));
    rotationMatrixHom->SetElement(0, 2, rotationMatrix->GetElement(0, 2));
    rotationMatrixHom->SetElement(0, 3, 0.0);
    rotationMatrixHom->SetElement(1, 0, rotationMatrix->GetElement(1, 0));
    rotationMatrixHom->SetElement(1, 1, rotationMatrix->GetElement(1, 1));
    rotationMatrixHom->SetElement(1, 2, rotationMatrix->GetElement(1, 2));
    rotationMatrixHom->SetElement(1, 3, 0.0);
    rotationMatrixHom->SetElement(2, 0, rotationMatrix->GetElement(2, 0));
    rotationMatrixHom->SetElement(2, 1, rotationMatrix->GetElement(2, 1));
    rotationMatrixHom->SetElement(2, 2, rotationMatrix->GetElement(2, 2));
    rotationMatrixHom->SetElement(2, 3, 0.0);
    rotationMatrixHom->SetElement(3, 0, 0.0);
    rotationMatrixHom->SetElement(3, 1, 0.0);
    rotationMatrixHom->SetElement(3, 2, 0.0);
    rotationMatrixHom->SetElement(3, 3, 1.0);
    }
}
