/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile$
Date:      $Date: 2007-05-03 15:51:25 -0400 (Thu, 03 May 2007) $
Version:   $Revision: 3328 $

=========================================================================auto=*/

#include "RealignCLP.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkITKImageWriter.h"
#include "vtkImageData.h"
#include "vtkTransform.h"
#include "vtkPolyData.h"
#include "vtkMath.h"
#include "vtkPoints.h"
#include "vtkMatrix4x4.h"
#include "vtkImageReslice.h"

#include "vtkPluginFilterWatcher.h"

#include "vtkPrincipalAxesAlign.h"

#include <fstream>
#include <string>

#include "vtkMRMLScene.h"
#include "vtkSmartPointer.h"
#include "vtkMRMLLinearTransformNode.h"

int main(int argc, char * argv[])
{
    PARSE_ARGS;
//    bool debug = true;

    if (debugSwitch)
      {
      std::cout << "Trying to get transforms out...\n";
      std::cout << "transform1 = " << transform1.c_str() << std::endl;
      std::cout << "transform2 = " << transform2.c_str() << std::endl;
      }
    
    // tease apart the scene files and the nodes
    std::string::size_type loc;
    std::string transform1Filename, transform2Filename;
    std::string transform1ID, transform2ID;

    loc = transform1.find_last_of("#");
    if (loc != std::string::npos)
      {
      transform1Filename = std::string(transform1.begin(),
                                       transform1.begin() + loc);
      loc++;
      
      transform1ID = std::string(transform1.begin()+loc, transform1.end());
      }
    
    loc = transform2.find_last_of("#");
    if (loc != std::string::npos)
      {
      transform2Filename = std::string(transform2.begin(),
                                       transform2.begin() + loc);
      loc++;
     
      transform2ID = std::string(transform2.begin()+loc, transform2.end());     
      }
    
    if (debugSwitch) 
      {
      std::cout << "The input volume is " << InputVolume.c_str() << std::endl;
      std::cout << "The output volume is " << OutputVolume.c_str() << std::endl;
      std::cout << "The reference volume is " << ReferenceVolume.c_str() << std::endl;
      std::cout << "The ACPC fiducial list is " <<  std::endl;
      for (int i = 0; i<ACPC.size(); i++)
        {
        std::cout << i << ": " << ACPC[i][0] << ", " << ACPC[i][1] << ", " << ACPC[i][2] << std::endl;
        }
      std::cout << "The midline fiducial list is: " << std::endl;
      for (int i= 0; i < Midline.size(); i++ )
        {
        std::cout << i << ": " << Midline[i][0] << ", " << Midline[i][1] << ", " << Midline[i][2] << std::endl;
        }
      std::cout << "The Interpolation type is " << InterpolationType.c_str() << std::endl;
      std::cout << "Transform1 filename: " << transform1Filename << std::endl;
      std::cout << "Transform1 ID: " << transform1ID << std::endl;
      std::cout << "Transform2 filename: " << transform2Filename << std::endl;
      std::cout << "Transform2 ID: " << transform2ID << std::endl;
      std::cout << "\nStarting..." << std::endl;
      }

    // if both transforms are specified, but are not the same, exit
    if (transform1Filename != "" && transform2Filename != "" &&
        transform1Filename != transform2Filename)
      {
      std::cerr << "Module only accepts transforms from the same scene. Two scenes were specified: " << transform1Filename << " and " << transform2Filename << std::endl;
      return EXIT_FAILURE;
      }

    // get the input transform
    vtkSmartPointer<vtkMRMLScene> scene = vtkMRMLScene::New();
    scene->SetURL( transform1Filename.c_str() );
    scene->Import();
    
    vtkMRMLNode *node = scene->GetNodeByID( transform1ID );
    vtkMRMLLinearTransformNode *outNode = NULL;
    if (node)
      {
      outNode = vtkMRMLLinearTransformNode::SafeDownCast(scene->GetNodeByID( transform2ID ));
      if (!outNode)
        {
        std::cout << "No output transform found. Specified transform ID = " << transform2ID << ". Only applying input transform" << std::endl;
        }
      }
    else
      {
      std::cerr << "No input transform found! Specified transform ID = " << transform1ID << std::endl;
      return EXIT_FAILURE;
      }
  
    // vtk vars
    vtkITKArchetypeImageSeriesReader* reader = NULL;
    vtkImageData *image;

    // set up filter watcher
    float numFilterSteps = 2.0;
    if (strcmp(ReferenceVolume.c_str(), "") != 0)
      {
      // add a filter step for reading the reference volume
      numFilterSteps++;
      }
    // increment after each filter is run
    float currentFilterOffset = 0.0;

    // used to pass the space directiosn matrix to the writer
    vtkMatrix4x4 *RasToIjkOut = vtkMatrix4x4::New();
    
    if (strcmp(InputVolume.c_str(), "") != 0)
      {
      reader = vtkITKArchetypeImageSeriesScalarReader::New();
      vtkPluginFilterWatcher watchReader(reader,
                                         "Read Volume",
                                         CLPProcessInformation,
                                         1.0/numFilterSteps,
                                         currentFilterOffset/numFilterSteps);
      if (debugSwitch)
        {
        watchReader.QuietOn();
        }
      currentFilterOffset++;
      reader->SetArchetype(InputVolume.c_str());
      reader->SetOutputScalarTypeToNative();
      reader->SetDesiredCoordinateOrientationToNative();
      reader->SetUseNativeOriginOn();
      reader->Update();
      
      image = reader->GetOutput();
      image->Update();
      }
    
    // RealignCalculate
    vtkTransform *trans = vtkTransform::New();
    trans->Identity();
    trans->PostMultiply();

    // check to see if had no input points
    if (Midline.size() == 0 && ACPC.size() == 0)
      {
      if (debugSwitch)
        {
        std::cout << "Both fid lists are empty (need to select points on the lists), using the input transform" << std::endl;
        }
      // use the input matrix
      vtkMRMLLinearTransformNode *transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
      if (transformNode != NULL)
        {
        trans->SetMatrix(transformNode->GetMatrixTransformToParent());
        if (debugSwitch)
          {
          std::cout << "Set the matrix to:" << std::endl;
          trans->GetMatrix()->Print(std::cout);
          }
        }
      }
    else
      {
      if (Midline.size() > 0)
        {
        if (debugSwitch)
          {
          std::cout << "Doing Midline..." << std::endl;
          }
        vtkMath *math = vtkMath::New();
        vtkPolyData *polydata = vtkPolyData::New();
        vtkPolyData *output = vtkPolyData::New();
        vtkPoints *points = vtkPoints::New();
        points->SetDataTypeToDouble();
        int x = Midline.size();
        if (debugSwitch)
          {
          std::cout << "Total number of midline points " << x << ", points data type = " << points->GetDataType() << std::endl;
          }
        points->SetNumberOfPoints(x);
        for (int i = 0; i < x; i++)
          {
          points->SetPoint(i, Midline[i][0], Midline[i][1], Midline[i][2]);
          if (debugSwitch)
            {
            double pt[3];
            points->GetPoint(i, pt);
            std::cout << "Set midline point " << i << " to " <<pt[0] << ", " << pt[1] << "," <<pt[2] << "\n";
            }
          }
        polydata->SetPoints(points);
        
        vtkPrincipalAxesAlign *pa = vtkPrincipalAxesAlign::New();
        if (debugSwitch)
          {        
          std::cout << "Set Input to PrincipalAxesAlign\n";
          }
        pa->SetInput(polydata);
        if (debugSwitch)
          {        
          std::cout << "Executing PrincipalAxesAlign\n";
          }
        vtkPluginFilterWatcher watchPA(pa,
                                       "Principle Axes Align",
                                       CLPProcessInformation,
                                       1.0/numFilterSteps,
                                       currentFilterOffset/numFilterSteps);
        currentFilterOffset++;
        pa->Update();
        
        vtkFloatingPointType *normal = pa->GetZAxis();
        vtkFloatingPointType nx = normal[0];
        vtkFloatingPointType ny = normal[1];
        vtkFloatingPointType nz = normal[2];
        if (debugSwitch)
          {
          std::cout << "Normal " << nx << " " << ny << " " << nz << std::endl;
          }
        
        vtkFloatingPointType Max = nx;
        if (ny*ny > Max*Max)
          {
          Max = ny;
          }
        if (nz*nz > Max*Max)
          {
          Max = nz;
          }
        vtkFloatingPointType sign = 1.0;
        if (Max < 0)
          {
          sign = -1.0;
          }
        
        // prepare the rotation matrix
        vtkMatrix4x4 *mat = vtkMatrix4x4::New();
        mat->Identity();
        int i = 0;
        for (int p = 0; p < 4; p++)
          {
          vtkFloatingPointType point = normal[p];
          mat->SetElement(i, 0, (sign*point));
          i++;
          }
        vtkFloatingPointType oneAndAlpha = 1.0 + mat->GetElement(0,0);
        mat->SetElement(0, 1, -1.0 * mat->GetElement(1,0));
        mat->SetElement(0, 2, (-1.0 * (mat->GetElement(2, 0)))); 
        mat->SetElement(2, 1, (-1.0 * (mat->GetElement(1, 0) * (mat->GetElement(2, 0) / oneAndAlpha))));
        mat->SetElement(1, 2, (-1.0 * (mat->GetElement(1, 0) * (mat->GetElement(2, 0) / oneAndAlpha))));
        mat->SetElement(1, 1, (1.0  - (mat->GetElement(1, 0) * (mat->GetElement(1, 0) / oneAndAlpha))));
        mat->SetElement(2, 2, (1.0  - (mat->GetElement(2, 0) * (mat->GetElement(2, 0) / oneAndAlpha))));
        
        // Check the sign of the determinant
        double det = mat->Determinant();
        if (debugSwitch)
          {
          std::cout << "Determinant " << det << endl;
          }
        vtkMatrix4x4 *matInverse = vtkMatrix4x4::New();
        matInverse->DeepCopy(mat);
        matInverse->Invert();
        trans->SetMatrix(matInverse);
        
        // clean up
        mat->Delete();
        matInverse->Delete();
        pa->Delete();
        points->Delete();
        polydata->Delete();
        output->Delete();
        math->Delete();
        }        

      // need at least two points
      if (ACPC.size() > 1)
        {
        if (debugSwitch)
          {
          std::cout << "Doing ACPC, size = " << ACPC.size() << "\n";
          }
        int y = ACPC.size();
        vtkFloatingPointType top = ACPC[0][2] - ACPC[1][2];
        vtkFloatingPointType bot = ACPC[0][1] - ACPC[1][1];
        vtkFloatingPointType tangent = atan(top/bot) * (180.0/(4.0*atan(1.0)));
        if (debugSwitch)
          {
          std::cout << "Tangent (top = " << top << ", bot = " << bot << ") = " << tangent << endl;
          }
        trans->RotateX(tangent * -1.0);
        }
      }
    
    double det = trans->GetMatrix()->Determinant();
    if (debugSwitch)
      {
      std::cout << "Determinant " << det << std::endl;
      }
    std::cout << "RAS to RAS Matrix to apply = " << std::endl;
    trans->GetMatrix()->Print(std::cout);

    // transform and save to output volume if requested
    if (OutputVolume.length() > 0)
      {
      if (debugSwitch)
        {
        std::cout << "Doing resample...copying image and changing information\n";
        }
      vtkImageData *Target = vtkImageData::New();
      Target->DeepCopy(image);
      
      // space directions vector is encoded in here
      vtkMatrix4x4 *RasToIjkIn = reader->GetRasToIjkMatrix();
      vtkMatrix4x4 *IjkToRasIn  = vtkMatrix4x4::New();
      IjkToRasIn->DeepCopy(RasToIjkIn);;
      IjkToRasIn->Invert();

      vtkTransform *IjkToRasInTransform = vtkTransform::New();
      IjkToRasInTransform->SetMatrix(IjkToRasIn);
      
      // for now until get a reference transform
      if (strcmp(ReferenceVolume.c_str(), "") != 0)
        {
        vtkITKArchetypeImageSeriesReader* refReader = vtkITKArchetypeImageSeriesScalarReader::New();
        vtkPluginFilterWatcher watchReader(refReader,
                                           "Read Volume",
                                           CLPProcessInformation,
                                           1.0/numFilterSteps,
                                           currentFilterOffset/numFilterSteps);
        if (debugSwitch)
          {
          watchReader.QuietOn();
          }
        currentFilterOffset++;
        refReader->SetArchetype(ReferenceVolume.c_str());
        refReader->SetOutputScalarTypeToNative();
        refReader->SetDesiredCoordinateOrientationToNative();
        refReader->SetUseNativeOriginOn();
        refReader->Update();
        // then get the matrix
        RasToIjkOut = refReader->GetRasToIjkMatrix();
        if (debugSwitch)
          {
          std::cout << "Got the reference volume " << ReferenceVolume.c_str() << " ras to ijk matrix:\n";
          RasToIjkOut->Print(std::cout);
          }
        refReader->Delete();
        }
      else
        {
        RasToIjkOut->DeepCopy(RasToIjkIn);
         if (debugSwitch)
          {
          std::cout << "No reference volume, using the input volume's ras to ijk matrix.\n";
          }
        }
      
      vtkTransform *RasToIjkOutTransform = vtkTransform::New();
      RasToIjkOutTransform->SetMatrix(RasToIjkOut);
      vtkTransform *ResliceTransform = vtkTransform::New();
      //ResliceTransform->PostMultiply();
      
      // build up the reslice transform from the calculated or passed in
      // transform,  the IJK to RAS transform from the input volume, and the
      // RAS to IJK transform from either the input volume or the reference
      // volume, if using it.
      // reslice transfrom = RAS to IJK out . RAS to RAS . IJK to RAS in
      
      // the main transform that we wish to apply is an RAS->RAS transform, so
      // first put the input image data into RAS space
      ResliceTransform->SetMatrix(IjkToRasIn);
      // then apply the transform
      ResliceTransform->Concatenate(trans);
      // now apply the out transform to put the image back into IJK space
      ResliceTransform->Concatenate(RasToIjkOut);
      
      if (debugSwitch)
        {        
        std::cout << "Input volume's RAS to IJK matrix = " << std:: endl;
        RasToIjkIn->Print(std::cout);
        std::cout << "Input volume's inverted matrix, IJK to RAS: \n";
        IjkToRasIn->Print(std::cout);
        std::cout << "trans = " << std::endl;
        trans->GetMatrix()->Print(std::cout);
        std::cout << "Reslice transform = " << std::endl;
        ResliceTransform->GetMatrix()->Print(std::cout);
        }

     
      
      // Set the input of the vtkImageReslice
      vtkImageReslice *reslice = vtkImageReslice::New();
      reslice->SetInput(Target);
      
      // if doing a full resample, would set the output spacing to user
      // entered values, as well as the extent
      if (InterpolationType.length() > 0)
        {
        if (strcmp(InterpolationType.c_str(), "NearestNeighor") == 0)
          {
          reslice->SetInterpolationModeToNearestNeighbor();
          }
        else if (strcmp(InterpolationType.c_str(), "Cubic") == 0)
          {
          reslice->SetInterpolationModeToCubic();
          }
        else if (strcmp(InterpolationType.c_str(), "Linear") == 0)
          {
          reslice->SetInterpolationModeToLinear();
          }
        }

      
       reslice->SetResliceTransform(ResliceTransform);

      // reslice!
      reslice->Update();

      // save, use the itk writer so that memory mapping works
      vtkITKImageWriter *writer = vtkITKImageWriter::New();
      writer->SetRasToIJKMatrix(RasToIjkOut);
      writer->SetFileName(OutputVolume.c_str());
      writer->SetInput(reslice->GetOutput());
      writer->Write();
        
      writer->Delete();
      reslice->Delete();
      Target->Delete();
      
      }

    // clean up   
    if (debugSwitch)
      {
      std::cout << "Deleting reader" << endl;
      }
    reader->Delete();

    if (outNode)
      {
      outNode->SetAndObserveMatrixTransformToParent(trans->GetMatrix());
      scene->Commit( transform2Filename.c_str() );
      }
    if (scene)
      {
      scene->Clear(1);
      scene->Delete();
      }
    if (trans)
      {
      trans->Delete();
      }
    return EXIT_SUCCESS;
}
