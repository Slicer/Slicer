/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkOrientedBSplineTransform.h"
#include "vtkOrientedGridTransform.h"

// VTK includes
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>

// STD includes
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{

enum TransformType
{
  NoTransform,
  Linear,
  Grid,
  BSpline,
  ThinPlateSpline,
  CompositePreMultiply,
  CompositePostMultiply
};

struct TransformSpec
{
  TransformType Type;
  bool Inverted; // stored as transform from parent
};

const char* GetTransformTypeName(TransformType type)
{
  switch (type)
  {
    case NoTransform: return "none";
    case Linear: return "linear";
    case Grid: return "grid";
    case BSpline: return "bspline";
    case ThinPlateSpline: return "tps";
    case CompositePreMultiply: return "composite(pre)";
    case CompositePostMultiply: return "composite(post)";
  }
  return "unknown";
}

std::string GetChainDescription(const std::vector<TransformSpec>& chain)
{
  std::ostringstream description;
  for (size_t i = 0; i < chain.size(); ++i)
  {
    description << (i > 0 ? " -> " : "") << GetTransformTypeName(chain[i].Type) << (chain[i].Inverted ? " (inverted)" : "");
  }
  return description.str();
}

// Smooth displacement of a few millimeters, different for each seed
void GetDisplacement(const double position[3], int seed, double displacement[3])
{
  const double amplitude = 4.0;
  displacement[0] = amplitude * sin(position[1] / 25.0 + seed);
  displacement[1] = amplitude * cos(position[2] / 30.0 + 2.0 * seed);
  displacement[2] = amplitude * sin(position[0] / 20.0 + 3.0 * seed);
}

vtkSmartPointer<vtkImageData> CreateDisplacementImage(int dimension, double spacing, int seed)
{
  vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
  image->SetExtent(0, dimension - 1, 0, dimension - 1, 0, dimension - 1);
  double origin = -spacing * (dimension - 1) / 2.0;
  image->SetOrigin(origin, origin, origin);
  image->SetSpacing(spacing, spacing, spacing);
  image->AllocateScalars(VTK_DOUBLE, 3);
  for (int k = 0; k < dimension; ++k)
  {
    for (int j = 0; j < dimension; ++j)
    {
      for (int i = 0; i < dimension; ++i)
      {
        double position[3] = { origin + i * spacing, origin + j * spacing, origin + k * spacing };
        double displacement[3];
        GetDisplacement(position, seed, displacement);
        for (int c = 0; c < 3; ++c)
        {
          image->SetScalarComponentFromDouble(i, j, k, c, displacement[c]);
        }
      }
    }
  }
  return image;
}

vtkSmartPointer<vtkMatrix4x4> CreateGridDirectionMatrix(int seed)
{
  vtkNew<vtkTransform> rotation;
  rotation->RotateWXYZ(5.0 + 3.0 * seed, 1.0, 2.0, 3.0);
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  matrix->DeepCopy(rotation->GetMatrix());
  return matrix;
}

vtkSmartPointer<vtkAbstractTransform> CreateTransform(TransformType type, int seed)
{
  switch (type)
  {
    case NoTransform:
    {
      return nullptr;
    }
    case Linear:
    {
      vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
      transform->Translate(5.0, -3.0 * seed, 2.0);
      transform->RotateWXYZ(10.0 + 5.0 * seed, 1.0, seed, 2.0);
      return transform;
    }
    case Grid:
    {
      vtkSmartPointer<vtkOrientedGridTransform> transform = vtkSmartPointer<vtkOrientedGridTransform>::New();
      transform->SetDisplacementGridData(CreateDisplacementImage(13, 10.0, seed));
      transform->SetGridDirectionMatrix(CreateGridDirectionMatrix(seed));
      transform->SetInterpolationModeToCubic();
      return transform;
    }
    case BSpline:
    {
      vtkSmartPointer<vtkOrientedBSplineTransform> transform = vtkSmartPointer<vtkOrientedBSplineTransform>::New();
      transform->SetCoefficientData(CreateDisplacementImage(11, 15.0, seed));
      transform->SetGridDirectionMatrix(CreateGridDirectionMatrix(seed));
      transform->SetBorderModeToZero();
      return transform;
    }
    case ThinPlateSpline:
    {
      vtkNew<vtkPoints> sourceLandmarks;
      vtkNew<vtkPoints> targetLandmarks;
      for (double x = -60.0; x <= 60.0; x += 40.0)
      {
        for (double y = -60.0; y <= 60.0; y += 40.0)
        {
          for (double z = -60.0; z <= 60.0; z += 40.0)
          {
            double source[3] = { x, y, z };
            double displacement[3];
            GetDisplacement(source, seed, displacement);
            sourceLandmarks->InsertNextPoint(source);
            targetLandmarks->InsertNextPoint(x + displacement[0], y + displacement[1], z + displacement[2]);
          }
        }
      }
      vtkSmartPointer<vtkThinPlateSplineTransform> transform = vtkSmartPointer<vtkThinPlateSplineTransform>::New();
      transform->SetBasisToR();
      transform->SetSourceLandmarks(sourceLandmarks);
      transform->SetTargetLandmarks(targetLandmarks);
      return transform;
    }
    case CompositePreMultiply:
    {
      // Linear transform is applied first, then the grid transform
      vtkSmartPointer<vtkGeneralTransform> transform = vtkSmartPointer<vtkGeneralTransform>::New();
      transform->PreMultiply();
      transform->Concatenate(CreateTransform(Grid, seed));
      transform->Concatenate(CreateTransform(Linear, seed + 1));
      return transform;
    }
    case CompositePostMultiply:
    {
      // B-spline transform is applied first, then the linear transform
      vtkSmartPointer<vtkGeneralTransform> transform = vtkSmartPointer<vtkGeneralTransform>::New();
      transform->PostMultiply();
      transform->Concatenate(CreateTransform(BSpline, seed));
      transform->Concatenate(CreateTransform(Linear, seed + 1));
      return transform;
    }
  }
  return nullptr;
}

// Test positions, inside the displacement grids
vtkSmartPointer<vtkPoints> CreateTestPoints()
{
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  for (double x = -30.0; x <= 30.0; x += 20.0)
  {
    for (double y = -30.0; y <= 30.0; y += 20.0)
    {
      for (double z = -30.0; z <= 30.0; z += 20.0)
      {
        points->InsertNextPoint(x + 0.3 * y, y - 0.2 * z, z + 0.1 * x);
      }
    }
  }
  return points;
}

double GetLargestDifference(vtkAbstractTransform* transform, vtkPoints* points, vtkPoints* expectedTransformedPoints)
{
  double largestDifference = 0.0;
  for (vtkIdType pointIndex = 0; pointIndex < points->GetNumberOfPoints(); ++pointIndex)
  {
    double transformedPoint[3];
    transform->TransformPoint(points->GetPoint(pointIndex), transformedPoint);
    largestDifference = std::max(largestDifference, sqrt(vtkMath::Distance2BetweenPoints(transformedPoint, expectedTransformedPoints->GetPoint(pointIndex))));
  }
  return largestDifference;
}

//---------------------------------------------------------------------------
// The first transform of the chain is the hardened node, the others are its parents, up to the world.
int TestHardenChain(const std::vector<TransformSpec>& chain)
{
  std::string chainDescription = GetChainDescription(chain);
  std::cout << "Harden: " << chainDescription << std::endl;

  vtkNew<vtkMRMLScene> scene;
  std::vector<vtkMRMLTransformNode*> nodes;
  std::vector<vtkSmartPointer<vtkAbstractTransform>> transforms;
  for (size_t i = 0; i < chain.size(); ++i)
  {
    vtkMRMLTransformNode* node = vtkMRMLTransformNode::SafeDownCast(scene->AddNewNodeByClass("vtkMRMLTransformNode"));
    vtkSmartPointer<vtkAbstractTransform> transform = CreateTransform(chain[i].Type, static_cast<int>(i));
    if (chain[i].Inverted)
    {
      node->SetAndObserveTransformFromParent(transform);
    }
    else
    {
      node->SetAndObserveTransformToParent(transform);
    }
    if (!nodes.empty())
    {
      nodes.back()->SetAndObserveTransformNodeID(node->GetID());
    }
    nodes.push_back(node);
    transforms.push_back(transform);
  }
  vtkMRMLTransformNode* hardenedNode = nodes[0];

  vtkSmartPointer<vtkPoints> points = CreateTestPoints();
  vtkNew<vtkGeneralTransform> transformToWorldBeforeHardening;
  hardenedNode->GetTransformToWorld(transformToWorldBeforeHardening);
  vtkNew<vtkPoints> pointsToWorld;
  transformToWorldBeforeHardening->TransformPoints(points, pointsToWorld);
  vtkNew<vtkGeneralTransform> transformFromWorldBeforeHardening;
  hardenedNode->GetTransformFromWorld(transformFromWorldBeforeHardening);
  vtkNew<vtkPoints> pointsFromWorld;
  transformFromWorldBeforeHardening->TransformPoints(points, pointsFromWorld);
  vtkNew<vtkPoints> pointsOwnTransform;
  if (vtkGeneralTransform::SafeDownCast(transforms[0]))
  {
    transforms[0]->TransformPoints(points, pointsOwnTransform);
  }

  CHECK_BOOL(hardenedNode->HardenTransform(), true);
  CHECK_NULL(hardenedNode->GetParentTransformNode());

  const double tolerance = 1e-3;

  vtkNew<vtkGeneralTransform> transformToWorldAfterHardening;
  hardenedNode->GetTransformToWorld(transformToWorldAfterHardening);
  double differenceToWorld = GetLargestDifference(transformToWorldAfterHardening, points, pointsToWorld);
  vtkNew<vtkGeneralTransform> transformFromWorldAfterHardening;
  hardenedNode->GetTransformFromWorld(transformFromWorldAfterHardening);
  double differenceFromWorld = GetLargestDifference(transformFromWorldAfterHardening, points, pointsFromWorld);
  if (differenceToWorld > tolerance || differenceFromWorld > tolerance)
  {
    std::cerr << "Line " << __LINE__ << ": transform changed by hardening " << chainDescription << ": largest difference to world " << differenceToWorld << " mm, from world "
              << differenceFromWorld << " mm" << std::endl;
    return EXIT_FAILURE;
  }

  // A composite transform that was set in the node is not modified (components are not added to it)
  if (vtkGeneralTransform::SafeDownCast(transforms[0]))
  {
    double differenceOwnTransform = GetLargestDifference(transforms[0], points, pointsOwnTransform);
    if (differenceOwnTransform > tolerance)
    {
      std::cerr << "Line " << __LINE__ << ": the transform of the hardened node is modified by hardening " << chainDescription << ": largest difference " << differenceOwnTransform
                << " mm" << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}

} // namespace

//---------------------------------------------------------------------------
int vtkMRMLTransformNodeHardenTest1(int vtkNotUsed(argc), char* vtkNotUsed(argv)[])
{
  // Single parent and linear-only chains (hardened as a matrix)
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, false }, { BSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, false }, { Linear, true }, { Linear, false }, { Linear, true } }));

  // Chains of 3 transforms
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, false }, { Grid, false }, { BSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, true }, { Grid, true }, { BSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, true }, { CompositePostMultiply, false }, { CompositePreMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, false }, { CompositePostMultiply, true }, { CompositePreMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, false }, { BSpline, false }, { ThinPlateSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, true }, { BSpline, true }, { ThinPlateSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, true }, { Linear, false }, { CompositePostMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, false }, { Linear, true }, { CompositePostMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, false }, { ThinPlateSpline, false }, { CompositePreMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, true }, { ThinPlateSpline, true }, { CompositePreMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, true }, { Grid, false }, { Linear, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, false }, { Grid, true }, { Linear, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, false }, { CompositePreMultiply, false }, { CompositePostMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, true }, { CompositePreMultiply, true }, { CompositePostMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, true }, { BSpline, false }, { Grid, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, false }, { BSpline, true }, { Grid, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, false }, { CompositePostMultiply, false }, { Linear, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, true }, { CompositePostMultiply, true }, { Linear, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, true }, { ThinPlateSpline, false }, { BSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, false }, { ThinPlateSpline, true }, { BSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, false }, { Linear, false }, { Grid, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, true }, { Linear, true }, { Grid, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, true }, { CompositePreMultiply, false }, { ThinPlateSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, false }, { CompositePreMultiply, true }, { ThinPlateSpline, false } }));

  // Chains of 4 transforms
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, false }, { Grid, false }, { BSpline, false }, { ThinPlateSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, true }, { Grid, true }, { BSpline, true }, { ThinPlateSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, true }, { CompositePostMultiply, false }, { CompositePreMultiply, true }, { ThinPlateSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, false }, { CompositePostMultiply, true }, { CompositePreMultiply, false }, { ThinPlateSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, false }, { BSpline, false }, { ThinPlateSpline, false }, { CompositePreMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, true }, { BSpline, true }, { ThinPlateSpline, true }, { CompositePreMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, true }, { Linear, false }, { CompositePostMultiply, true }, { CompositePreMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, false }, { Linear, true }, { CompositePostMultiply, false }, { CompositePreMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, false }, { ThinPlateSpline, false }, { CompositePreMultiply, false }, { CompositePostMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, true }, { ThinPlateSpline, true }, { CompositePreMultiply, true }, { CompositePostMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, true }, { Grid, false }, { Linear, true }, { CompositePostMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, false }, { Grid, true }, { Linear, false }, { CompositePostMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, false }, { CompositePreMultiply, false }, { CompositePostMultiply, false }, { Linear, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, true }, { CompositePreMultiply, true }, { CompositePostMultiply, true }, { Linear, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, true }, { BSpline, false }, { Grid, true }, { Linear, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, false }, { BSpline, true }, { Grid, false }, { Linear, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, false }, { CompositePostMultiply, false }, { Linear, false }, { Grid, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, true }, { CompositePostMultiply, true }, { Linear, true }, { Grid, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, true }, { ThinPlateSpline, false }, { BSpline, true }, { Grid, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, false }, { ThinPlateSpline, true }, { BSpline, false }, { Grid, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, false }, { Linear, false }, { Grid, false }, { BSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, true }, { Linear, true }, { Grid, true }, { BSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, true }, { CompositePreMultiply, false }, { ThinPlateSpline, true }, { BSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, false }, { CompositePreMultiply, true }, { ThinPlateSpline, false }, { BSpline, true } }));

  // Chains of 5 transforms
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, false }, { Grid, false }, { BSpline, false }, { ThinPlateSpline, false }, { CompositePreMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, true }, { Grid, true }, { BSpline, true }, { ThinPlateSpline, true }, { CompositePreMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, true }, { CompositePostMultiply, false }, { CompositePreMultiply, true }, { ThinPlateSpline, false }, { BSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Linear, false }, { CompositePostMultiply, true }, { CompositePreMultiply, false }, { ThinPlateSpline, true }, { BSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, false }, { BSpline, false }, { ThinPlateSpline, false }, { CompositePreMultiply, false }, { CompositePostMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, true }, { BSpline, true }, { ThinPlateSpline, true }, { CompositePreMultiply, true }, { CompositePostMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, true }, { Linear, false }, { CompositePostMultiply, true }, { CompositePreMultiply, false }, { ThinPlateSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { Grid, false }, { Linear, true }, { CompositePostMultiply, false }, { CompositePreMultiply, true }, { ThinPlateSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, false }, { ThinPlateSpline, false }, { CompositePreMultiply, false }, { CompositePostMultiply, false }, { Linear, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, true }, { ThinPlateSpline, true }, { CompositePreMultiply, true }, { CompositePostMultiply, true }, { Linear, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, true }, { Grid, false }, { Linear, true }, { CompositePostMultiply, false }, { CompositePreMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { BSpline, false }, { Grid, true }, { Linear, false }, { CompositePostMultiply, true }, { CompositePreMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, false }, { CompositePreMultiply, false }, { CompositePostMultiply, false }, { Linear, false }, { Grid, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, true }, { CompositePreMultiply, true }, { CompositePostMultiply, true }, { Linear, true }, { Grid, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, true }, { BSpline, false }, { Grid, true }, { Linear, false }, { CompositePostMultiply, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { ThinPlateSpline, false }, { BSpline, true }, { Grid, false }, { Linear, true }, { CompositePostMultiply, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, false }, { CompositePostMultiply, false }, { Linear, false }, { Grid, false }, { BSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, true }, { CompositePostMultiply, true }, { Linear, true }, { Grid, true }, { BSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, true }, { ThinPlateSpline, false }, { BSpline, true }, { Grid, false }, { Linear, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePreMultiply, false }, { ThinPlateSpline, true }, { BSpline, false }, { Grid, true }, { Linear, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, false }, { Linear, false }, { Grid, false }, { BSpline, false }, { ThinPlateSpline, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, true }, { Linear, true }, { Grid, true }, { BSpline, true }, { ThinPlateSpline, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, true }, { CompositePreMultiply, false }, { ThinPlateSpline, true }, { BSpline, false }, { Grid, true } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { CompositePostMultiply, false }, { CompositePreMultiply, true }, { ThinPlateSpline, false }, { BSpline, true }, { Grid, false } }));

  // The hardened node has no transform
  CHECK_EXIT_SUCCESS(TestHardenChain({ { NoTransform, false }, { Grid, true }, { Linear, false } }));
  CHECK_EXIT_SUCCESS(TestHardenChain({ { NoTransform, false }, { ThinPlateSpline, false }, { CompositePreMultiply, true }, { BSpline, false }, { Linear, true } }));

  std::cout << "Success" << std::endl;
  return EXIT_SUCCESS;
}
