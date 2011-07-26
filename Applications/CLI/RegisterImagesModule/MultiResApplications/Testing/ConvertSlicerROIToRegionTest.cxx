/*=========================================================================

  Program:   Slicer
  Module:    $HeadURL$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================*/

#include <itkImage.h>


#include "ConvertSlicerROIToRegion.h"

#include "RITestingMacros.h"

namespace
{
template <class T, class J>
void assign3(T& vec, J x, J y, J z)
{
  vec[0] = x;
  vec[1] = y;
  vec[2] = z;
}
}

int ConvertSlicerROIToRegionTest(int itkNotUsed(argc), char* itkNotUsed(argv)[])
{
  itk::Point<double, 3> p1;
  assign3(p1, 0.0, 0.0, 0.0);
  itk::Point<double, 3> p2;
  assign3(p2, 1.0, 5.0, 2.0);
  itk::Point<double, 3> p3;
  assign3(p3, -1.0, 10.0, -2.0);
  itk::Point<double, 3> p4;
  assign3(p4, 1000.0, 1000.0, 1000.0);

  itk::Image<char, 3>::Pointer testimage1 =
    itk::Image<char, 3>::New();
  itk::Image<char, 3>::Pointer testimage2 =
    itk::Image<char, 3>::New();

  itk::Index<3> ind = {{0,0,0}};
  itk::Size<3> size = {{10,10,10}};
  itk::Vector<double, 3> spacing1;
  assign3(spacing1, 1, 5, 2);
  itk::Vector<double, 3> spacing2;
  assign3(spacing2, -1, 2, -1);

  itk::ImageRegion<3> imagereg1 = itk::ImageRegion<3>(ind, size);
  testimage1->SetRegions(imagereg1);
  testimage1->SetSpacing(spacing1);
  testimage1->Allocate();
  
  itk::ImageRegion<3> imagereg2 = itk::ImageRegion<3>(ind, size);
  testimage2->SetRegions(imagereg2);
  testimage2->SetSpacing(spacing2);
  testimage2->Allocate();

  itk::ImageRegion<3> testreg1 = convertPointsToRegion(p1, p2, testimage1);
  TEST_ASSERT_EQUALS_VEC_3(testreg1.GetSize(), 1, 1, 1);
  TEST_ASSERT_EQUALS_VEC_3(testreg1.GetIndex(), 0, 0, 0);
  itk::ImageRegion<3> testreg2 = convertPointsToRegion(p2, p1, testimage1);
  TEST_ASSERT_EQUALS_VEC_3(testreg2.GetSize(), 1, 1, 1);
  TEST_ASSERT_EQUALS_VEC_3(testreg2.GetIndex(), 0, 0, 0);
  itk::ImageRegion<3> testreg3 = convertPointsToRegion(p1, p3, testimage1);
  TEST_ASSERT_EQUALS_VEC_3(testreg3.GetSize(), 1, 2, 1);
  TEST_ASSERT_EQUALS_VEC_3(testreg3.GetIndex(), -1, 0, -1);
  itk::ImageRegion<3> testreg4 = convertPointsToRegion(p1, p4, testimage1);
  TEST_ASSERT_EQUALS_VEC_3(testreg4.GetSize(), 1000, 200, 500);
  TEST_ASSERT_EQUALS_VEC_3(testreg4.GetIndex(), 0, 0, 0);

  itk::ImageRegion<3> testreg5 = convertPointsToRegion(p1, p3, testimage2);
  TEST_ASSERT_EQUALS_VEC_3(testreg5.GetSize(), 1, 5, 2);
  TEST_ASSERT_EQUALS_VEC_3(testreg5.GetIndex(), 0, 0, 0);
  itk::ImageRegion<3> testreg6 = convertPointsToRegion(p3, p1, testimage2);
  TEST_ASSERT_EQUALS_VEC_3(testreg6.GetSize(), 1, 5, 2);
  TEST_ASSERT_EQUALS_VEC_3(testreg6.GetIndex(), 0, 0, 0);
  itk::ImageRegion<3> testreg7 = convertPointsToRegion(p1, p2, testimage2);
  TEST_ASSERT_EQUALS_VEC_3(testreg7.GetSize(), 1, 3, 2);
  TEST_ASSERT_EQUALS_VEC_3(testreg7.GetIndex(), -1, 0, -2);
  itk::ImageRegion<3> testreg8 = convertPointsToRegion(p1, p4, testimage2);
  TEST_ASSERT_EQUALS_VEC_3(testreg8.GetSize(), 1000, 500, 1000);
  TEST_ASSERT_EQUALS_VEC_3(testreg8.GetIndex(), -1000, 0, -1000);

  itk::Point<double, 3> ponehalf;
  assign3(ponehalf, .5, 2.5, 1.0);

  // Testing spatial object
  itk::SlicerBoxSpatialObject<3>::Pointer box1 =
    convertPointsToBoxSpatialObject(p1, p2);
  TEST_ASSERT(box1->IsInside(p1));
  TEST_ASSERT(box1->IsInside(p2));
  TEST_ASSERT(box1->IsInside(ponehalf));
  TEST_ASSERT(!box1->IsInside(p3));
  TEST_ASSERT(!box1->IsInside(p4));

  // test point ordering doesnt matter
  itk::SlicerBoxSpatialObject<3>::Pointer box2 =
    convertPointsToBoxSpatialObject(p2, p1);
  TEST_ASSERT(box2->IsInside(p1));
  TEST_ASSERT(box2->IsInside(p2));
  TEST_ASSERT(box2->IsInside(ponehalf));
  TEST_ASSERT(!box2->IsInside(p3));
  TEST_ASSERT(!box2->IsInside(p4));

  itk::SlicerBoxSpatialObject<3>::Pointer box3 =
    convertPointsToBoxSpatialObject(p1, p4);
  TEST_ASSERT(box3->IsInside(p1));
  TEST_ASSERT(box3->IsInside(p2));
  TEST_ASSERT(box3->IsInside(ponehalf));
  TEST_ASSERT(!box3->IsInside(p3));
  TEST_ASSERT(box3->IsInside(p4));

  return EXIT_SUCCESS;
}
