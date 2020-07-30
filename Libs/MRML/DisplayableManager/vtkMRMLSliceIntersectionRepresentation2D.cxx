/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkMRMLSliceIntersectionRepresentation2D.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkMRMLSliceIntersectionRepresentation2D.h"


#include <deque>

#include "vtkMRMLApplicationLogic.h"
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLSliceLogic.h"
#include "vtkMRMLSliceNode.h"

#include "vtkCallbackCommand.h"
#include "vtkCommand.h"
#include "vtkPolyDataMapper2D.h"
#include "vtkPlane.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkSphereSource.h"
#include "vtkActor2D.h"
#include "vtkObjectFactory.h"
#include "vtkProperty2D.h"
#include "vtkTextProperty.h"
#include "vtkAssemblyPath.h"
#include "vtkMath.h"
#include "vtkInteractorObserver.h"
#include "vtkLine.h"
#include "vtkLineSource.h"
#include "vtkCoordinate.h"
#include "vtkGlyph2D.h"
#include "vtkCursor2D.h"
#include "vtkPolyDataAlgorithm.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkLeaderActor2D.h"
#include "vtkTransform.h"
#include "vtkTextMapper.h"
#include "vtkActor2D.h"
#include "vtkWindow.h"

static const int CONTROL_MODIFIER = 1;
static const int SHIFT_MODIFIER = 2;

vtkStandardNewMacro(vtkMRMLSliceIntersectionRepresentation2D);
vtkCxxSetObjectMacro(vtkMRMLSliceIntersectionRepresentation2D, MRMLApplicationLogic, vtkMRMLApplicationLogic);

class SliceIntersectionDisplayPipeline
{
public:

  //----------------------------------------------------------------------
  SliceIntersectionDisplayPipeline()
  {
    this->LineSource = vtkSmartPointer<vtkLineSource>::New();
    this->Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
    this->Property = vtkSmartPointer<vtkProperty2D>::New();
    this->Actor = vtkSmartPointer<vtkActor2D>::New();
    this->Actor->SetVisibility(false); // invisible until slice node is set

    this->Mapper->SetInputConnection(this->LineSource->GetOutputPort());
    this->Actor->SetMapper(this->Mapper);
    this->Actor->SetProperty(this->Property);
  }

  //----------------------------------------------------------------------
  virtual ~SliceIntersectionDisplayPipeline()
  {
    this->SetAndObserveSliceLogic(nullptr, nullptr);
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
  void GetActors2D(vtkPropCollection *pc)
  {
    pc->AddItem(this->Actor);
  }

  //----------------------------------------------------------------------
  void AddActors(vtkRenderer* renderer)
  {
    if (!renderer)
      {
      return;
      }
    renderer->AddViewProp(this->Actor);
  }

  //----------------------------------------------------------------------
  void ReleaseGraphicsResources(vtkWindow *win)
  {
    this->Actor->ReleaseGraphicsResources(win);
  }

  //----------------------------------------------------------------------
  int RenderOverlay(vtkViewport *viewport)
  {
    int count = 0;
    if (this->Actor->GetVisibility())
      {
      count += this->Actor->RenderOverlay(viewport);
      }
    return count;
  }


  //----------------------------------------------------------------------
  void RemoveActors(vtkRenderer* renderer)
  {
    if (!renderer)
      {
      return;
      }
    renderer->RemoveViewProp(this->Actor);
  }

  //----------------------------------------------------------------------
  void SetVisibility(bool visibility)
  {
    this->Actor->SetVisibility(visibility);
  }

  //----------------------------------------------------------------------
  bool GetVisibility()
  {
    return this->Actor->GetVisibility();
  }

  vtkSmartPointer<vtkLineSource> LineSource;
  vtkSmartPointer<vtkPolyDataMapper2D> Mapper;
  vtkSmartPointer<vtkProperty2D> Property;
  vtkSmartPointer<vtkActor2D> Actor;
  vtkWeakPointer<vtkMRMLSliceLogic> SliceLogic;
  vtkWeakPointer<vtkCallbackCommand> Callback;
};

class vtkMRMLSliceIntersectionRepresentation2D::vtkInternal
{
public:
  vtkInternal(vtkMRMLSliceIntersectionRepresentation2D * external);
  ~vtkInternal();

  static int IntersectWithFinitePlane(double n[3], double o[3], double pOrigin[3], double px[3], double py[3], double x0[3], double x1[3]);

  vtkMRMLSliceIntersectionRepresentation2D* External;

  vtkSmartPointer<vtkMRMLSliceNode> SliceNode;

  std::deque<SliceIntersectionDisplayPipeline*> SliceIntersectionDisplayPipelines;
  vtkNew<vtkCallbackCommand> SliceNodeModifiedCommand;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkMRMLSliceIntersectionRepresentation2D::vtkInternal
::vtkInternal(vtkMRMLSliceIntersectionRepresentation2D * external)
{
  this->External = external;
}

//---------------------------------------------------------------------------
vtkMRMLSliceIntersectionRepresentation2D::vtkInternal::~vtkInternal() = default;


//---------------------------------------------------------------------------
int vtkMRMLSliceIntersectionRepresentation2D::vtkInternal::IntersectWithFinitePlane(double n[3], double o[3],
  double pOrigin[3], double px[3], double py[3],
  double x0[3], double x1[3])
{
  // Since we are dealing with convex shapes, if there is an intersection a
  // single line is produced as output. So all this is necessary is to
  // intersect the four bounding lines of the finite line and find the two
  // intersection points.
  int numInts = 0;
  double t, *x = x0;
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
vtkMRMLSliceIntersectionRepresentation2D::vtkMRMLSliceIntersectionRepresentation2D()
{
  this->MRMLApplicationLogic = nullptr;

  this->Internal = new vtkInternal(this);
  this->Internal->SliceNodeModifiedCommand->SetClientData(this);
  this->Internal->SliceNodeModifiedCommand->SetCallback(vtkMRMLSliceIntersectionRepresentation2D::SliceNodeModifiedCallback);

  this->SliceIntersectionPoint[0] = 0.0;
  this->SliceIntersectionPoint[1] = 0.0;
  this->SliceIntersectionPoint[2] = 0.0;
  this->SliceIntersectionPoint[3] = 1.0; // to allow easy homogeneous tranformations
}

//----------------------------------------------------------------------
vtkMRMLSliceIntersectionRepresentation2D::~vtkMRMLSliceIntersectionRepresentation2D()
{
  this->SetSliceNode(nullptr);
  this->SetMRMLApplicationLogic(nullptr);
  delete this->Internal;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::GetActors2D(vtkPropCollection *pc)
{
  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
    {
    (*sliceIntersectionIt)->GetActors2D(pc);
    }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::ReleaseGraphicsResources(vtkWindow *win)
{
  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
    {
    (*sliceIntersectionIt)->ReleaseGraphicsResources(win);
    }
}

//----------------------------------------------------------------------
int vtkMRMLSliceIntersectionRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;

  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
    {
    count += (*sliceIntersectionIt)->RenderOverlay(viewport);
    }
  return count;
}


//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os, indent);
}


//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::SliceNodeModifiedCallback(
  vtkObject* caller, unsigned long vtkNotUsed(eid), void* clientData, void* vtkNotUsed(callData))
{
  vtkMRMLSliceIntersectionRepresentation2D* self = vtkMRMLSliceIntersectionRepresentation2D::SafeDownCast((vtkObject*)clientData);

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
void vtkMRMLSliceIntersectionRepresentation2D::SliceNodeModified(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
    {
    return;
    }
  if (sliceNode == this->Internal->SliceNode)
    {
    // update all slice intersection
    for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
      sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
      {
      this->UpdateSliceIntersectionDisplay(*sliceIntersectionIt);
      }
    }
}

//----------------------------------------------------------------------
SliceIntersectionDisplayPipeline* vtkMRMLSliceIntersectionRepresentation2D::GetDisplayPipelineFromSliceLogic(vtkMRMLSliceLogic* sliceLogic)
{
  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
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
void vtkMRMLSliceIntersectionRepresentation2D::UpdateSliceIntersectionDisplay(SliceIntersectionDisplayPipeline *pipeline)
{
  if (!pipeline || !this->Internal->SliceNode || pipeline->SliceLogic == nullptr)
    {
    return;
    }
  vtkMRMLSliceNode* intersectingSliceNode = pipeline->SliceLogic->GetSliceNode();
  if (!pipeline->SliceLogic || !this->GetVisibility()
    || !intersectingSliceNode
    || this->Internal->SliceNode->GetViewGroup() != intersectingSliceNode->GetViewGroup()
    || !intersectingSliceNode->IsMappedInLayout())
    {
    pipeline->SetVisibility(false);
    return;
    }

  vtkMRMLModelDisplayNode* displayNode = nullptr;
  vtkMRMLSliceLogic *sliceLogic = nullptr;
  vtkMRMLApplicationLogic *mrmlAppLogic = this->GetMRMLApplicationLogic();
  if (mrmlAppLogic)
    {
    sliceLogic = mrmlAppLogic->GetSliceLogic(intersectingSliceNode);
    }
  if (sliceLogic)
    {
    displayNode = sliceLogic->GetSliceModelDisplayNode();
    }
  if (displayNode)
    {
    if (!displayNode->GetSliceIntersectionVisibility())
      {
      pipeline->SetVisibility(false);
      return;
      }
    pipeline->Property->SetLineWidth(displayNode->GetLineWidth());
    }

  pipeline->Property->SetColor(intersectingSliceNode->GetLayoutColor());

  vtkMatrix4x4* intersectingXYToRAS = intersectingSliceNode->GetXYToRAS();
  vtkMatrix4x4* xyToRAS = this->Internal->SliceNode->GetXYToRAS();

  //double slicePlaneAngleDifference = vtkMath::AngleBetweenVectors()

  vtkNew<vtkMatrix4x4> rasToXY;
  vtkMatrix4x4::Invert(xyToRAS, rasToXY);
  vtkNew<vtkMatrix4x4> intersectingXYToXY;
  vtkMatrix4x4::Multiply4x4(rasToXY, intersectingXYToRAS, intersectingXYToXY);

  double slicePlaneNormal[3] = { 0.,0.,1. };
  double slicePlaneOrigin[3] = { 0., 0., 0. };

  int* intersectingSliceSizeDimensions = intersectingSliceNode->GetDimensions();
  double intersectingPlaneOrigin[4] = { 0, 0, 0, 1 };
  double intersectingPlaneX[4] = { double(intersectingSliceSizeDimensions[0]), 0., 0., 1. };
  double intersectingPlaneY[4] = { 0., double(intersectingSliceSizeDimensions[1]), 0., 1. };
  intersectingXYToXY->MultiplyPoint(intersectingPlaneOrigin, intersectingPlaneOrigin);
  intersectingXYToXY->MultiplyPoint(intersectingPlaneX, intersectingPlaneX);
  intersectingXYToXY->MultiplyPoint(intersectingPlaneY, intersectingPlaneY);

  double intersectionPoint1[4] = { 0.0, 0.0, 0.0, 1.0 };
  double intersectionPoint2[4] = { 0.0, 0.0, 0.0, 1.0 };

  int intersectionFound = vtkMRMLSliceIntersectionRepresentation2D::vtkInternal::IntersectWithFinitePlane(slicePlaneNormal, slicePlaneOrigin,
    intersectingPlaneOrigin, intersectingPlaneX, intersectingPlaneY,
    intersectionPoint1, intersectionPoint2);
  if (!intersectionFound)
    {
    pipeline->SetVisibility(false);
    return;
    }

  pipeline->LineSource->SetPoint1(intersectionPoint1);
  pipeline->LineSource->SetPoint2(intersectionPoint2);

  pipeline->SetVisibility(true);
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::SetSliceNode(vtkMRMLSliceNode* sliceNode)
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
  this->UpdateIntersectingSliceNodes();
}

//----------------------------------------------------------------------
vtkMRMLSliceNode* vtkMRMLSliceIntersectionRepresentation2D::GetSliceNode()
{
  return this->Internal->SliceNode;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::AddIntersectingSliceLogic(vtkMRMLSliceLogic* sliceLogic)
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

  SliceIntersectionDisplayPipeline* pipeline = new SliceIntersectionDisplayPipeline;
  pipeline->SetAndObserveSliceLogic(sliceLogic, this->Internal->SliceNodeModifiedCommand);
  pipeline->AddActors(this->Renderer);
  this->Internal->SliceIntersectionDisplayPipelines.push_back(pipeline);
  this->UpdateSliceIntersectionDisplay(pipeline);
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::RemoveIntersectingSliceNode(vtkMRMLSliceNode* sliceNode)
{
  if (!sliceNode)
    {
    return;
    }
  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
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
      this->Internal->SliceIntersectionDisplayPipelines.erase(sliceIntersectionIt);
      break;
      }
    }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::UpdateIntersectingSliceNodes()
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
  for (sliceLogics->InitTraversal(it);
    (sliceLogic = vtkMRMLSliceLogic::SafeDownCast(sliceLogics->GetNextItemAsObject(it)));)
    {
    this->AddIntersectingSliceLogic(sliceLogic);
    }
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::RemoveAllIntersectingSliceNodes()
{
  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
    {
    (*sliceIntersectionIt)->RemoveActors(this->Renderer);
    delete (*sliceIntersectionIt);
    }
  this->Internal->SliceIntersectionDisplayPipelines.clear();
}

//----------------------------------------------------------------------
double* vtkMRMLSliceIntersectionRepresentation2D::GetSliceIntersectionPoint()
{
  size_t numberOfIntersections = this->Internal->SliceIntersectionDisplayPipelines.size();
  int numberOfFoundIntersectionPoints = 0;
  this->SliceIntersectionPoint[0] = 0.0;
  this->SliceIntersectionPoint[1] = 0.0;
  this->SliceIntersectionPoint[2] = 0.0;
  if (!this->Internal->SliceNode)
    {
    return this->SliceIntersectionPoint;
    }
  for (size_t slice1Index = 0; slice1Index < numberOfIntersections - 1; slice1Index++)
    {
    if (!this->Internal->SliceIntersectionDisplayPipelines[slice1Index]->GetVisibility())
      {
      continue;
      }
    vtkLineSource* line1 = this->Internal->SliceIntersectionDisplayPipelines[slice1Index]->LineSource;
    double* line1Point1 = line1->GetPoint1();
    double* line1Point2 = line1->GetPoint2();
    for (size_t slice2Index = slice1Index + 1; slice2Index < numberOfIntersections; slice2Index++)
      {
      if (!this->Internal->SliceIntersectionDisplayPipelines[slice2Index]->GetVisibility())
        {
        continue;
        }
      vtkLineSource* line2 = this->Internal->SliceIntersectionDisplayPipelines[slice2Index]->LineSource;
      double line1ParametricPosition = 0;
      double line2ParametricPosition = 0;
      if (vtkLine::Intersection(line1Point1, line1Point2,
        line2->GetPoint1(), line2->GetPoint2(),
        line1ParametricPosition, line2ParametricPosition))
        {
        this->SliceIntersectionPoint[0] += line1Point1[0] + line1ParametricPosition * (line1Point2[0] - line1Point1[0]);
        this->SliceIntersectionPoint[1] += line1Point1[1] + line1ParametricPosition * (line1Point2[1] - line1Point1[1]);
        this->SliceIntersectionPoint[2] += line1Point1[2] + line1ParametricPosition * (line1Point2[2] - line1Point1[2]);
        numberOfFoundIntersectionPoints++;
        }
      }
    }
  if (numberOfFoundIntersectionPoints > 0)
    {
    this->SliceIntersectionPoint[0] /= numberOfFoundIntersectionPoints;
    this->SliceIntersectionPoint[1] /= numberOfFoundIntersectionPoints;
    this->SliceIntersectionPoint[2] /= numberOfFoundIntersectionPoints;
    }
  else
    {
    // No slice intersections, use slice centerpoint
    int* sliceDimension = this->Internal->SliceNode->GetDimensions();
    this->SliceIntersectionPoint[0] = sliceDimension[0] / 2.0;
    this->SliceIntersectionPoint[0] = sliceDimension[1] / 2.0;
    this->SliceIntersectionPoint[0] = 0.0;
    }
  return this->SliceIntersectionPoint;
}

//----------------------------------------------------------------------
void vtkMRMLSliceIntersectionRepresentation2D::TransformIntersectingSlices(vtkMatrix4x4* rotatedSliceToSliceTransformMatrix)
  {
  std::deque<int> wasModified;
  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
    {
    if (!(*sliceIntersectionIt)
      || !(*sliceIntersectionIt)->GetVisibility())
      {
      continue;
      }
    vtkMRMLSliceNode* sliceNode = (*sliceIntersectionIt)->SliceLogic->GetSliceNode();
    wasModified.push_back(sliceNode->StartModify());

    vtkNew<vtkMatrix4x4> rotatedSliceToRAS;
    vtkMatrix4x4::Multiply4x4(rotatedSliceToSliceTransformMatrix, sliceNode->GetSliceToRAS(),
      rotatedSliceToRAS);

    sliceNode->GetSliceToRAS()->DeepCopy(rotatedSliceToRAS);
    }

  for (std::deque<SliceIntersectionDisplayPipeline*>::iterator sliceIntersectionIt = this->Internal->SliceIntersectionDisplayPipelines.begin();
    sliceIntersectionIt != this->Internal->SliceIntersectionDisplayPipelines.end(); ++sliceIntersectionIt)
    {
    if (!(*sliceIntersectionIt)
      || !(*sliceIntersectionIt)->GetVisibility())
      {
      continue;
      }
    vtkMRMLSliceNode* sliceNode = (*sliceIntersectionIt)->SliceLogic->GetSliceNode();
    sliceNode->UpdateMatrices();
    sliceNode->EndModify(wasModified.front());
    wasModified.pop_front();
    }
}
