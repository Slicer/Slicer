/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRAblationLogic.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObjectFactory.h"

#include "vtkMRAblationLogic.h"
#include "vtkMRAblation.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkSlicerApplication.h"
#include "vtkImageThermalMap.h"


vtkMRAblationLogic* vtkMRAblationLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRAblationLogic");
  if(ret)
    {
      return (vtkMRAblationLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRAblationLogic;
}


//----------------------------------------------------------------------------
vtkMRAblationLogic::vtkMRAblationLogic()
{
  this->MRAblationNode = NULL;
}

//----------------------------------------------------------------------------
vtkMRAblationLogic::~vtkMRAblationLogic()
{
  this->SetMRAblationNode(NULL);
}

//----------------------------------------------------------------------------
void vtkMRAblationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

void vtkMRAblationLogic::Apply(vtkSlicerVolumesGUI *volsGUI)
{
    // check if MRML node is present 
    if (this->MRAblationNode == NULL)
    {
        vtkErrorMacro("No input MRAblationNode found");
        return;
    }

    if (this->MRAblationNode->GetImageDirectory() == NULL ||
            this->MRAblationNode->GetWorkingDirectory() == NULL)
    {
        vtkErrorMacro("\n\nError: The image directory or working directory is not specified.\n");
        return;
    }

    // Sort images
    std::string imageDir(this->MRAblationNode->GetImageDirectory());
    vtksys::Directory dir;
    int loaded = dir.Load(imageDir.c_str());
    if (! loaded)
    {
        vtkErrorMacro("\n\nError: Couldn't open the image directory: " << imageDir.c_str());
        return;
    }

    int noOfSlices = this->MRAblationNode->GetSlices();
    int noOfTimepoints = this->MRAblationNode->GetTimepoints();
    int noOfFiles = dir.GetNumberOfFiles(); // including . and ..
    int noOfSeq = 4;
    int count = 1;

    // testing
    std::string *files = new std::string [noOfFiles];
    int ii  = 0;
    while (ii < noOfFiles) {
        files[ii] = dir.GetFile(ii);
        ii++;
    }

    // bubble sort the file array
    std::string t;
    for (int x=0; x < noOfFiles-1; x++)
        for (int y=0; y < noOfFiles-x-1; y++)
            if (files[y] > files[y+1])
            {
                t = files[y];
                files[y] = files[y+1];
                files[y+1] = t;
            }

    vtkMRMLScalarVolumeNode *coldRealNode = NULL;
    vtkMRMLScalarVolumeNode *coldImaginaryNode = NULL;
    vtkMRMLScalarVolumeNode *hotRealNode = NULL;
    vtkMRMLScalarVolumeNode *hotImaginaryNode = NULL;
    
    while (count <= noOfTimepoints)
    {
        if ((noOfFiles - 2) >= count * noOfSlices * noOfSeq)
        {
            // std::cerr << "count = " << count << std::endl;
                        // plus . and .. at the beginning 
            int startIndex = (count - 1) * noOfSlices * noOfSeq + 2; 
            for (int s = 0; s < noOfSeq; s++)
            {
                                char volName[50];
                if (s == 0) 
                {
                    sprintf(volName, "tp-%d-mag", count);
                }
                else if (s == 1) 
                {
                    sprintf(volName, "tp-%d-phs", count);
                }
                else if (s == 2) 
                {
                    sprintf(volName, "tp-%d-rea", count);
                }
                else 
                {
                    sprintf(volName, "tp-%d-img", count);
                }

                // Create the temp working dir
                std::string tmp(this->MRAblationNode->GetWorkingDirectory());
#if defined(_WIN32) || defined(__CYGWIN__)
                tmp += "\\mrtmp";
#else
                tmp += "/mrtmp";
#endif
                vtksys::SystemTools::MakeDirectory(tmp.c_str());
                // std::cerr << "tmp dir = " << tmp << std::endl;

                int index = startIndex + s;
                std::string first;
                for (int p = 0; p < noOfSlices; p++) 
                {
                    std::string fileName = files[index];
#if defined(_WIN32) || defined(__CYGWIN__)
                    std::string source = imageDir + "\\" + fileName; 
#else
                    std::string source = imageDir + "/" + fileName;
#endif
                    // std::cerr << "count: " << count;
                    // std::cerr << "  seq: " << s;
                    // std::cerr << "  slice: " << p;
                    // std::cerr << "  flie: " << source << endl;

                    if (p == 0)
                    {
#if defined(_WIN32) || defined(__CYGWIN__)
                        first = tmp + "\\" + fileName; 
#else
                        first = tmp + "/" + fileName;
#endif
                    }
                    vtksys::SystemTools::CopyFileAlways(source.c_str(), tmp.c_str());
                    index += noOfSeq; 
                }

                if (volsGUI != NULL) 
                {
                // If a file has been selected for loading...
                    if ( first.c_str() )
                    {

                        int size = first.size();
                        char *fileName = new char [size];
                        strcpy(fileName, first.c_str());
                        vtkSlicerVolumesLogic* volumeLogic = volsGUI->GetLogic();
                        vtkMRMLVolumeNode *volumeNode = volumeLogic->AddArchetypeVolume( fileName, 1, 0 );
                        delete [] fileName;

                        if ( volumeNode == NULL )
                        {
                            // TODO: generate an error...
                        }
                        else
                        {
                            volsGUI->GetLoadVolumeButton()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
                            volsGUI->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID( volumeNode->GetID() );
                            volsGUI->GetApplicationLogic()->PropagateVolumeSelection();
                            volumeNode->SetName(volName);
                            volsGUI->GetVolumeDisplayWidget()->SetVolumeNode(volumeNode);
                                                        volsGUI->SetAndObserveMRMLNode(volumeNode);

                            // cold (first) real image
                            if (count == 1 && s == 2)
                            {
                                coldRealNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(volumeNode->GetID()));
                             }
                             // cold (first) imaginary image
                             if (count == 1 && s == 3)
                             {
                                 coldImaginaryNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(volumeNode->GetID()));
                             }
                             // hot real image (second and afer)
                             if (count > 1 && s == 2)
                             {
                                 hotRealNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(volumeNode->GetID()));
                             }
                             // hot imaginary image (second and afer)
                             if (count > 1 && s == 3)
                             {
                                 hotImaginaryNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(volumeNode->GetID()));
                             }
                        }
                    }
                }

                vtksys::SystemTools::RemoveADirectory(tmp.c_str());
                                 
                // compute thermal map
                if (coldRealNode != NULL && coldImaginaryNode != NULL &&
                    hotRealNode != NULL && hotImaginaryNode != NULL &&
                    s == 3)
                {
                    vtkImageThermalMap *tmap = vtkImageThermalMap::New();
                    tmap->SetTE(this->MRAblationNode->GetTE());
                    tmap->SetW0(this->MRAblationNode->Getw0());
                    tmap->SetTC(this->MRAblationNode->GetTC());
                    tmap->AddInput(coldRealNode->GetImageData());
                    tmap->AddInput(coldImaginaryNode->GetImageData());
                    tmap->AddInput(hotRealNode->GetImageData());
                    tmap->AddInput(hotImaginaryNode->GetImageData());

                    tmap->Update();
                                      
                    vtkMRMLScalarVolumeNode *mapNode = vtkMRMLScalarVolumeNode::New();
                    mapNode->Copy(coldRealNode);
                    char name[30];
                    sprintf(name, "tmap-%d", count);
                    mapNode->SetName(name);
                    sprintf(name,"vtkMRMLScalarVolumeNode%d", (noOfTimepoints * 4 + count));
                    // ID is set in AddNode
                    //mapNode->SetID(name);
                    mapNode->SetScene(this->GetMRMLScene());
                    this->GetMRMLScene()->AddNode(mapNode);

                    vtkImageData* image = vtkImageData::New(); 
                    image->DeepCopy(tmap->GetOutput());
                    mapNode->SetImageData(image);
                    image->Delete();

                    volsGUI->GetApplicationLogic()->GetSelectionNode()->SetActiveVolumeID(name);

                    tmap->Delete();
                    mapNode->Delete();
                    this->Modified();

                }
            }
            delete [] files;

        }
        else
        {
            // start timer
            return;
        }
        count++;
    }

    return;
}
