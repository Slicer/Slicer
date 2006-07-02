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
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkSmoothPolyDataFilter.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkReverseSense.h"
#include "vtkPolyDataNormals.h"
#include "vtkStripper.h"
#include "vtkPolyDataWriter.h"
#include "vtkImageChangeInformation.h"

#include <string>

int main(int argc, char * argv[])
{
    PARSE_ARGS;
    bool debug = false;

    if (debug) 
    {
        std::cout << "The input volume is: " << InputVolume << std::endl;
        std::cout << "The output directory is: " << OutputDirectory << std::endl;
        std::cout << "The starting label is: " << StartLabel << std::endl;
        std::cout << "The ending label is: " << EndLabel << std::endl;
        if (EndLabel <= StartLabel) 
        {
            std::cout << "The model name is: " << Name << std::endl;
        }
        else 
        {
            // will make multiple
            std::cout << "Do joint smoothing flag is: " << JointSmoothing << std::endl;
        }
        std::cout << "Number of smoothing iterations: " << Smooth << std::endl;
        std::cout << "Number of decimate iterations: " << Decimate << std::endl;
        std::cout << "Split normals? " << SplitNormals << std::endl;
        std::cout << "Calculate point normals? " << PointNormals << std::endl;
        std::cout << "Filter type: " << FilterType << std::endl;
        std::cout << "\nStarting..." << std::endl;
    }

    // vtk and helper variables
    vtkITKArchetypeImageSeriesReader* reader = NULL;
    vtkImageData * image;
    vtkDiscreteMarchingCubes  * cubes = NULL;
    vtkWindowedSincPolyDataFilter *smoother = NULL;
    bool makeMultiple = (EndLabel > StartLabel ? true : false);
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
    vtkTransform * rot = NULL;
    vtkReverseSense * reverser = NULL;
    vtkTransformPolyDataFilter * transformer = NULL;
    vtkPolyDataNormals *normals = NULL;
    vtkStripper * stripper = NULL;
    vtkPolyDataWriter * writer = NULL;


    // check for the input file
    FILE * infile;
    infile = fopen(InputVolume.c_str(),"r");
    if (infile == NULL)
    {
        std::cerr << "ERROR: cannot open input volume file " << InputVolume << endl;
        char waiting;
        std::cout << "Press a key and hit return: ";
        std::cin >> waiting;

        return EXIT_FAILURE;
    }
    fclose(infile);

    // Read the file
    reader = vtkITKArchetypeImageSeriesScalarReader::New();
    reader->SetArchetype(InputVolume.c_str());
    reader->SetOutputScalarTypeToNative();
    reader->SetDesiredCoordinateOrientationToNative();
    reader->SetUseNativeOriginOn();
    reader->Update();

    std::cout << "Done reading the file " << InputVolume << endl;

    vtkImageChangeInformation *ici = vtkImageChangeInformation::New();
    ici->SetInput (reader->GetOutput());
    ici->SetOutputSpacing( 1, 1, 1 );
    ici->SetOutputOrigin( 0, 0, 0 );
    ici->Update();

    image = ici->GetOutput();
    image->Update();

    // If making mulitple models, figure out which labels have voxels
    if (makeMultiple) 
    {
        hist = vtkImageAccumulate::New();
        hist->SetInput(image); 
        hist->SetComponentExtent(0, 1023, 0, 0, 0, 0);
        hist->SetComponentOrigin(0, 0, 0);
        hist->SetComponentSpacing(1,1,1);

        cubes = vtkDiscreteMarchingCubes::New();
        cubes->SetInput(image);
        cubes->GenerateValues((EndLabel-StartLabel +1), StartLabel, EndLabel);
        cubes->Update();

        if (JointSmoothing)
        {
            std::cout << "makeMultiple and JointSmoothing...\n";
            float passBand = 0.001;
            smoother = vtkWindowedSincPolyDataFilter::New();
            smoother->SetInput(cubes->GetOutput());
            smoother->SetNumberOfIterations(Smooth);
            smoother->BoundarySmoothingOff();
            smoother->FeatureEdgeSmoothingOff();
            smoother->SetFeatureAngle(120.0l);
            smoother->SetPassBand(passBand);
            smoother->NonManifoldSmoothingOn();
            smoother->NormalizeCoordinatesOn();
            std::cout << "Smoothing..." << endl;
            smoother->Update();
        }
        std::cout    << "Making histogram..." <<    endl;
        hist->Update();
    }
    else 
    {
        EndLabel = StartLabel;
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
        std::cerr << "The volume is not 3D.\n";
        std::cerr << "\tImage data extents: " << extents[0] << " " << extents[1] << " " << extents[2] << " " << extents[3] << " " << extents[4] << " " << extents[5] << endl;
        return EXIT_FAILURE;
    }

    // Get the RAS to IJK matrix and invert it to get the IJK to RAS which will need
    // to be applied to the model as it will be built in pixel space

    rot = vtkTransform::New();
    // TODO: get the ras to vtk matrix
    rot->SetMatrix(reader->GetRasToIjkMatrix());
    if (debug)
    {
        std::cout << "RasToIjk matrix from file = ";
        rot->GetMatrix()->Print(std::cout);
    }
    rot->Inverse();

    for (int i = StartLabel; i <= EndLabel; i++)
    {
        if (makeMultiple)
        {
            labelFrequency = (((hist->GetOutput())->GetPointData())->GetScalars())->GetTuple1(i);
            if (debug)
            {
                if (labelFrequency > 0) 
                {
                    std::cout << "Label    " << i << "    has    " << labelFrequency    << " voxels.\n";
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
            labelName    = Name + stringI;
        } 
        else 
        {
            // just make one
            labelName = Name;
        }
        std::cout    << "Making model " << labelName    << endl;



        // threshold


        if (JointSmoothing == 0)
        {    
            imageThreshold = vtkImageThreshold::New();
            imageThreshold->SetInput(image);
            imageThreshold->SetReplaceIn(1);
            imageThreshold->SetReplaceOut(1);
            imageThreshold->SetInValue(200);
            imageThreshold->SetOutValue(0);

            std::cout << "Thresholding to " << i << endl;
            imageThreshold->ThresholdBetween(i,i);
            (imageThreshold->GetOutput())->ReleaseDataFlagOn();
            // TODO: add progress

            imageToStructuredPoints = vtkImageToStructuredPoints::New();
            imageToStructuredPoints->SetInput(imageThreshold->GetOutput());
            imageToStructuredPoints->Update();
        } 
        else 
        {
            // use the output of the smoother

            threshold = vtkThreshold::New();
            threshold->SetInput(smoother->GetOutput());
            threshold->SetAttributeModeToUseCellData();

            std::cout << "Thresholding to " << i << endl;
            threshold->ThresholdBetween(i,i);
            (threshold->GetOutput())->ReleaseDataFlagOn();

            geometryFilter = vtkGeometryFilter::New();
            geometryFilter->SetInput(threshold->GetOutput());

        }


        if (JointSmoothing == 0)
        {
            mcubes = vtkMarchingCubes::New();

            mcubes->SetInput(imageToStructuredPoints->GetOutput());
            mcubes->SetValue(0,100.5);
            mcubes->ComputeScalarsOff();
            mcubes->ComputeGradientsOff();
            mcubes->ComputeNormalsOff();
            (mcubes->GetOutput())->ReleaseDataFlagOn();
            // TODO: add progress
            std::cout << "Marching " << labelName << endl;
            mcubes->Update();

            if (debug)
            {
                std::cout << "Number of polygons = " << (mcubes->GetOutput())->GetNumberOfPolys() << endl;
            }

            if ((mcubes->GetOutput())->GetNumberOfPolys()  == 0) 
            {
                std::cout << "Cannot create a model from label "<< i << "\nNo polygons can be created,\nthere may be no voxels with this label in the volume.";
                imageThreshold->SetInput(NULL);
                imageToStructuredPoints->SetInput(NULL);
                mcubes->SetInput(NULL);
                rot->Delete();
                imageThreshold->Delete();
                imageToStructuredPoints->Delete();
                mcubes->Delete();
                continue;
            }
        } 
        else 
        {
            std::cout << "Skipping marching cubes...\n";
        }

        // In switch from vtk 4 to vtk 5, vtkDecimate was deprecated from the Patented dir, use vtkDecimatePro
        // TODO: look at vtkQuadraticDecimation
        decimator = vtkDecimatePro::New();
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

        std::cout << "Decimating ... \n";
        //TODO add progress to decimator
        decimator->Update();
        if (debug)
        {
            std::cout << "After decimation, number of polygons = " << (decimator->GetOutput())->GetNumberOfPolys() << endl;
        }

        if ((rot->GetMatrix())->Determinant() < 0) 
        {
            if (debug)
            {
                std::cout << "Determinant " << (rot->GetMatrix())->Determinant() << " is less than zero, reversing...\n";
            }
            reverser = vtkReverseSense::New();
            reverser->SetInput(decimator->GetOutput());
            reverser->ReverseNormalsOn();
            (reverser->GetOutput())->ReleaseDataFlagOn();
            // TODO: add progress
        }

        if (JointSmoothing == 0)
        {
            if (strcmp(FilterType.c_str(),"Sinc") == 0)
            {

                smootherSinc = vtkWindowedSincPolyDataFilter::New();
                smootherSinc->SetPassBand(0.1);
                if (Smooth == 1)
                {
                    std::cerr << "Warning: Smoothing iterations of 1 not allowed for Sinc filter, using 2" << endl;
                    Smooth = 2;
                }
                if ((rot->GetMatrix())->Determinant() < 0) 
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
                //TODO: insert progress
                std::cout << "Smoothing...\n";
                smootherSinc->Update();
            }
            else 
            {
                smootherPoly = vtkSmoothPolyDataFilter::New();
                // this next line massively rounds corners
                smootherPoly->SetRelaxationFactor(0.33);
                smootherPoly->SetFeatureAngle(60);
                smootherPoly->SetConvergence(0);

                if ((rot->GetMatrix())->Determinant() < 0) 
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
                //TODO: insert progress
                std::cout << "Smoothing...\n";
                smootherPoly->Update();
            }
        }

        transformer = vtkTransformPolyDataFilter::New();
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
            if ((rot->GetMatrix())->Determinant() < 0)
            { 
                transformer->SetInput(reverser->GetOutput());
            }
            else 
            {
                transformer->SetInput(decimator->GetOutput());
            }
        }

        transformer->SetTransform(rot);
        if (debug)
        {
            std::cout << "Transforming using inversed matrix:\n";
            rot->GetMatrix()->Print(std::cout);
        }

        // TODO: add progress
        (transformer->GetOutput())->ReleaseDataFlagOn();

        normals = vtkPolyDataNormals::New();
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
        std::cout << "Splitting normals...\n";
        // TODO: add progress
        (normals->GetOutput())->ReleaseDataFlagOn();

        stripper = vtkStripper::New();
        stripper->SetInput(normals->GetOutput());
        std::cout << "Stripping...\n";
        // TODO: add progress
        (stripper->GetOutput())->ReleaseDataFlagOff();

        // the poly data output from the stripper can be set as an input to a model's polydata
        (stripper->GetOutput())->Update();

        // but for now we're just going to write it out

        writer = vtkPolyDataWriter::New();
        writer->SetInput(stripper->GetOutput());
        //writer->SetFileType(2);
        std::string fileName = OutputDirectory + "/" + labelName + ".vtk";
        writer->SetFileName(fileName.c_str());
        // TODO: add progress
        std::cout << "Writing model " << " " << labelName << " to file " << writer->GetFileName()  << endl;
        writer->Write();

        writer->SetInput(NULL);
        writer->Delete();

    } // end of loop over labels


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

    // Clean up

    if (cubes) 
    {
        cubes->SetInput(NULL);
        cubes->Delete();
    }
    if (smoother)
    {
        smoother->SetInput(NULL);
        smoother->Delete();
    }    
    if (hist) 
    {
        hist->SetInput(NULL);
        hist->Delete();
    }
    if (smootherSinc)
    {
        smootherSinc->SetInput(NULL);
        smootherSinc->Delete();
    }
    if (smootherPoly)
    {
        smootherPoly->SetInput(NULL);
        smootherPoly->Delete();
    }
    if (decimator)
    {
        decimator->SetInput(NULL);
        decimator->Delete();
    }
    if (mcubes)
    {
        mcubes->SetInput(NULL);
        mcubes->Delete();
    }
    if (imageThreshold)
    {
        imageThreshold->SetInput(NULL);
        imageThreshold->Delete();
    }
    if (threshold)
    {
        threshold->SetInput(NULL);
        threshold->Delete();
    }
    if (imageToStructuredPoints)
    {
        imageToStructuredPoints->SetInput(NULL);
        imageToStructuredPoints->Delete();
    }
    if (geometryFilter)
    {
        geometryFilter->SetInput(NULL);
        geometryFilter->Delete();
    }
    if (rot)
    {
        rot->SetInput(NULL);
        rot->Delete();
    }
    if (reverser)
    {
        reverser->SetInput(NULL);
        reverser->Delete();
    }
    if (transformer)
    {
        transformer->SetInput(NULL);
        transformer->Delete();
    }
    if (normals)
    {
        normals->SetInput(NULL);
        normals->Delete();
    }
    if (stripper)
    {
        stripper->SetInput(NULL);
        stripper->Delete();
    }
    if (ici)
    {
        ici->Delete();
    }
    if (debug)
    {
        char waiting;
        std::cout << "Press a key and hit return: ";
        std::cin >> waiting;
    }
    // return
    return EXIT_SUCCESS;
}
