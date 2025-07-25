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

#include "vtkMRMLSliceIntersectionInteractionRepresentation.h"

#include "vtkMRMLSliceIntersectionInteractionRepresentationHelper.h"

#include <deque>

#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLSliceDisplayNode.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLViewNode.h"

#include "vtkActor2D.h"
#include "vtkArcSource.h"
#include "vtkAppendPolyData.h"
#include "vtkCallbackCommand.h"
#include "vtkCamera.h"
#include "vtkCommand.h"
#include "vtkConeSource.h"
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
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSphereSource.h"
#include "vtkTransform.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTubeFilter.h"
#include "vtkWindow.h"

// MRML includes
#include <vtkMRMLInteractionEventData.h>

// Handles type
enum
{
  Arrows = 0,
  Circles = 1,
};

// Settings
static const int HANDLES_TYPE = Arrows;
static const double OPACITY_RANGE = 1000.0;
static const double FOV_HANDLES_MARGIN = 0.03;         // 3% margin
static const double HIDE_INTERSECTION_GAP_SIZE = 0.05; // 5.0% of the slice view width
static const double INTERACTION_SIZE_PIXELS = 20.0;
static const double HANDLES_MIN_LINE_LENGTH = 50.0;
static const double THICK_SLAB_HANDLES_MIN_LINE_LENGTH = 50.0;

// Intersection line
static const double INTERSECTION_LINE_RESOLUTION = 50; // default = 8

// Thick slab line
static const double THICK_SLAB_LINE_RESOLUTION = 50; // default = 8

// Handles
static const double HANDLES_CIRCLE_THETA_RESOLUTION = 100; // default = 8
static const double HANDLES_CIRCLE_PHI_RESOLUTION = 100;   // default = 8
static const double SLICEOFFSET_HANDLE_DEFAULT_POSITION[3] = { 0.0, 0.0, 0.0 };
static const double SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[3] = { 0.0, 1.0, 0.0 };
static const double SLICEOFFSET_HANDLE_CIRCLE_RADIUS = 7.0;
static const double SLICEOFFSET_HANDLE_ARROW_RADIUS = 3.0;
static const double SLICEOFFSET_HANDLE_ARROW_LENGTH = 60.0;
static const double SLICEOFFSET_HANDLE_ARROW_TIP_ANGLE = 27; // degrees
static const double THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_POSITION[3] = { 0.0, 0.0, 0.0 };
static const double THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_ORIENTATION[3] = { 0.0, 1.0, 0.0 };
static const double THICK_SLAB_TRANSLATION_HANDLE_ARROW_RADIUS = 3.0;
static const double THICK_SLAB_TRANSLATION_HANDLE_ARROW_LENGTH = 60.0;
static const double THICK_SLAB_TRANSLATION_HANDLE_ARROW_TIP_ANGLE = 27; // degrees
static const double ROTATION_HANDLE_DEFAULT_POSITION[3] = { 0.0, 0.0, 0.0 };
static const double ROTATION_HANDLE_DEFAULT_ORIENTATION[3] = { 0.0, 1.0, 0.0 };
static const double ROTATION_HANDLE_CIRCLE_RADIUS = 10.0;
static const double ROTATION_HANDLE_ARROW_RADIUS = 3.0;
static const double ROTATION_HANDLE_ARROW_LENGTH = 60.0;
static const double ROTATION_HANDLE_ARROW_TIP_ANGLE = 27; // degrees
static const double TRANSLATION_HANDLE_OUTER_RADIUS = 9.0;
static const double TRANSLATION_HANDLE_INNER_RADIUS = 7.0;

vtkStandardNewMacro(vtkMRMLSliceIntersectionInteractionRepresentation);
vtkCxxSetObjectMacro(vtkMRMLSliceIntersectionInteractionRepresentation, MRMLApplicationLogic, vtkMRMLApplicationLogic);

class SliceIntersectionInteractionDisplayPipeline
{
public:
  //----------------------------------------------------------------------
  SliceIntersectionInteractionDisplayPipeline()
  {
    // Intersection line 1 (first half)
    this->IntersectionLine1 = vtkSmartPointer<vtkLineSource>::New();
    this->IntersectionLine1->SetResolution(INTERSECTION_LINE_RESOLUTION);
    this->IntersectionLine1->Update();
    this->IntersectionLine1Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->IntersectionLine1Property = vtkSmartPointer<vtkProperty2D>::New();
    this->IntersectionLine1Actor = vtkSmartPointer<vtkActor2D>::New();
    this->IntersectionLine1Actor->SetVisibility(false); // invisible until slice node is set
    this->IntersectionLine1Mapper->SetInputConnection(this->IntersectionLine1->GetOutputPort());
    this->IntersectionLine1Actor->SetMapper(this->IntersectionLine1Mapper);
    this->IntersectionLine1Actor->SetProperty(this->IntersectionLine1Property);

    // Intersection line 2 (second half)
    this->IntersectionLine2 = vtkSmartPointer<vtkLineSource>::New();
    this->IntersectionLine2->SetResolution(INTERSECTION_LINE_RESOLUTION);
    this->IntersectionLine2->Update();
    this->IntersectionLine2Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->IntersectionLine2Property = vtkSmartPointer<vtkProperty2D>::New();
    this->IntersectionLine2Actor = vtkSmartPointer<vtkActor2D>::New();
    this->IntersectionLine2Actor->SetVisibility(false); // invisible until slice node is set
    this->IntersectionLine2Mapper->SetInputConnection(this->IntersectionLine2->GetOutputPort());
    this->IntersectionLine2Actor->SetMapper(this->IntersectionLine2Mapper);
    this->IntersectionLine2Actor->SetProperty(this->IntersectionLine2Property);

    // First half of the first thick slab line
    this->ThickSlabLine1FirstHalf = vtkSmartPointer<vtkLineSource>::New();
    this->ThickSlabLine1FirstHalf->SetResolution(THICK_SLAB_LINE_RESOLUTION);
    this->ThickSlabLine1FirstHalf->Update();
    this->ThickSlabLine1FirstHalfMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine1FirstHalfProperty = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine1FirstHalfActor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine1FirstHalfActor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine1FirstHalfMapper->SetInputConnection(this->ThickSlabLine1FirstHalf->GetOutputPort());
    this->ThickSlabLine1FirstHalfActor->SetMapper(this->ThickSlabLine1FirstHalfMapper);
    this->ThickSlabLine1FirstHalfActor->SetProperty(this->ThickSlabLine1FirstHalfProperty);

    // Second half of the first thick slab line
    this->ThickSlabLine1SecondHalf = vtkSmartPointer<vtkLineSource>::New();
    this->ThickSlabLine1SecondHalf->SetResolution(THICK_SLAB_LINE_RESOLUTION);
    this->ThickSlabLine1SecondHalf->Update();
    this->ThickSlabLine1SecondHalfMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine1SecondHalfProperty = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine1SecondHalfActor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine1SecondHalfActor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine1SecondHalfMapper->SetInputConnection(this->ThickSlabLine1SecondHalf->GetOutputPort());
    this->ThickSlabLine1SecondHalfActor->SetMapper(this->ThickSlabLine1SecondHalfMapper);
    this->ThickSlabLine1SecondHalfActor->SetProperty(this->ThickSlabLine1SecondHalfProperty);

    // First half of the second thick slab line
    this->ThickSlabLine2FirstHalf = vtkSmartPointer<vtkLineSource>::New();
    this->ThickSlabLine2FirstHalf->SetResolution(THICK_SLAB_LINE_RESOLUTION);
    this->ThickSlabLine2FirstHalf->Update();
    this->ThickSlabLine2FirstHalfMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine2FirstHalfProperty = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine2FirstHalfActor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine2FirstHalfActor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine2FirstHalfMapper->SetInputConnection(this->ThickSlabLine2FirstHalf->GetOutputPort());
    this->ThickSlabLine2FirstHalfActor->SetMapper(this->ThickSlabLine2FirstHalfMapper);
    this->ThickSlabLine2FirstHalfActor->SetProperty(this->ThickSlabLine2FirstHalfProperty);

    // Second half of the second thick slab line
    this->ThickSlabLine2SecondHalf = vtkSmartPointer<vtkLineSource>::New();
    this->ThickSlabLine2SecondHalf->SetResolution(THICK_SLAB_LINE_RESOLUTION);
    this->ThickSlabLine2SecondHalf->Update();
    this->ThickSlabLine2SecondHalfMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine2SecondHalfProperty = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine2SecondHalfActor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine2SecondHalfActor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine2SecondHalfMapper->SetInputConnection(this->ThickSlabLine2SecondHalf->GetOutputPort());
    this->ThickSlabLine2SecondHalfActor->SetMapper(this->ThickSlabLine2SecondHalfMapper);
    this->ThickSlabLine2SecondHalfActor->SetProperty(this->ThickSlabLine2SecondHalfProperty);

    // Center sphere
    this->TranslationOuterHandle = vtkSmartPointer<vtkSphereSource>::New();
    this->TranslationOuterHandle->SetRadius(TRANSLATION_HANDLE_OUTER_RADIUS);
    this->TranslationOuterHandle->SetThetaResolution(HANDLES_CIRCLE_THETA_RESOLUTION);
    this->TranslationOuterHandle->SetPhiResolution(HANDLES_CIRCLE_PHI_RESOLUTION);
    this->TranslationOuterHandle->Update();
    this->TranslationOuterHandleMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->TranslationOuterHandleProperty = vtkSmartPointer<vtkProperty2D>::New();
    this->TranslationOuterHandleActor = vtkSmartPointer<vtkActor2D>::New();
    this->TranslationOuterHandleActor->SetVisibility(false); // invisible until slice node is set
    this->TranslationOuterHandleMapper->SetInputConnection(this->TranslationOuterHandle->GetOutputPort());
    this->TranslationOuterHandleActor->SetMapper(this->TranslationOuterHandleMapper);
    this->TranslationOuterHandleActor->SetProperty(this->TranslationOuterHandleProperty);
    this->TranslationInnerHandle = vtkSmartPointer<vtkSphereSource>::New();
    this->TranslationInnerHandle->SetRadius(TRANSLATION_HANDLE_INNER_RADIUS);
    this->TranslationInnerHandle->SetThetaResolution(HANDLES_CIRCLE_THETA_RESOLUTION);
    this->TranslationInnerHandle->SetPhiResolution(HANDLES_CIRCLE_PHI_RESOLUTION);
    this->TranslationInnerHandle->Update();
    this->TranslationInnerHandleMapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->TranslationInnerHandleProperty = vtkSmartPointer<vtkProperty2D>::New();
    this->TranslationInnerHandleActor = vtkSmartPointer<vtkActor2D>::New();
    this->TranslationInnerHandleActor->SetVisibility(false); // invisible until slice node is set
    this->TranslationInnerHandleMapper->SetInputConnection(this->TranslationInnerHandle->GetOutputPort());
    this->TranslationInnerHandleActor->SetMapper(this->TranslationInnerHandleMapper);
    this->TranslationInnerHandleActor->SetProperty(this->TranslationInnerHandleProperty);

    // Initialize handles
    this->CreateRotationHandles();
    this->CreateSliceOffsetHandles();
    this->CreateThickSlabTranslationHandles();
    this->SetIntersectionHandlesVisibility(false);
    this->SetThickSlabHandlesVisibility(false);
    this->NeedToRender = true;

    // Handle points
    this->RotationHandlePoints = vtkSmartPointer<vtkPolyData>::New();
    this->TranslationHandlePoints = vtkSmartPointer<vtkPolyData>::New();
    this->SliceOffsetHandlePoints = vtkSmartPointer<vtkPolyData>::New();
    this->ThickSlabHandlePoints = vtkSmartPointer<vtkPolyData>::New();
  }

  //----------------------------------------------------------------------
  virtual ~SliceIntersectionInteractionDisplayPipeline() { this->SetAndObserveSliceLogic(nullptr, nullptr); }

  //----------------------------------------------------------------------
  void CreateRotationHandles()
  {
    // Create list of points to store position of handles
    this->RotationHandle1Points = vtkSmartPointer<vtkPoints>::New();
    this->RotationHandle2Points = vtkSmartPointer<vtkPoints>::New();

    // We don't know if there is enough space, will set it later.
    this->RotationHandle1Displayable = false;
    this->RotationHandle2Displayable = false;

    // Handle default position and orientation
    double handleOriginDefault[3] = { ROTATION_HANDLE_DEFAULT_POSITION[0], ROTATION_HANDLE_DEFAULT_POSITION[1], ROTATION_HANDLE_DEFAULT_POSITION[2] };
    double handleOrientationDefault[3] = { ROTATION_HANDLE_DEFAULT_ORIENTATION[0], ROTATION_HANDLE_DEFAULT_ORIENTATION[1], ROTATION_HANDLE_DEFAULT_ORIENTATION[2] };
    double handleOrientationDefaultInv[3] = { -ROTATION_HANDLE_DEFAULT_ORIENTATION[0], -ROTATION_HANDLE_DEFAULT_ORIENTATION[1], -ROTATION_HANDLE_DEFAULT_ORIENTATION[2] };
    double handleOrientationPerpendicular[3] = { ROTATION_HANDLE_DEFAULT_ORIENTATION[1], -ROTATION_HANDLE_DEFAULT_ORIENTATION[0], ROTATION_HANDLE_DEFAULT_ORIENTATION[2] };

    if (HANDLES_TYPE == Arrows)
    {
      // Define cone size
      double coneAngleRad = (ROTATION_HANDLE_ARROW_TIP_ANGLE * vtkMath::Pi()) / 180.0;
      double coneRadius = 2 * ROTATION_HANDLE_ARROW_RADIUS;
      double coneLength = coneRadius / tan(coneAngleRad);

      // Define arc points
      double arcTipR[3] = { handleOrientationDefault[0], handleOrientationDefault[1], handleOrientationDefault[2] };
      vtkMath::MultiplyScalar(arcTipR, ROTATION_HANDLE_ARROW_LENGTH / 2.0);
      double arcTipL[3] = { handleOrientationDefaultInv[0], handleOrientationDefaultInv[1], handleOrientationDefaultInv[2] };
      vtkMath::MultiplyScalar(arcTipL, ROTATION_HANDLE_ARROW_LENGTH / 2.0);
      double arcCenter[3] = { handleOrientationPerpendicular[0], handleOrientationPerpendicular[1], handleOrientationPerpendicular[2] };
      vtkMath::MultiplyScalar(arcCenter, -ROTATION_HANDLE_ARROW_LENGTH / 3.0);

      // Translate arc to origin
      double arcRadiusVector[3] = { arcTipR[0] - arcCenter[0], arcTipR[1] - arcCenter[1], arcTipR[2] - arcCenter[2] };
      double arcRadius = vtkMath::Norm(arcRadiusVector);
      double arcChordVector[3] = { arcTipR[0] - arcTipL[0], arcTipR[1] - arcTipL[1], arcTipR[2] - arcTipL[2] };
      double arcChord = vtkMath::Norm(arcChordVector);
      double arcSagitta1 = arcRadius + sqrt(pow(arcRadius, 2) - pow(arcChord / 2, 2));
      double arcSagitta2 = arcRadius - sqrt(pow(arcRadius, 2) - pow(arcChord / 2, 2));
      double arcSagitta[3] = { handleOrientationPerpendicular[0], handleOrientationPerpendicular[1], handleOrientationPerpendicular[2] };
      if (arcSagitta1 > arcSagitta2) // keep shortest sagitta
      {
        vtkMath::MultiplyScalar(arcSagitta, arcSagitta2);
      }
      else // keep shortest sagitta
      {
        vtkMath::MultiplyScalar(arcSagitta, arcSagitta1);
      }
      arcTipR[0] = arcTipR[0] - arcSagitta[0];
      arcTipR[1] = arcTipR[1] - arcSagitta[1];
      arcTipR[2] = arcTipR[2] - arcSagitta[2];
      arcTipL[0] = arcTipL[0] - arcSagitta[0];
      arcTipL[1] = arcTipL[1] - arcSagitta[1];
      arcTipL[2] = arcTipL[2] - arcSagitta[2];
      arcCenter[0] = arcCenter[0] - arcSagitta[0];
      arcCenter[1] = arcCenter[1] - arcSagitta[1];
      arcCenter[2] = arcCenter[2] - arcSagitta[2];

      // Define intermediate points for interaction
      double arcMidR[3] = { arcTipR[0] / 2.0, arcTipR[1] / 2.0, arcTipR[2] / 2.0 };
      double arcMidL[3] = { arcTipL[0] / 2.0, arcTipL[1] / 2.0, arcTipL[2] / 2.0 };

      // Define arc tangent vectors
      double arcRadiusVectorR[3] = { arcTipR[0] - arcCenter[0], arcTipR[1] - arcCenter[1], arcTipR[2] - arcCenter[2] };
      double arcRadiusVectorL[3] = { arcTipL[0] - arcCenter[0], arcTipL[1] - arcCenter[1], arcTipL[2] - arcCenter[2] };
      double arcTangentVectorR[3] = { -arcRadiusVectorR[1], arcRadiusVectorR[0], 0.0 };
      double arcTangentVectorL[3] = { arcRadiusVectorL[1], -arcRadiusVectorL[0], 0.0 };
      vtkMath::Normalize(arcTangentVectorR);
      vtkMath::Normalize(arcTangentVectorL);

      // Define cone positions to construct arrows
      double coneCenterR[3] = { arcTipR[0] + arcTangentVectorR[0] * (coneLength / 2.0),
                                arcTipR[1] + arcTangentVectorR[1] * (coneLength / 2.0),
                                arcTipR[2] + arcTangentVectorR[2] * (coneLength / 2.0) };
      double coneCenterL[3] = { arcTipL[0] + arcTangentVectorL[0] * (coneLength / 2.0),
                                arcTipL[1] + arcTangentVectorL[1] * (coneLength / 2.0),
                                arcTipL[2] + arcTangentVectorL[2] * (coneLength / 2.0) };

      // Rotation handle 1
      vtkNew<vtkArcSource> rotationHandle1ArcSource;
      rotationHandle1ArcSource->SetResolution(50);
      rotationHandle1ArcSource->SetPoint1(arcTipR);
      rotationHandle1ArcSource->SetPoint2(arcTipL);
      rotationHandle1ArcSource->SetCenter(arcCenter);
      vtkNew<vtkTubeFilter> rotationHandle1ArcTubeFilter;
      rotationHandle1ArcTubeFilter->SetInputConnection(rotationHandle1ArcSource->GetOutputPort());
      rotationHandle1ArcTubeFilter->SetRadius(ROTATION_HANDLE_ARROW_RADIUS);
      rotationHandle1ArcTubeFilter->SetNumberOfSides(16);
      rotationHandle1ArcTubeFilter->SetCapping(true);
      vtkNew<vtkConeSource> rotationHandle1RightConeSource;
      rotationHandle1RightConeSource->SetResolution(50);
      rotationHandle1RightConeSource->SetRadius(coneRadius);
      rotationHandle1RightConeSource->SetDirection(arcTangentVectorR);
      rotationHandle1RightConeSource->SetHeight(coneLength);
      rotationHandle1RightConeSource->SetCenter(coneCenterR);
      vtkNew<vtkConeSource> rotationHandle1LeftConeSource;
      rotationHandle1LeftConeSource->SetResolution(50);
      rotationHandle1LeftConeSource->SetRadius(coneRadius);
      rotationHandle1LeftConeSource->SetDirection(arcTangentVectorL);
      rotationHandle1LeftConeSource->SetHeight(coneLength);
      rotationHandle1LeftConeSource->SetCenter(coneCenterL);
      this->RotationHandle1 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->RotationHandle1->AddInputConnection(rotationHandle1ArcTubeFilter->GetOutputPort());
      this->RotationHandle1->AddInputConnection(rotationHandle1RightConeSource->GetOutputPort());
      this->RotationHandle1->AddInputConnection(rotationHandle1LeftConeSource->GetOutputPort());
      this->RotationHandle1ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->RotationHandle1TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->RotationHandle1TransformFilter->SetInputConnection(this->RotationHandle1->GetOutputPort());
      this->RotationHandle1TransformFilter->SetTransform(this->RotationHandle1ToWorldTransform);
      this->RotationHandle1Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->RotationHandle1Property = vtkSmartPointer<vtkProperty2D>::New();
      this->RotationHandle1Actor = vtkSmartPointer<vtkActor2D>::New();
      this->RotationHandle1Actor->SetVisibility(false); // invisible until slice node is set
      this->RotationHandle1Mapper->SetInputConnection(this->RotationHandle1TransformFilter->GetOutputPort());
      this->RotationHandle1Actor->SetMapper(this->RotationHandle1Mapper);
      this->RotationHandle1Actor->SetProperty(this->RotationHandle1Property);

      // Rotation handle 2
      vtkNew<vtkArcSource> rotationHandle2ArcSource;
      rotationHandle2ArcSource->SetResolution(50);
      rotationHandle2ArcSource->SetPoint1(arcTipR);
      rotationHandle2ArcSource->SetPoint2(arcTipL);
      rotationHandle2ArcSource->SetCenter(arcCenter);
      vtkNew<vtkTubeFilter> rotationHandle2ArcTubeFilter;
      rotationHandle2ArcTubeFilter->SetInputConnection(rotationHandle2ArcSource->GetOutputPort());
      rotationHandle2ArcTubeFilter->SetRadius(ROTATION_HANDLE_ARROW_RADIUS);
      rotationHandle2ArcTubeFilter->SetNumberOfSides(16);
      rotationHandle2ArcTubeFilter->SetCapping(true);
      vtkNew<vtkConeSource> rotationHandle2RightConeSource;
      rotationHandle2RightConeSource->SetResolution(50);
      rotationHandle2RightConeSource->SetRadius(coneRadius);
      rotationHandle2RightConeSource->SetDirection(arcTangentVectorR);
      rotationHandle2RightConeSource->SetHeight(coneLength);
      rotationHandle2RightConeSource->SetCenter(coneCenterR);
      vtkNew<vtkConeSource> rotationHandle2LeftConeSource;
      rotationHandle2LeftConeSource->SetResolution(50);
      rotationHandle2LeftConeSource->SetRadius(coneRadius);
      rotationHandle2LeftConeSource->SetDirection(arcTangentVectorL);
      rotationHandle2LeftConeSource->SetHeight(coneLength);
      rotationHandle2LeftConeSource->SetCenter(coneCenterL);
      this->RotationHandle2 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->RotationHandle2->AddInputConnection(rotationHandle2ArcTubeFilter->GetOutputPort());
      this->RotationHandle2->AddInputConnection(rotationHandle2RightConeSource->GetOutputPort());
      this->RotationHandle2->AddInputConnection(rotationHandle2LeftConeSource->GetOutputPort());
      this->RotationHandle2ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->RotationHandle2TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->RotationHandle2TransformFilter->SetInputConnection(this->RotationHandle2->GetOutputPort());
      this->RotationHandle2TransformFilter->SetTransform(this->RotationHandle2ToWorldTransform);
      this->RotationHandle2Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->RotationHandle2Property = vtkSmartPointer<vtkProperty2D>::New();
      this->RotationHandle2Actor = vtkSmartPointer<vtkActor2D>::New();
      this->RotationHandle2Actor->SetVisibility(false); // invisible until slice node is set
      this->RotationHandle2Mapper->SetInputConnection(this->RotationHandle2TransformFilter->GetOutputPort());
      this->RotationHandle2Actor->SetMapper(this->RotationHandle2Mapper);
      this->RotationHandle2Actor->SetProperty(this->RotationHandle2Property);

      // Handle points
      this->RotationHandle1Points->InsertNextPoint(handleOriginDefault);
      this->RotationHandle1Points->InsertNextPoint(arcTipR);
      this->RotationHandle1Points->InsertNextPoint(arcMidR);
      this->RotationHandle1Points->InsertNextPoint(coneCenterR);
      this->RotationHandle1Points->InsertNextPoint(arcTipL);
      this->RotationHandle1Points->InsertNextPoint(arcMidL);
      this->RotationHandle1Points->InsertNextPoint(coneCenterL);
      this->RotationHandle2Points->InsertNextPoint(handleOriginDefault);
      this->RotationHandle2Points->InsertNextPoint(arcTipR);
      this->RotationHandle2Points->InsertNextPoint(arcMidR);
      this->RotationHandle2Points->InsertNextPoint(coneCenterR);
      this->RotationHandle2Points->InsertNextPoint(arcTipL);
      this->RotationHandle2Points->InsertNextPoint(arcMidL);
      this->RotationHandle2Points->InsertNextPoint(coneCenterL);
    }
    else if (HANDLES_TYPE == Circles)
    {
      // Rotation sphere 1
      vtkNew<vtkSphereSource> rotationHandle1SphereSource;
      rotationHandle1SphereSource->SetRadius(ROTATION_HANDLE_CIRCLE_RADIUS);
      rotationHandle1SphereSource->SetThetaResolution(HANDLES_CIRCLE_THETA_RESOLUTION);
      rotationHandle1SphereSource->SetPhiResolution(HANDLES_CIRCLE_PHI_RESOLUTION);
      rotationHandle1SphereSource->SetCenter(handleOriginDefault);
      rotationHandle1SphereSource->Update();
      this->RotationHandle1 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->RotationHandle1->AddInputConnection(rotationHandle1SphereSource->GetOutputPort());
      this->RotationHandle1ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->RotationHandle1TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->RotationHandle1TransformFilter->SetInputConnection(this->RotationHandle1->GetOutputPort());
      this->RotationHandle1TransformFilter->SetTransform(this->RotationHandle1ToWorldTransform);
      this->RotationHandle1Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->RotationHandle1Property = vtkSmartPointer<vtkProperty2D>::New();
      this->RotationHandle1Actor = vtkSmartPointer<vtkActor2D>::New();
      this->RotationHandle1Actor->SetVisibility(false); // invisible until slice node is set
      this->RotationHandle1Mapper->SetInputConnection(this->RotationHandle1TransformFilter->GetOutputPort());
      this->RotationHandle1Actor->SetMapper(this->RotationHandle1Mapper);
      this->RotationHandle1Actor->SetProperty(this->RotationHandle1Property);

      // Rotation sphere 2
      vtkNew<vtkSphereSource> rotationHandle2SphereSource;
      rotationHandle2SphereSource->SetRadius(ROTATION_HANDLE_CIRCLE_RADIUS);
      rotationHandle2SphereSource->SetThetaResolution(HANDLES_CIRCLE_THETA_RESOLUTION);
      rotationHandle2SphereSource->SetPhiResolution(HANDLES_CIRCLE_PHI_RESOLUTION);
      rotationHandle2SphereSource->SetCenter(handleOriginDefault);
      rotationHandle2SphereSource->Update();
      this->RotationHandle2 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->RotationHandle2->AddInputConnection(rotationHandle2SphereSource->GetOutputPort());
      this->RotationHandle2ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->RotationHandle2TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->RotationHandle2TransformFilter->SetInputConnection(this->RotationHandle2->GetOutputPort());
      this->RotationHandle2TransformFilter->SetTransform(this->RotationHandle2ToWorldTransform);
      this->RotationHandle2Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->RotationHandle2Property = vtkSmartPointer<vtkProperty2D>::New();
      this->RotationHandle2Actor = vtkSmartPointer<vtkActor2D>::New();
      this->RotationHandle2Actor->SetVisibility(false); // invisible until slice node is set
      this->RotationHandle2Mapper->SetInputConnection(this->RotationHandle2TransformFilter->GetOutputPort());
      this->RotationHandle2Actor->SetMapper(this->RotationHandle2Mapper);
      this->RotationHandle2Actor->SetProperty(this->RotationHandle2Property);

      // Handle points
      this->RotationHandle1Points->InsertNextPoint(handleOriginDefault);
      this->RotationHandle2Points->InsertNextPoint(handleOriginDefault);
    }
  }

  //----------------------------------------------------------------------
  void CreateSliceOffsetHandles()
  {
    // Create list of points to store position of handles
    this->SliceOffsetHandle1Points = vtkSmartPointer<vtkPoints>::New();
    this->SliceOffsetHandle2Points = vtkSmartPointer<vtkPoints>::New();

    // We don't know if there is enough space, will set it later.
    this->SliceOffsetHandle1Displayable = false;
    this->SliceOffsetHandle2Displayable = false;

    // Handle default position and orientation
    double handleOriginDefault[3] = { SLICEOFFSET_HANDLE_DEFAULT_POSITION[0], SLICEOFFSET_HANDLE_DEFAULT_POSITION[1], SLICEOFFSET_HANDLE_DEFAULT_POSITION[2] };
    double handleOrientationDefault[3] = { SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[0], SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[1], SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[2] };
    double handleOrientationDefaultInv[3] = { -SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[0], -SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[1], -SLICEOFFSET_HANDLE_DEFAULT_ORIENTATION[2] };

    if (HANDLES_TYPE == Arrows)
    {
      // Define cone size
      double coneAngleRad = (SLICEOFFSET_HANDLE_ARROW_TIP_ANGLE * vtkMath::Pi()) / 180.0;
      double coneRadius = 2 * SLICEOFFSET_HANDLE_ARROW_RADIUS;
      double coneLength = coneRadius / tan(coneAngleRad);

      // Define cylinder size
      double cylinderLength = SLICEOFFSET_HANDLE_ARROW_LENGTH - coneLength * 2;

      // Define cone positions to construct arrows
      double coneCenterR[3] = { handleOrientationDefault[0], handleOrientationDefault[1], handleOrientationDefault[2] };
      vtkMath::MultiplyScalar(coneCenterR, cylinderLength / 2 + coneLength / 2);
      double coneTipR[3] = { handleOrientationDefault[0], handleOrientationDefault[1], handleOrientationDefault[2] };
      vtkMath::MultiplyScalar(coneTipR, cylinderLength / 2 + coneLength);
      double coneBaseR[3] = { handleOrientationDefault[0], handleOrientationDefault[1], handleOrientationDefault[2] };
      vtkMath::MultiplyScalar(coneBaseR, cylinderLength / 2);
      double coneCenterL[3] = { handleOrientationDefaultInv[0], handleOrientationDefaultInv[1], handleOrientationDefaultInv[2] };
      vtkMath::MultiplyScalar(coneCenterL, cylinderLength / 2 + coneLength / 2);
      double coneTipL[3] = { handleOrientationDefaultInv[0], handleOrientationDefaultInv[1], handleOrientationDefaultInv[2] };
      vtkMath::MultiplyScalar(coneTipL, cylinderLength / 2 + coneLength);
      double coneBaseL[3] = { handleOrientationDefaultInv[0], handleOrientationDefaultInv[1], handleOrientationDefaultInv[2] };
      vtkMath::MultiplyScalar(coneBaseL, cylinderLength / 2);

      // Translation handle 1
      vtkNew<vtkLineSource> sliceOffsetHandle1LineSource;
      sliceOffsetHandle1LineSource->SetResolution(50);
      sliceOffsetHandle1LineSource->SetPoint1(coneBaseR);
      sliceOffsetHandle1LineSource->SetPoint2(coneBaseL);
      vtkNew<vtkTubeFilter> sliceOffsetHandle1LineTubeFilter;
      sliceOffsetHandle1LineTubeFilter->SetInputConnection(sliceOffsetHandle1LineSource->GetOutputPort());
      sliceOffsetHandle1LineTubeFilter->SetRadius(SLICEOFFSET_HANDLE_ARROW_RADIUS);
      sliceOffsetHandle1LineTubeFilter->SetNumberOfSides(16);
      sliceOffsetHandle1LineTubeFilter->SetCapping(true);
      vtkNew<vtkConeSource> sliceOffsetHandle1RightConeSource;
      sliceOffsetHandle1RightConeSource->SetResolution(50);
      sliceOffsetHandle1RightConeSource->SetRadius(coneRadius);
      sliceOffsetHandle1RightConeSource->SetDirection(handleOrientationDefault);
      sliceOffsetHandle1RightConeSource->SetHeight(coneLength);
      sliceOffsetHandle1RightConeSource->SetCenter(coneCenterR);
      vtkNew<vtkConeSource> sliceOffsetHandle1LeftConeSource;
      sliceOffsetHandle1LeftConeSource->SetResolution(50);
      sliceOffsetHandle1LeftConeSource->SetRadius(coneRadius);
      sliceOffsetHandle1LeftConeSource->SetDirection(handleOrientationDefaultInv);
      sliceOffsetHandle1LeftConeSource->SetHeight(coneLength);
      sliceOffsetHandle1LeftConeSource->SetCenter(coneCenterL);
      this->SliceOffsetHandle1 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->SliceOffsetHandle1->AddInputConnection(sliceOffsetHandle1LineTubeFilter->GetOutputPort());
      this->SliceOffsetHandle1->AddInputConnection(sliceOffsetHandle1RightConeSource->GetOutputPort());
      this->SliceOffsetHandle1->AddInputConnection(sliceOffsetHandle1LeftConeSource->GetOutputPort());
      this->SliceOffsetHandle1ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->SliceOffsetHandle1TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->SliceOffsetHandle1TransformFilter->SetInputConnection(this->SliceOffsetHandle1->GetOutputPort());
      this->SliceOffsetHandle1TransformFilter->SetTransform(this->SliceOffsetHandle1ToWorldTransform);
      this->SliceOffsetHandle1Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->SliceOffsetHandle1Property = vtkSmartPointer<vtkProperty2D>::New();
      this->SliceOffsetHandle1Actor = vtkSmartPointer<vtkActor2D>::New();
      this->SliceOffsetHandle1Actor->SetVisibility(false); // invisible until slice node is set
      this->SliceOffsetHandle1Mapper->SetInputConnection(this->SliceOffsetHandle1TransformFilter->GetOutputPort());
      this->SliceOffsetHandle1Actor->SetMapper(this->SliceOffsetHandle1Mapper);
      this->SliceOffsetHandle1Actor->SetProperty(this->SliceOffsetHandle1Property);

      // Translation handle 2
      vtkNew<vtkLineSource> sliceOffsetHandle2LineSource;
      sliceOffsetHandle2LineSource->SetResolution(50);
      sliceOffsetHandle2LineSource->SetPoint1(coneBaseR);
      sliceOffsetHandle2LineSource->SetPoint2(coneBaseL);
      vtkNew<vtkTubeFilter> sliceOffsetHandle2LineTubeFilter;
      sliceOffsetHandle2LineTubeFilter->SetInputConnection(sliceOffsetHandle2LineSource->GetOutputPort());
      sliceOffsetHandle2LineTubeFilter->SetRadius(SLICEOFFSET_HANDLE_ARROW_RADIUS);
      sliceOffsetHandle2LineTubeFilter->SetNumberOfSides(16);
      sliceOffsetHandle2LineTubeFilter->SetCapping(true);
      vtkNew<vtkConeSource> sliceOffsetHandle2RightConeSource;
      sliceOffsetHandle2RightConeSource->SetResolution(50);
      sliceOffsetHandle2RightConeSource->SetRadius(coneRadius);
      sliceOffsetHandle2RightConeSource->SetDirection(handleOrientationDefault);
      sliceOffsetHandle2RightConeSource->SetHeight(coneLength);
      sliceOffsetHandle2RightConeSource->SetCenter(coneCenterR);
      vtkNew<vtkConeSource> sliceOffsetHandle2LeftConeSource;
      sliceOffsetHandle2LeftConeSource->SetResolution(50);
      sliceOffsetHandle2LeftConeSource->SetRadius(coneRadius);
      sliceOffsetHandle2LeftConeSource->SetDirection(handleOrientationDefaultInv);
      sliceOffsetHandle2LeftConeSource->SetHeight(coneLength);
      sliceOffsetHandle2LeftConeSource->SetCenter(coneCenterL);
      this->SliceOffsetHandle2 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->SliceOffsetHandle2->AddInputConnection(sliceOffsetHandle2LineTubeFilter->GetOutputPort());
      this->SliceOffsetHandle2->AddInputConnection(sliceOffsetHandle2RightConeSource->GetOutputPort());
      this->SliceOffsetHandle2->AddInputConnection(sliceOffsetHandle2LeftConeSource->GetOutputPort());
      this->SliceOffsetHandle2ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->SliceOffsetHandle2TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->SliceOffsetHandle2TransformFilter->SetInputConnection(this->SliceOffsetHandle2->GetOutputPort());
      this->SliceOffsetHandle2TransformFilter->SetTransform(this->SliceOffsetHandle2ToWorldTransform);
      this->SliceOffsetHandle2Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->SliceOffsetHandle2Property = vtkSmartPointer<vtkProperty2D>::New();
      this->SliceOffsetHandle2Actor = vtkSmartPointer<vtkActor2D>::New();
      this->SliceOffsetHandle2Actor->SetVisibility(false); // invisible until slice node is set
      this->SliceOffsetHandle2Mapper->SetInputConnection(this->SliceOffsetHandle2TransformFilter->GetOutputPort());
      this->SliceOffsetHandle2Actor->SetMapper(this->SliceOffsetHandle2Mapper);
      this->SliceOffsetHandle2Actor->SetProperty(this->SliceOffsetHandle2Property);

      // Handle points
      this->SliceOffsetHandle1Points->InsertNextPoint(handleOriginDefault);
      this->SliceOffsetHandle1Points->InsertNextPoint(coneTipR);
      this->SliceOffsetHandle1Points->InsertNextPoint(coneCenterR);
      this->SliceOffsetHandle1Points->InsertNextPoint(coneBaseR);
      this->SliceOffsetHandle1Points->InsertNextPoint(coneTipL);
      this->SliceOffsetHandle1Points->InsertNextPoint(coneCenterL);
      this->SliceOffsetHandle1Points->InsertNextPoint(coneBaseL);
      this->SliceOffsetHandle2Points->InsertNextPoint(handleOriginDefault);
      this->SliceOffsetHandle2Points->InsertNextPoint(coneTipR);
      this->SliceOffsetHandle2Points->InsertNextPoint(coneCenterR);
      this->SliceOffsetHandle2Points->InsertNextPoint(coneBaseR);
      this->SliceOffsetHandle2Points->InsertNextPoint(coneTipL);
      this->SliceOffsetHandle2Points->InsertNextPoint(coneCenterL);
      this->SliceOffsetHandle2Points->InsertNextPoint(coneBaseL);
    }
    else if (HANDLES_TYPE == Circles)
    {
      // Translation sphere 1
      vtkNew<vtkSphereSource> sliceOffsetHandle1SphereSource;
      sliceOffsetHandle1SphereSource->SetRadius(SLICEOFFSET_HANDLE_CIRCLE_RADIUS);
      sliceOffsetHandle1SphereSource->SetThetaResolution(HANDLES_CIRCLE_THETA_RESOLUTION);
      sliceOffsetHandle1SphereSource->SetPhiResolution(HANDLES_CIRCLE_PHI_RESOLUTION);
      sliceOffsetHandle1SphereSource->SetCenter(handleOriginDefault);
      sliceOffsetHandle1SphereSource->Update();
      this->SliceOffsetHandle1 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->SliceOffsetHandle1->AddInputConnection(sliceOffsetHandle1SphereSource->GetOutputPort());
      this->SliceOffsetHandle1ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->SliceOffsetHandle1TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->SliceOffsetHandle1TransformFilter->SetInputConnection(this->SliceOffsetHandle1->GetOutputPort());
      this->SliceOffsetHandle1TransformFilter->SetTransform(this->SliceOffsetHandle1ToWorldTransform);
      this->SliceOffsetHandle1Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->SliceOffsetHandle1Property = vtkSmartPointer<vtkProperty2D>::New();
      this->SliceOffsetHandle1Actor = vtkSmartPointer<vtkActor2D>::New();
      this->SliceOffsetHandle1Actor->SetVisibility(false); // invisible until slice node is set
      this->SliceOffsetHandle1Mapper->SetInputConnection(this->SliceOffsetHandle1TransformFilter->GetOutputPort());
      this->SliceOffsetHandle1Actor->SetMapper(this->SliceOffsetHandle1Mapper);
      this->SliceOffsetHandle1Actor->SetProperty(this->SliceOffsetHandle1Property);

      // Translation sphere 2
      vtkNew<vtkSphereSource> sliceOffsetHandle2SphereSource;
      sliceOffsetHandle2SphereSource->SetRadius(SLICEOFFSET_HANDLE_CIRCLE_RADIUS);
      sliceOffsetHandle2SphereSource->SetThetaResolution(HANDLES_CIRCLE_THETA_RESOLUTION);
      sliceOffsetHandle2SphereSource->SetPhiResolution(HANDLES_CIRCLE_PHI_RESOLUTION);
      sliceOffsetHandle2SphereSource->SetCenter(handleOriginDefault);
      sliceOffsetHandle2SphereSource->Update();
      this->SliceOffsetHandle2 = vtkSmartPointer<vtkAppendPolyData>::New();
      this->SliceOffsetHandle2->AddInputConnection(sliceOffsetHandle2SphereSource->GetOutputPort());
      this->SliceOffsetHandle2ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
      this->SliceOffsetHandle2TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      this->SliceOffsetHandle2TransformFilter->SetInputConnection(this->SliceOffsetHandle2->GetOutputPort());
      this->SliceOffsetHandle2TransformFilter->SetTransform(this->SliceOffsetHandle2ToWorldTransform);
      this->SliceOffsetHandle2Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
      this->SliceOffsetHandle2Property = vtkSmartPointer<vtkProperty2D>::New();
      this->SliceOffsetHandle2Actor = vtkSmartPointer<vtkActor2D>::New();
      this->SliceOffsetHandle2Actor->SetVisibility(false); // invisible until slice node is set
      this->SliceOffsetHandle2Mapper->SetInputConnection(this->SliceOffsetHandle2TransformFilter->GetOutputPort());
      this->SliceOffsetHandle2Actor->SetMapper(this->SliceOffsetHandle2Mapper);
      this->SliceOffsetHandle2Actor->SetProperty(this->SliceOffsetHandle2Property);

      // Handle points
      this->SliceOffsetHandle1Points->InsertNextPoint(handleOriginDefault);
      this->SliceOffsetHandle2Points->InsertNextPoint(handleOriginDefault);
    }
  }

  //----------------------------------------------------------------------
  void CreateThickSlabTranslationHandles()
  {
    // Create list of points to store position of handles
    this->ThickSlabLine1Handle1Points = vtkSmartPointer<vtkPoints>::New();
    this->ThickSlabLine1Handle2Points = vtkSmartPointer<vtkPoints>::New();
    this->ThickSlabLine2Handle1Points = vtkSmartPointer<vtkPoints>::New();
    this->ThickSlabLine2Handle2Points = vtkSmartPointer<vtkPoints>::New();

    // We don't know if there is enough space, will set it later.
    this->ThickSlabLine1Handle1Displayable = false;
    this->ThickSlabLine1Handle2Displayable = false;
    this->ThickSlabLine2Handle1Displayable = false;
    this->ThickSlabLine2Handle2Displayable = false;

    // Handle default position and orientation
    double handleOriginDefault[3] = { THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_POSITION[0],
                                      THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_POSITION[1],
                                      THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_POSITION[2] };
    double handleOrientationDefault[3] = { THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_ORIENTATION[0],
                                           THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_ORIENTATION[1],
                                           THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_ORIENTATION[2] };
    double handleOrientationDefaultInv[3] = { -THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_ORIENTATION[0],
                                              -THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_ORIENTATION[1],
                                              -THICK_SLAB_TRANSLATION_HANDLE_DEFAULT_ORIENTATION[2] };

    // Define cone size
    double coneAngleRad = (THICK_SLAB_TRANSLATION_HANDLE_ARROW_TIP_ANGLE * vtkMath::Pi()) / 180.0;
    double coneRadius = 2 * THICK_SLAB_TRANSLATION_HANDLE_ARROW_RADIUS;
    double coneLength = coneRadius / tan(coneAngleRad);

    // Define cylinder size
    double cylinderLength = THICK_SLAB_TRANSLATION_HANDLE_ARROW_LENGTH - coneLength * 2;

    // Define cone positions to construct arrows
    double coneCenterR[3] = { handleOrientationDefault[0], handleOrientationDefault[1], handleOrientationDefault[2] };
    vtkMath::MultiplyScalar(coneCenterR, cylinderLength / 2 + coneLength / 2);
    double coneTipR[3] = { handleOrientationDefault[0], handleOrientationDefault[1], handleOrientationDefault[2] };
    vtkMath::MultiplyScalar(coneTipR, cylinderLength / 2 + coneLength);
    double coneBaseR[3] = { handleOrientationDefault[0], handleOrientationDefault[1], handleOrientationDefault[2] };
    vtkMath::MultiplyScalar(coneBaseR, cylinderLength / 2);
    double coneCenterL[3] = { handleOrientationDefaultInv[0], handleOrientationDefaultInv[1], handleOrientationDefaultInv[2] };
    vtkMath::MultiplyScalar(coneCenterL, cylinderLength / 2 + coneLength / 2);
    double coneTipL[3] = { handleOrientationDefaultInv[0], handleOrientationDefaultInv[1], handleOrientationDefaultInv[2] };
    vtkMath::MultiplyScalar(coneTipL, cylinderLength / 2 + coneLength);
    double coneBaseL[3] = { handleOrientationDefaultInv[0], handleOrientationDefaultInv[1], handleOrientationDefaultInv[2] };
    vtkMath::MultiplyScalar(coneBaseL, cylinderLength / 2);

    // Translation line 1 first handle
    vtkNew<vtkLineSource> thickSlabLine1Handle1LineSource;
    thickSlabLine1Handle1LineSource->SetResolution(50);
    thickSlabLine1Handle1LineSource->SetPoint1(coneBaseR);
    thickSlabLine1Handle1LineSource->SetPoint2(coneBaseL);
    vtkNew<vtkTubeFilter> thickSlabLine1Handle1LineTubeFilter;
    thickSlabLine1Handle1LineTubeFilter->SetInputConnection(thickSlabLine1Handle1LineSource->GetOutputPort());
    thickSlabLine1Handle1LineTubeFilter->SetRadius(THICK_SLAB_TRANSLATION_HANDLE_ARROW_RADIUS);
    thickSlabLine1Handle1LineTubeFilter->SetNumberOfSides(16);
    thickSlabLine1Handle1LineTubeFilter->SetCapping(true);
    vtkNew<vtkConeSource> thickSlabLine1Handle1RightConeSource;
    thickSlabLine1Handle1RightConeSource->SetResolution(50);
    thickSlabLine1Handle1RightConeSource->SetRadius(coneRadius);
    thickSlabLine1Handle1RightConeSource->SetDirection(handleOrientationDefault);
    thickSlabLine1Handle1RightConeSource->SetHeight(coneLength);
    thickSlabLine1Handle1RightConeSource->SetCenter(coneCenterR);
    vtkNew<vtkConeSource> thickSlabLine1Handle1LeftConeSource;
    thickSlabLine1Handle1LeftConeSource->SetResolution(50);
    thickSlabLine1Handle1LeftConeSource->SetRadius(coneRadius);
    thickSlabLine1Handle1LeftConeSource->SetDirection(handleOrientationDefaultInv);
    thickSlabLine1Handle1LeftConeSource->SetHeight(coneLength);
    thickSlabLine1Handle1LeftConeSource->SetCenter(coneCenterL);
    this->ThickSlabLine1Handle1 = vtkSmartPointer<vtkAppendPolyData>::New();
    this->ThickSlabLine1Handle1->AddInputConnection(thickSlabLine1Handle1LineTubeFilter->GetOutputPort());
    this->ThickSlabLine1Handle1->AddInputConnection(thickSlabLine1Handle1RightConeSource->GetOutputPort());
    this->ThickSlabLine1Handle1->AddInputConnection(thickSlabLine1Handle1LeftConeSource->GetOutputPort());
    this->ThickSlabLine1Handle1ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
    this->ThickSlabLine1Handle1TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    this->ThickSlabLine1Handle1TransformFilter->SetInputConnection(this->ThickSlabLine1Handle1->GetOutputPort());
    this->ThickSlabLine1Handle1TransformFilter->SetTransform(this->ThickSlabLine1Handle1ToWorldTransform);
    this->ThickSlabLine1Handle1Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine1Handle1Property = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine1Handle1Actor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine1Handle1Actor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine1Handle1Mapper->SetInputConnection(this->ThickSlabLine1Handle1TransformFilter->GetOutputPort());
    this->ThickSlabLine1Handle1Actor->SetMapper(this->ThickSlabLine1Handle1Mapper);
    this->ThickSlabLine1Handle1Actor->SetProperty(this->ThickSlabLine1Handle1Property);

    // Translation line 1 second handle
    vtkNew<vtkLineSource> thickSlabLine1Handle2LineSource;
    thickSlabLine1Handle2LineSource->SetResolution(50);
    thickSlabLine1Handle2LineSource->SetPoint1(coneBaseR);
    thickSlabLine1Handle2LineSource->SetPoint2(coneBaseL);
    vtkNew<vtkTubeFilter> thickSlabLine1Handle2LineTubeFilter;
    thickSlabLine1Handle2LineTubeFilter->SetInputConnection(thickSlabLine1Handle2LineSource->GetOutputPort());
    thickSlabLine1Handle2LineTubeFilter->SetRadius(THICK_SLAB_TRANSLATION_HANDLE_ARROW_RADIUS);
    thickSlabLine1Handle2LineTubeFilter->SetNumberOfSides(16);
    thickSlabLine1Handle2LineTubeFilter->SetCapping(true);
    vtkNew<vtkConeSource> thickSlabLine1Handle2RightConeSource;
    thickSlabLine1Handle2RightConeSource->SetResolution(50);
    thickSlabLine1Handle2RightConeSource->SetRadius(coneRadius);
    thickSlabLine1Handle2RightConeSource->SetDirection(handleOrientationDefault);
    thickSlabLine1Handle2RightConeSource->SetHeight(coneLength);
    thickSlabLine1Handle2RightConeSource->SetCenter(coneCenterR);
    vtkNew<vtkConeSource> thickSlabLine1Handle2LeftConeSource;
    thickSlabLine1Handle2LeftConeSource->SetResolution(50);
    thickSlabLine1Handle2LeftConeSource->SetRadius(coneRadius);
    thickSlabLine1Handle2LeftConeSource->SetDirection(handleOrientationDefaultInv);
    thickSlabLine1Handle2LeftConeSource->SetHeight(coneLength);
    thickSlabLine1Handle2LeftConeSource->SetCenter(coneCenterL);
    this->ThickSlabLine1Handle2 = vtkSmartPointer<vtkAppendPolyData>::New();
    this->ThickSlabLine1Handle2->AddInputConnection(thickSlabLine1Handle2LineTubeFilter->GetOutputPort());
    this->ThickSlabLine1Handle2->AddInputConnection(thickSlabLine1Handle2RightConeSource->GetOutputPort());
    this->ThickSlabLine1Handle2->AddInputConnection(thickSlabLine1Handle2LeftConeSource->GetOutputPort());
    this->ThickSlabLine1Handle2ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
    this->ThickSlabLine1Handle2TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    this->ThickSlabLine1Handle2TransformFilter->SetInputConnection(this->ThickSlabLine1Handle2->GetOutputPort());
    this->ThickSlabLine1Handle2TransformFilter->SetTransform(this->ThickSlabLine1Handle2ToWorldTransform);
    this->ThickSlabLine1Handle2Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine1Handle2Property = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine1Handle2Actor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine1Handle2Actor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine1Handle2Mapper->SetInputConnection(this->ThickSlabLine1Handle2TransformFilter->GetOutputPort());
    this->ThickSlabLine1Handle2Actor->SetMapper(this->ThickSlabLine1Handle2Mapper);
    this->ThickSlabLine1Handle2Actor->SetProperty(this->ThickSlabLine1Handle2Property);

    // Translation line 1 first handle
    vtkNew<vtkLineSource> thickSlabLine2Handle1LineSource;
    thickSlabLine2Handle1LineSource->SetResolution(50);
    thickSlabLine2Handle1LineSource->SetPoint1(coneBaseR);
    thickSlabLine2Handle1LineSource->SetPoint2(coneBaseL);
    vtkNew<vtkTubeFilter> thickSlabLine2Handle1LineTubeFilter;
    thickSlabLine2Handle1LineTubeFilter->SetInputConnection(thickSlabLine2Handle1LineSource->GetOutputPort());
    thickSlabLine2Handle1LineTubeFilter->SetRadius(THICK_SLAB_TRANSLATION_HANDLE_ARROW_RADIUS);
    thickSlabLine2Handle1LineTubeFilter->SetNumberOfSides(16);
    thickSlabLine2Handle1LineTubeFilter->SetCapping(true);
    vtkNew<vtkConeSource> thickSlabLine2Handle1RightConeSource;
    thickSlabLine2Handle1RightConeSource->SetResolution(50);
    thickSlabLine2Handle1RightConeSource->SetRadius(coneRadius);
    thickSlabLine2Handle1RightConeSource->SetDirection(handleOrientationDefault);
    thickSlabLine2Handle1RightConeSource->SetHeight(coneLength);
    thickSlabLine2Handle1RightConeSource->SetCenter(coneCenterR);
    vtkNew<vtkConeSource> thickSlabLine2Handle1LeftConeSource;
    thickSlabLine2Handle1LeftConeSource->SetResolution(50);
    thickSlabLine2Handle1LeftConeSource->SetRadius(coneRadius);
    thickSlabLine2Handle1LeftConeSource->SetDirection(handleOrientationDefaultInv);
    thickSlabLine2Handle1LeftConeSource->SetHeight(coneLength);
    thickSlabLine2Handle1LeftConeSource->SetCenter(coneCenterL);
    this->ThickSlabLine2Handle1 = vtkSmartPointer<vtkAppendPolyData>::New();
    this->ThickSlabLine2Handle1->AddInputConnection(thickSlabLine2Handle1LineTubeFilter->GetOutputPort());
    this->ThickSlabLine2Handle1->AddInputConnection(thickSlabLine2Handle1RightConeSource->GetOutputPort());
    this->ThickSlabLine2Handle1->AddInputConnection(thickSlabLine2Handle1LeftConeSource->GetOutputPort());
    this->ThickSlabLine2Handle1ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
    this->ThickSlabLine2Handle1TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    this->ThickSlabLine2Handle1TransformFilter->SetInputConnection(this->ThickSlabLine2Handle1->GetOutputPort());
    this->ThickSlabLine2Handle1TransformFilter->SetTransform(this->ThickSlabLine2Handle1ToWorldTransform);
    this->ThickSlabLine2Handle1Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine2Handle1Property = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine2Handle1Actor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine2Handle1Actor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine2Handle1Mapper->SetInputConnection(this->ThickSlabLine2Handle1TransformFilter->GetOutputPort());
    this->ThickSlabLine2Handle1Actor->SetMapper(this->ThickSlabLine2Handle1Mapper);
    this->ThickSlabLine2Handle1Actor->SetProperty(this->ThickSlabLine2Handle1Property);

    // Translation line 1 second handle
    vtkNew<vtkLineSource> thickSlabLine2Handle2LineSource;
    thickSlabLine2Handle2LineSource->SetResolution(50);
    thickSlabLine2Handle2LineSource->SetPoint1(coneBaseR);
    thickSlabLine2Handle2LineSource->SetPoint2(coneBaseL);
    vtkNew<vtkTubeFilter> thickSlabLine2Handle2LineTubeFilter;
    thickSlabLine2Handle2LineTubeFilter->SetInputConnection(thickSlabLine2Handle2LineSource->GetOutputPort());
    thickSlabLine2Handle2LineTubeFilter->SetRadius(THICK_SLAB_TRANSLATION_HANDLE_ARROW_RADIUS);
    thickSlabLine2Handle2LineTubeFilter->SetNumberOfSides(16);
    thickSlabLine2Handle2LineTubeFilter->SetCapping(true);
    vtkNew<vtkConeSource> thickSlabLine2Handle2RightConeSource;
    thickSlabLine2Handle2RightConeSource->SetResolution(50);
    thickSlabLine2Handle2RightConeSource->SetRadius(coneRadius);
    thickSlabLine2Handle2RightConeSource->SetDirection(handleOrientationDefault);
    thickSlabLine2Handle2RightConeSource->SetHeight(coneLength);
    thickSlabLine2Handle2RightConeSource->SetCenter(coneCenterR);
    vtkNew<vtkConeSource> thickSlabLine2Handle2LeftConeSource;
    thickSlabLine2Handle2LeftConeSource->SetResolution(50);
    thickSlabLine2Handle2LeftConeSource->SetRadius(coneRadius);
    thickSlabLine2Handle2LeftConeSource->SetDirection(handleOrientationDefaultInv);
    thickSlabLine2Handle2LeftConeSource->SetHeight(coneLength);
    thickSlabLine2Handle2LeftConeSource->SetCenter(coneCenterL);
    this->ThickSlabLine2Handle2 = vtkSmartPointer<vtkAppendPolyData>::New();
    this->ThickSlabLine2Handle2->AddInputConnection(thickSlabLine2Handle2LineTubeFilter->GetOutputPort());
    this->ThickSlabLine2Handle2->AddInputConnection(thickSlabLine2Handle2RightConeSource->GetOutputPort());
    this->ThickSlabLine2Handle2->AddInputConnection(thickSlabLine2Handle2LeftConeSource->GetOutputPort());
    this->ThickSlabLine2Handle2ToWorldTransform = vtkSmartPointer<vtkTransform>::New();
    this->ThickSlabLine2Handle2TransformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
    this->ThickSlabLine2Handle2TransformFilter->SetInputConnection(this->ThickSlabLine2Handle2->GetOutputPort());
    this->ThickSlabLine2Handle2TransformFilter->SetTransform(this->ThickSlabLine2Handle2ToWorldTransform);
    this->ThickSlabLine2Handle2Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->ThickSlabLine2Handle2Property = vtkSmartPointer<vtkProperty2D>::New();
    this->ThickSlabLine2Handle2Actor = vtkSmartPointer<vtkActor2D>::New();
    this->ThickSlabLine2Handle2Actor->SetVisibility(false); // invisible until slice node is set
    this->ThickSlabLine2Handle2Mapper->SetInputConnection(this->ThickSlabLine2Handle2TransformFilter->GetOutputPort());
    this->ThickSlabLine2Handle2Actor->SetMapper(this->ThickSlabLine2Handle2Mapper);
    this->ThickSlabLine2Handle2Actor->SetProperty(this->ThickSlabLine2Handle2Property);

    // Handle points
    this->ThickSlabLine1Handle1Points->InsertNextPoint(handleOriginDefault);
    this->ThickSlabLine1Handle1Points->InsertNextPoint(coneTipR);
    this->ThickSlabLine1Handle1Points->InsertNextPoint(coneCenterR);
    this->ThickSlabLine1Handle1Points->InsertNextPoint(coneBaseR);
    this->ThickSlabLine1Handle1Points->InsertNextPoint(coneTipL);
    this->ThickSlabLine1Handle1Points->InsertNextPoint(coneCenterL);
    this->ThickSlabLine1Handle1Points->InsertNextPoint(coneBaseL);

    this->ThickSlabLine1Handle2Points->InsertNextPoint(handleOriginDefault);
    this->ThickSlabLine1Handle2Points->InsertNextPoint(coneTipR);
    this->ThickSlabLine1Handle2Points->InsertNextPoint(coneCenterR);
    this->ThickSlabLine1Handle2Points->InsertNextPoint(coneBaseR);
    this->ThickSlabLine1Handle2Points->InsertNextPoint(coneTipL);
    this->ThickSlabLine1Handle2Points->InsertNextPoint(coneCenterL);
    this->ThickSlabLine1Handle2Points->InsertNextPoint(coneBaseL);

    this->ThickSlabLine2Handle1Points->InsertNextPoint(handleOriginDefault);
    this->ThickSlabLine2Handle1Points->InsertNextPoint(coneTipR);
    this->ThickSlabLine2Handle1Points->InsertNextPoint(coneCenterR);
    this->ThickSlabLine2Handle1Points->InsertNextPoint(coneBaseR);
    this->ThickSlabLine2Handle1Points->InsertNextPoint(coneTipL);
    this->ThickSlabLine2Handle1Points->InsertNextPoint(coneCenterL);
    this->ThickSlabLine2Handle1Points->InsertNextPoint(coneBaseL);

    this->ThickSlabLine2Handle2Points->InsertNextPoint(handleOriginDefault);
    this->ThickSlabLine2Handle2Points->InsertNextPoint(coneTipR);
    this->ThickSlabLine2Handle2Points->InsertNextPoint(coneCenterR);
    this->ThickSlabLine2Handle2Points->InsertNextPoint(coneBaseR);
    this->ThickSlabLine2Handle2Points->InsertNextPoint(coneTipL);
    this->ThickSlabLine2Handle2Points->InsertNextPoint(coneCenterL);
    this->ThickSlabLine2Handle2Points->InsertNextPoint(coneBaseL);
  }

  //----------------------------------------------------------------------
  void SetAndObserveSliceLogic(vtkMRMLSliceLogic* sliceLogic, vtkCallbackCommand* callback)
  {
    if (sliceLogic != this->SliceLogic || callback != this->Callback)
    {
      if (this->SliceLogic && this->Callback)
      {
        this->SliceLogic->RemoveObserver(this->Callback);
      }
      if (sliceLogic)
      {
        sliceLogic->AddObserver(vtkCommand::ModifiedEvent, callback);
        sliceLogic->AddObserver(vtkMRMLSliceLogic::CompositeModifiedEvent, callback);
      }
      this->SliceLogic = sliceLogic;
    }
    this->Callback = callback;
  }

  //----------------------------------------------------------------------
  void GetActors2D(vtkPropCollection* pc)
  {
    pc->AddItem(this->IntersectionLine1Actor);
    pc->AddItem(this->IntersectionLine2Actor);
    pc->AddItem(this->ThickSlabLine1FirstHalfActor);
    pc->AddItem(this->ThickSlabLine1SecondHalfActor);
    pc->AddItem(this->ThickSlabLine2FirstHalfActor);
    pc->AddItem(this->ThickSlabLine2SecondHalfActor);
    pc->AddItem(this->TranslationOuterHandleActor);
    pc->AddItem(this->TranslationInnerHandleActor);
    pc->AddItem(this->RotationHandle1Actor);
    pc->AddItem(this->RotationHandle2Actor);
    pc->AddItem(this->SliceOffsetHandle1Actor);
    pc->AddItem(this->SliceOffsetHandle2Actor);
    pc->AddItem(this->ThickSlabLine1Handle1Actor);
    pc->AddItem(this->ThickSlabLine1Handle2Actor);
    pc->AddItem(this->ThickSlabLine2Handle1Actor);
    pc->AddItem(this->ThickSlabLine2Handle2Actor);
  }

  //----------------------------------------------------------------------
  void AddActors(vtkRenderer* renderer)
  {
    if (!renderer)
    {
      return;
    }
    renderer->AddViewProp(this->IntersectionLine1Actor);
    renderer->AddViewProp(this->IntersectionLine2Actor);
    renderer->AddViewProp(this->ThickSlabLine1FirstHalfActor);
    renderer->AddViewProp(this->ThickSlabLine1SecondHalfActor);
    renderer->AddViewProp(this->ThickSlabLine2FirstHalfActor);
    renderer->AddViewProp(this->ThickSlabLine2SecondHalfActor);
    renderer->AddViewProp(this->TranslationOuterHandleActor);
    renderer->AddViewProp(this->TranslationInnerHandleActor);
    renderer->AddViewProp(this->RotationHandle1Actor);
    renderer->AddViewProp(this->RotationHandle2Actor);
    renderer->AddViewProp(this->SliceOffsetHandle1Actor);
    renderer->AddViewProp(this->SliceOffsetHandle2Actor);
    renderer->AddViewProp(this->ThickSlabLine1Handle1Actor);
    renderer->AddViewProp(this->ThickSlabLine1Handle2Actor);
    renderer->AddViewProp(this->ThickSlabLine2Handle1Actor);
    renderer->AddViewProp(this->ThickSlabLine2Handle2Actor);
  }

  //----------------------------------------------------------------------
  void ReleaseGraphicsResources(vtkWindow* win)
  {
    this->IntersectionLine1Actor->ReleaseGraphicsResources(win);
    this->IntersectionLine2Actor->ReleaseGraphicsResources(win);
    this->ThickSlabLine1FirstHalfActor->ReleaseGraphicsResources(win);
    this->ThickSlabLine1SecondHalfActor->ReleaseGraphicsResources(win);
    this->ThickSlabLine2FirstHalfActor->ReleaseGraphicsResources(win);
    this->ThickSlabLine2SecondHalfActor->ReleaseGraphicsResources(win);
    this->TranslationOuterHandleActor->ReleaseGraphicsResources(win);
    this->TranslationInnerHandleActor->ReleaseGraphicsResources(win);
    this->RotationHandle1Actor->ReleaseGraphicsResources(win);
    this->RotationHandle2Actor->ReleaseGraphicsResources(win);
    this->SliceOffsetHandle1Actor->ReleaseGraphicsResources(win);
    this->SliceOffsetHandle2Actor->ReleaseGraphicsResources(win);
    this->ThickSlabLine1Handle1Actor->ReleaseGraphicsResources(win);
    this->ThickSlabLine1Handle2Actor->ReleaseGraphicsResources(win);
    this->ThickSlabLine2Handle1Actor->ReleaseGraphicsResources(win);
    this->ThickSlabLine2Handle2Actor->ReleaseGraphicsResources(win);
  }

  //----------------------------------------------------------------------
  void RemoveActors(vtkRenderer* renderer)
  {
    if (!renderer)
    {
      return;
    }
    renderer->RemoveViewProp(this->IntersectionLine1Actor);
    renderer->RemoveViewProp(this->IntersectionLine2Actor);
    renderer->RemoveViewProp(this->ThickSlabLine1FirstHalfActor);
    renderer->RemoveViewProp(this->ThickSlabLine1SecondHalfActor);
    renderer->RemoveViewProp(this->ThickSlabLine2FirstHalfActor);
    renderer->RemoveViewProp(this->ThickSlabLine2SecondHalfActor);
    renderer->RemoveViewProp(this->TranslationOuterHandleActor);
    renderer->RemoveViewProp(this->TranslationInnerHandleActor);
    renderer->RemoveViewProp(this->RotationHandle1Actor);
    renderer->RemoveViewProp(this->RotationHandle2Actor);
    renderer->RemoveViewProp(this->SliceOffsetHandle1Actor);
    renderer->RemoveViewProp(this->SliceOffsetHandle2Actor);
    renderer->RemoveViewProp(this->ThickSlabLine1Handle1Actor);
    renderer->RemoveViewProp(this->ThickSlabLine1Handle2Actor);
    renderer->RemoveViewProp(this->ThickSlabLine2Handle1Actor);
    renderer->RemoveViewProp(this->ThickSlabLine2Handle2Actor);
  }

  //----------------------------------------------------------------------
  int RenderOverlay(vtkViewport* viewport)
  {
    int count = 0;
    if (this->IntersectionLine1Actor->GetVisibility())
    {
      this->IntersectionLine1Actor->RenderOverlay(viewport);
    }
    if (this->IntersectionLine2Actor->GetVisibility())
    {
      this->IntersectionLine2Actor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine1FirstHalfActor->GetVisibility())
    {
      this->ThickSlabLine1FirstHalfActor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine1SecondHalfActor->GetVisibility())
    {
      this->ThickSlabLine1SecondHalfActor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine2FirstHalfActor->GetVisibility())
    {
      this->ThickSlabLine2FirstHalfActor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine2SecondHalfActor->GetVisibility())
    {
      this->ThickSlabLine2SecondHalfActor->RenderOverlay(viewport);
    }
    if (this->TranslationOuterHandleActor->GetVisibility())
    {
      this->TranslationOuterHandleActor->RenderOverlay(viewport);
    }
    if (this->TranslationInnerHandleActor->GetVisibility())
    {
      this->TranslationInnerHandleActor->RenderOverlay(viewport);
    }
    if (this->RotationHandle1Actor->GetVisibility())
    {
      this->RotationHandle1Actor->RenderOverlay(viewport);
    }
    if (this->RotationHandle2Actor->GetVisibility())
    {
      this->RotationHandle2Actor->RenderOverlay(viewport);
    }
    if (this->SliceOffsetHandle1Actor->GetVisibility())
    {
      this->SliceOffsetHandle1Actor->RenderOverlay(viewport);
    }
    if (this->SliceOffsetHandle2Actor->GetVisibility())
    {
      this->SliceOffsetHandle2Actor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine1Handle1Actor->GetVisibility())
    {
      this->ThickSlabLine1Handle1Actor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine1Handle2Actor->GetVisibility())
    {
      this->ThickSlabLine1Handle2Actor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine2Handle1Actor->GetVisibility())
    {
      this->ThickSlabLine2Handle1Actor->RenderOverlay(viewport);
    }
    if (this->ThickSlabLine2Handle2Actor->GetVisibility())
    {
      this->ThickSlabLine2Handle2Actor->RenderOverlay(viewport);
    }
    this->NeedToRender = false;
    return count;
  }

  //----------------------------------------------------------------------
  void SetIntersectionsVisibility(bool visibility)
  {
    if (static_cast<bool>(this->IntersectionLine1Actor->GetVisibility()) != visibility //
        || static_cast<bool>(this->IntersectionLine2Actor->GetVisibility()) != visibility)
    {
      this->NeedToRender = true;
    }
    this->IntersectionLine1Actor->SetVisibility(visibility);
    this->IntersectionLine2Actor->SetVisibility(visibility);
  }

  //----------------------------------------------------------------------
  void SetThicknessVisibility(bool visibility)
  {
    if (static_cast<bool>(this->ThickSlabLine1FirstHalfActor->GetVisibility()) != visibility     //
        || static_cast<bool>(this->ThickSlabLine1SecondHalfActor->GetVisibility()) != visibility //
        || static_cast<bool>(this->ThickSlabLine2FirstHalfActor->GetVisibility()) != visibility  //
        || static_cast<bool>(this->ThickSlabLine2SecondHalfActor->GetVisibility()) != visibility)
    {
      this->NeedToRender = true;
    }

    this->ThickSlabLine1FirstHalfActor->SetVisibility(visibility);
    this->ThickSlabLine1SecondHalfActor->SetVisibility(visibility);
    this->ThickSlabLine2FirstHalfActor->SetVisibility(visibility);
    this->ThickSlabLine2SecondHalfActor->SetVisibility(visibility);
  }

  //----------------------------------------------------------------------
  void SetIntersectionHandlesVisibility(bool visibility)
  {
    bool rotationHandle1Visible = visibility && (this->HandlesVisibilityMode != vtkMRMLSliceDisplayNode::NeverVisible) //
                                  && this->RotationHandlesVisible && this->RotationHandle1Displayable;
    bool rotationHandle2Visible = visibility && (this->HandlesVisibilityMode != vtkMRMLSliceDisplayNode::NeverVisible) //
                                  && this->RotationHandlesVisible && this->RotationHandle2Displayable;
    bool sliceOffsetHandle1Visible = visibility && (this->HandlesVisibilityMode != vtkMRMLSliceDisplayNode::NeverVisible) //
                                     && this->TranslationHandlesVisible && this->SliceOffsetHandle1Displayable;
    bool sliceOffsetHandle2Visible = visibility && (this->HandlesVisibilityMode != vtkMRMLSliceDisplayNode::NeverVisible) //
                                     && this->TranslationHandlesVisible && this->SliceOffsetHandle2Displayable;
    bool translationHandleVisible = visibility && (this->HandlesVisibilityMode != vtkMRMLSliceDisplayNode::NeverVisible) //
                                    && this->TranslationHandlesVisible && this->SliceIntersectionMode == vtkMRMLSliceDisplayNode::FullLines;

    if (static_cast<bool>(this->RotationHandle1Actor->GetVisibility()) != rotationHandle1Visible             //
        || static_cast<bool>(this->RotationHandle2Actor->GetVisibility()) != rotationHandle2Visible          //
        || static_cast<bool>(this->SliceOffsetHandle1Actor->GetVisibility()) != sliceOffsetHandle1Visible    //
        || static_cast<bool>(this->SliceOffsetHandle2Actor->GetVisibility()) != sliceOffsetHandle2Visible    //
        || static_cast<bool>(this->TranslationOuterHandleActor->GetVisibility()) != translationHandleVisible //
        || static_cast<bool>(this->TranslationInnerHandleActor->GetVisibility()) != translationHandleVisible)
    {
      this->NeedToRender = true;
    }
    this->RotationHandle1Actor->SetVisibility(rotationHandle1Visible);
    this->RotationHandle2Actor->SetVisibility(rotationHandle2Visible);
    this->SliceOffsetHandle1Actor->SetVisibility(sliceOffsetHandle1Visible);
    this->SliceOffsetHandle2Actor->SetVisibility(sliceOffsetHandle2Visible);
    this->TranslationOuterHandleActor->SetVisibility(translationHandleVisible);
    this->TranslationInnerHandleActor->SetVisibility(translationHandleVisible);
  }

  //----------------------------------------------------------------------
  void SetThickSlabHandlesVisibility(bool visibility)
  {
    bool thickSlabLine1Handle1Visible = visibility && this->ThickSlabHandlesVisible && this->ThickSlabLine1Handle1Displayable;
    bool thickSlabLine1Handle2Visible = visibility && this->ThickSlabHandlesVisible && this->ThickSlabLine1Handle2Displayable;
    bool thickSlabLine2Handle1Visible = visibility && this->ThickSlabHandlesVisible && this->ThickSlabLine2Handle1Displayable;
    bool thickSlabLine2Handle2Visible = visibility && this->ThickSlabHandlesVisible && this->ThickSlabLine2Handle2Displayable;

    if (static_cast<bool>(this->ThickSlabLine1Handle1Actor->GetVisibility()) != thickSlabLine1Handle1Visible    //
        || static_cast<bool>(this->ThickSlabLine1Handle2Actor->GetVisibility()) != thickSlabLine1Handle2Visible //
        || static_cast<bool>(this->ThickSlabLine2Handle1Actor->GetVisibility()) != thickSlabLine2Handle1Visible //
        || static_cast<bool>(this->ThickSlabLine2Handle2Actor->GetVisibility()) != thickSlabLine2Handle2Visible)
    {
      this->NeedToRender = true;
    }
    this->ThickSlabLine1Handle1Actor->SetVisibility(thickSlabLine1Handle1Visible);
    this->ThickSlabLine1Handle2Actor->SetVisibility(thickSlabLine1Handle2Visible);
    this->ThickSlabLine2Handle1Actor->SetVisibility(thickSlabLine2Handle1Visible);
    this->ThickSlabLine2Handle2Actor->SetVisibility(thickSlabLine2Handle2Visible);
  }

  //----------------------------------------------------------------------
  void SetHandlesOpacity(double opacity)
  {
    this->TranslationOuterHandleProperty->SetOpacity(opacity);
    this->TranslationInnerHandleProperty->SetOpacity(opacity);
    this->RotationHandle1Property->SetOpacity(opacity);
    this->RotationHandle2Property->SetOpacity(opacity);
    this->SliceOffsetHandle1Property->SetOpacity(opacity);
    this->SliceOffsetHandle2Property->SetOpacity(opacity);
  }

  //----------------------------------------------------------------------
  bool GetIntersectionVisibility() { return this->IntersectionLine1Actor->GetVisibility(); }

  //----------------------------------------------------------------------
  bool GetThickSlabVisibility() { return this->ThickSlabLine1FirstHalfActor->GetVisibility(); }

  //----------------------------------------------------------------------
  bool GetVisibility() { return this->GetIntersectionVisibility() || this->GetThickSlabVisibility(); }

  vtkSmartPointer<vtkLineSource> IntersectionLine1;
  vtkSmartPointer<vtkPolyDataMapper2D> IntersectionLine1Mapper;
  vtkSmartPointer<vtkProperty2D> IntersectionLine1Property;
  vtkSmartPointer<vtkActor2D> IntersectionLine1Actor;

  vtkSmartPointer<vtkLineSource> IntersectionLine2;
  vtkSmartPointer<vtkPolyDataMapper2D> IntersectionLine2Mapper;
  vtkSmartPointer<vtkProperty2D> IntersectionLine2Property;
  vtkSmartPointer<vtkActor2D> IntersectionLine2Actor;

  // Thick Slab line 1 first and second halves
  vtkSmartPointer<vtkLineSource> ThickSlabLine1FirstHalf;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine1FirstHalfMapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine1FirstHalfProperty;
  vtkSmartPointer<vtkActor2D> ThickSlabLine1FirstHalfActor;

  vtkSmartPointer<vtkLineSource> ThickSlabLine1SecondHalf;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine1SecondHalfMapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine1SecondHalfProperty;
  vtkSmartPointer<vtkActor2D> ThickSlabLine1SecondHalfActor;

  // Thick Slab line 2 first and second halves
  vtkSmartPointer<vtkLineSource> ThickSlabLine2FirstHalf;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine2FirstHalfMapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine2FirstHalfProperty;
  vtkSmartPointer<vtkActor2D> ThickSlabLine2FirstHalfActor;

  vtkSmartPointer<vtkLineSource> ThickSlabLine2SecondHalf;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine2SecondHalfMapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine2SecondHalfProperty;
  vtkSmartPointer<vtkActor2D> ThickSlabLine2SecondHalfActor;

  vtkSmartPointer<vtkSphereSource> TranslationOuterHandle;
  vtkSmartPointer<vtkPolyDataMapper2D> TranslationOuterHandleMapper;
  vtkSmartPointer<vtkProperty2D> TranslationOuterHandleProperty;
  vtkSmartPointer<vtkActor2D> TranslationOuterHandleActor;
  vtkSmartPointer<vtkSphereSource> TranslationInnerHandle;
  vtkSmartPointer<vtkPolyDataMapper2D> TranslationInnerHandleMapper;
  vtkSmartPointer<vtkProperty2D> TranslationInnerHandleProperty;
  vtkSmartPointer<vtkActor2D> TranslationInnerHandleActor;

  vtkSmartPointer<vtkAppendPolyData> RotationHandle1;
  vtkSmartPointer<vtkTransform> RotationHandle1ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> RotationHandle1TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> RotationHandle1Mapper;
  vtkSmartPointer<vtkProperty2D> RotationHandle1Property;
  vtkSmartPointer<vtkActor2D> RotationHandle1Actor;

  vtkSmartPointer<vtkAppendPolyData> RotationHandle2;
  vtkSmartPointer<vtkTransform> RotationHandle2ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> RotationHandle2TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> RotationHandle2Mapper;
  vtkSmartPointer<vtkProperty2D> RotationHandle2Property;
  vtkSmartPointer<vtkActor2D> RotationHandle2Actor;

  vtkSmartPointer<vtkAppendPolyData> SliceOffsetHandle1;
  vtkSmartPointer<vtkTransform> SliceOffsetHandle1ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> SliceOffsetHandle1TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> SliceOffsetHandle1Mapper;
  vtkSmartPointer<vtkProperty2D> SliceOffsetHandle1Property;
  vtkSmartPointer<vtkActor2D> SliceOffsetHandle1Actor;

  vtkSmartPointer<vtkAppendPolyData> SliceOffsetHandle2;
  vtkSmartPointer<vtkTransform> SliceOffsetHandle2ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> SliceOffsetHandle2TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> SliceOffsetHandle2Mapper;
  vtkSmartPointer<vtkProperty2D> SliceOffsetHandle2Property;
  vtkSmartPointer<vtkActor2D> SliceOffsetHandle2Actor;

  vtkSmartPointer<vtkAppendPolyData> ThickSlabLine1Handle1;
  vtkSmartPointer<vtkTransform> ThickSlabLine1Handle1ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> ThickSlabLine1Handle1TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine1Handle1Mapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine1Handle1Property;
  vtkSmartPointer<vtkActor2D> ThickSlabLine1Handle1Actor;

  vtkSmartPointer<vtkAppendPolyData> ThickSlabLine1Handle2;
  vtkSmartPointer<vtkTransform> ThickSlabLine1Handle2ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> ThickSlabLine1Handle2TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine1Handle2Mapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine1Handle2Property;
  vtkSmartPointer<vtkActor2D> ThickSlabLine1Handle2Actor;

  vtkSmartPointer<vtkAppendPolyData> ThickSlabLine2Handle1;
  vtkSmartPointer<vtkTransform> ThickSlabLine2Handle1ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> ThickSlabLine2Handle1TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine2Handle1Mapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine2Handle1Property;
  vtkSmartPointer<vtkActor2D> ThickSlabLine2Handle1Actor;

  vtkSmartPointer<vtkAppendPolyData> ThickSlabLine2Handle2;
  vtkSmartPointer<vtkTransform> ThickSlabLine2Handle2ToWorldTransform;
  vtkSmartPointer<vtkTransformPolyDataFilter> ThickSlabLine2Handle2TransformFilter;
  vtkSmartPointer<vtkPolyDataMapper2D> ThickSlabLine2Handle2Mapper;
  vtkSmartPointer<vtkProperty2D> ThickSlabLine2Handle2Property;
  vtkSmartPointer<vtkActor2D> ThickSlabLine2Handle2Actor;

  vtkWeakPointer<vtkMRMLSliceLogic> SliceLogic;
  vtkWeakPointer<vtkCallbackCommand> Callback;

  vtkSmartPointer<vtkPolyData> RotationHandlePoints;
  vtkSmartPointer<vtkPolyData> TranslationHandlePoints;
  vtkSmartPointer<vtkPolyData> SliceOffsetHandlePoints;
  vtkSmartPointer<vtkPolyData> ThickSlabHandlePoints;

  vtkSmartPointer<vtkPoints> SliceOffsetHandle1Points;
  vtkSmartPointer<vtkPoints> SliceOffsetHandle2Points;
  vtkSmartPointer<vtkPoints> RotationHandle1Points;
  vtkSmartPointer<vtkPoints> RotationHandle2Points;
  vtkSmartPointer<vtkPoints> ThickSlabLine1Handle1Points;
  vtkSmartPointer<vtkPoints> ThickSlabLine1Handle2Points;
  vtkSmartPointer<vtkPoints> ThickSlabLine2Handle1Points;
  vtkSmartPointer<vtkPoints> ThickSlabLine2Handle2Points;

  bool SliceOffsetHandle1Displayable = false;
  bool SliceOffsetHandle2Displayable = false;
  bool RotationHandle1Displayable = false;
  bool RotationHandle2Displayable = false;
  bool ThickSlabLine1Handle1Displayable = false;
  bool ThickSlabLine1Handle2Displayable = false;
  bool ThickSlabLine2Handle1Displayable = false;
  bool ThickSlabLine2Handle2Displayable = false;
  bool RotationHandlesVisible = false;
  bool TranslationHandlesVisible = false;
  bool ThickSlabHandlesVisible = false;
  int HandlesVisibilityMode = vtkMRMLSliceDisplayNode::NeverVisible;
  int SliceIntersectionMode = vtkMRMLSliceDisplayNode::SkipLineCrossings;
  // Indicates that this representation has changed and thus re-rendering is needed
  bool NeedToRender = true;
};

class vtkMRMLSliceIntersectionInteractionRepresentation::vtkInternal
{
public:
  vtkInternal(vtkMRMLSliceIntersectionInteractionRepresentation* external);
  ~vtkInternal();

  vtkMRMLSliceIntersectionInteractionRepresentation* External;

  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;
  vtkSmartPointer<vtkMRMLSliceDisplayNode> SliceDisplayNode;

  std::deque<SliceIntersectionInteractionDisplayPipeline*> SliceIntersectionInteractionDisplayPipelines;
  vtkNew<vtkCallbackCommand> SliceNodeModifiedCommand;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLSliceIntersectionInteractionRepresentation::vtkInternal::vtkInternal(vtkMRMLSliceIntersectionInteractionRepresentation* external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkMRMLSliceIntersectionInteractionRepresentation::vtkInternal::~vtkInternal() = default;

//----------------------------------------------------------------------
vtkMRMLSliceIntersectionInteractionRepresentation::vtkMRMLSliceIntersectionInteractionRepresentation()
{
  this->MRMLApplicationLogic = nullptr;

  this->Internal = new vtkInternal(this);
  this->Internal->SliceNodeModifiedCommand->SetClientData(this);
  this->Internal->SliceNodeModifiedCommand->SetCallback(vtkMRMLSliceIntersectionInteractionRepresentation::SliceNodeModifiedCallback);

  this->SliceIntersectionPoint[0] = 0.0;
  this->SliceIntersectionPoint[1] = 0.0;
  this->SliceIntersectionPoint[2] = 0.0;
  this->SliceIntersectionPoint[3] = 1.0; // to allow easy homogeneous transformations

  this->SliceIntersectionPointFound = false; // indicate whether a valid slice intersection point was found or not

  // Set interaction size. Determines the maximum distance for interaction.
  this->InteractionSize = INTERACTION_SIZE_PIXELS;

  // Helper
  this->Helper = vtkSmartPointer<vtkMRMLSliceIntersectionInteractionRepresentationHelper>::New();
}

//----------------------------------------------------------------------
vtkMRMLSliceIntersectionInteractionRepresentation::~vtkMRMLSliceIntersectionInteractionRepresentation()
{
  this->SetSliceNode(nullptr);
  this->SetMRMLApplicationLogic(nullptr);
  delete this->Internal;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::GetActors2D(vtkPropCollection* pc)
{
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    (*sliceIntersectionIt)->GetActors2D(pc);
  }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::ReleaseGraphicsResources(vtkWindow* win)
{
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    (*sliceIntersectionIt)->ReleaseGraphicsResources(win);
  }
}

//----------------------------------------------------------------------
int vtkMRMLSliceIntersectionInteractionRepresentation::RenderOverlay(vtkViewport* viewport)
{
  int count = 0;

  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    count += (*sliceIntersectionIt)->RenderOverlay(viewport);
  }
  return count;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::PrintSelf(ostream& os, vtkIndent indent)
{
  // Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::SliceNodeModifiedCallback(vtkObject* caller, unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkMRMLSliceIntersectionInteractionRepresentation* self = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast((vtkObject*)clientData);
  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);
  if (sliceNode)
  {
    // The slice view's node is modified
    self->SliceNodeModified(sliceNode);
    return;
  }

  // One of the intersecting slices are modified, update all slice intersections
  self->SliceNodeModified(self->Internal->SliceNode);
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::SliceNodeModified(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
  {
    return;
  }
  if (sliceNode == this->Internal->SliceNode)
  {
    // update all slice intersection
    for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
         sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
         ++sliceIntersectionIt)
    {
      this->UpdateSliceIntersectionDisplay(*sliceIntersectionIt);
    }
    bool handlesVisible = false;
    vtkMRMLSliceDisplayNode* displayNode = this->GetSliceDisplayNode();
    if (displayNode)
    {
      int componentType = displayNode->GetActiveComponentType();
      handlesVisible =
        componentType != vtkMRMLSliceDisplayNode::ComponentNone && componentType != vtkMRMLSliceDisplayNode::ComponentSliceIntersection; // hide handles during interaction:
    }
    this->SetPipelinesHandlesVisibility(handlesVisible);
  }
}

//----------------------------------------------------------------------
SliceIntersectionInteractionDisplayPipeline* vtkMRMLSliceIntersectionInteractionRepresentation::GetDisplayPipelineFromSliceLogic(vtkMRMLSliceLogic* sliceLogic)
{
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    if (!(*sliceIntersectionIt)->SliceLogic)
    {
      continue;
    }
    if (sliceLogic == (*sliceIntersectionIt)->SliceLogic)
    {
      // found it
      return *sliceIntersectionIt;
    }
  }
  return nullptr;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::UpdateSliceIntersectionDisplay(SliceIntersectionInteractionDisplayPipeline* pipeline)
{
  if (!pipeline || !this->Internal->SliceNode || pipeline->SliceLogic == nullptr)
  {
    return;
  }
  vtkMRMLSliceNode* intersectingSliceNode = pipeline->SliceLogic->GetSliceNode();
  if (!pipeline->SliceLogic || !this->GetVisibility()                                       //
      || !intersectingSliceNode                                                             //
      || this->Internal->SliceNode->GetViewGroup() != intersectingSliceNode->GetViewGroup() //
      || !intersectingSliceNode->IsMappedInLayout())
  {
    pipeline->SetIntersectionsVisibility(false);
    pipeline->SetThicknessVisibility(false);
    pipeline->SetIntersectionHandlesVisibility(false);
    pipeline->SetThickSlabHandlesVisibility(false);
    if (pipeline->NeedToRender)
    {
      this->NeedToRenderOn();
    }
    return;
  }

  // Set properties of slice intersection lines
  vtkMRMLSliceDisplayNode* displayNode = this->GetSliceDisplayNode(intersectingSliceNode);
  if (!displayNode)
  {
    pipeline->SetIntersectionsVisibility(false);
    pipeline->SetThicknessVisibility(false);
    pipeline->SetIntersectionHandlesVisibility(false);
    pipeline->SetThickSlabHandlesVisibility(false);
    if (pipeline->NeedToRender)
    {
      this->NeedToRenderOn();
    }
    return;
  }

  bool showThickSlabLines = displayNode->GetIntersectingThickSlabVisibility();
  bool showThickSlabHandles = showThickSlabLines && displayNode->GetIntersectingThickSlabInteractive();

  bool showInteractiveSliceIntersectionLines = displayNode->GetIntersectingSlicesVisibility();
  bool showInteractiveSliceHandles = showInteractiveSliceIntersectionLines && displayNode->GetIntersectingSlicesInteractive();

  bool isInteractive = displayNode->GetIntersectingThickSlabInteractive() || displayNode->GetIntersectingSlicesInteractive();
  if (!((showThickSlabLines || showInteractiveSliceIntersectionLines) && isInteractive))
  {
    // Nothing to do
    pipeline->SetIntersectionsVisibility(false);
    pipeline->SetThicknessVisibility(false);
    pipeline->SetIntersectionHandlesVisibility(false);
    pipeline->SetThickSlabHandlesVisibility(false);
    if (pipeline->NeedToRender)
    {
      this->NeedToRenderOn();
    }
    return;
  }

  // We know that we need the slice intersection points, find them
  // Get slice node transforms
  vtkMatrix4x4* xyToRAS = this->Internal->SliceNode->GetXYToRAS();
  vtkNew<vtkMatrix4x4> rasToXY;
  vtkMatrix4x4::Invert(xyToRAS, rasToXY);

  // Get slice intersection point XY
  this->ComputeSliceIntersectionPoint();
  double sliceIntersectionPoint[4] = { this->SliceIntersectionPoint[0], this->SliceIntersectionPoint[1], this->SliceIntersectionPoint[2], 1 };

  // Get outer intersection line tips
  vtkMatrix4x4* intersectingXYToRAS = intersectingSliceNode->GetXYToRAS();
  vtkNew<vtkMatrix4x4> intersectingXYToXY;
  vtkMatrix4x4::Multiply4x4(rasToXY, intersectingXYToRAS, intersectingXYToXY);
  double intersectionLineTip1[3] = { 0.0, 0.0, 0.0 };
  double intersectionLineTip2[3] = { 0.0, 0.0, 0.0 };
  int intersectionFound = this->Helper->GetLineTipsFromIntersectingSliceNode(intersectingSliceNode, intersectingXYToXY, intersectionLineTip1, intersectionLineTip2);
  if (!intersectionFound) // Pipelines not visible if no intersection is found
  {
    pipeline->SetIntersectionsVisibility(false);
    pipeline->SetIntersectionHandlesVisibility(false);
    if (pipeline->NeedToRender)
    {
      this->NeedToRenderOn();
    }
    return;
  }

  // Get current slice view bounds
  vtkMRMLSliceNode* currentSliceNode = this->GetSliceNode();
  double sliceViewBounds[4] = {};
  this->Helper->GetSliceViewBoundariesXY(currentSliceNode, sliceViewBounds);
  double sliceViewWidth = sliceViewBounds[1] - sliceViewBounds[0];

  // Add margin to slice view bounds
  sliceViewBounds[0] = sliceViewBounds[0] + (sliceViewBounds[1] - sliceViewBounds[0]) * FOV_HANDLES_MARGIN;
  sliceViewBounds[1] = sliceViewBounds[1] - (sliceViewBounds[1] - sliceViewBounds[0]) * FOV_HANDLES_MARGIN;
  sliceViewBounds[2] = sliceViewBounds[2] + (sliceViewBounds[3] - sliceViewBounds[2]) * FOV_HANDLES_MARGIN;
  sliceViewBounds[3] = sliceViewBounds[3] - (sliceViewBounds[3] - sliceViewBounds[2]) * FOV_HANDLES_MARGIN;

  // Define slice intersection point if not visible
  if (this->SliceIntersectionPointFound == false)
  {
    // Mid-point of intersection line
    sliceIntersectionPoint[0] = (intersectionLineTip1[0] + intersectionLineTip2[0]) / 2.0;
    sliceIntersectionPoint[1] = (intersectionLineTip1[1] + intersectionLineTip2[1]) / 2.0;
    sliceIntersectionPoint[2] = (intersectionLineTip1[2] + intersectionLineTip2[2]) / 2.0;
    this->SliceIntersectionPoint[0] = sliceIntersectionPoint[0];
    this->SliceIntersectionPoint[1] = sliceIntersectionPoint[1];
    this->SliceIntersectionPoint[2] = sliceIntersectionPoint[2];
  }

  // Get outer intersection line tips adjusted to FOV margins
  double intersectionOuterLineTip1[3] = { intersectionLineTip1[0], intersectionLineTip1[1], intersectionLineTip1[2] };
  double intersectionOuterLineTip2[3] = { intersectionLineTip2[0], intersectionLineTip2[1], intersectionLineTip2[2] };
  if ((sliceIntersectionPoint[0] > sliceViewBounds[0]) && // If intersection point is within FOV
      (sliceIntersectionPoint[0] < sliceViewBounds[1]) && //
      (sliceIntersectionPoint[1] > sliceViewBounds[2]) && //
      (sliceIntersectionPoint[1] < sliceViewBounds[3]))
  {
    if ((intersectionOuterLineTip1[0] < sliceViewBounds[0]) || // If line tip 1 is outside the FOV
        (intersectionOuterLineTip1[0] > sliceViewBounds[1]) || //
        (intersectionOuterLineTip1[1] < sliceViewBounds[2]) || //
        (intersectionOuterLineTip1[1] > sliceViewBounds[3]))
    {
      this->Helper->GetIntersectionWithSliceViewBoundaries(intersectionOuterLineTip1, sliceIntersectionPoint, sliceViewBounds, intersectionOuterLineTip1);
    }
    if ((intersectionOuterLineTip2[0] < sliceViewBounds[0]) || // If line tip 2 is outside the FOV
        (intersectionOuterLineTip2[0] > sliceViewBounds[1]) || //
        (intersectionOuterLineTip2[1] < sliceViewBounds[2]) || //
        (intersectionOuterLineTip2[1] > sliceViewBounds[3]))
    {
      this->Helper->GetIntersectionWithSliceViewBoundaries(intersectionOuterLineTip2, sliceIntersectionPoint, sliceViewBounds, intersectionOuterLineTip2);
    }
  }

  // Get inner intersection line tips
  double intersectionInnerLineTip1[3] = { 0.0, 0.0, 0.0 };
  double intersectionInnerLineTip2[3] = { 0.0, 0.0, 0.0 };
  if (this->SliceIntersectionPointFound == false)
  {
    // If no slice intersections, define inner line tips in segment center and avoid gap in line
    intersectionInnerLineTip1[0] = (intersectionOuterLineTip1[0] + intersectionOuterLineTip2[0]) / 2.0;
    intersectionInnerLineTip1[1] = (intersectionOuterLineTip1[1] + intersectionOuterLineTip2[1]) / 2.0;
    intersectionInnerLineTip2[0] = (intersectionOuterLineTip1[0] + intersectionOuterLineTip2[0]) / 2.0;
    intersectionInnerLineTip2[1] = (intersectionOuterLineTip1[1] + intersectionOuterLineTip2[1]) / 2.0;
  }
  else
  {
    // If slice intersections, use slice intersection point to define inner and outer line tips
    if (pipeline->SliceIntersectionMode == vtkMRMLSliceDisplayNode::FullLines)
    {
      intersectionInnerLineTip1[0] = sliceIntersectionPoint[0];
      intersectionInnerLineTip1[1] = sliceIntersectionPoint[1];
      intersectionInnerLineTip2[0] = sliceIntersectionPoint[0];
      intersectionInnerLineTip2[1] = sliceIntersectionPoint[1];
    }
    else if (pipeline->SliceIntersectionMode == vtkMRMLSliceDisplayNode::SkipLineCrossings)
    {
      double intersectionPointToOuterLineTip1[3] = { intersectionOuterLineTip1[0] - sliceIntersectionPoint[0], intersectionOuterLineTip1[1] - sliceIntersectionPoint[1], 0.0 };
      double intersectionPointToOuterLineTip2[3] = { intersectionOuterLineTip2[0] - sliceIntersectionPoint[0], intersectionOuterLineTip2[1] - sliceIntersectionPoint[1], 0.0 };
      double intersectionPointToOuterLineTip1Distance = vtkMath::Norm(intersectionPointToOuterLineTip1);
      double intersectionPointToOuterLineTip2Distance = vtkMath::Norm(intersectionPointToOuterLineTip2);
      vtkMath::Normalize(intersectionPointToOuterLineTip1);
      vtkMath::Normalize(intersectionPointToOuterLineTip2);

      // Compute angle between intersection line segments
      double dotProduct = vtkMath::Dot(intersectionPointToOuterLineTip1, intersectionPointToOuterLineTip2);
      double cosineValue = dotProduct / (vtkMath::Norm(intersectionPointToOuterLineTip1) * vtkMath::Norm(intersectionPointToOuterLineTip2));
      if (cosineValue > 1.0)
      {
        cosineValue = 1.0;
      }
      if (cosineValue < -1.0)
      {
        cosineValue = -1.0;
      }
      double angleDeg = acos(cosineValue) * (180.0 / vtkMath::Pi());
      if (angleDeg < 90.0) // Avoid parallel line segments oriented in the same direction
      {
        // Define inner line tips in segment center
        intersectionInnerLineTip1[0] = (intersectionOuterLineTip1[0] + intersectionOuterLineTip2[0]) / 2.0;
        intersectionInnerLineTip1[1] = (intersectionOuterLineTip1[1] + intersectionOuterLineTip2[1]) / 2.0;
        intersectionInnerLineTip2[0] = (intersectionOuterLineTip1[0] + intersectionOuterLineTip2[0]) / 2.0;
        intersectionInnerLineTip2[1] = (intersectionOuterLineTip1[1] + intersectionOuterLineTip2[1]) / 2.0;
      }
      else
      {
        // Remove line segments shorter than gap size
        double gapSize = sliceViewWidth * HIDE_INTERSECTION_GAP_SIZE; // gap size computed according to slice view size
        if (intersectionPointToOuterLineTip1Distance > gapSize)       // line segment visible
        {
          intersectionInnerLineTip1[0] = sliceIntersectionPoint[0] + intersectionPointToOuterLineTip1[0] * gapSize;
          intersectionInnerLineTip1[1] = sliceIntersectionPoint[1] + intersectionPointToOuterLineTip1[1] * gapSize;
        }
        else // line segment not visible
        {
          intersectionInnerLineTip1[0] = intersectionOuterLineTip1[0];
          intersectionInnerLineTip1[1] = intersectionOuterLineTip1[1];
        }
        if (intersectionPointToOuterLineTip2Distance > gapSize) // line segment visible
        {
          intersectionInnerLineTip2[0] = sliceIntersectionPoint[0] + intersectionPointToOuterLineTip2[0] * gapSize;
          intersectionInnerLineTip2[1] = sliceIntersectionPoint[1] + intersectionPointToOuterLineTip2[1] * gapSize;
        }
        else // line segment not visible
        {
          intersectionInnerLineTip2[0] = intersectionOuterLineTip2[0];
          intersectionInnerLineTip2[1] = intersectionOuterLineTip2[1];
        }
      }
    }
    else
    {
      vtkWarningMacro("vtkMRMLSliceIntersectionInteractionRepresentation::UpdateSliceIntersectionDisplay failed: unknown visualization mode.");
      if (pipeline->NeedToRender)
      {
        this->NeedToRenderOn();
      }
    }
  }

  double sliceOffsetHandleOrientation2D[2] = { intersectionOuterLineTip2[1] - intersectionOuterLineTip1[1],
                                               intersectionOuterLineTip1[0] - intersectionOuterLineTip2[0] }; // perpendicular to intersection line

  // Define intersection lines, regardless of the visibility
  pipeline->IntersectionLine1->SetPoint1(intersectionLineTip1);
  pipeline->IntersectionLine1->SetPoint2(intersectionInnerLineTip1);
  pipeline->IntersectionLine2->SetPoint1(intersectionLineTip2);
  pipeline->IntersectionLine2->SetPoint2(intersectionInnerLineTip2);

  if (showInteractiveSliceIntersectionLines)
  {
    // Set line properties
    pipeline->IntersectionLine1Property->SetLineWidth(this->GetLineThicknessFromMode(displayNode->GetIntersectingSlicesLineThicknessMode()));
    pipeline->IntersectionLine2Property->SetLineWidth(this->GetLineThicknessFromMode(displayNode->GetIntersectingSlicesLineThicknessMode()));
    pipeline->IntersectionLine1Property->SetColor(intersectingSliceNode->GetLayoutColor());
    pipeline->IntersectionLine2Property->SetColor(intersectingSliceNode->GetLayoutColor());
    if (showInteractiveSliceHandles)
    {
      // set handle properties
      pipeline->TranslationHandlesVisible = displayNode->GetIntersectingSlicesTranslationEnabled();
      pipeline->RotationHandlesVisible = displayNode->GetIntersectingSlicesRotationEnabled();
      pipeline->HandlesVisibilityMode = displayNode->GetIntersectingSlicesInteractiveHandlesVisibilityMode();
      pipeline->SliceIntersectionMode = displayNode->GetIntersectingSlicesIntersectionMode();
      pipeline->TranslationOuterHandleProperty->SetColor(0, 0, 0);       // black color
      pipeline->TranslationInnerHandleProperty->SetColor(255, 255, 255); // white color
      pipeline->RotationHandle1Property->SetColor(intersectingSliceNode->GetLayoutColor());
      pipeline->RotationHandle2Property->SetColor(intersectingSliceNode->GetLayoutColor());
      pipeline->SliceOffsetHandle1Property->SetColor(intersectingSliceNode->GetLayoutColor());
      pipeline->SliceOffsetHandle2Property->SetColor(intersectingSliceNode->GetLayoutColor());
      // Determine visibility of handles according to line spacing
      double line1Vector[2] = { intersectionOuterLineTip1[0] - intersectionInnerLineTip1[0], intersectionOuterLineTip1[1] - intersectionInnerLineTip1[1] };
      double line1VectorNorm = vtkMath::Norm2D(line1Vector);
      double line2Vector[2] = { intersectionOuterLineTip2[0] - intersectionInnerLineTip2[0], intersectionOuterLineTip2[1] - intersectionInnerLineTip2[1] };
      double line2VectorNorm = vtkMath::Norm2D(line2Vector);
      vtkMath::Normalize2D(line1Vector);
      vtkMath::Normalize2D(line2Vector);
      double dotProduct = vtkMath::Dot2D(line1Vector, line2Vector);
      if (line1VectorNorm > HANDLES_MIN_LINE_LENGTH)
      {
        pipeline->RotationHandle1Displayable = true;
        pipeline->SliceOffsetHandle1Displayable = true;
      }
      else // hide handles if there is no sufficient space in line
      {
        pipeline->RotationHandle1Displayable = false;
        pipeline->SliceOffsetHandle1Displayable = false;
      }
      if (line2VectorNorm > HANDLES_MIN_LINE_LENGTH)
      {
        pipeline->RotationHandle2Displayable = true;
        pipeline->SliceOffsetHandle2Displayable = true;
      }
      else // hide handles if there is no sufficient space in line
      {
        pipeline->RotationHandle2Displayable = false;
        pipeline->SliceOffsetHandle2Displayable = false;
      }
      if (dotProduct > 0.0)
      // this means vectors are oriented towards the same direction
      // This may occur when using "SkipLineCrossings" intersection mode.
      {
        if (line1VectorNorm < line2VectorNorm)
        // hide handles closer to intersection point
        {
          pipeline->RotationHandle1Displayable = false;
          pipeline->SliceOffsetHandle1Displayable = false;
        }
        else
        // hide handles closer to intersection point
        {
          pipeline->RotationHandle2Displayable = false;
          pipeline->SliceOffsetHandle2Displayable = false;
        }
      }

      // Set translation handle position
      vtkNew<vtkPoints> translationHandlePoints;
      double translationHandlePosition[3] = { sliceIntersectionPoint[0], sliceIntersectionPoint[1], 0.0 };
      pipeline->TranslationOuterHandle->SetCenter(translationHandlePosition[0], translationHandlePosition[1], translationHandlePosition[2]);
      pipeline->TranslationInnerHandle->SetCenter(translationHandlePosition[0], translationHandlePosition[1], translationHandlePosition[2]);
      translationHandlePoints->InsertNextPoint(translationHandlePosition);
      pipeline->TranslationHandlePoints->SetPoints(translationHandlePoints);

      // Set position of rotation handles
      double rotationHandle1Position[2] = { intersectionOuterLineTip1[0], intersectionOuterLineTip1[1] };
      double rotationHandle2Position[2] = { intersectionOuterLineTip2[0], intersectionOuterLineTip2[1] };
      double rotationHandle1Orientation[2] = { intersectionInnerLineTip1[1] - intersectionOuterLineTip1[1],
                                               intersectionOuterLineTip1[0] - intersectionInnerLineTip1[0] }; // perpendicular to intersection line segment
      double rotationHandle2Orientation[2] = { intersectionInnerLineTip2[1] - intersectionOuterLineTip2[1],
                                               intersectionOuterLineTip2[0] - intersectionInnerLineTip2[0] }; // perpendicular to intersection line segment
      vtkNew<vtkMatrix4x4> rotationHandle1ToWorldMatrix, rotationHandle2ToWorldMatrix;                        // Compute transformation matrix (rotation + translation)
      this->Helper->ComputeHandleToWorldTransformMatrix(rotationHandle1Position, rotationHandle1Orientation, rotationHandle1ToWorldMatrix);
      this->Helper->ComputeHandleToWorldTransformMatrix(rotationHandle2Position, rotationHandle2Orientation, rotationHandle2ToWorldMatrix);
      pipeline->RotationHandle1ToWorldTransform->Identity();
      pipeline->RotationHandle1ToWorldTransform->SetMatrix(rotationHandle1ToWorldMatrix); // Update handles to world transform
      pipeline->RotationHandle2ToWorldTransform->Identity();
      pipeline->RotationHandle2ToWorldTransform->SetMatrix(rotationHandle2ToWorldMatrix); // Update handles to world transform

      // Update rotation handle interaction points
      vtkNew<vtkPoints> rotationHandlePoints;
      if (pipeline->RotationHandle1Displayable) // Handle 1
      {
        double rotationHandle1Point[3] = { 0.0, 0.0, 0.0 };
        double rotationHandle1Point_h[4] = { 0.0, 0.0, 0.0, 1.0 }; // homogeneous coordinates
        for (int i = 0; i < pipeline->RotationHandle1Points->GetNumberOfPoints(); i++)
        {
          pipeline->RotationHandle1Points->GetPoint(i, rotationHandle1Point);
          rotationHandle1Point_h[0] = rotationHandle1Point[0];
          rotationHandle1Point_h[1] = rotationHandle1Point[1];
          rotationHandle1Point_h[2] = rotationHandle1Point[2];
          rotationHandle1ToWorldMatrix->MultiplyPoint(rotationHandle1Point_h, rotationHandle1Point_h);
          rotationHandle1Point[0] = rotationHandle1Point_h[0];
          rotationHandle1Point[1] = rotationHandle1Point_h[1];
          rotationHandle1Point[2] = rotationHandle1Point_h[2];
          rotationHandlePoints->InsertNextPoint(rotationHandle1Point);
        }
      }
      if (pipeline->RotationHandle2Displayable) // Handle 2
      {
        double rotationHandle2Point[3] = { 0.0, 0.0, 0.0 };
        double rotationHandle2Point_h[4] = { 0.0, 0.0, 0.0, 1.0 }; // homogeneous coordinates
        for (int j = 0; j < pipeline->RotationHandle2Points->GetNumberOfPoints(); j++)
        {
          pipeline->RotationHandle2Points->GetPoint(j, rotationHandle2Point);
          rotationHandle2Point_h[0] = rotationHandle2Point[0];
          rotationHandle2Point_h[1] = rotationHandle2Point[1];
          rotationHandle2Point_h[2] = rotationHandle2Point[2];
          rotationHandle2ToWorldMatrix->MultiplyPoint(rotationHandle2Point_h, rotationHandle2Point_h);
          rotationHandle2Point[0] = rotationHandle2Point_h[0];
          rotationHandle2Point[1] = rotationHandle2Point_h[1];
          rotationHandle2Point[2] = rotationHandle2Point_h[2];
          rotationHandlePoints->InsertNextPoint(rotationHandle2Point);
        }
      }

      // Set position of slice offset handles
      double sliceOffsetHandle1Position[2] = { (rotationHandle1Position[0] + sliceIntersectionPoint[0]) / 2, (rotationHandle1Position[1] + sliceIntersectionPoint[1]) / 2 };
      double sliceOffsetHandle2Position[2] = { (rotationHandle2Position[0] + sliceIntersectionPoint[0]) / 2, (rotationHandle2Position[1] + sliceIntersectionPoint[1]) / 2 };
      vtkNew<vtkMatrix4x4> sliceOffsetHandle1ToWorldMatrix, sliceOffsetHandle2ToWorldMatrix; // Compute transformation matrix (rotation + translation)
      this->Helper->ComputeHandleToWorldTransformMatrix(sliceOffsetHandle1Position, sliceOffsetHandleOrientation2D, sliceOffsetHandle1ToWorldMatrix);
      this->Helper->ComputeHandleToWorldTransformMatrix(sliceOffsetHandle2Position, sliceOffsetHandleOrientation2D, sliceOffsetHandle2ToWorldMatrix);
      pipeline->SliceOffsetHandle1ToWorldTransform->Identity();
      pipeline->SliceOffsetHandle1ToWorldTransform->SetMatrix(sliceOffsetHandle1ToWorldMatrix); // Update handles to world transform
      pipeline->SliceOffsetHandle2ToWorldTransform->Identity();
      pipeline->SliceOffsetHandle2ToWorldTransform->SetMatrix(sliceOffsetHandle2ToWorldMatrix); // Update handles to world transform

      // Update slice offset handle interaction points
      vtkNew<vtkPoints> sliceOffsetHandlePoints;
      double sliceOffsetHandlePoint[3] = { 0.0, 0.0, 0.0 };
      double sliceOffsetHandlePoint_h[4] = { 0.0, 0.0, 0.0, 1.0 }; // homogeneous coordinates
      if (pipeline->SliceOffsetHandle1Displayable)                 // Handle 1
      {
        for (int i = 0; i < pipeline->SliceOffsetHandle1Points->GetNumberOfPoints(); i++)
        {
          pipeline->SliceOffsetHandle1Points->GetPoint(i, sliceOffsetHandlePoint);
          sliceOffsetHandlePoint_h[0] = sliceOffsetHandlePoint[0];
          sliceOffsetHandlePoint_h[1] = sliceOffsetHandlePoint[1];
          sliceOffsetHandlePoint_h[2] = sliceOffsetHandlePoint[2];
          sliceOffsetHandle1ToWorldMatrix->MultiplyPoint(sliceOffsetHandlePoint_h, sliceOffsetHandlePoint_h);
          sliceOffsetHandlePoint[0] = sliceOffsetHandlePoint_h[0];
          sliceOffsetHandlePoint[1] = sliceOffsetHandlePoint_h[1];
          sliceOffsetHandlePoint[2] = sliceOffsetHandlePoint_h[2];
          sliceOffsetHandlePoints->InsertNextPoint(sliceOffsetHandlePoint);
        }
      }
      if (pipeline->SliceOffsetHandle2Displayable) // Handle 2
      {
        for (int j = 0; j < pipeline->SliceOffsetHandle2Points->GetNumberOfPoints(); j++)
        {
          pipeline->SliceOffsetHandle2Points->GetPoint(j, sliceOffsetHandlePoint);
          sliceOffsetHandlePoint_h[0] = sliceOffsetHandlePoint[0];
          sliceOffsetHandlePoint_h[1] = sliceOffsetHandlePoint[1];
          sliceOffsetHandlePoint_h[2] = sliceOffsetHandlePoint[2];
          sliceOffsetHandle2ToWorldMatrix->MultiplyPoint(sliceOffsetHandlePoint_h, sliceOffsetHandlePoint_h);
          sliceOffsetHandlePoint[0] = sliceOffsetHandlePoint_h[0];
          sliceOffsetHandlePoint[1] = sliceOffsetHandlePoint_h[1];
          sliceOffsetHandlePoint[2] = sliceOffsetHandlePoint_h[2];
          sliceOffsetHandlePoints->InsertNextPoint(sliceOffsetHandlePoint);
        }
      }

      // Define points along intersection lines for interaction
      vtkPoints* line1PointsDefault;
      vtkPoints* line2PointsDefault;
      line1PointsDefault = pipeline->IntersectionLine1->GetOutput()->GetPoints();
      line2PointsDefault = pipeline->IntersectionLine2->GetOutput()->GetPoints();

      // Get closer handles to intersection line 1
      vtkIdType numLinePoints;
      double linePoint[3] = { 0.0, 0.0, 0.0 };
      double linePointToRotationHandle1[2] = { 0.0, 0.0 };
      double linePointToSliceOffsetHandle1[2] = { 0.0, 0.0 };
      double linePointToRotationHandle1Distance;
      double linePointToSliceOffsetHandle1Distance;
      numLinePoints = line1PointsDefault->GetNumberOfPoints();
      for (int i = 0; i < numLinePoints; i++)
      {
        // Get line point coordinates
        line1PointsDefault->GetPoint(i, linePoint);
        // Distance to rotation handle
        linePointToRotationHandle1[0] = rotationHandle1Position[0] - linePoint[0];
        linePointToRotationHandle1[1] = rotationHandle1Position[1] - linePoint[1];
        linePointToRotationHandle1Distance = vtkMath::Norm2D(linePointToRotationHandle1);
        // Distance to slice offset handle
        linePointToSliceOffsetHandle1[0] = sliceOffsetHandle1Position[0] - linePoint[0];
        linePointToSliceOffsetHandle1[1] = sliceOffsetHandle1Position[1] - linePoint[1];
        linePointToSliceOffsetHandle1Distance = vtkMath::Norm2D(linePointToSliceOffsetHandle1);
        // Determine closer handle and insert interaction points
        if (linePointToRotationHandle1Distance < linePointToSliceOffsetHandle1Distance)
        {
          rotationHandlePoints->InsertNextPoint(linePoint);
        }
        else
        {
          sliceOffsetHandlePoints->InsertNextPoint(linePoint);
        }
      }

      // Get closer handles to intersection line 2
      double linePointToRotationHandle2[2] = { 0.0, 0.0 };
      double linePointToSliceOffsetHandle2[2] = { 0.0, 0.0 };
      double linePointToRotationHandle2Distance;
      double linePointToSliceOffsetHandle2Distance;
      numLinePoints = line2PointsDefault->GetNumberOfPoints();
      for (int i = 0; i < numLinePoints; i++)
      {
        // Get line point coordinates
        line2PointsDefault->GetPoint(i, linePoint);
        // Distance to rotation handle
        linePointToRotationHandle2[0] = rotationHandle2Position[0] - linePoint[0];
        linePointToRotationHandle2[1] = rotationHandle2Position[1] - linePoint[1];
        linePointToRotationHandle2Distance = vtkMath::Norm2D(linePointToRotationHandle2);
        // Distance to slice offset handle
        linePointToSliceOffsetHandle2[0] = sliceOffsetHandle2Position[0] - linePoint[0];
        linePointToSliceOffsetHandle2[1] = sliceOffsetHandle2Position[1] - linePoint[1];
        linePointToSliceOffsetHandle2Distance = vtkMath::Norm2D(linePointToSliceOffsetHandle2);
        // Determine closer handle and insert interaction points
        if (linePointToRotationHandle2Distance < linePointToSliceOffsetHandle2Distance)
        {
          rotationHandlePoints->InsertNextPoint(linePoint);
        }
        else
        {
          sliceOffsetHandlePoints->InsertNextPoint(linePoint);
        }
      }
      // Set interaction points
      pipeline->RotationHandlePoints->SetPoints(rotationHandlePoints);
      pipeline->SliceOffsetHandlePoints->SetPoints(sliceOffsetHandlePoints);

      // Visibility
      pipeline->SetIntersectionHandlesVisibility(true);
    }
    else
    {
      pipeline->SetIntersectionHandlesVisibility(false);
    }
    pipeline->SetIntersectionsVisibility(true);
  }
  else
  {
    pipeline->SetIntersectionsVisibility(false);
    pipeline->SetIntersectionHandlesVisibility(false);
  }

  if (showThickSlabLines)
  {
    double thickSlabLine1FirstHalfPoint1[4] = { intersectionLineTip1[0], intersectionLineTip1[1], intersectionLineTip1[2], 1 };
    double thickSlabLine1FirstHalfPoint2[4] = { sliceIntersectionPoint[0], sliceIntersectionPoint[1], sliceIntersectionPoint[2], 1 };
    double thickSlabLine1SecondHalfPoint1[4] = { intersectionLineTip2[0], intersectionLineTip2[1], intersectionLineTip2[2], 1 };
    double thickSlabLine1SecondHalfPoint2[4] = { sliceIntersectionPoint[0], sliceIntersectionPoint[1], sliceIntersectionPoint[2], 1 };
    double thickSlabLine2FirstHalfPoint1[4] = { intersectionLineTip1[0], intersectionLineTip1[1], intersectionLineTip1[2], 1 };
    double thickSlabLine2FirstHalfPoint2[4] = { sliceIntersectionPoint[0], sliceIntersectionPoint[1], sliceIntersectionPoint[2], 1 };
    double thickSlabLine2SecondHalfPoint1[4] = { intersectionLineTip2[0], intersectionLineTip2[1], intersectionLineTip2[2], 1 };
    double thickSlabLine2SecondHalfPoint2[4] = { sliceIntersectionPoint[0], sliceIntersectionPoint[1], sliceIntersectionPoint[2], 1 };

    double slabThickness = intersectingSliceNode->GetSlabReconstructionThickness() / 2;

    // Find the angle of the intersection line
    double ydiff = intersectionLineTip1[1] - intersectionLineTip2[1];
    double xdiff = intersectionLineTip1[0] - intersectionLineTip2[0];
    double angle = atan2(ydiff, xdiff); // In radians

    // Find line normal to the slice intersection line in XY coords
    double normalAngle = angle + vtkMath::Pi() / 2;
    double offsetUnitVector_XY[3] = { cos(normalAngle), sin(normalAngle), 0 };

    // Find that offset vector in RAS space
    vtkSmartPointer<vtkMatrix3x3> xyToRas3x3 = vtkSmartPointer<vtkMatrix3x3>::New();
    xyToRas3x3->Identity();
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        double val = xyToRAS->GetElement(i, j);
        xyToRas3x3->SetElement(i, j, val);
      }
    }

    double offsetVectorRas[3] = { 0, 0, 0 };
    xyToRas3x3->MultiplyPoint(offsetUnitVector_XY, offsetVectorRas);

    // Normalize
    vtkMath::Normalize(offsetVectorRas);

    // Scale by the thickness
    offsetVectorRas[0] *= slabThickness;
    offsetVectorRas[1] *= slabThickness;
    offsetVectorRas[2] *= slabThickness;

    // Now map back to XY
    double offsetVector_XY[3] = { 0, 0, 0 };
    vtkNew<vtkMatrix3x3> rasToXY3x3;
    vtkMatrix3x3::Invert(xyToRas3x3, rasToXY3x3);
    rasToXY3x3->MultiplyPoint(offsetVectorRas, offsetVector_XY);

    // Translate the lines along the normal vector
    // First half of first line
    thickSlabLine1FirstHalfPoint1[0] += offsetVector_XY[0];
    thickSlabLine1FirstHalfPoint1[1] += offsetVector_XY[1];
    thickSlabLine1FirstHalfPoint2[0] += offsetVector_XY[0];
    thickSlabLine1FirstHalfPoint2[1] += offsetVector_XY[1];
    // Second half of first line
    thickSlabLine1SecondHalfPoint1[0] += offsetVector_XY[0];
    thickSlabLine1SecondHalfPoint1[1] += offsetVector_XY[1];
    thickSlabLine1SecondHalfPoint2[0] += offsetVector_XY[0];
    thickSlabLine1SecondHalfPoint2[1] += offsetVector_XY[1];
    // First half of second line
    thickSlabLine2FirstHalfPoint1[0] -= offsetVector_XY[0];
    thickSlabLine2FirstHalfPoint1[1] -= offsetVector_XY[1];
    thickSlabLine2FirstHalfPoint2[0] -= offsetVector_XY[0];
    thickSlabLine2FirstHalfPoint2[1] -= offsetVector_XY[1];
    // Second half of second line
    thickSlabLine2SecondHalfPoint1[0] -= offsetVector_XY[0];
    thickSlabLine2SecondHalfPoint1[1] -= offsetVector_XY[1];
    thickSlabLine2SecondHalfPoint2[0] -= offsetVector_XY[0];
    thickSlabLine2SecondHalfPoint2[1] -= offsetVector_XY[1];

    // Now update the points
    pipeline->ThickSlabLine1FirstHalf->SetPoint1(thickSlabLine1FirstHalfPoint1);
    pipeline->ThickSlabLine1FirstHalf->SetPoint2(thickSlabLine1FirstHalfPoint2);
    pipeline->ThickSlabLine1SecondHalf->SetPoint1(thickSlabLine1SecondHalfPoint1);
    pipeline->ThickSlabLine1SecondHalf->SetPoint2(thickSlabLine1SecondHalfPoint2);
    pipeline->ThickSlabLine2FirstHalf->SetPoint1(thickSlabLine2FirstHalfPoint1);
    pipeline->ThickSlabLine2FirstHalf->SetPoint2(thickSlabLine2FirstHalfPoint2);
    pipeline->ThickSlabLine2SecondHalf->SetPoint1(thickSlabLine2SecondHalfPoint1);
    pipeline->ThickSlabLine2SecondHalf->SetPoint2(thickSlabLine2SecondHalfPoint2);

    // Now the corresponding properties
    pipeline->ThickSlabLine1FirstHalfProperty->SetLineWidth(displayNode->GetLineWidth());
    pipeline->ThickSlabLine1FirstHalfProperty->SetColor(intersectingSliceNode->GetLayoutColor());
    pipeline->ThickSlabLine1SecondHalfProperty->SetLineWidth(displayNode->GetLineWidth());
    pipeline->ThickSlabLine1SecondHalfProperty->SetColor(intersectingSliceNode->GetLayoutColor());
    pipeline->ThickSlabLine2FirstHalfProperty->SetLineWidth(displayNode->GetLineWidth());
    pipeline->ThickSlabLine2FirstHalfProperty->SetColor(intersectingSliceNode->GetLayoutColor());
    pipeline->ThickSlabLine2SecondHalfProperty->SetLineWidth(displayNode->GetLineWidth());
    pipeline->ThickSlabLine2SecondHalfProperty->SetColor(intersectingSliceNode->GetLayoutColor());

    // First we'll need to find which handles are visible for each line segment
    // Calculate the length of each line half for each thick slab line
    if (showThickSlabHandles)
    {
      // First half of the first thick slab line
      double thickSlabLine1FirstHalfVector[2] = { thickSlabLine1FirstHalfPoint1[0] - thickSlabLine1FirstHalfPoint2[0],
                                                  thickSlabLine1FirstHalfPoint1[1] - thickSlabLine1FirstHalfPoint2[1] };
      double thickSlabLine1FirstHalfVectorNorm = vtkMath::Norm2D(thickSlabLine1FirstHalfVector);
      // Second half of the first thick slab line
      double thickSlabLine1SecondHalfVector[2] = { thickSlabLine1SecondHalfPoint1[0] - thickSlabLine1SecondHalfPoint2[0],
                                                   thickSlabLine1SecondHalfPoint1[1] - thickSlabLine1SecondHalfPoint2[1] };
      double thickSlabLine1SecondHalfVectorNorm = vtkMath::Norm2D(thickSlabLine1SecondHalfVector);
      // First half of the second thick slab line
      double thickSlabLine2FirstHalfVector[2] = { thickSlabLine2FirstHalfPoint1[0] - thickSlabLine2FirstHalfPoint2[0],
                                                  thickSlabLine2FirstHalfPoint1[1] - thickSlabLine2FirstHalfPoint2[1] };
      double thickSlabLine2FirstHalfVectorNorm = vtkMath::Norm2D(thickSlabLine2FirstHalfVector);
      // Second half of the first thick slab line
      double thickSlabLine2SecondHalfVector[2] = { thickSlabLine2SecondHalfPoint1[0] - thickSlabLine2SecondHalfPoint2[0],
                                                   thickSlabLine2SecondHalfPoint1[1] - thickSlabLine2SecondHalfPoint2[1] };
      double thickSlabLine2SecondHalfVectorNorm = vtkMath::Norm2D(thickSlabLine2SecondHalfVector);

      pipeline->ThickSlabLine1Handle1Displayable = (thickSlabLine1FirstHalfVectorNorm > THICK_SLAB_HANDLES_MIN_LINE_LENGTH);
      pipeline->ThickSlabLine1Handle2Displayable = (thickSlabLine1SecondHalfVectorNorm > THICK_SLAB_HANDLES_MIN_LINE_LENGTH);
      pipeline->ThickSlabLine2Handle1Displayable = (thickSlabLine2FirstHalfVectorNorm > THICK_SLAB_HANDLES_MIN_LINE_LENGTH);
      pipeline->ThickSlabLine2Handle2Displayable = (thickSlabLine2SecondHalfVectorNorm > THICK_SLAB_HANDLES_MIN_LINE_LENGTH);

      // Set position of thick slab offset handles
      double thickSlabLine1Handle1Position[2] = { (thickSlabLine1FirstHalfPoint1[0] + thickSlabLine1FirstHalfPoint2[0]) / 2,
                                                  (thickSlabLine1FirstHalfPoint1[1] + thickSlabLine1FirstHalfPoint2[1]) / 2 };
      double thickSlabLine1Handle2Position[2] = { (thickSlabLine1SecondHalfPoint1[0] + thickSlabLine1SecondHalfPoint2[0]) / 2,
                                                  (thickSlabLine1SecondHalfPoint1[1] + thickSlabLine1SecondHalfPoint2[1]) / 2 };
      double thickSlabLine2Handle1Position[2] = { (thickSlabLine2FirstHalfPoint1[0] + thickSlabLine2FirstHalfPoint2[0]) / 2,
                                                  (thickSlabLine2FirstHalfPoint1[1] + thickSlabLine2FirstHalfPoint2[1]) / 2 };
      double thickSlabLine2Handle2Position[2] = { (thickSlabLine2SecondHalfPoint1[0] + thickSlabLine2SecondHalfPoint2[0]) / 2,
                                                  (thickSlabLine2SecondHalfPoint1[1] + thickSlabLine2SecondHalfPoint2[1]) / 2 };
      vtkNew<vtkMatrix4x4> thickSlabLine1Handle1ToWorldMatrix;
      vtkNew<vtkMatrix4x4> thickSlabLine1Handle2ToWorldMatrix;
      vtkNew<vtkMatrix4x4> thickSlabLine2Handle1ToWorldMatrix;
      vtkNew<vtkMatrix4x4> thickSlabLine2Handle2ToWorldMatrix;
      this->Helper->ComputeHandleToWorldTransformMatrix(thickSlabLine1Handle1Position, sliceOffsetHandleOrientation2D, thickSlabLine1Handle1ToWorldMatrix);
      this->Helper->ComputeHandleToWorldTransformMatrix(thickSlabLine1Handle2Position, sliceOffsetHandleOrientation2D, thickSlabLine1Handle2ToWorldMatrix);
      this->Helper->ComputeHandleToWorldTransformMatrix(thickSlabLine2Handle1Position, sliceOffsetHandleOrientation2D, thickSlabLine2Handle1ToWorldMatrix);
      this->Helper->ComputeHandleToWorldTransformMatrix(thickSlabLine2Handle2Position, sliceOffsetHandleOrientation2D, thickSlabLine2Handle2ToWorldMatrix);
      pipeline->ThickSlabLine1Handle1ToWorldTransform->Identity();
      pipeline->ThickSlabLine1Handle1ToWorldTransform->SetMatrix(thickSlabLine1Handle1ToWorldMatrix);
      pipeline->ThickSlabLine1Handle2ToWorldTransform->Identity();
      pipeline->ThickSlabLine1Handle2ToWorldTransform->SetMatrix(thickSlabLine1Handle2ToWorldMatrix);
      pipeline->ThickSlabLine2Handle1ToWorldTransform->Identity();
      pipeline->ThickSlabLine2Handle1ToWorldTransform->SetMatrix(thickSlabLine2Handle1ToWorldMatrix);
      pipeline->ThickSlabLine2Handle2ToWorldTransform->Identity();
      pipeline->ThickSlabLine2Handle2ToWorldTransform->SetMatrix(thickSlabLine2Handle2ToWorldMatrix);
      vtkNew<vtkPoints> thickSlabHandlePoints;
      double thickSlabHandlePoint[3] = { 0.0, 0.0, 0.0 };
      double thickSlabHandlePoint_h[4] = { 0.0, 0.0, 0.0, 1.0 }; // homogeneous coordinates
      if (pipeline->ThickSlabLine1Handle1Displayable)
      {
        for (int i = 0; i < pipeline->ThickSlabLine1Handle1Points->GetNumberOfPoints(); i++)
        {
          pipeline->ThickSlabLine1Handle1Points->GetPoint(i, thickSlabHandlePoint);
          thickSlabHandlePoint_h[0] = thickSlabHandlePoint[0];
          thickSlabHandlePoint_h[1] = thickSlabHandlePoint[1];
          thickSlabHandlePoint_h[2] = thickSlabHandlePoint[2];
          thickSlabLine1Handle1ToWorldMatrix->MultiplyPoint(thickSlabHandlePoint_h, thickSlabHandlePoint_h);
          thickSlabHandlePoint[0] = thickSlabHandlePoint_h[0];
          thickSlabHandlePoint[1] = thickSlabHandlePoint_h[1];
          thickSlabHandlePoint[2] = thickSlabHandlePoint_h[2];
          thickSlabHandlePoints->InsertNextPoint(thickSlabHandlePoint);
        }
      }
      if (pipeline->ThickSlabLine1Handle2Displayable)
      {
        for (int i = 0; i < pipeline->ThickSlabLine1Handle2Points->GetNumberOfPoints(); i++)
        {
          pipeline->ThickSlabLine1Handle2Points->GetPoint(i, thickSlabHandlePoint);
          thickSlabHandlePoint_h[0] = thickSlabHandlePoint[0];
          thickSlabHandlePoint_h[1] = thickSlabHandlePoint[1];
          thickSlabHandlePoint_h[2] = thickSlabHandlePoint[2];
          thickSlabLine1Handle2ToWorldMatrix->MultiplyPoint(thickSlabHandlePoint_h, thickSlabHandlePoint_h);
          thickSlabHandlePoint[0] = thickSlabHandlePoint_h[0];
          thickSlabHandlePoint[1] = thickSlabHandlePoint_h[1];
          thickSlabHandlePoint[2] = thickSlabHandlePoint_h[2];
          thickSlabHandlePoints->InsertNextPoint(thickSlabHandlePoint);
        }
      }
      if (pipeline->ThickSlabLine2Handle1Displayable)
      {
        for (int i = 0; i < pipeline->ThickSlabLine2Handle1Points->GetNumberOfPoints(); i++)
        {
          pipeline->ThickSlabLine2Handle1Points->GetPoint(i, thickSlabHandlePoint);
          thickSlabHandlePoint_h[0] = thickSlabHandlePoint[0];
          thickSlabHandlePoint_h[1] = thickSlabHandlePoint[1];
          thickSlabHandlePoint_h[2] = thickSlabHandlePoint[2];
          thickSlabLine2Handle1ToWorldMatrix->MultiplyPoint(thickSlabHandlePoint_h, thickSlabHandlePoint_h);
          thickSlabHandlePoint[0] = thickSlabHandlePoint_h[0];
          thickSlabHandlePoint[1] = thickSlabHandlePoint_h[1];
          thickSlabHandlePoint[2] = thickSlabHandlePoint_h[2];
          thickSlabHandlePoints->InsertNextPoint(thickSlabHandlePoint);
        }
      }
      if (pipeline->ThickSlabLine2Handle2Displayable)
      {
        for (int i = 0; i < pipeline->ThickSlabLine2Handle2Points->GetNumberOfPoints(); i++)
        {
          pipeline->ThickSlabLine2Handle2Points->GetPoint(i, thickSlabHandlePoint);
          thickSlabHandlePoint_h[0] = thickSlabHandlePoint[0];
          thickSlabHandlePoint_h[1] = thickSlabHandlePoint[1];
          thickSlabHandlePoint_h[2] = thickSlabHandlePoint[2];
          thickSlabLine2Handle2ToWorldMatrix->MultiplyPoint(thickSlabHandlePoint_h, thickSlabHandlePoint_h);
          thickSlabHandlePoint[0] = thickSlabHandlePoint_h[0];
          thickSlabHandlePoint[1] = thickSlabHandlePoint_h[1];
          thickSlabHandlePoint[2] = thickSlabHandlePoint_h[2];
          thickSlabHandlePoints->InsertNextPoint(thickSlabHandlePoint);
        }
      }

      // Define points along thick slab lines for interaction
      std::vector<vtkPoints*> thickSlabLinePointData = { pipeline->ThickSlabLine1FirstHalf->GetOutput()->GetPoints(),
                                                         pipeline->ThickSlabLine1SecondHalf->GetOutput()->GetPoints(),
                                                         pipeline->ThickSlabLine2FirstHalf->GetOutput()->GetPoints(),
                                                         pipeline->ThickSlabLine2SecondHalf->GetOutput()->GetPoints() };

      for (vtkPoints* p : thickSlabLinePointData)
      {
        vtkIdType numLinePoints;
        double linePoint[3] = { 0.0, 0.0, 0.0 };
        numLinePoints = p->GetNumberOfPoints();
        for (int i = 0; i < numLinePoints; i++)
        {
          // Get line point coordinates
          p->GetPoint(i, linePoint);
          thickSlabHandlePoints->InsertNextPoint(linePoint);
        }
      }
      pipeline->ThickSlabHandlePoints->SetPoints(thickSlabHandlePoints);
      pipeline->SetThickSlabHandlesVisibility(true);
      pipeline->ThickSlabHandlesVisible = true;
    }
    else
    {
      pipeline->SetThickSlabHandlesVisibility(false);
      pipeline->ThickSlabHandlesVisible = false;
    }
    pipeline->SetThicknessVisibility(true);
  }
  else
  {
    pipeline->ThickSlabHandlesVisible = false;
    pipeline->SetThickSlabHandlesVisibility(false);
    pipeline->SetThicknessVisibility(false);
  }
  this->NeedToRenderOn();
}

//----------------------------------------------------------------------
vtkMRMLSliceDisplayNode* vtkMRMLSliceIntersectionInteractionRepresentation::GetSliceDisplayNode()
{
  if (this->Internal->SliceDisplayNode)
  {
    return this->Internal->SliceDisplayNode;
  }
  vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode(this->Internal->SliceNode);
  this->SetSliceDisplayNode(sliceDisplayNode);
  return sliceDisplayNode;
}

//----------------------------------------------------------------------
vtkMRMLSliceDisplayNode* vtkMRMLSliceIntersectionInteractionRepresentation::GetSliceDisplayNode(vtkMRMLSliceNode* sliceNode)
{
  vtkMRMLApplicationLogic* mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (!mrmlAppLogic)
  {
    return nullptr;
  }
  vtkMRMLSliceLogic* sliceLogic = mrmlAppLogic->GetSliceLogic(sliceNode);
  if (!sliceLogic)
  {
    return nullptr;
  }
  return sliceLogic->GetSliceDisplayNode();
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::SetSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (sliceNode == this->Internal->SliceNode)
  {
    // no change
    return;
  }
  if (this->Internal->SliceNode)
  {
    this->Internal->SliceNode->RemoveObserver(this->Internal->SliceNodeModifiedCommand);
  }
  if (sliceNode)
  {
    sliceNode->AddObserver(vtkCommand::ModifiedEvent, this->Internal->SliceNodeModifiedCommand.GetPointer());
  }
  this->Internal->SliceNode = sliceNode;

  this->SetSliceDisplayNode(nullptr);
  vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode(sliceNode);
  this->SetSliceDisplayNode(sliceDisplayNode);

  this->UpdateIntersectingSliceNodes();
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::SetSliceDisplayNode(vtkMRMLSliceDisplayNode* sliceDisplayNode)
{
  if (this->Internal->SliceDisplayNode == sliceDisplayNode)
  {
    // no change
    return;
  }
  if (this->Internal->SliceDisplayNode)
  {
    this->Internal->SliceDisplayNode->RemoveObserver(this->Internal->SliceNodeModifiedCommand);
  }
  if (sliceDisplayNode)
  {
    sliceDisplayNode->AddObserver(vtkCommand::ModifiedEvent, this->Internal->SliceNodeModifiedCommand.GetPointer());
  }
  this->Internal->SliceDisplayNode = sliceDisplayNode;
  this->UpdateIntersectingSliceNodes();
}

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionInteractionRepresentation::DistanceFromSliceIntersectionToPoint(vtkMRMLSliceNode* intersectingSlice, double point_RAS[3], double& distance)
{
  // First we need to find the representation for the intersecting slice node
  if (!intersectingSlice)
  {
    return false;
  }
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    if (!(*sliceIntersectionIt)->SliceLogic)
    {
      continue;
    }
    if (intersectingSlice == (*sliceIntersectionIt)->SliceLogic->GetSliceNode())
    {
      // First get the points defining the slice intersection line
      double point1_XY[3] = { 0 };
      double point2_XY[3] = { 0 };
      (*sliceIntersectionIt)->IntersectionLine1->GetPoint1(point1_XY);
      (*sliceIntersectionIt)->IntersectionLine1->GetPoint2(point2_XY);

      // Set homogeneous coords
      double point1_XY_h[4] = { point1_XY[0], point1_XY[1], point1_XY[2], 1 };
      double point2_XY_h[4] = { point2_XY[0], point2_XY[1], point2_XY[2], 1 };

      // Now convert to RAS coordinates
      vtkMatrix4x4* XYtoRAS = this->Internal->SliceNode->GetXYToRAS();
      double point1_RAS[4] = { 0, 0, 0, 1 };
      double point2_RAS[4] = { 0, 0, 0, 1 };
      XYtoRAS->MultiplyPoint(point1_XY_h, point1_RAS);
      XYtoRAS->MultiplyPoint(point2_XY_h, point2_RAS);

      // Set the distance
      double distance2 = vtkLine::DistanceToLine(point_RAS, point1_RAS, point2_RAS);
      distance = sqrt(distance2);
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceIntersectionInteractionRepresentation::GetSliceNode()
{
  return this->Internal->SliceNode;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::AddIntersectingSliceLogic(vtkMRMLSliceLogic* sliceLogic)
{
  if (!sliceLogic)
  {
    return;
  }
  if (sliceLogic->GetSliceNode() == this->Internal->SliceNode)
  {
    // it is the slice itself, not an intersecting slice
    return;
  }
  if (this->GetDisplayPipelineFromSliceLogic(sliceLogic))
  {
    // slice node already added
    return;
  }

  SliceIntersectionInteractionDisplayPipeline* pipeline = new SliceIntersectionInteractionDisplayPipeline;
  pipeline->SetAndObserveSliceLogic(sliceLogic, this->Internal->SliceNodeModifiedCommand);
  pipeline->AddActors(this->Renderer);
  this->Internal->SliceIntersectionInteractionDisplayPipelines.push_back(pipeline);
  this->UpdateSliceIntersectionDisplay(pipeline);
  this->SliceNodeModified(this->Internal->SliceNode);
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::RemoveIntersectingSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
  {
    return;
  }
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    if (!(*sliceIntersectionIt)->SliceLogic)
    {
      continue;
    }
    if (sliceNode == (*sliceIntersectionIt)->SliceLogic->GetSliceNode())
    {
      // found it
      (*sliceIntersectionIt)->RemoveActors(this->Renderer);
      delete (*sliceIntersectionIt);
      this->Internal->SliceIntersectionInteractionDisplayPipelines.erase(sliceIntersectionIt);
      break;
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::UpdateIntersectingSliceNodes()
{
  this->RemoveAllIntersectingSliceNodes();
  if (!this->GetSliceNode() || !this->MRMLApplicationLogic)
  {
    return;
  }
  vtkCollection* sliceLogics = this->MRMLApplicationLogic->GetSliceLogics();
  if (!sliceLogics)
  {
    return;
  }
  vtkMRMLSliceLogic* sliceLogic;
  vtkCollectionSimpleIterator it;
  for (sliceLogics->InitTraversal(it); (sliceLogic = vtkMRMLSliceLogic::SafeDownCast(sliceLogics->GetNextItemAsObject(it)));)
  {
    this->AddIntersectingSliceLogic(sliceLogic);
  }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::RemoveAllIntersectingSliceNodes()
{
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    (*sliceIntersectionIt)->RemoveActors(this->Renderer);
    delete (*sliceIntersectionIt);
  }
  this->Internal->SliceIntersectionInteractionDisplayPipelines.clear();
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::ComputeSliceIntersectionPoint()
{
  this->SliceIntersectionPoint[0] = 0.0;
  this->SliceIntersectionPoint[1] = 0.0;
  this->SliceIntersectionPoint[2] = 0.0;

  this->SliceIntersectionPointFound = false;

  size_t numberOfIntersections = this->Internal->SliceIntersectionInteractionDisplayPipelines.size();
  int numberOfFoundIntersectionPoints = 0;
  if (!this->Internal->SliceNode)
  {
    return;
  }

  // Get intersection point
  for (size_t slice1Index = 0; slice1Index < numberOfIntersections - 1; slice1Index++)
  {
    if (!this->Internal->SliceIntersectionInteractionDisplayPipelines[slice1Index]->GetVisibility())
    {
      continue;
    }
    // Get line 1 points
    vtkLineSource* line1R = this->Internal->SliceIntersectionInteractionDisplayPipelines[slice1Index]->IntersectionLine1;
    vtkLineSource* line1L = this->Internal->SliceIntersectionInteractionDisplayPipelines[slice1Index]->IntersectionLine2;
    double* line1PointR = line1R->GetPoint1();
    double* line1PointL = line1L->GetPoint1();

    // Get extended line 1 points
    double line1Direction[3] = { line1PointR[0] - line1PointL[0], line1PointR[1] - line1PointL[1], line1PointR[2] - line1PointL[2] };
    double line1DirectionInv[3] = { -line1Direction[0], -line1Direction[1], -line1Direction[2] };
    double line1Length = vtkMath::Norm(line1Direction);
    vtkMath::Normalize(line1Direction);
    vtkMath::Normalize(line1DirectionInv);
    double extendLineFactor = 100;
    double extendedLine1PointR[3] = { line1PointL[0] + line1Direction[0] * line1Length * extendLineFactor,
                                      line1PointL[1] + line1Direction[1] * line1Length * extendLineFactor,
                                      line1PointL[2] + line1Direction[2] * line1Length * extendLineFactor };
    double extendedLine1PointL[3] = { line1PointR[0] + line1DirectionInv[0] * line1Length * extendLineFactor,
                                      line1PointR[1] + line1DirectionInv[1] * line1Length * extendLineFactor,
                                      line1PointR[2] + line1DirectionInv[2] * line1Length * extendLineFactor };

    for (size_t slice2Index = slice1Index + 1; slice2Index < numberOfIntersections; slice2Index++)
    {
      if (!this->Internal->SliceIntersectionInteractionDisplayPipelines[slice2Index]->GetVisibility())
      {
        continue;
      }
      // Get line 2 points
      vtkLineSource* line2R = this->Internal->SliceIntersectionInteractionDisplayPipelines[slice2Index]->IntersectionLine1;
      vtkLineSource* line2L = this->Internal->SliceIntersectionInteractionDisplayPipelines[slice2Index]->IntersectionLine2;
      double* line2PointR = line2R->GetPoint1();
      double* line2PointL = line2L->GetPoint1();

      // Get extended line 2 points
      double line2Direction[3] = { line2PointR[0] - line2PointL[0], line2PointR[1] - line2PointL[1], line2PointR[2] - line2PointL[2] };
      double line2DirectionInv[3] = { -line2Direction[0], -line2Direction[1], -line2Direction[2] };
      double line2Length = vtkMath::Norm(line2Direction);
      vtkMath::Normalize(line2Direction);
      vtkMath::Normalize(line2DirectionInv);
      double extendedLine2PointR[3] = { line2PointL[0] + line2Direction[0] * line2Length * extendLineFactor,
                                        line2PointL[1] + line2Direction[1] * line2Length * extendLineFactor,
                                        line2PointL[2] + line2Direction[2] * line2Length * extendLineFactor };
      double extendedLine2PointL[3] = { line2PointR[0] + line2DirectionInv[0] * line2Length * extendLineFactor,
                                        line2PointR[1] + line2DirectionInv[1] * line2Length * extendLineFactor,
                                        line2PointR[2] + line2DirectionInv[2] * line2Length * extendLineFactor };

      // Compute intersection
      double line1ParametricPosition = 0;
      double line2ParametricPosition = 0;
      if (vtkLine::Intersection(extendedLine1PointR, extendedLine1PointL, extendedLine2PointR, extendedLine2PointL, line1ParametricPosition, line2ParametricPosition))
      {
        this->SliceIntersectionPoint[0] += extendedLine1PointR[0] + line1ParametricPosition * (extendedLine1PointL[0] - extendedLine1PointR[0]);
        this->SliceIntersectionPoint[1] += extendedLine1PointR[1] + line1ParametricPosition * (extendedLine1PointL[1] - extendedLine1PointR[1]);
        this->SliceIntersectionPoint[2] += extendedLine1PointR[2] + line1ParametricPosition * (extendedLine1PointL[2] - extendedLine1PointR[2]);
        numberOfFoundIntersectionPoints++;
      }
    }
  }
  if (numberOfFoundIntersectionPoints > 0)
  {
    this->SliceIntersectionPointFound = true;
    this->SliceIntersectionPoint[0] /= numberOfFoundIntersectionPoints;
    this->SliceIntersectionPoint[1] /= numberOfFoundIntersectionPoints;
    this->SliceIntersectionPoint[2] /= numberOfFoundIntersectionPoints;
  }
  else
  {
    this->SliceIntersectionPointFound = false;
    // No slice intersections, use slice centerpoint
    int* sliceDimension = this->Internal->SliceNode->GetDimensions();
    this->SliceIntersectionPoint[0] = sliceDimension[0] / 2.0;
    this->SliceIntersectionPoint[1] = sliceDimension[1] / 2.0;
    this->SliceIntersectionPoint[2] = 0.0;
  }
}
//----------------------------------------------------------------------
double* vtkMRMLSliceIntersectionInteractionRepresentation::GetSliceIntersectionPoint()
{
  return this->SliceIntersectionPoint;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::TransformIntersectingSlices(vtkMatrix4x4* rotatedSliceToSliceTransformMatrix)
{
  std::deque<int> wasModified;
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    if (!(*sliceIntersectionIt) //
        || !(*sliceIntersectionIt)->GetIntersectionVisibility())
    {
      continue;
    }
    vtkMRMLSliceNode* sliceNode = (*sliceIntersectionIt)->SliceLogic->GetSliceNode();
    wasModified.push_back(sliceNode->StartModify());

    vtkNew<vtkMatrix4x4> rotatedSliceToRAS;
    vtkMatrix4x4::Multiply4x4(rotatedSliceToSliceTransformMatrix, sliceNode->GetSliceToRAS(), rotatedSliceToRAS);

    sliceNode->GetSliceToRAS()->DeepCopy(rotatedSliceToRAS);
  }

  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    if (!(*sliceIntersectionIt) //
        || !(*sliceIntersectionIt)->GetIntersectionVisibility())
    {
      continue;
    }
    vtkMRMLSliceNode* sliceNode = (*sliceIntersectionIt)->SliceLogic->GetSliceNode();
    sliceNode->UpdateMatrices();
    sliceNode->EndModify(wasModified.front());
    wasModified.pop_front();
  }
}

//----------------------------------------------------------------------
double vtkMRMLSliceIntersectionInteractionRepresentation::GetMaximumHandlePickingDistance2()
{
  double maximumHandlePickingDistance = this->InteractionSize / 2.0 + this->PickingTolerance * this->GetScreenScaleFactor();
  return maximumHandlePickingDistance * maximumHandlePickingDistance;
}

//-----------------------------------------------------------------------------
std::string vtkMRMLSliceIntersectionInteractionRepresentation::CanInteract(vtkMRMLInteractionEventData* interactionEventData,
                                                                           int& foundComponentType,
                                                                           int& foundComponentIndex,
                                                                           double& closestDistance2,
                                                                           double& handleOpacity)
{
  foundComponentType = vtkMRMLSliceDisplayNode::ComponentNone;
  closestDistance2 = VTK_DOUBLE_MAX; // in display coordinate system
  foundComponentIndex = -1;
  handleOpacity = 0.0; // Value to encode handles opacity value as a function of the distance of the mouse cursor to the handle position
  std::string intersectingSliceNodeID;
  double maxPickingDistanceFromControlPoint2 = this->GetMaximumHandlePickingDistance2();
  double extendedPickingDistanceFromControlPoint2 = maxPickingDistanceFromControlPoint2 + OPACITY_RANGE;
  double displayPosition3[3] = { 0.0, 0.0, 0.0 };
  // Display position is valid in case of desktop interactions. Otherwise it is a 3D only context such as
  // virtual reality, and then we expect a valid world position in the absence of display position.
  if (interactionEventData->IsDisplayPositionValid())
  {
    const int* displayPosition = interactionEventData->GetDisplayPosition();
    displayPosition3[0] = static_cast<double>(displayPosition[0]);
    displayPosition3[1] = static_cast<double>(displayPosition[1]);
  }
  else if (!interactionEventData->IsWorldPositionValid())
  {
    return nullptr;
  }

  bool intersectingSlicesTranslationEnabled = true;
  bool intersectingSlicesRotationEnabled = true;
  vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (sliceDisplayNode)
  {
    intersectingSlicesTranslationEnabled = sliceDisplayNode->GetIntersectingSlicesTranslationEnabled();
    intersectingSlicesRotationEnabled = sliceDisplayNode->GetIntersectingSlicesRotationEnabled();
  }

  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    if (!(*sliceIntersectionIt))
    {
      continue;
    }
    vtkMRMLSliceNode* sliceNode = this->GetSliceNode();
    if (sliceNode)
    {
      double handleDisplayPos[4] = { 0.0, 0.0, 0.0, 1.0 };

      vtkNew<vtkMatrix4x4> rasToxyMatrix;
      vtkMatrix4x4::Invert(sliceNode->GetXYToRAS(), rasToxyMatrix);

      vtkMRMLSliceDisplayNode* displayNode = this->GetSliceDisplayNode();
      vtkMRMLSliceNode* intersectingSliceNode = (*sliceIntersectionIt)->SliceLogic->GetSliceNode();
      vtkMRMLSliceDisplayNode* intersectingDisplayNode = this->GetSliceDisplayNode(intersectingSliceNode);
      bool thickSlabInteractionEnabled = intersectingDisplayNode->GetIntersectingThickSlabInteractive();
      double opacity = 0.0;
      HandleInfoList handleInfoList = this->GetHandleInfoList((*sliceIntersectionIt));
      for (HandleInfo handleInfo : handleInfoList)
      {

        // Ignore handle if the corresponding interaction mode is disabled
        bool intersectionLineVisible = (*sliceIntersectionIt)->GetIntersectionVisibility();
        bool intersectionInteractive = displayNode->GetIntersectingSlicesInteractive();
        bool thickSlabLineVisible = (*sliceIntersectionIt)->GetThickSlabVisibility();
        bool isTranslationHandle = (handleInfo.ComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateIntersectingSlicesHandle //
                                    || handleInfo.ComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateSingleIntersectingSliceHandle);
        bool isRotationHandle = handleInfo.ComponentType == vtkMRMLSliceDisplayNode::ComponentRotateIntersectingSlicesHandle;
        bool isThickSlabHandle = handleInfo.ComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateIntersectingThickSlabHandle;

        if (isTranslationHandle && !(intersectingSlicesTranslationEnabled && intersectionLineVisible && intersectionInteractive))
        {
          // translation is disabled and this is a translation handle
          continue;
        }
        if (isRotationHandle && !(intersectingSlicesRotationEnabled && intersectionLineVisible && intersectionInteractive))
        {
          // rotation is disabled and this is a rotation handle
          continue;
        }
        if (isThickSlabHandle && !(thickSlabLineVisible && thickSlabInteractionEnabled))
        {
          // thick slab translation is disabled and this is a thick slab handle
          continue;
        }

        double* handleWorldPos = handleInfo.PositionWorld;
        rasToxyMatrix->MultiplyPoint(handleWorldPos, handleDisplayPos);
        handleDisplayPos[2] = displayPosition3[2]; // Handles are always projected
        double dist2 = vtkMath::Distance2BetweenPoints(handleDisplayPos, displayPosition3);
        if (dist2 < extendedPickingDistanceFromControlPoint2)
        {
          if (dist2 < maxPickingDistanceFromControlPoint2)
          {
            handleOpacity = 1.0;
            if (dist2 < closestDistance2                                                                            //
                || (handleInfo.ComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateIntersectingSlicesHandle //
                    && foundComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateSingleIntersectingSliceHandle))
            {
              closestDistance2 = dist2;

              if (foundComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateIntersectingSlicesHandle //
                  && handleInfo.ComponentType == vtkMRMLSliceDisplayNode::ComponentTranslateSingleIntersectingSliceHandle)
              {
                // We have already found an acceptable TranslateIntersectingSlices handle, which has higher priority
                // than single-slice TranslateSingleIntersectingSlice handle.
                continue;
              }

              foundComponentType = handleInfo.ComponentType;
              foundComponentIndex = handleInfo.Index;
              intersectingSliceNodeID = handleInfo.IntersectingSliceNodeID;
            }
          }
          else
          {
            opacity = (dist2 - extendedPickingDistanceFromControlPoint2) / (-OPACITY_RANGE);
            if (opacity > handleOpacity)
            {
              handleOpacity = opacity;
            }
          }
        }
      }
    }
    else
    {
      HandleInfoList handleInfoList = this->GetHandleInfoList((*sliceIntersectionIt));
      for (HandleInfo handleInfo : handleInfoList)
      {
        double* handleWorldPos = handleInfo.PositionWorld;
        double handleDisplayPos[3] = { 0 };

        if (interactionEventData->IsDisplayPositionValid())
        {
          double pixelTolerance = this->InteractionSize / 2.0 / vtkMRMLAbstractThreeDViewDisplayableManager::GetViewScaleFactorAtPosition(this->Renderer, handleWorldPos)
                                  + this->PickingTolerance * this->GetScreenScaleFactor();
          this->Renderer->SetWorldPoint(handleWorldPos);
          this->Renderer->WorldToDisplay();
          this->Renderer->GetDisplayPoint(handleDisplayPos);
          handleDisplayPos[2] = 0.0;
          double dist2 = vtkMath::Distance2BetweenPoints(handleDisplayPos, displayPosition3);
          if (dist2 < pixelTolerance * pixelTolerance && dist2 < closestDistance2)
          {
            closestDistance2 = dist2;
            foundComponentType = handleInfo.ComponentType;
            foundComponentIndex = handleInfo.Index;
            intersectingSliceNodeID = handleInfo.IntersectingSliceNodeID;
          }
        }
        else
        {
          const double* worldPosition = interactionEventData->GetWorldPosition();
          double worldTolerance = this->InteractionSize / 2.0 + this->PickingTolerance / interactionEventData->GetWorldToPhysicalScale();
          double dist2 = vtkMath::Distance2BetweenPoints(handleWorldPos, worldPosition);
          if (dist2 < worldTolerance * worldTolerance && dist2 < closestDistance2)
          {
            closestDistance2 = dist2;
            foundComponentType = handleInfo.ComponentType;
            foundComponentIndex = handleInfo.Index;
            intersectingSliceNodeID = handleInfo.IntersectingSliceNodeID;
          }
        }
      }
    }
  }
  return intersectingSliceNodeID;
}

//----------------------------------------------------------------------
vtkMRMLSliceIntersectionInteractionRepresentation::HandleInfoList vtkMRMLSliceIntersectionInteractionRepresentation::GetHandleInfoList(
  SliceIntersectionInteractionDisplayPipeline* pipeline)
{
  vtkMRMLSliceNode* currentSliceNode = this->GetSliceNode();                      // Get slice node
  vtkMRMLSliceNode* intersectingSliceNode = pipeline->SliceLogic->GetSliceNode(); // Get intersecting slice node
  std::string intersectingSliceNodeID = intersectingSliceNode->GetID();           // Get intersection slice node ID
  vtkMatrix4x4* currentXYToRAS = currentSliceNode->GetXYToRAS();                  // Get XY to RAS transform matrix
  HandleInfoList handleInfoList;
  for (int i = 0; i < pipeline->RotationHandlePoints->GetNumberOfPoints(); ++i)
  {
    double handlePositionLocal[3] = { 0 };
    double handlePositionLocal_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    double handlePositionWorld[3] = { 0 };
    double handlePositionWorld_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    pipeline->RotationHandlePoints->GetPoint(i, handlePositionLocal);
    handlePositionLocal_h[0] = handlePositionLocal[0];
    handlePositionLocal_h[1] = handlePositionLocal[1];
    handlePositionLocal_h[2] = handlePositionLocal[2];
    currentXYToRAS->MultiplyPoint(handlePositionLocal_h, handlePositionWorld_h);
    handlePositionWorld[0] = handlePositionWorld_h[0];
    handlePositionWorld[1] = handlePositionWorld_h[1];
    handlePositionWorld[2] = handlePositionWorld_h[2];
    HandleInfo info(i, vtkMRMLSliceDisplayNode::ComponentRotateIntersectingSlicesHandle, intersectingSliceNodeID, handlePositionWorld, handlePositionLocal);
    handleInfoList.push_back(info);
  }

  for (int i = 0; i < pipeline->TranslationHandlePoints->GetNumberOfPoints(); ++i)
  {
    double handlePositionLocal[3] = { 0 };
    double handlePositionLocal_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    double handlePositionWorld[3] = { 0 };
    double handlePositionWorld_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    pipeline->TranslationHandlePoints->GetPoint(i, handlePositionLocal);
    handlePositionLocal_h[0] = handlePositionLocal[0];
    handlePositionLocal_h[1] = handlePositionLocal[1];
    handlePositionLocal_h[2] = handlePositionLocal[2];
    currentXYToRAS->MultiplyPoint(handlePositionLocal_h, handlePositionWorld_h);
    handlePositionWorld[0] = handlePositionWorld_h[0];
    handlePositionWorld[1] = handlePositionWorld_h[1];
    handlePositionWorld[2] = handlePositionWorld_h[2];
    HandleInfo info(i, vtkMRMLSliceDisplayNode::ComponentTranslateIntersectingSlicesHandle, intersectingSliceNodeID, handlePositionWorld, handlePositionLocal);
    handleInfoList.push_back(info);
  }

  for (int i = 0; i < pipeline->SliceOffsetHandlePoints->GetNumberOfPoints(); ++i)
  {
    double handlePositionLocal[3] = { 0 };
    double handlePositionLocal_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    double handlePositionWorld[3] = { 0 };
    double handlePositionWorld_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    pipeline->SliceOffsetHandlePoints->GetPoint(i, handlePositionLocal);
    handlePositionLocal_h[0] = handlePositionLocal[0];
    handlePositionLocal_h[1] = handlePositionLocal[1];
    handlePositionLocal_h[2] = handlePositionLocal[2];
    currentXYToRAS->MultiplyPoint(handlePositionLocal_h, handlePositionWorld_h);
    handlePositionWorld[0] = handlePositionWorld_h[0];
    handlePositionWorld[1] = handlePositionWorld_h[1];
    handlePositionWorld[2] = handlePositionWorld_h[2];
    HandleInfo info(i, vtkMRMLSliceDisplayNode::ComponentTranslateSingleIntersectingSliceHandle, intersectingSliceNodeID, handlePositionWorld, handlePositionLocal);
    handleInfoList.push_back(info);
  }

  for (int i = 0; i < pipeline->ThickSlabHandlePoints->GetNumberOfPoints(); ++i)
  {
    double handlePositionLocal[3] = { 0 };
    double handlePositionLocal_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    double handlePositionWorld[3] = { 0 };
    double handlePositionWorld_h[4] = { 0.0, 0.0, 0.0, 1.0 };
    pipeline->ThickSlabHandlePoints->GetPoint(i, handlePositionLocal);
    handlePositionLocal_h[0] = handlePositionLocal[0];
    handlePositionLocal_h[1] = handlePositionLocal[1];
    handlePositionLocal_h[2] = handlePositionLocal[2];
    currentXYToRAS->MultiplyPoint(handlePositionLocal_h, handlePositionWorld_h);
    handlePositionWorld[0] = handlePositionWorld_h[0];
    handlePositionWorld[1] = handlePositionWorld_h[1];
    handlePositionWorld[2] = handlePositionWorld_h[2];
    HandleInfo info(i, vtkMRMLSliceDisplayNode::ComponentTranslateIntersectingThickSlabHandle, intersectingSliceNodeID, handlePositionWorld, handlePositionLocal);
    handleInfoList.push_back(info);
  }

  return handleInfoList;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::SetPipelinesHandlesVisibility(bool visible)
{
  // Update handles visibility in all display pipelines
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    if ((*sliceIntersectionIt)->HandlesVisibilityMode == vtkMRMLSliceDisplayNode::AlwaysVisible)
    {
      visible = true;
    }
    else if ((*sliceIntersectionIt)->HandlesVisibilityMode == vtkMRMLSliceDisplayNode::NeverVisible)
    {
      visible = false;
    }
    (*sliceIntersectionIt)->SetIntersectionHandlesVisibility(visible);
    (*sliceIntersectionIt)->SetThickSlabHandlesVisibility(visible);
    if ((*sliceIntersectionIt)->NeedToRender)
    {
      this->NeedToRenderOn();
    }
  }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::SetPipelinesHandlesOpacity(double opacity)
{
  // Update handles visibility in all display pipelines
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    // Force visible handles in the "handles always visible" mode is activated
    if ((*sliceIntersectionIt)->HandlesVisibilityMode == vtkMRMLSliceDisplayNode::AlwaysVisible)
    {
      opacity = 1.0;
    }

    (*sliceIntersectionIt)->SetHandlesOpacity(opacity);
    if ((*sliceIntersectionIt)->NeedToRender)
    {
      this->NeedToRenderOn();
    }
  }
}

//----------------------------------------------------------------------
bool vtkMRMLSliceIntersectionInteractionRepresentation::IsMouseCursorInSliceView(double cursorPosition[2])
{
  // Get current slice view bounds
  vtkMRMLSliceNode* currentSliceNode = this->GetSliceNode();
  double sliceViewBounds[4] = {};
  this->Helper->GetSliceViewBoundariesXY(currentSliceNode, sliceViewBounds);

  // Check mouse cursor position
  bool inSliceView;
  if ((cursorPosition[0] > sliceViewBounds[0]) && //
      (cursorPosition[0] < sliceViewBounds[1]) && //
      (cursorPosition[1] > sliceViewBounds[2]) && //
      (cursorPosition[1] < sliceViewBounds[3]))
  {
    inSliceView = true;
  }
  else
  {
    inSliceView = false;
  }
  return inSliceView;
}

//-----------------------------------------------------------------------------
bool vtkMRMLSliceIntersectionInteractionRepresentation::IsDisplayable()
{
  vtkMRMLSliceDisplayNode* sliceDisplayNode = this->GetSliceDisplayNode();
  if (!sliceDisplayNode                     //
      || !this->ViewNode                    //
      || !sliceDisplayNode->GetVisibility() //
      || !sliceDisplayNode->IsDisplayableInView(this->ViewNode->GetID()))
  {
    return false;
  }

  if (vtkMRMLSliceNode::SafeDownCast(this->ViewNode))
  {
    if (!sliceDisplayNode->GetVisibility2D())
    {
      return false;
    }
  }
  if (vtkMRMLViewNode::SafeDownCast(this->ViewNode))
  {
    if (!sliceDisplayNode->GetVisibility3D())
    {
      return false;
    }
  }
  return true;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionInteractionRepresentation::UpdateFromMRML(vtkMRMLNode* caller, unsigned long event, void* callData /*=nullptr*/)
{
  Superclass::UpdateFromMRML(caller, event, callData);

  this->NeedToRenderOn();

  vtkMRMLSliceIntersectionInteractionRepresentation* self = vtkMRMLSliceIntersectionInteractionRepresentation::SafeDownCast((vtkObject*)callData);

  vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(caller);
  if (sliceNode)
  {
    self->SliceNodeModified(sliceNode);
    return;
  }

  vtkMRMLSliceLogic* sliceLogic = vtkMRMLSliceLogic::SafeDownCast(caller);
  if (sliceLogic)
  {
    self->UpdateSliceIntersectionDisplay(self->GetDisplayPipelineFromSliceLogic(sliceLogic));
    return;
  }
}

//----------------------------------------------------------------------
int vtkMRMLSliceIntersectionInteractionRepresentation::GetTranslateArrowCursor(const std::string& intersectingSliceNodeID)
{
  for (std::deque<SliceIntersectionInteractionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionInteractionDisplayPipelines.begin();
       sliceIntersectionIt != this->Internal->SliceIntersectionInteractionDisplayPipelines.end();
       ++sliceIntersectionIt)
  {
    vtkMRMLSliceLogic* sliceLogic = (*sliceIntersectionIt)->SliceLogic;
    if (!sliceLogic)
    {
      continue;
    }
    if (!sliceLogic->GetSliceNode()             //
        || !sliceLogic->GetSliceNode()->GetID() //
        || sliceLogic->GetSliceNode()->GetID() != intersectingSliceNodeID)
    {
      continue;
    }

    // We have found the slice
    vtkLineSource* line = nullptr;
    if ((*sliceIntersectionIt)->IntersectionLine1Actor->GetVisibility())
    {
      line = (*sliceIntersectionIt)->IntersectionLine1;
    }
    else if ((*sliceIntersectionIt)->IntersectionLine2Actor->GetVisibility())
    {
      line = (*sliceIntersectionIt)->IntersectionLine2;
    }
    if (!line)
    {
      // no intersection is visible
      break;
    }

    double pt1[3] = { 0.0, 0.0, 0.0 };
    double pt2[3] = { 0.0, 0.0, 0.0 };
    line->GetPoint1(pt1);
    line->GetPoint2(pt2);
    // angle of intersection line in the slice view
    // (between -180..180, +x axis = 0 deg, +y axis = 90 deg, -y axis = -90 deg)
    double angle = vtkMath::DegreesFromRadians(atan2(pt1[1] - pt2[1], pt1[0] - pt2[0]));
    if ((fabs(angle) > 180.0 - 22.5) //
        || (fabs(angle) < 22.5))
    {
      return VTK_CURSOR_SIZENS;
    }
    if (fabs(fabs(angle) - 90.0) < 22.5)
    {
      return VTK_CURSOR_SIZEWE;
    }
    if ((fabs(angle - 135.0) < 22.5) //
        || (fabs(angle + 45.0) < 22.5))
    {
      return VTK_CURSOR_SIZENE;
    }
    return VTK_CURSOR_SIZENW;
  }
  return VTK_CURSOR_DEFAULT;
}

//----------------------------------------------------------------------
double vtkMRMLSliceIntersectionInteractionRepresentation::GetLineThicknessFromMode(int lineThicknessMode)
{
  switch (lineThicknessMode)
  {
    case vtkMRMLSliceDisplayNode::FineLines: return 1.0;
    case vtkMRMLSliceDisplayNode::MediumLines: return 2.0;
    case vtkMRMLSliceDisplayNode::ThickLines: return 3.0;
    default: return 1.0;
  }
}
