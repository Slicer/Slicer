/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile$
Date:      $Date$
Version:   $Revision$

=========================================================================auto=*/

#include "ModelMakerCLP.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"
#include "vtkImageData.h"
#include "vtkImageAccumulate.h"
#include "vtkDiscreteMarchingCubes.h"
#include "vtkMarchingCubes.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkPointData.h"
#include "vtkTransform.h"
#include "vtkImageThreshold.h"
#include "vtkThreshold.h"
#include "vtkUnstructuredGrid.h"
#include "vtkImageToStructuredPoints.h"
#include "vtkGeometryFilter.h"
#include "vtkDecimatePro.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkReverseSense.h"
#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"
#include "vtkPolyDataWriter.h"
#include "vtkImageChangeInformation.h"

#include "vtkPluginFilterWatcher.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkSlicerColorLogic.h"

#include <fstream>
#include <string>
#include <map>

typedef std::map<int, std::string> LabelAnatomyContainer;

int ImportAnatomyLabelFile( std::string, LabelAnatomyContainer &);

int main(int argc, char * argv[])
{
    PARSE_ARGS;
    bool debug = false;

    if (debug) 
      {
      std::cout << "The input volume is: " << InputVolume << std::endl;
      std::cout << "The output directory is: " << OutputDirectory << std::endl;
      std::cout << "The labels are: " << std::endl;
      for (int l = 0; l < Labels.size(); l++)
        {
        std::cout << "\tList element " << l << " = " << Labels[l] << std::endl;
        }
      std::cout << "The starting label is: " << StartLabel << std::endl;
      std::cout << "The ending label is: " << EndLabel << std::endl;
      std::cout << "The model name is: " << Name << std::endl;
      std::cout << "Do joint smoothing flag is: " << JointSmoothing << std::endl;
      std::cout << "Number of smoothing iterations: " << Smooth << std::endl;
      std::cout << "Number of decimate iterations: " << Decimate << std::endl;
      std::cout << "Split normals? " << SplitNormals << std::endl;
      std::cout << "Calculate point normals? " << PointNormals << std::endl;
      std::cout << "Filter type: " << FilterType << std::endl;
      std::cout << "Output model scene file: " << (ModelSceneFile.size() > 0 ? ModelSceneFile[0].c_str() : "None") << std::endl;
      std::cout << "Color table file : " << ColorTable.c_str() << std::endl;
      std::cout << "\nStarting..." << std::endl;
      }

    // get the model hierarchy id from the scene file
    std::string::size_type loc;
    std::string sceneFilename;
    std::string modelHierarchyID;

    if (ModelSceneFile.size() == 0)
      {
      std::cerr << "ERROR: no model hierarchy node defined!" << endl;
      return EXIT_FAILURE;
      }
    
    loc = ModelSceneFile[0].find_last_of("#");
    if (loc != std::string::npos)
      {
      sceneFilename = std::string(ModelSceneFile[0].begin(),
                                  ModelSceneFile[0].begin() + loc);
      loc++;
      
      modelHierarchyID = std::string(ModelSceneFile[0].begin()+loc, ModelSceneFile[0].end());
      }

    if (debug)
      {
      std::cout << "Models file: " << sceneFilename << std::endl;
      std::cout << "Model Hierarchy ID: " << modelHierarchyID << std::endl;
      }

     // check for the model mrml file
    if (sceneFilename == "")
      {
      std::cout << "No file to store models!" << std::endl;
      return EXIT_FAILURE;
      }

    // get the directory of the scene file
    std::string rootDir
      = vtksys::SystemTools::GetParentDirectory( sceneFilename.c_str() );
    
    vtkMRMLScene *modelScene = NULL;
    modelScene = vtkMRMLScene::New();

    modelScene->SetURL(sceneFilename.c_str());
    modelScene->Import();
    
    // make sure we have a model hierarchy node
    vtkMRMLNode *rnd = modelScene->GetNodeByID( modelHierarchyID );
    
    if (!rnd)
      {
      std::cerr << "Error: no model hierarchy node at ID \""
                << modelHierarchyID << "\"" << std::endl;
      return EXIT_FAILURE;
      }
    

    vtkMRMLModelHierarchyNode *rtnd = vtkMRMLModelHierarchyNode::SafeDownCast(rnd);
  
    LabelAnatomyContainer labelToAnatomy;

    vtkMRMLColorTableNode *colorNode = NULL;
    vtkMRMLColorTableStorageNode *colorStorageNode = NULL;
    
    int useColorNode = 0;
    if (ColorTable !=  "")
      {      
      useColorNode = 1;
      }
    else if (AnatomyLabelFile != "" && AnatomyLabelFile != "NoneSpecified")
      {
      // if an anatomy label file is specified, populate a map with its contents
      if (ImportAnatomyLabelFile( AnatomyLabelFile, labelToAnatomy ))
        {
        return EXIT_FAILURE;
        }
      }

    // vtk and helper variables
    vtkITKArchetypeImageSeriesReader* reader = NULL;
    vtkImageData * image;
    vtkDiscreteMarchingCubes  * cubes = NULL;
    vtkWindowedSincPolyDataFilter *smoother = NULL;
    bool makeMultiple = false;
    bool useStartEnd = false;
    vtkImageAccumulate *hist = NULL;
    std::vector<int> skippedModels;
    std::vector<int> madeModels;
    vtkWindowedSincPolyDataFilter * smootherSinc = NULL;    
    vtkSmoothPolyDataFilter * smootherPoly = NULL;

    vtkDecimatePro * decimator = NULL;
    vtkMarchingCubes * mcubes = NULL;
    vtkImageThreshold * imageThreshold = NULL;
    vtkThreshold * threshold = NULL;
    vtkImageToStructuredPoints * imageToStructuredPoints = NULL;
    vtkGeometryFilter * geometryFilter = NULL;
    vtkTransform * transformIJKtoRAS = NULL;
    vtkReverseSense * reverser = NULL;
    vtkTransformPolyDataFilter * transformer = NULL;
    vtkPolyDataNormals *normals = NULL;
    vtkStripper * stripper = NULL;
    vtkPolyDataWriter * writer = NULL;

    vtkSlicerColorLogic *colorLogic = NULL;
    // keep track of number of models that will be generated, for filter
    // watcher reporting
    float numModelsToGenerate = 1.0;
    float numSingletonFilterSteps;
    float numRepeatedFilterSteps;
    float numFilterSteps;
    // increment after each filter is run
    float currentFilterOffset = 0.0;
    // if using the labels vector, get out the min/max
    int labelsMin, labelsMax;

    // figure out if we're making multiple models
    if (Labels.size() > 0)
      {
      makeMultiple = true;
      // TODO: sort the vector
      labelsMin = Labels[0];
      labelsMax = Labels[Labels.size()-1];
      }    
    else if (GenerateAll)
      {
      makeMultiple = true;
      }
    else if (EndLabel > StartLabel)
      {
      makeMultiple = true;
      useStartEnd = true;
      }
    
    if (makeMultiple) 
      {
      numSingletonFilterSteps = 4;
      if (JointSmoothing)
        {
        numRepeatedFilterSteps = 7;
        }
      else
        {
        numRepeatedFilterSteps = 9;
        }
      if (useStartEnd)
        {
        numModelsToGenerate = EndLabel - StartLabel +1;
        }
      else
        {
        numModelsToGenerate = Labels.size();
        }
      }
    else
      {
      numSingletonFilterSteps = 1;
      numRepeatedFilterSteps = 9;
      }
    numFilterSteps = numSingletonFilterSteps + (numRepeatedFilterSteps * numModelsToGenerate);
    if (debug)
      {
      std::cout << "useStartEnd = " << useStartEnd << ", numModelsToGenerate = "<< numModelsToGenerate << ", numFilterSteps " << numFilterSteps << endl;
      }
    // check for the input file
    FILE * infile;
    infile = fopen(InputVolume.c_str(),"r");
    if (infile == NULL)
      {
      std::cerr << "ERROR: cannot open input volume file " << InputVolume << endl;
      if (debug)
        {
        char waiting;
        std::cout << "Press a key and hit return: " << std::endl;
        std::cin >> waiting;
        }
      return EXIT_FAILURE;
      }
    fclose(infile);

    // Read the file
    reader = vtkITKArchetypeImageSeriesScalarReader::New();
    vtkPluginFilterWatcher watchReader(reader,
                                       "Read Volume",
                                       CLPProcessInformation,
                                       1.0/numFilterSteps, currentFilterOffset/numFilterSteps);
    if (debug)
      {
      watchReader.QuietOn();
      }
    currentFilterOffset++;
    reader->SetArchetype(InputVolume.c_str());
    reader->SetOutputScalarTypeToNative();
    reader->SetDesiredCoordinateOrientationToNative();
    reader->SetUseNativeOriginOn();
    reader->Update();

    vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
    ici->SetInput (reader->GetOutput());
    ici->SetOutputSpacing( 1, 1, 1 );
    ici->SetOutputOrigin( 0, 0, 0 );
    ici->Update();

    image = ici->GetOutput();
    image->Update();
    
    if (useColorNode)
      {
      colorNode = vtkMRMLColorTableNode::New();
      modelScene->AddNode(colorNode);

      // read the colour file
      if (debug)
        {
        std::cout << "Colour table file name = " << ColorTable.c_str() << std::endl;
        }
      colorStorageNode = vtkMRMLColorTableStorageNode::New();
      colorStorageNode->SetFileName(ColorTable.c_str());
      modelScene->AddNode(colorStorageNode);

      if (debug)
        {
        std::cout << "Setting the colour node's storage node id to " << colorStorageNode->GetID() << ", it's file name = " << colorStorageNode->GetFileName() << std::endl;
        }
      colorNode->SetStorageNodeID(colorStorageNode->GetID());
      if (!colorStorageNode->ReadData(colorNode))
        {
        std::cerr << "Error reading colour file " << colorStorageNode->GetFileName() << endl;
        return EXIT_FAILURE;
        }
      if (debug)
        {
        std::cout << "Color node after reading file = " << endl;
        colorNode->Print(std::cout);
        }
      }

    // each hierarchy node needs a display node
    vtkMRMLModelDisplayNode *dnd = vtkMRMLModelDisplayNode::New();
    dnd->SetVisibility(1);
    modelScene->AddNode(dnd);
    rtnd->SetAndObserveDisplayNodeID( dnd->GetID() );
    dnd->Delete();
  
    // If making mulitple models, figure out which labels have voxels
    if (makeMultiple) 
      {
      hist = vtkImageAccumulate::New();
      hist->SetInput(image); 
      hist->SetComponentExtent(0, 1023, 0, 0, 0, 0);
      hist->SetComponentOrigin(0, 0, 0);
      hist->SetComponentSpacing(1,1,1);      
      
      cubes = vtkDiscreteMarchingCubes::New();
      std::string comment = "Discrete Marching Cubes";
      vtkPluginFilterWatcher watchDMCubes(cubes,
                                          comment.c_str(),
                                          CLPProcessInformation,
                                          1.0/numFilterSteps,
                                          currentFilterOffset/numFilterSteps);
      if (debug)
        {
        watchDMCubes.QuietOn();
        }
      currentFilterOffset += 1.0;
      cubes->SetInput(image);
      if (useStartEnd)
        {
        cubes->GenerateValues((EndLabel-StartLabel +1), StartLabel, EndLabel);
        }
      else
        {
        cubes->GenerateValues((labelsMax - labelsMin + 1), labelsMin, labelsMax);
        }
      cubes->Update();
      
      if (JointSmoothing)
        {
        float passBand = 0.001;
        smoother = vtkWindowedSincPolyDataFilter::New();
        std::string comment = "Joint Smooth All Models";
        vtkPluginFilterWatcher watchSmoother(smoother,
                                             comment.c_str(),
                                             CLPProcessInformation,
                                             1.0/numFilterSteps,
                                             currentFilterOffset/numFilterSteps);
        currentFilterOffset += 1.0;
        if (debug)
          {
          watchSmoother.QuietOn();
          }
        cubes->ReleaseDataFlagOn();
        smoother->SetInput(cubes->GetOutput());
        smoother->SetNumberOfIterations(Smooth);
        smoother->BoundarySmoothingOff();
        smoother->FeatureEdgeSmoothingOff();
        smoother->SetFeatureAngle(120.0l);
        smoother->SetPassBand(passBand);
        smoother->NonManifoldSmoothingOn();
        smoother->NormalizeCoordinatesOn();

        smoother->Update();
        //        smoother->ReleaseDataFlagOn();
        }

      vtkPluginFilterWatcher watchImageAccumulate(hist,
                                                 "Histogram All Models",
                                                 CLPProcessInformation,
                                                 1.0/numFilterSteps,
                                                 currentFilterOffset/numFilterSteps);
      currentFilterOffset += 1.0;
      if (debug)
        {
        watchImageAccumulate.QuietOn();
        }
      hist->Update();
      double *max = hist->GetMax();
      double *min = hist->GetMin();
       if (debug)
         {
         std::cout << "Min = " << min[0] << " and max = " << max[0] << endl;
         }
       
      if (GenerateAll)
        {
        if (debug)
          {
          std::cout << "GenerateAll flag is true, resetting the start and end labels from: " << StartLabel << " and " << EndLabel << " to " << min[0] << " and " << max[0] << endl;
          }
        StartLabel = (int)floor(min[0]);
        EndLabel = (int)floor(max[0]);
        // recalculate the number of filter steps, discount the labels with no
        // voxels       
        numModelsToGenerate = 0;
        for (int i = StartLabel; i <= EndLabel; i++)
          {
          if((int)floor((((hist->GetOutput())->GetPointData())->GetScalars())->GetTuple1(i)) > 0)
            {
            if (debug && i < 0 && i > -100) { std::cout << i << " "; }
            numModelsToGenerate++;
            }
          }
        if (debug)
          {
          std::cout << endl << "GenerateAll: there are " << numModelsToGenerate << " models to be generated." << endl;
          }
        numFilterSteps = numSingletonFilterSteps + (numRepeatedFilterSteps * numModelsToGenerate);
        }
      
      if (useColorNode)
        {
        // but if we didn't get a named color node, try to guess
        if (colorNode == NULL)
          {
          std::cerr << "ERROR: must have a color node! Should be associated with the input label map volume.\n";
          return EXIT_FAILURE;
          }
        }
      } // end of make multiple
    else 
      {
      if (useStartEnd)
        {
        EndLabel = StartLabel;
        }
      }

    // ModelMakerMarch
    int labelFrequency;
    std::string labelName;
    
    // get the dimensions, marching cubes only works on 3d
    int extents[6];
    image->GetExtent(extents);
    if (debug)
      {
      std::cout << "Image data extents: " << extents[0] << " " << extents[1] << " " << extents[2] << " " << extents[3] << " " << extents[4] << " " << extents[5] << endl;
      }
    if (extents[0] == extents[1] ||
        extents[2] == extents[3] ||
        extents[4] == extents[5]) 
      {
      std::cerr << "The volume is not 3D." << endl;
      std::cerr << "\tImage data extents: " << extents[0] << " " << extents[1] << " " << extents[2] << " " << extents[3] << " " << extents[4] << " " << extents[5] << endl;
      return EXIT_FAILURE;
      }
    // Get the RAS to IJK matrix and invert it to get the IJK to RAS which will need
    // to be applied to the model as it will be built in pixel space
    
    transformIJKtoRAS = vtkTransform::New();
    transformIJKtoRAS->SetMatrix(reader->GetRasToIjkMatrix());
    if (debug)
      {
      std::cout << "RasToIjk matrix from file = ";
      transformIJKtoRAS->GetMatrix()->Print(std::cout);
      std::cout << endl;
      }
    transformIJKtoRAS->Inverse();

    //
    // Loop through all the labels
    //
    std::vector<int> loopLabels;
    if (useStartEnd)
      {
      // set up the loop list with all the labels between start and end
      for (int i = StartLabel; i <= EndLabel; i++)
        {
        loopLabels.push_back(i);
        }
      }
    else
      {
      // just copy the list of labels into the new var
      for (int i = 0; i < Labels.size(); i++)
        {
        loopLabels.push_back(Labels[i]);
        }
      }
    for (int l = 0; l < loopLabels.size(); l++)
      {
      // get the label out of the vector
      int i = loopLabels[l];
      
      if (makeMultiple)
        {
        labelFrequency = (int)floor((((hist->GetOutput())->GetPointData())->GetScalars())->GetTuple1(i));
        if (debug)
          {
          if (labelFrequency > 0) 
            {
            std::cout << "Label    " << i << "    has    " << labelFrequency    << " voxels." << endl;
            }
          }
        if (labelFrequency ==0) 
          {
          skippedModels.push_back(i);
          continue;
          }
        else 
          {
          madeModels.push_back(i);
          }
        
        // name this model
        // TODO: get the label name from the colour look up table
        std::stringstream    stream;
        stream <<    i;
        std::string stringI =    stream.str();
        if (colorNode != NULL)
          {
          if (colorNode->GetColorNameWithoutSpaces(i, "_").c_str() != NULL)
            {
            labelName = Name + std::string("_") + stringI + std::string("_") + std::string(colorNode->GetColorNameWithoutSpaces(i, "_"));
            if (debug)
              {
              std::cout << "Got color name, set label name = " << labelName.c_str() << " (color name w/o spaces = " << colorNode->GetColorNameWithoutSpaces(i, "_") << endl;
              }
            }
          else
            {
            // colour is out of range
            labelName = Name + std::string("_") + stringI;
            }
          }
        else
          {
          if (labelToAnatomy.find(i) != labelToAnatomy.end())
            {
            labelName = labelToAnatomy[i];
            }
          else
            {
            if (!SkipUnNamed)
              {
              labelName  = Name + std::string("_") + stringI;
              }
            else
              {
              continue;
              }
            }
          }
        } // end of making multiples 
      else 
        {
        // just make one
        if (colorNode != NULL)
          {
          if (colorNode->GetColorNameWithoutSpaces(i, "_").c_str() != NULL)
            {
            std::stringstream    stream;
            stream <<    i;
            std::string stringI =    stream.str();
            labelName = Name + std::string("_") + stringI + std::string("_") + std::string(colorNode->GetColorNameWithoutSpaces(i, "_"));
            }
          }
        else
          {
          labelName = Name;
          }
        }
      
      // threshold
      if (JointSmoothing == 0)
        {    
        imageThreshold = vtkImageThreshold::New();
        std::string comment = "Threshold " + labelName;
        vtkPluginFilterWatcher watchImageThreshold(imageThreshold,
                                                   comment.c_str(),
                                                   CLPProcessInformation,
                                                   1.0/numFilterSteps,
                                                   currentFilterOffset/numFilterSteps);
        currentFilterOffset += 1.0;
        if (debug)
          {
          watchImageThreshold.QuietOn();
          }
        imageThreshold->SetInput(image);
        imageThreshold->SetReplaceIn(1);
        imageThreshold->SetReplaceOut(1);
        imageThreshold->SetInValue(200);
        imageThreshold->SetOutValue(0);

        imageThreshold->ThresholdBetween(i,i);
        (imageThreshold->GetOutput())->ReleaseDataFlagOn();
        imageThreshold->ReleaseDataFlagOn();
            
        imageToStructuredPoints = vtkImageToStructuredPoints::New();
        imageToStructuredPoints->SetInput(imageThreshold->GetOutput());
        imageToStructuredPoints->Update();
        imageToStructuredPoints->ReleaseDataFlagOn();
        } 
      else 
        {
        // use the output of the smoother
          
        threshold = vtkThreshold::New();
        std::string comment = "Threshold " + labelName;
        vtkPluginFilterWatcher watchThreshold(threshold,
                                              comment.c_str(),
                                              CLPProcessInformation,
                                              1.0/numFilterSteps,
                                              currentFilterOffset/numFilterSteps);
        currentFilterOffset += 1.0;
        if (debug)
          {
          watchThreshold.QuietOn();
          }
        threshold->SetInput(smoother->GetOutput());
        // In VTK 5.0, this is deprecated - the default behaviour seems to
        // be okay
        // threshold->SetAttributeModeToUseCellData();
        
        threshold->ThresholdBetween(i,i);
        (threshold->GetOutput())->ReleaseDataFlagOn();
        threshold->ReleaseDataFlagOn();
        
        geometryFilter = vtkGeometryFilter::New();
        geometryFilter->SetInput(threshold->GetOutput());
        geometryFilter->ReleaseDataFlagOn();
        }

      // if not joint smoothing, may need to skip this label
      int skipLabel = 0;
      if (JointSmoothing == 0)
        {
        mcubes = vtkMarchingCubes::New();
        std::string comment = "Marching Cubes " + labelName;
        vtkPluginFilterWatcher watchThreshold(mcubes,
                                              comment.c_str(),
                                              CLPProcessInformation,
                                              1.0/numFilterSteps, 
                                              currentFilterOffset/numFilterSteps);
        currentFilterOffset += 1.0;
        if (debug)
          {
          watchThreshold.QuietOn();
          }
        mcubes->SetInput(imageToStructuredPoints->GetOutput());
        mcubes->SetValue(0,100.5);
        mcubes->ComputeScalarsOff();
        mcubes->ComputeGradientsOff();
        mcubes->ComputeNormalsOff();
        (mcubes->GetOutput())->ReleaseDataFlagOn();
        mcubes->Update();
        
        if (debug)
          {
          std::cout << "\nNumber of polygons = " << (mcubes->GetOutput())->GetNumberOfPolys() << endl;
          }

        if ((mcubes->GetOutput())->GetNumberOfPolys()  == 0) 
          {
          std::cout << "Cannot create a model from label "<< i << "\nNo polygons can be created,\nthere may be no voxels with this label in the volume." << endl;
          if (transformIJKtoRAS)
            {
            transformIJKtoRAS->Delete();
            transformIJKtoRAS = NULL;
            }
          if (imageThreshold)
            {
            if (debug)
              {
              std::cout << "Setting image threshold input to null" << endl;
              }
            imageThreshold->SetInput(NULL);
            imageThreshold->RemoveAllInputs();              
            imageThreshold->Delete();
            imageThreshold = NULL;
              
            }
          if (imageToStructuredPoints)
            {
            imageToStructuredPoints->SetInput(NULL);
            imageToStructuredPoints->Delete();
            imageToStructuredPoints = NULL;
            }
          if (mcubes)
            {
            mcubes->SetInput(NULL);
            mcubes->Delete();
            mcubes = NULL;
            }
          skipLabel = 1;
          std::cout << "...continuing" << endl;
          continue;
          }
        }
      else 
        {
        std::cout << "Skipping marching cubes..." << endl;
        }
      if (!skipLabel)
        {
      // In switch from vtk 4 to vtk 5, vtkDecimate was deprecated from the Patented dir, use vtkDecimatePro
      // TODO: look at vtkQuadraticDecimation
      decimator = vtkDecimatePro::New();
      std::string comment = "Decimate " + labelName;
      vtkPluginFilterWatcher watchImageThreshold(decimator,
                                                 comment.c_str(),
                                                 CLPProcessInformation,
                                                 1.0/numFilterSteps, 
                                                 currentFilterOffset/numFilterSteps);
      currentFilterOffset += 1.0;
      if (debug)
          {
          watchImageThreshold.QuietOn();
          }
      if (JointSmoothing == 0)
        {
        decimator->SetInput(mcubes->GetOutput());
        }
      else
        {
        decimator->SetInput(geometryFilter->GetOutput());
        }
      decimator->SetFeatureAngle(60);
      //decimator->SetMaximumIterations(Decimate);
      //decimator->SetMaximumSubIterations(0);

      //decimator->PreserveEdgesOn();
      decimator->SplittingOff();
      decimator->PreserveTopologyOn();
      
      decimator->SetMaximumError(1);
      decimator->SetTargetReduction(Decimate);
      //decimator->SetInitialError(0.0002);
      //decimator->SetErrorIncrement(0.002);
      (decimator->GetOutput())->ReleaseDataFlagOff();

      decimator->Update();
      if (debug)
        {
        std::cout << "After decimation, number of polygons = " << (decimator->GetOutput())->GetNumberOfPolys() << endl;
        }

      if ((transformIJKtoRAS->GetMatrix())->Determinant() < 0) 
        {
        if (debug)
          {
          std::cout << "Determinant " << (transformIJKtoRAS->GetMatrix())->Determinant() << " is less than zero, reversing..." << endl;
          }
        reverser = vtkReverseSense::New();
        std::string comment = "Reverse " + labelName;
        vtkPluginFilterWatcher watchReverser(reverser,
                                             comment.c_str(),
                                             CLPProcessInformation,
                                             1.0/numFilterSteps, 
                                             currentFilterOffset/numFilterSteps);
        currentFilterOffset += 1.0;
        if (debug)
          {
          watchReverser.QuietOn();
          }
        reverser->SetInput(decimator->GetOutput());
        reverser->ReverseNormalsOn();
        (reverser->GetOutput())->ReleaseDataFlagOn();
        }

      if (JointSmoothing == 0)
        {
        if (strcmp(FilterType.c_str(),"Sinc") == 0)
          {
            
          smootherSinc = vtkWindowedSincPolyDataFilter::New();
          std::string comment = "Smooth " + labelName;
          vtkPluginFilterWatcher watchSmoother(smootherSinc,
                                               comment.c_str(),
                                               CLPProcessInformation,
                                               1.0/numFilterSteps, 
                                               currentFilterOffset/numFilterSteps);
          currentFilterOffset += 1.0;
          if (debug)
            {
            watchSmoother.QuietOn();
            }
          smootherSinc->SetPassBand(0.1);
          if (Smooth == 1)
            {
            std::cerr << "Warning: Smoothing iterations of 1 not allowed for Sinc filter, using 2" << endl;
            Smooth = 2;
            }
          if ((transformIJKtoRAS->GetMatrix())->Determinant() < 0) 
            {
            smootherSinc->SetInput(reverser->GetOutput());
            } 
          else 
            {
            smootherSinc->SetInput(decimator->GetOutput());
            }
          smootherSinc->SetNumberOfIterations(Smooth);
          smootherSinc->FeatureEdgeSmoothingOff();
          smootherSinc->BoundarySmoothingOff();
          (smootherSinc->GetOutput())->ReleaseDataFlagOn();
          // smootherSinc->ReleaseDataFlagOn();

          smootherSinc->Update();
          }
        else 
          {
          smootherPoly = vtkSmoothPolyDataFilter::New();
          std::string comment = "Smooth " + labelName;
          vtkPluginFilterWatcher watchSmoother(smootherPoly,
                                               comment.c_str(),
                                               CLPProcessInformation,
                                               1.0/numFilterSteps, 
                                               currentFilterOffset/numFilterSteps);
          currentFilterOffset += 1.0;
          if (debug)
            {
            watchSmoother.QuietOn();
            }
        
          // this next line massively rounds corners
          smootherPoly->SetRelaxationFactor(0.33);
          smootherPoly->SetFeatureAngle(60);
          smootherPoly->SetConvergence(0);
          
          if ((transformIJKtoRAS->GetMatrix())->Determinant() < 0) 
            {
            smootherPoly->SetInput(reverser->GetOutput());
            } 
          else 
            {
            smootherPoly->SetInput(decimator->GetOutput());
            }
          smootherPoly->SetNumberOfIterations(Smooth);
          smootherPoly->FeatureEdgeSmoothingOff();
          smootherPoly->BoundarySmoothingOff();
          (smootherPoly->GetOutput())->ReleaseDataFlagOn();
          //smootherPoly->ReleaseDataFlagOn();
            
          smootherPoly->Update();
          }
        }

      transformer = vtkTransformPolyDataFilter::New();
      std::string comment1 = "Transform " + labelName;
      vtkPluginFilterWatcher watchTransformer(transformer,
                                              comment1.c_str(),
                                              CLPProcessInformation,
                                              1.0/numFilterSteps, 
                                              currentFilterOffset/numFilterSteps);
      currentFilterOffset += 1.0;
      if (debug)
        {
        watchTransformer.QuietOn();
        }
      if (JointSmoothing == 0)
        {
        if (strcmp(FilterType.c_str(),"Sinc") == 0)
          {
          transformer->SetInput(smootherSinc->GetOutput());
          }
        else 
          {
          transformer->SetInput(smootherPoly->GetOutput());
          }
        }
      else
        {
        if ((transformIJKtoRAS->GetMatrix())->Determinant() < 0)
          { 
          transformer->SetInput(reverser->GetOutput());
          }
        else 
          {
          transformer->SetInput(decimator->GetOutput());
          }
        }

      transformer->SetTransform(transformIJKtoRAS);
      if (debug)
        {
        transformIJKtoRAS->GetMatrix()->Print(std::cout);
        }

      (transformer->GetOutput())->ReleaseDataFlagOn();

      normals = vtkPolyDataNormals::New();
      std::string comment2 = "Normals " + labelName;
      vtkPluginFilterWatcher watchNormals(normals,
                                          comment2.c_str(),
                                          CLPProcessInformation,
                                          1.0/numFilterSteps, 
                                          currentFilterOffset/numFilterSteps);
      currentFilterOffset += 1.0;
      if (debug)
        {
        watchNormals.QuietOn();
        }
      
      if (PointNormals)
        {
        normals->ComputePointNormalsOn();
        }
      else 
        {
        normals->ComputePointNormalsOff();
        }
      normals->SetInput(transformer->GetOutput());
      normals->SetFeatureAngle(60);
      normals->SetSplitting(SplitNormals);

      (normals->GetOutput())->ReleaseDataFlagOn();

      stripper = vtkStripper::New();
      std::string comment3 = "Strip " + labelName;
      vtkPluginFilterWatcher watchStripper(stripper,
                                           comment3.c_str(),
                                           CLPProcessInformation,
                                           1.0/numFilterSteps, 
                                           currentFilterOffset/numFilterSteps);
      currentFilterOffset += 1.0;
      if (debug)
        {
        watchStripper.QuietOn();
        }
      
      stripper->SetInput(normals->GetOutput());
      
      (stripper->GetOutput())->ReleaseDataFlagOff();
      
      // the poly data output from the stripper can be set as an input to a model's polydata
      (stripper->GetOutput())->Update();

      // but for now we're just going to write it out

      writer = vtkPolyDataWriter::New();
      std::string comment4 = "Write " + labelName;
      vtkPluginFilterWatcher watchWriter(writer,
                                         comment4.c_str(),
                                         CLPProcessInformation,
                                         1.0/numFilterSteps, 
                                         currentFilterOffset/numFilterSteps);
      currentFilterOffset += 1.0;
      if (debug)
        {
        watchWriter.QuietOn();
        }
      
      writer->SetInput(stripper->GetOutput());
      writer->SetFileType(2);
      std::string fileName;
      if (OutputDirectory != "")
        {
        fileName = OutputDirectory + std::string("/") + labelName + std::string(".vtk");
        }
      else
        {
        std::cout << "WARNING: output directory is an empty string..." << endl;
        fileName = labelName + std::string(".vtk");
        }
      writer->SetFileName(fileName.c_str());

      if (debug)
        {
        std::cout << "Writing model " << " " << labelName << " to file " << writer->GetFileName()  << endl;
        }
      writer->Write();
        
      writer->SetInput(NULL);
      writer->Delete();
      writer = NULL;
      if (modelScene != NULL)
        {
        if (debug)
          {
          std::cout << "Adding model " << labelName << " to the output scene, with filename " << fileName.c_str() << endl;
          }
        // each model needs a mrml node, a storage node and a display node
        vtkMRMLModelNode *mnode = vtkMRMLModelNode::New();
        mnode->SetScene(modelScene);
        mnode->SetName(labelName.c_str());
        
        vtkMRMLModelStorageNode *snode = vtkMRMLModelStorageNode::New();
        snode->SetFileName(fileName.c_str());
        if (modelScene->AddNode(snode) == NULL)
          {
          std::cerr << "ERROR: unable to add the storage node to the model scene" << endl;
          }        
        vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::New();
        dnode->SetPolyData(mnode->GetPolyData());
        dnode->SetColor(0.5, 0.5, 0.5);
        double *rgba;
        if (colorNode != NULL)
          {
          rgba = colorNode->GetLookupTable()->GetTableValue(i);
          if (rgba != NULL)
            {
            if (debug)
              {
              std::cout << "Got colour: " << rgba[0] << " " << rgba[1] << " " << rgba[2] << " " << rgba[3] << endl;
              }
            dnode->SetColor(rgba[0], rgba[1], rgba[2]);
            }
          else
            {
            std::cerr << "Couldn't get look up table value for " << i << ", display node colour is not set (grey)" << endl;
            }
          }
          
        dnode->SetVisibility(1);
        modelScene->AddNode(dnode);
        if (debug)
          {
          std::cout << "Added display node: id = " << (dnode->GetID() == NULL ? "(null)" : dnode->GetID()) << endl;
          std::cout << "Setting model's storage node: id = " << (snode->GetID() == NULL ? "(null)" : snode->GetID()) << endl;
          }
        mnode->SetReferenceStorageNodeID(snode->GetID());        
        mnode->SetAndObserveDisplayNodeID(dnode->GetID());
        modelScene->AddNode(mnode);

        // put it in the hierarchy
        vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::New();
        modelScene->AddNode(mhnd);
        mhnd->SetParentNodeID( rnd->GetID() );
        mhnd->SetModelNodeID( mnode->GetID() );
    
        if (debug)
          {
          std::cout << "...done adding model to output scene" << endl;
          }
        // clean up
        dnode->Delete();
        dnode = NULL;
        snode->Delete();
        snode = NULL;
        mnode->Delete();
        mnode = NULL;
        mhnd->Delete();
        mhnd = NULL;
        }
        } // end of skipping an empty label
      } // end of loop over labels
    if (debug)
      {
      std::cout << "End of looping over labels" << endl;
     } 
    // Report what was done
    if (madeModels.size() > 0) 
      {
      std::cout << "Made models from labels:";
      for (unsigned int i = 0; i < madeModels.size(); i++)
        {
        std::cout << " " << madeModels[i];
        }
      std::cout << endl;
      }
    if (skippedModels.size() > 0) 
      {
      std::cout << "Skipped making models from labels:";
      for (unsigned int i = 0; i < skippedModels.size(); i++)
        {
        std::cout << " " << skippedModels[i];
        }
      std::cout << endl;
      }
    if (ModelSceneFile[0] != "")
      {
      if (debug)
        {
        std::cout << "Writing to model scene output file: " << ModelSceneFile[0].c_str();
        std::cout << ", to url: " << modelScene->GetURL() << std::endl;
        }
      modelScene->Commit();
      }
    
    // Clean up
    if (debug)
      {
      std::cout << "Cleaning up" << endl;
      }
    if (cubes) 
      {
      if (debug)
        {
        std::cout << "Deleting cubes" << endl;
        }
      cubes->SetInput(NULL);
      cubes->Delete();
      }
    if (smoother)
      {
      if (debug)
        {
        std::cout << "Deleting smoother" << endl;
        }
      smoother->SetInput(NULL);
      smoother->Delete();
      }    
    if (hist) 
      {
      if (debug)
        {
        std::cout << "Deleting hist" << endl;
        }
      hist->SetInput(NULL);
      hist->Delete();
      }
    if (smootherSinc)
      {
      if (debug)
        {
        std::cout << "Deleting smootherSinc" << endl;
        }
      smootherSinc->SetInput(NULL);
      smootherSinc->Delete();
      }
    if (smootherPoly)
      {
      if (debug)
        {
        std::cout << "Deleting smoother poly" << endl;
        }
      smootherPoly->SetInput(NULL);
      smootherPoly->Delete();
      }
    if (decimator)
      {
      if (debug)
        {
        std::cout << "Deleting decimator" << endl;
        }
      decimator->SetInput(NULL);
      decimator->Delete();
      }
    if (mcubes)
      {
      if (debug)
        {
        std::cout << "Deleting mcubes" << endl;
        }
      mcubes->SetInput(NULL);
      mcubes->Delete();
      }
    if (imageThreshold)
      {
      if (debug)
        {
        std::cout << "Deleting image threshold" << endl;
        }
      imageThreshold->SetInput(NULL);
      imageThreshold->RemoveAllInputs();
      imageThreshold->Delete();
      if (debug)
        {
        std::cout << "... done deleting image threshold" << endl;
        }
      }
    if (threshold)
      {
      if (debug)
        {
        cout << "Deleting threshold" << endl;
        }
      threshold->SetInput(NULL);
      threshold->Delete();
      }
    if (imageToStructuredPoints)
      {
      if (debug)
        {
        std::cout << "Deleting iamge to structured points" << endl;
        }
      imageToStructuredPoints->SetInput(NULL);
      imageToStructuredPoints->Delete();
      }
    if (geometryFilter)
      {
      if (debug)
        {
        cout << "Deleting geometry filter" << endl;
        }
      geometryFilter->SetInput(NULL);
      geometryFilter->Delete();
      }
    if (transformIJKtoRAS)
      {
      if (debug)
        {
        std::cout << "Deleting transform ijk to ras" << endl;
        }
      transformIJKtoRAS->SetInput(NULL);
      transformIJKtoRAS->Delete();
      }
    if (reverser)
      {
      if (debug)
        {
        std::cout << "Deleting reverser" << endl;
        }
      reverser->SetInput(NULL);
      reverser->Delete();
      }
    if (transformer)
      {
      if (debug)
        {
        std::cout << "Deleting transformer" << endl;
        }
      transformer->SetInput(NULL);
      transformer->Delete();
      }
    if (normals)
      {
      if (debug)
        {
        std::cout << "Deleting normals" << endl;
        }
      normals->SetInput(NULL);
      normals->Delete();
      }
    if (stripper)
      {
      if (debug)
        {
        std::cout << "Deleting stripper" << endl;
        }
      stripper->SetInput(NULL);
      stripper->Delete();
      }
    if (ici)
      {
      if (debug)
        {
        std::cout << "Deleting ici, no set input null" << endl;
        }
      ici->SetInput(NULL);
      ici->Delete();
      }
    if (debug)
      {
      std::cout << "Deleting reader" << endl;
      }
    reader->Delete();
    
    if (modelScene)
      {
      if (debug)
        {
        std::cout << "Deleting model scene" << endl;
        }
      modelScene->Clear(1);
      modelScene->Delete();
      modelScene = NULL;
      }
    return EXIT_SUCCESS;
}

int ImportAnatomyLabelFile( std::string anatomyLabelFile,
                             LabelAnatomyContainer &map)

{
  std::ifstream fin(anatomyLabelFile.c_str(),std::ios::in|std::ios::binary);
  if (fin.fail())
    {
    std::cerr << "ImportAnatomyLabelFile: Cannot open " << anatomyLabelFile << " for input" << std::endl;
    return EXIT_FAILURE;
    }

  char label[81];
  char anatomy[81];
  char aLine[81];
  
  fin.getline(aLine, 80);
  while (!fin.eof())
    {    
    fin.getline(anatomy, 80, ',');
    fin.getline(label, 80);
    std::string anatomyStr(anatomy);
    map[atoi(label)] = anatomyStr;
    }

  fin.close();
  return EXIT_SUCCESS;
}
