/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkBinaryMask3DQuadEdgeMeshSourceTest.cxx,v $
  Language:  C++
  Date:      $Date: 2009-04-06 11:27:19 $
  Version:   $Revision: 1.3 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>

#include "itkOrientedImage.h"
#include "itkContinuousIndex.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkQuadEdgeMesh.h"
#include "itkTriangleCell.h"

#include "itkBinaryMask3DMeshSource.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkVTKPolyDataWriter.h"
#include "itkQuadEdgeMeshScalarDataVTKPolyDataWriter.h"
#include "itkQuadEdgeMeshDecimationCriteria.h"
#include "itkQuadEdgeMeshQuadricDecimation.h"
#include "itkQuadEdgeMeshSquaredEdgeLengthDecimation.h"
#include "itkQuadEdgeMeshNormalFilter.h"
#include "itkQuadEdgeMeshExtendedTraits.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkConstNeighborhoodIterator.h"

#include "itkBSplineScatteredDataPointSetToImageFilter.h"
#include "itkPointSet.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "vtkPolyData.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkDataSet.h"
#include "vtkFieldData.h"
#include "vtkCellData.h"
#include "vtkCellArray.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkCharArray.h"

#include "vtkMassProperties.h"
#include "vtkWarpVector.h"

#include "itkTriangleMeshToBinaryImageFilter.h"
#include "itkImageDuplicator.h"

#include "ChangeTrackerSurfaceTrackingMetricCLP.h"

#include <algorithm>
#include <assert.h>

#define CWIDTH (float(5)) // CWIDTH*2+1 is the number of points used to calculate NCC
#define SWIDTH (float(6)) // (SWIDTH+CWIDTH)*2+1 is the total number of points sampled o
#define CENTERPT (CWIDTH+SWIDTH)
#define LINESPACING .25
#define SAVE_LINE_PROFILES 0
#define DECIMATION_CONST .1
#define NCCTHRESHOLD .1

  // Define the dimension of the images
  const unsigned int Dimension = 3;
  typedef double PixelType;

  // Declare the types of the output images
  typedef itk::OrientedImage<PixelType,   Dimension>   ImageType; // float to use uniform for all images
  typedef itk::ImageFileReader<ImageType> ReaderType;
  typedef itk::ImageFileWriter<ImageType> WriterType;
  typedef itk::BinaryThresholdImageFilter<ImageType,ImageType> ThreshType;
  typedef itk::BSplineInterpolateImageFunction<ImageType,PixelType> InterpolatorType;
  typedef itk::LinearInterpolateImageFunction<ImageType,PixelType> LinearInterpolatorType;


  typedef itk::Vector< PixelType, 1 >         SDAVectorType;
  typedef itk::Image< SDAVectorType, Dimension > SDAVectorImageType;
  typedef SDAVectorImageType::PixelType SDAVectorPixelType;

  typedef itk::PointSet< SDAVectorPixelType, Dimension >      PointSetType;  
  typedef PointSetType::Pointer                   PointSetPointer;
  typedef PointSetType::PointType                 PointSetPointType;
  typedef PointSetType::PointsContainerPointer    PointsContainerPointer;
  typedef PointSetType::PointDataContainerPointer PointDataContainerPointer;
  typedef PointSetType::PointDataContainer        PointDataContainer;

  typedef itk::VectorIndexSelectionCastImageFilter< SDAVectorImageType,
    ImageType > IndexFilterType;
  typedef IndexFilterType::Pointer IndexFilterPointer;
  typedef itk::BSplineScatteredDataPointSetToImageFilter< PointSetType,
    SDAVectorImageType > SDAFilterType;
  typedef SDAFilterType::Pointer SDAFilterPointer;


  typedef itk::Index<Dimension>                     IndexType;
  typedef itk::Size<Dimension>                      SizeType;
  typedef itk::ImageRegion<Dimension>               RegionType;
  typedef ImageType::PixelType                      PixelType;

  typedef itk::ImageRegionIteratorWithIndex<ImageType> IteratorType;

  // Declare the type of the Mesh
  typedef float CoordType;
  typedef itk::Vector<CoordType,3> VectorType;
  typedef itk::QuadEdgeMeshExtendedTraits <
    VectorType,
    Dimension,
    2,
    CoordType,
    CoordType,
    VectorType,
    bool,
    bool > Traits;
//  typedef itk::QuadEdgeMesh<CoordType, 3>              MeshType;
//  typedef itk::QuadEdgeMesh<CoordType, 3>              OutputMeshType;
  typedef itk::QuadEdgeMesh < VectorType, Dimension, Traits > MeshType;
  typedef itk::QuadEdgeMesh < VectorType, Dimension, Traits > OutputMeshType;
  typedef MeshType::PointType                       PointType;
  typedef itk::QuadEdgeMeshScalarDataVTKPolyDataWriter< MeshType >  MeshWriterType;
//  typedef itk::VTKPolyDataWriter<MeshType> MeshWriterType;

  typedef itk::QuadEdgeMeshNormalFilter< MeshType, OutputMeshType > NormalFilterType;
  typedef itk::BinaryMask3DMeshSource< ImageType, MeshType >   MeshSourceType;
  
  typedef OutputMeshType::PointsContainer::Iterator PointsIterator;
  typedef OutputMeshType::CellsContainer::Iterator CellsIterator;
  typedef OutputMeshType::PointDataContainerIterator PointDataIterator;

  typedef itk::TriangleCell<MeshType::CellType> TriangleType;

  typedef itk::TriangleMeshToBinaryImageFilter<MeshType,ImageType> Mesh2ImageType;
  typedef itk::ImageDuplicator<ImageType> DupType;

void ShiftVector(std::vector<PixelType>&, int);
vtkPolyData* ITKMesh2PolyData(MeshType::Pointer);
void PrintSurfaceStatistics(vtkPolyData*);
vtkFloatArray* SmoothVMF(MeshType::Pointer,vtkFloatArray*);
vtkFloatArray* SmoothVDF(MeshType::Pointer,vtkFloatArray*);
float SurfaceVolume(vtkPolyData*);
float SurfaceArea(vtkPolyData*);
void WriteMesh(MeshType::Pointer, const char*);
MeshType::Pointer WarpITKMesh(MeshType::Pointer, vtkFloatArray*);
void WriteImage(ImageType::Pointer, const char*);
ImageType::Pointer MarkChanges(ImageType::Pointer, ImageType::Pointer);
void ResetMaskBorder(ImageType::Pointer, double);

int main(int argc, char **argv){
  PARSE_ARGS;

  const char* inputMaskName = baselineSegmentationVolume.c_str();
  const char* inputImage1Name = baselineVolume.c_str();
  const char* inputImage2Name = followupVolume.c_str();
  char* outputMeshName = "mesh.vtk";
  std::string tmpDirName = "/tmp/";

  double decimationConst = DECIMATION_CONST;

  std::cout << "Parameters:" << std::endl;
  std::cout << " * correlation window width: " << CWIDTH*2+1 << std::endl;
  std::cout << " * sampling window width:    " << (SWIDTH+CWIDTH)*2+1 << std::endl;
  std::cout << " * line sample spacing:      " << LINESPACING << std::endl;
  std::cout << " * surface decimation const: " << decimationConst << std::endl;
  std::cout << " * NCC threshold:            " << NCCTHRESHOLD << std::endl;

  ImageType::Pointer mask;

  // threshold segmentation mask to 1
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(inputMaskName);
   
  ThreshType::Pointer thresh = ThreshType::New();
  thresh->SetInput(reader->GetOutput());
  thresh->SetLowerThreshold(1);
  thresh->SetUpperThreshold(255);
  thresh->SetInsideValue(1);  
  thresh->Update();

  mask = thresh->GetOutput();

  /*
  WriteImage(mask, outputVolume.c_str());
  std::ofstream report(reportFileName.c_str());
  report << "TEST TEST TEST" << std::endl;

  return 0;
  */

  ReaderType::Pointer r1 = ReaderType::New();
  ReaderType::Pointer r2 = ReaderType::New();
  r1->SetFileName(inputImage1Name);
  r2->SetFileName(inputImage2Name);
  r1->Update();
  r2->Update();
  
  ImageType::Pointer image1 = r1->GetOutput(), image2 = r2->GetOutput();

  MeshSourceType::Pointer meshSource = MeshSourceType::New();

  // reset mask values close to the border, so that the line search and
  // marching cubes don't get confused
  ResetMaskBorder(mask, (CWIDTH+SWIDTH+1)*LINESPACING);

  // generate surface mesh for the label in the padded mask image
  meshSource->SetInput( mask );
  meshSource->SetObjectValue(1);
  meshSource->Update();

  // decimate the mesh to reduce the number of points
  typedef itk::NumberOfFacesCriterion< MeshType > CriterionType;
  typedef itk::QuadEdgeMeshSquaredEdgeLengthDecimation< 
    MeshType, MeshType, CriterionType > DecimationType;

  CriterionType::Pointer criterion = CriterionType::New();
  criterion->SetTopologicalChange( false );
  std::cout << "Target number of cells after decimation: " << 
    (unsigned) (decimationConst*meshSource->GetNumberOfCells()) << std::endl;
  criterion->SetNumberOfElements( unsigned(decimationConst*meshSource->GetNumberOfCells()));
  
  MeshType::Pointer mcmesh = meshSource->GetOutput();
  //std::cout << "Saving MC mesh...";
  //WriteMesh(mcmesh, (tmpDirName+"mc_mesh.vtk").c_str());
  //std::cout << "done" << std::endl;
 
  DecimationType::Pointer decimate = DecimationType::New();
  decimate->SetInput( meshSource->GetOutput() );
  decimate->SetCriterion( criterion );
  decimate->Update();
  
  MeshType::Pointer dMesh = decimate->GetOutput();
  std::cout << "Decimation complete" << std::endl;
  std::cout << "Decimated surface points: " << dMesh->GetPoints()->Size() << std::endl;
  std::cout << "Decimated surface cells: " << dMesh->GetCells()->Size() << std::endl;
  WriteMesh(dMesh, (tmpDirName+"decimated_mesh.vtk").c_str());

  NormalFilterType::Pointer normals = NormalFilterType::New( );
  normals->SetInput( decimate->GetOutput() );
  normals->Update( );

  OutputMeshType::Pointer mesh = normals->GetOutput();
  OutputMeshType::PointDataContainerPointer pointData = 
    mesh->GetPointData();
  PointDataIterator pdIt = mesh->GetPointData()->Begin(), pdItEnd = mesh->GetPointData()->End();
  PointsIterator pIt = mesh->GetPoints()->Begin(), pItEnd = mesh->GetPoints()->End();

  // Note: normals are oriented outwards
  InterpolatorType::Pointer interp1 = InterpolatorType::New();
  interp1->SetSplineOrder(3);
  interp1->SetInputImage(image1);
  InterpolatorType::Pointer interp2 = InterpolatorType::New();
  interp2->SetSplineOrder(3);
  interp2->SetInputImage(image2);
  InterpolatorType::Pointer interpM = InterpolatorType::New();
  interpM->SetSplineOrder(3);
  interpM->SetInputImage(mask);

  // this is the point set that will be used by bspline interpolator later
  PointSetType::Pointer pointSet = PointSetType::New();
  PointsContainerPointer psContainer = pointSet->GetPoints();
  psContainer->Reserve(mesh->GetPoints()->Size());
  PointDataContainerPointer pdContainer = PointDataContainer::New();
  pdContainer->Reserve(mesh->GetPoints()->Size());
  pointSet->SetPointData( pdContainer );
 
  int progress1 = 0, progress2 = 0, totalPoints = mesh->GetPoints()->Size();
  
  // arrays to keep the surface shift measurements
  vtkFloatArray *nccShiftArray = vtkFloatArray::New();
  nccShiftArray->SetNumberOfComponents(3);
  nccShiftArray->SetNumberOfTuples(totalPoints);
  nccShiftArray->SetName("NCCShift");

  vtkFloatArray *dnccShiftArray = vtkFloatArray::New();
  dnccShiftArray->SetNumberOfComponents(3);
  dnccShiftArray->SetNumberOfTuples(totalPoints);
  dnccShiftArray->SetName("dNCCShift");

  vtkFloatArray *nccArray = vtkFloatArray::New();
  nccArray->SetNumberOfComponents(1);
  nccArray->SetNumberOfTuples(totalPoints);
  nccArray->SetName("Correlation");

  vtkFloatArray *dnccArray = vtkFloatArray::New();
  dnccArray->SetNumberOfComponents(1);
  dnccArray->SetNumberOfTuples(totalPoints);
  dnccArray->SetName("dCorrelation");

  vtkCharArray *dirArray = vtkCharArray::New();
  dirArray->SetNumberOfComponents(1);
  dirArray->SetNumberOfTuples(totalPoints);
  dirArray->SetName("Direction");

  vtkCharArray *ddirArray = vtkCharArray::New();
  ddirArray->SetNumberOfComponents(1);
  ddirArray->SetNumberOfTuples(totalPoints);
  ddirArray->SetName("dDirection");

  vtkFloatArray *bndryShiftArray = vtkFloatArray::New();
  bndryShiftArray->SetNumberOfComponents(3);
  bndryShiftArray->SetNumberOfTuples(totalPoints);
  bndryShiftArray->SetName("BoundaryShift");

  std::vector<float> dnccRanked;
  float meanShift = 0;

  while(pIt!=pItEnd){    
    OutputMeshType::PixelType normal = pdIt->Value();
    OutputMeshType::PointType point = pIt->Value();

    unsigned i = 0;
    
    progress2 = (int) ((float)pIt->Index()/totalPoints*100.);
    if(progress1 != progress2 && !(progress2%10)){
      std::cout << progress2 << " ";
      std::cout.flush();
      progress1 = progress2;
    }

    i = 0;
    std::vector<PixelType> profile1;
    std::vector<PixelType> profile2;
    std::vector<PixelType> dprofile1;
    std::vector<PixelType> dprofile2;
    double mean1 = 0, mean2 = 0, std1 = 0, std2 = 0, delta;
    double dmean1 = 0, dmean2 = 0, dstd1 = 0, dstd2 = 0, ddelta;
    double sampling = LINESPACING;
    for(double step=-CWIDTH-SWIDTH;step<=CWIDTH+SWIDTH;step++,i++){
      ImageType::PointType imagePt, origImagePt;
      ImageType::IndexType imageIdx;
      itk::ContinuousIndex<PixelType,3> imageContIdx;

      origImagePt[0] = point[0]; 
      origImagePt[1] = point[1]; 
      origImagePt[2] = point[2]; 

      imagePt[0] = point[0]+normal[0]*step*sampling;
      imagePt[1] = point[1]+normal[1]*step*sampling;
      imagePt[2] = point[2]+normal[2]*step*sampling;

      ImageType::PixelType v1, v2, vM = 0;
      if(!mask->TransformPhysicalPointToIndex(imagePt,imageIdx)){
        std::cout << "Image point: " << imagePt << ", step = " << step << 
          " sampling = " << sampling << std::endl;
        mask->TransformPhysicalPointToContinuousIndex(imagePt,imageContIdx);
        std::cout << "Shifted point index: " << imageContIdx << std::endl;
        mask->TransformPhysicalPointToContinuousIndex(origImagePt,imageContIdx);
        std::cout << "Original point cont index: " << imageContIdx << std::endl;
        bool inside = mask->TransformPhysicalPointToIndex(origImagePt,imageIdx);
        std::cout << "Original point index: " << imageIdx << " is inside " << int(inside) << std::endl;
        return -1;
        /*
        nccMax = 0;
        dnccMax = 0;
        bestFit = 0;
        dbestFit = 0;
        // no surface adjustment
        bndryShiftArray->InsertTuple3(pIt->Index(), 0, 0, 0);
        continue;
        */
      }
      mask->TransformPhysicalPointToContinuousIndex(imagePt,imageContIdx);
      v1 = interp1->EvaluateAtContinuousIndex(imageContIdx);
      v2 = interp2->EvaluateAtContinuousIndex(imageContIdx);

      profile1.push_back(v1);
      profile2.push_back(v2);
      dprofile1.push_back(v1);      
      dprofile2.push_back(v2);
      mean1 += v1;
      mean2 += v2;

      if(i>0){ // calculate the derivative
        dprofile1[i-1] = profile1[i]-profile1[i-1];
        dprofile2[i-1] = profile2[i]-profile2[i-1];
        dmean1 += dprofile1[i-1];
        dmean2 += dprofile2[i-1];
      }
      if(step == CWIDTH+SWIDTH){
        dprofile1[i] = dprofile1[i-1];
        dprofile2[i] = dprofile2[i-1];
        dmean1 += dprofile1[i];
        dmean2 += dprofile2[i];
      }      
    }

    double dncc, dnccMax = 0, dbestFit = 0;
    double ncc, nccMax = 0, bestFit = 0;
    if(1){
      // search for the shift that maximizes the NCC between the two profiles
      mean1 = 0; mean2 = 0; dmean1 = 0; dmean2 = 0;
      std1 = 0; std2 = 0; dstd1 = 0; dstd2 = 0;
      unsigned p1, p2;

      for(p1=CENTERPT-CWIDTH;p1<=CENTERPT+CWIDTH;p1++){
        mean1 += profile1[p1];
        dmean1 += dprofile1[p1];
      }
      mean1 /= double(CWIDTH*2+1);
      dmean1 /= double(CWIDTH*2+1);
      for(p1=CENTERPT-CWIDTH;p1<=CENTERPT+CWIDTH;p1++){
        std1 += (profile1[p1]-mean1)*(profile1[p1]-mean1);
        dstd1 += (dprofile1[p1]-dmean1)*(dprofile1[p1]-dmean1);
      }
      std1 = sqrt(std1);
      dstd1 = sqrt(dstd1);

      for(int s=-SWIDTH;s<=SWIDTH;s++){    
        ncc = 0; dncc = 0;
        std2 = 0; dstd2 = 0;
        mean2 = 0; dmean2 = 0;
        for(p2=s+CENTERPT-CWIDTH;p2<=s+CENTERPT+CWIDTH;p2++){
          mean2 += profile2[p2];
          dmean2 += dprofile2[p2];
        }
        mean2 /= float(CWIDTH*2+1);
        dmean2 /= float(CWIDTH*2+1);
        for(p2=s+CENTERPT-CWIDTH;p2<=s+CENTERPT+CWIDTH;p2++){
          std2 += (profile2[p2]-mean2)*(profile2[p2]-mean2);
          dstd2 += (dprofile2[p2]-dmean2)*(dprofile2[p2]-dmean2);
        }
        std2 = sqrt(std2);
        dstd2 = sqrt(dstd2);

        for(int p2=CENTERPT-CWIDTH+s,p1=CENTERPT-CWIDTH;p2<=CENTERPT+CWIDTH+s;p1++,p2++){
          ncc += (profile1[p1]-mean1)*(profile2[p2]-mean2);
          dncc += (dprofile1[p1]-dmean1)*(dprofile2[p2]-dmean2);
        } 

        ncc /= std1*std2;
        dncc /= dstd1*dstd2;

        if(ncc>nccMax){
          nccMax = ncc;
          bestFit = s;
        }
        if(dncc>dnccMax){
          dnccMax = dncc;
          dbestFit = s;
        }
      }
    } else {
      // surface smoothing moved the boundary too far
      nccMax = 0;
      dnccMax = 0;
      bestFit = 0;
      dbestFit = 0;
      // no surface adjustment
      bndryShiftArray->InsertTuple3(pIt->Index(), 0, 0, 0);
    }

    ShiftVector(profile1, bestFit);
    ShiftVector(dprofile1, dbestFit);
   

    meanShift += dbestFit;
    dnccRanked.push_back(dbestFit*(float)LINESPACING);

    nccShiftArray->InsertTuple3(pIt->Index(), normal[0]*bestFit*sampling, 
      normal[1]*bestFit*sampling, normal[2]*bestFit*sampling);
    dnccShiftArray->InsertTuple3(pIt->Index(), normal[0]*dbestFit*sampling, 
      normal[1]*dbestFit*sampling, normal[2]*dbestFit*sampling);
    nccArray->InsertTuple1(pIt->Index(), nccMax);
    dnccArray->InsertTuple1(pIt->Index(), dnccMax);
    dirArray->InsertTuple1(pIt->Index(), bestFit);
    ddirArray->InsertTuple1(pIt->Index(), dbestFit);

    ++pIt;++pdIt;
  }
  std::cout << std::endl;

  std::vector<float>::iterator dnccThreshIt = dnccRanked.begin()+dnccRanked.size()/2;
  std::nth_element(dnccRanked.begin(), dnccThreshIt, dnccRanked.end());

  float meanSurfaceMotion = meanShift/(float)totalPoints*LINESPACING;
  float medianSurfaceMotion = (*dnccThreshIt);

  std::cout << "Mean shift: " << meanSurfaceMotion << std::endl;
  std::cout << "Median shift: " << medianSurfaceMotion << std::endl;

  std::cout << "Smoothing displacements...";
  //vtkFloatArray* dnccShiftArraySmooth = SmoothVMF(mesh, dnccShiftArray);
  std::cout << "done" << std::endl;

//  for(unsigned i=0;i<mesh->GetPoints()->Size();i++)
//    if(dnccArray->GetTuple1(i) < (*dnccThreshIt)){
//      dnccShiftArray->InsertTuple3(i, 0, 0, 0);
//      dnccShiftArraySmooth->InsertTuple3(i, 0, 0, 0);
//    }

  // convert itk mesh to vtk polydata, save into file together with the
  // surface displacements
  
  // report statistics for the mc surface, decimated surface, and warped
  // surface
  vtkPolyData *vtksurf = ITKMesh2PolyData(mesh);
  vtkPolyData *vtkmcsurf = ITKMesh2PolyData(mcmesh);

  vtksurf->GetPointData()->AddArray(nccShiftArray);
  nccShiftArray->Delete();
  vtksurf->GetPointData()->AddArray(dnccShiftArray);
  dnccShiftArray->Delete();
//  vtksurf->GetPointData()->AddArray(dnccShiftArraySmooth);
//  dnccShiftArraySmooth->Delete();
  vtksurf->GetPointData()->AddArray(dnccArray);
  dnccArray->Delete();
  vtksurf->GetPointData()->AddArray(nccArray);
  nccArray->Delete();
  vtksurf->GetPointData()->AddArray(dirArray);
  dirArray->Delete();
  vtksurf->GetPointData()->AddArray(ddirArray);
  ddirArray->Delete();
  vtksurf->GetPointData()->AddArray(bndryShiftArray);

  vtkPolyDataWriter *pdw = vtkPolyDataWriter::New();
  pdw->SetFileName((tmpDirName+outputMeshName).c_str());
  pdw->SetInput(vtksurf);
  pdw->Update();

  vtkWarpVector *warper = vtkWarpVector::New();
//  vtksurf->GetPointData()->SetActiveVectors("dNCCShiftsmooth");
  vtksurf->GetPointData()->SetActiveVectors("dNCCShift");
  warper->SetInput(vtksurf);
  warper->Update();


  float vol1, vol2, area1, area2, mcarea1;

  std::cout << " === MC surface statistics: " << std::endl;
  PrintSurfaceStatistics(vtkmcsurf);
  std::cout << " === Decimated surface statistics: " << std::endl;
  PrintSurfaceStatistics(vtksurf);
  vol1 = SurfaceVolume(vtksurf);
  std::cout << " === Warped surface statistics: " << std::endl;
  vtksurf->SetPoints(warper->GetOutput()->GetPoints());
  
//  vtkWarpVector *warper1 = vtkWarpVector::New();
//  vtksurf->GetPointData()->SetActiveVectors("BoundaryShift");
//  warper1->SetInput(vtksurf);
//  warper1->Update();
//  vtksurf->SetPoints(warper1->GetOutput()->GetPoints());


  vol2 = SurfaceVolume(vtksurf);
  area1 = SurfaceArea(vtksurf);
  mcarea1 = SurfaceArea(vtkmcsurf);

  PrintSurfaceStatistics(vtksurf);
  std::cout << "Volume change: " << vol2-vol1 << std::endl;
  std::cout << "Mean-based volume change: " << std::endl;
  std::cout << "   MeanVCD    : " << meanSurfaceMotion*area1 << std::endl;
  std::cout << "   MeanVCDMC  : " << meanSurfaceMotion*mcarea1 << std::endl;
  std::cout << "Median-based volume change:"<< std::endl;
  std::cout << "   MedianVCD  : " << medianSurfaceMotion*area1 << std::endl;
  std::cout << "   MedianVCDMC: " << medianSurfaceMotion*mcarea1 << std::endl;

  vtkPolyDataWriter *pdw1 = vtkPolyDataWriter::New();
  pdw1->SetFileName((tmpDirName+"warped_mesh.vtk").c_str());
  pdw1->SetInput(vtksurf);
  pdw1->Update();

  WriterType::Pointer imageWriter1 = WriterType::New();
  imageWriter1->SetFileName((tmpDirName+"mask_with_lines.nrrd").c_str());
  imageWriter1->SetInput(mask);
  imageWriter1->SetUseCompression(1);
  imageWriter1->Update();

  Mesh2ImageType::Pointer m2i1 = Mesh2ImageType::New();
  Mesh2ImageType::Pointer m2i2 = Mesh2ImageType::New();
  ImageType::Pointer tp0image, tp1image;

  m2i1->SetInput(mesh);
  m2i1->SetInfoImage(mask);
  m2i1->SetInsideValue(255);
  m2i1->Update();
  tp0image = m2i1->GetOutput();
  WriteImage(tp0image, (tmpDirName+"initial_mask.nrrd").c_str());

  WarpITKMesh(mesh, dnccShiftArray);
  
  m2i2->SetInput(mesh);
  m2i2->SetInfoImage(mask);
  m2i2->SetInsideValue(255);
  m2i2->Update();
  tp1image = m2i2->GetOutput();
  WriteImage(tp1image, (tmpDirName+"warped_mask.nrrd").c_str());

  ImageType::Pointer changes = MarkChanges(tp0image, tp1image);
  WriteImage(changes, outputVolume.c_str());

  return EXIT_SUCCESS;
}

void ShiftVector(std::vector<PixelType>& v, int s){
//  static int cnt = 0;
  int vs = v.size(), i;
//  std::cout << "Shift value: " << s << std::endl;
//  std::cout << "Initial array: ";
//  for(i=0;i<vs;i++)
//    std::cout << v[i] << " ";
//  std::cout << std::endl;
  if(!s)
    return;
  if(s>0) { // shift left
    for(i=vs-1;i>=abs(s);i--){
      v[i] = v[i-abs(s)];
    }
    for(;i>=0;i--)
      v[i] = 0;
 
  } else {
   
    for(i=0;i<vs-abs(s);i++){
      v[i] = v[i+abs(s)];
    }
    for(;i<vs;i++)
      v[i] = 0;

  }
//  std::cout << "Shifted array: ";
//  for(i=0;i<vs;i++)
//    std::cout << v[i] << " ";
//  std::cout << std::endl;
//  if(cnt++>10)
//    abort();
}

vtkPolyData* ITKMesh2PolyData(MeshType::Pointer mesh){
  vtkPolyData *surface = vtkPolyData::New();
  vtkPoints *surfacePoints = vtkPoints::New();
//  vtkCellArray *surfaceCells = vtkCellArray::New();

  surfacePoints->SetNumberOfPoints(mesh->GetPoints()->Size());
//  surfaceCells->EstimateSize(mesh->GetCells()->Size(), 3);
  
  PointsIterator pIt = mesh->GetPoints()->Begin(), pItEnd = mesh->GetPoints()->End();
  CellsIterator cIt = mesh->GetCells()->Begin(), cItEnd = mesh->GetCells()->End();

  while(pIt!=pItEnd){
    OutputMeshType::PointType pt = pIt->Value();
    surfacePoints->SetPoint(pIt->Index(), pt[0], pt[1], pt[2]);
    
    ++pIt;
  }

  surface->SetPoints(surfacePoints);
  surface->Allocate();

  while(cIt!=cItEnd){
    OutputMeshType::CellType *cell = cIt->Value();
    OutputMeshType::CellType::PointIdIterator pidIt = cell->PointIdsBegin(); 
    vtkIdType cIds[3];
    cIds[0] = *pidIt;
    cIds[1] = *(pidIt+1);
    cIds[2] = *(pidIt+2);
    surface->InsertNextCell(VTK_TRIANGLE, 3, cIds);

    ++cIt;
  }
  return surface;
}

void PrintSurfaceStatistics(vtkPolyData* surf){

  vtkMassProperties *mp = vtkMassProperties::New();  
  mp->SetInput(surf);
  mp->Update();

  std::cout << "Volume: " << mp->GetVolume() << std::endl;
  std::cout << "Surface area: " << mp->GetSurfaceArea() << std::endl;
  std::cout << "Shape index: " << mp->GetNormalizedShapeIndex() << std::endl;

}

float SurfaceVolume(vtkPolyData* surf){
  vtkMassProperties *mp = vtkMassProperties::New();  
  mp->SetInput(surf);
  mp->Update();
  return mp->GetVolume();
}

float SurfaceArea(vtkPolyData* surf){
  vtkMassProperties *mp = vtkMassProperties::New();  
  mp->SetInput(surf);
  mp->Update();
  return mp->GetSurfaceArea();
}

/* Take the QE mesh and the array with vectors defined for each point. Iterate
 * over the neighborhood of each surface vertex, and smooth the vectors based
 * in some smoothing filter
 *
 * Vector Median Filter
 *
 */
vtkFloatArray* SmoothVMF(MeshType::Pointer mesh,vtkFloatArray* vec){
  PointsIterator pIt = mesh->GetPoints()->Begin(), pItEnd = mesh->GetPoints()->End();
  CellsIterator cIt = mesh->GetCells()->Begin(), cItEnd = mesh->GetCells()->End();

  vtkFloatArray *newVecArray = vtkFloatArray::New();
  newVecArray->SetNumberOfComponents(3);
  newVecArray->SetNumberOfTuples(mesh->GetPoints()->Size());
  newVecArray->SetName((std::string(vec->GetName())+"smooth").c_str());

  while(pIt!=pItEnd){
    MeshType::QEType* edge = mesh->FindEdge(pIt->Index());
    MeshType::QEType* tmp = edge;
    MeshType::CellIdentifier cellId(0);
    MeshType::PointIdentifier pointId(0);
    MeshType::CellPixelType cellVal;
    unsigned orig = tmp->GetOrigin();

    std::vector<MeshType::PointIdentifier> neighbors;
    neighbors.push_back(orig);
    do {
      neighbors.push_back(tmp->GetDestination());      
      tmp = tmp->GetOnext();
    } while(tmp!=edge);

    // calculate distances from each point to each neighbor
    float cumDist, minDist = 10000;
    unsigned mini = 10000;
    unsigned nnei = neighbors.size(), i, j;
    for(i=0;i<nnei;i++){
      MeshType::CellPixelType n1, n2;
      vnl_vector<MeshType::CellPixelType::ComponentType> v1, v2;
      v1.set_size(3);
      v2.set_size(3);
      double *v1val = vec->GetTuple3(neighbors[i]);
      v1[0] = v1val[0];
      v1[1] = v1val[1];
      v1[2] = v1val[2];

      cumDist = 0;
      for(j=0;j<nnei;j++){
        if(i==j)
          continue;
        double *v2val = vec->GetTuple3(neighbors[j]);
        v2[0] = v2val[0];
        v2[1] = v2val[1];
        v2[2] = v2val[2];
        cumDist += (v1-v2).two_norm();
//        if(pIt->Index()==778 || pIt->Index()==873)
//          std::cout << pIt->Index() << ": " << neighbors[i] << " to " << neighbors[j] << ": " << angle(v1,v2) << std::endl;
      }
//      if(pIt->Index()==778 || pIt->Index()==873)
//        std::cout << "i=" << i << ", cumAngle=" << cumAngle << std::endl;
      if(cumDist<minDist){
          minDist = cumDist;
          mini = i;
       }
    }

//    if(pIt->Index()==778 || pIt->Index()==873)
//      std::cout << pIt->Index() << ": new value: " << neighbors[mini] << std::endl;
    double *median = vec->GetTuple3(neighbors[mini]);    
    newVecArray->InsertTuple3(pIt->Index(), median[0], median[1], median[2]);

    ++pIt;
//    std::cout << std::endl;
  }

  return newVecArray;
}

// Vector Directional Filter
vtkFloatArray* SmoothVDF(MeshType::Pointer mesh,vtkFloatArray* vec){
  PointsIterator pIt = mesh->GetPoints()->Begin(), pItEnd = mesh->GetPoints()->End();
  CellsIterator cIt = mesh->GetCells()->Begin(), cItEnd = mesh->GetCells()->End();

  vtkFloatArray *newVecArray = vtkFloatArray::New();
  newVecArray->SetNumberOfComponents(3);
  newVecArray->SetNumberOfTuples(mesh->GetPoints()->Size());
  newVecArray->SetName((std::string(vec->GetName())+"smooth").c_str());

  while(pIt!=pItEnd){
    MeshType::QEType* edge = mesh->FindEdge(pIt->Index());
    MeshType::QEType* tmp = edge;
    MeshType::CellIdentifier cellId(0);
    MeshType::PointIdentifier pointId(0);
    MeshType::CellPixelType cellVal;
    unsigned orig = tmp->GetOrigin();

    std::vector<MeshType::PointIdentifier> neighbors;
    neighbors.push_back(orig);
    do {
      neighbors.push_back(tmp->GetDestination());      
      tmp = tmp->GetOnext();
    } while(tmp!=edge);

    // calculate angles from each point to each neighbor
    float cumAngle, minAngle = 10000;
    unsigned mini = 10000;
    unsigned nnei = neighbors.size(), i, j;
    for(i=0;i<nnei;i++){
      MeshType::CellPixelType n1, n2;
      vnl_vector<MeshType::CellPixelType::ComponentType> v1, v2;
      v1.set_size(3);
      v2.set_size(3);
      double *v1val = vec->GetTuple3(neighbors[i]);
      v1[0] = v1val[0];
      v1[1] = v1val[1];
      v1[2] = v1val[2];
      v1.normalize();

      cumAngle = 0;
      for(j=0;j<nnei;j++){
        if(i==j)
          continue;
        double *v2val = vec->GetTuple3(neighbors[j]);
        v2[0] = v2val[0];
        v2[1] = v2val[1];
        v2[2] = v2val[2];
        v2.normalize();
        cumAngle += angle(v1,v2);
//        if(pIt->Index()==778 || pIt->Index()==873)
//          std::cout << pIt->Index() << ": " << neighbors[i] << " to " << neighbors[j] << ": " << angle(v1,v2) << std::endl;
      }
//      if(pIt->Index()==778 || pIt->Index()==873)
//        std::cout << "i=" << i << ", cumAngle=" << cumAngle << std::endl;
      if(cumAngle<minAngle){
          minAngle = cumAngle;
          mini = i;
       }
    }

//    if(pIt->Index()==778 || pIt->Index()==873)
//      std::cout << pIt->Index() << ": new value: " << neighbors[mini] << std::endl;
    double *median = vec->GetTuple3(neighbors[mini]);    
    newVecArray->InsertTuple3(pIt->Index(), median[0], median[1], median[2]);

    ++pIt;
//    std::cout << std::endl;
  }

  return newVecArray;
}

void WriteMesh(MeshType::Pointer mesh, const char* fname){
  vtkPolyData *vtksurf = ITKMesh2PolyData(mesh);
  vtkPolyDataWriter *pdw = vtkPolyDataWriter::New();
  pdw->SetFileName(fname);
  pdw->SetInput(vtksurf);
  pdw->Update();
}

MeshType::Pointer WarpITKMesh(MeshType::Pointer mesh, vtkFloatArray* v){
  PointsIterator pIt = mesh->GetPoints()->Begin(), pItEnd = mesh->GetPoints()->End();
  while(pIt!=pItEnd){
    MeshType::PointType point = pIt->Value();
    double *dv;
    dv = v->GetTuple3(pIt->Index());
    point[0] += dv[0];
    point[1] += dv[1];
    point[2] += dv[2];
    mesh->SetPoint(pIt->Index(), point);
    mesh->GetPoint(pIt->Index(), &point);
    ++pIt;
  }
  return mesh;
}

void WriteImage(ImageType::Pointer image, const char* fname){
  WriterType::Pointer imageWriter1 = WriterType::New();
  imageWriter1->SetFileName(fname);
  imageWriter1->SetInput(image);
  imageWriter1->SetUseCompression(1);
  imageWriter1->Update();
}

ImageType::Pointer MarkChanges(ImageType::Pointer tp0, ImageType::Pointer tp1){
  DupType::Pointer dup = DupType::New();
  dup->SetInputImage(tp0);
  dup->Update();

  ImageType::Pointer output = dup->GetOutput();

  IteratorType it0(tp0, tp0->GetBufferedRegion());
  IteratorType it1(tp1, tp1->GetBufferedRegion());
  IteratorType it(output, output->GetBufferedRegion());
  it0.GoToBegin(); it1.GoToBegin(); it.GoToBegin();
  for(;!it0.IsAtEnd();++it0,++it1,++it){
    if(it0.Get()==0 && it1.Get()!=0)
      // growth
      it.Set(14);
    else if(it0.Get()!=0 && it1.Get()==0)
      // shrinkage
      it.Set(12);    
    else
      it.Set(0);
  }
  return output;
}

void ResetMaskBorder(ImageType::Pointer image, double padValue){


  ImageType::SpacingType spacing = image->GetSpacing();
  ImageType::SizeType padSize, imageSize;

  padSize[0] = padValue/spacing[0]+1;
  padSize[1] = padValue/spacing[1]+1;
  padSize[2] = padValue/spacing[2]+1;

  std::cout << "Pad size: " << padSize << std::endl;

  imageSize = image->GetBufferedRegion().GetSize();

  IteratorType it(image, image->GetBufferedRegion());
  for(it.GoToBegin();!it.IsAtEnd();++it){
    ImageType::IndexType idx = it.GetIndex();
    if(idx[0]<padSize[0] || idx[0]>imageSize[0]-padSize[0] ||
       idx[1]<padSize[1] || idx[1]>imageSize[1]-padSize[1] ||
       idx[2]<padSize[2] || idx[2]>imageSize[2]-padSize[2]){
      it.Set(0);
    }       
  }
  WriteImage(image, "padded_mask.nrrd");
}
