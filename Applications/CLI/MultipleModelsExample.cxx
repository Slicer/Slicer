/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile$
Date:      $Date$
Version:   $Revision$

=========================================================================auto=*/

#include "MultipleModelsExampleCLP.h"

#include "vtkPluginFilterWatcher.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLModelHierarchyNode.h"

#include "vtkConeSource.h"
#include "vtkCubeSource.h"
#include "vtkSphereSource.h"
#include "vtkPolyData.h"
#include "vtkDebugLeaks.h"
#include "vtkLookupTable.h"

#include "vtksys/SystemTools.hxx"

#include <fstream>
#include <string>
#include <map>


int main(int argc, char * argv[])
{
  PARSE_ARGS;
  vtkDebugLeaks::SetExitError(true);

  std::cout << "Label image file: " << InputVolume << std::endl;
  std::cout << "Color table file: " << ColorTable << std::endl;
  std::cout << "Number of models: " << NumberOfModels << std::endl;

  // put in code to read the color table file
  //
  vtkMRMLColorTableNode *colorNode = NULL;
  vtkMRMLColorTableStorageNode *colorStorageNode = NULL;
  
  // tease apart the scene files and the nodes
  std::string::size_type loc;
  std::string sceneFilename;
  std::string modelHierarchyID;

  std::string models = Models[0];
  
  loc = models.find_last_of("#");
  if (loc != std::string::npos)
    {
    sceneFilename = std::string(models.begin(),
                                models.begin() + loc);
    loc++;
    
    modelHierarchyID = std::string(models.begin()+loc, models.end());
    }

  std::cout << "Models file: " << sceneFilename << std::endl;
  std::cout << "Model Hierarchy ID: " << modelHierarchyID << std::endl;
    
  vtkMRMLScene *modelScene = NULL;

  // check for the model mrml file
  if (sceneFilename == "")
    {
    std::cout << "No file to store models!" << std::endl;
    return EXIT_FAILURE;
    }


  // get the directory of the scene file
  std::string rootDir
    = vtksys::SystemTools::GetParentDirectory( sceneFilename.c_str() );
  
  // make a scene
  modelScene = vtkMRMLScene::New();

  modelScene->SetURL(sceneFilename.c_str());
  modelScene->Import();

  // get the color node
  colorNode = vtkMRMLColorTableNode::New();
  modelScene->AddNode(colorNode);
  colorStorageNode = vtkMRMLColorTableStorageNode::New();
  colorStorageNode->SetFileName(ColorTable.c_str());
  modelScene->AddNode(colorStorageNode);
  colorNode->SetAndObserveStorageNodeID(colorStorageNode->GetID());
  if (!colorStorageNode->ReadData(colorNode))
    {
    std::cerr << "Error reading colour file " << colorStorageNode->GetFileName() << endl;
    return EXIT_FAILURE;
    }
  
  // make sure we have a model hierarchy node
  vtkMRMLNode *rnd = modelScene->GetNodeByID( modelHierarchyID );

  if (!rnd)
    {
    std::cerr << "Error: no model hierarchy node at ID \""
              << modelHierarchyID << "\"" << std::endl;
    return EXIT_FAILURE;
    }


  vtkMRMLModelHierarchyNode *rtnd = vtkMRMLModelHierarchyNode::SafeDownCast(rnd);
  
  vtkMRMLModelDisplayNode *dnd = vtkMRMLModelDisplayNode::New();
  dnd->SetColor(0.8, 0.8, 0.0);
  dnd->SetVisibility(1);
  modelScene->AddNode(dnd);
  rtnd->SetAndObserveDisplayNodeID( dnd->GetID() );
  dnd->Delete();

  // Add the models to the scene
  double size = 20.0;
  if (NumberOfModels > 0)
    {
    vtkSphereSource *sphere = vtkSphereSource::New();
    sphere->SetRadius( size );
    sphere->Update();

    vtkMRMLModelNode *mnd = vtkMRMLModelNode::New();
    mnd->SetScene( modelScene );
    mnd->SetName( "sphere" );
    mnd->SetAndObservePolyData( sphere->GetOutput() );
    mnd->SetModifiedSinceRead(1);
    modelScene->AddNode(mnd);
    
    vtkMRMLModelStorageNode *snd = vtkMRMLModelStorageNode::New();
    snd->SetFileName((rootDir + "/sphere.vtp").c_str());
    modelScene->AddNode(snd);
    
    vtkMRMLModelDisplayNode *dnd1 = vtkMRMLModelDisplayNode::New();
    dnd1->SetPolyData(mnd->GetPolyData());
    dnd1->SetColor(0.8, 0.0, 0.0);
    // over ride from the color node
    if (colorNode != NULL)
      {
      double *rgba = colorNode->GetLookupTable()->GetTableValue(1);
      if (rgba != NULL)
        {
        dnd1->SetColor(rgba[0], rgba[1], rgba[2]);
        }
      }
    dnd1->SetVisibility(1);
    modelScene->AddNode(dnd1);

    mnd->SetAndObserveStorageNodeID(snd->GetID());        
    mnd->SetAndObserveDisplayNodeID(dnd1->GetID());

    // force the storage node to write out its data
    snd->WriteData(mnd);
    
    vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::New();
    modelScene->AddNode(mhnd);

    mhnd->SetParentNodeID( rnd->GetID() );
    mhnd->SetModelNodeID( mnd->GetID() );

    // clean up
    dnd1->Delete();
    snd->Delete();
    mnd->Delete();
    mhnd->Delete();
    }
  if (NumberOfModels > 1)
    {
    vtkConeSource *cone = vtkConeSource::New();
    cone->SetCenter(-2*size, 0.0, 0.0);
    cone->SetRadius( size );
    cone->SetHeight( size );
    cone->Update();

    vtkMRMLModelNode *mnd = vtkMRMLModelNode::New();
    mnd->SetScene( modelScene );
    mnd->SetName( "cone" );
    mnd->SetAndObservePolyData( cone->GetOutput() );
    mnd->SetModifiedSinceRead(1);
    modelScene->AddNode(mnd);
    
    vtkMRMLModelStorageNode *snd = vtkMRMLModelStorageNode::New();
    snd->SetFileName((rootDir + "/cone.vtp").c_str());
    modelScene->AddNode(snd);

    vtkMRMLModelDisplayNode *dnd2 = vtkMRMLModelDisplayNode::New();
    dnd2->SetPolyData(mnd->GetPolyData());
    dnd2->SetColor(0.0, 0.8, 0.0);
    // over ride from the color node
    if (colorNode != NULL)
      {
      double *rgba = colorNode->GetLookupTable()->GetTableValue(2);
      if (rgba != NULL)
        {
        dnd2->SetColor(rgba[0], rgba[1], rgba[2]);
        }
      }
    dnd2->SetVisibility(1);
    modelScene->AddNode(dnd2);

    mnd->SetAndObserveStorageNodeID(snd->GetID());        
    mnd->SetAndObserveDisplayNodeID(dnd2->GetID());

    // force the storage node to write out its data
    snd->WriteData(mnd);
    
    vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::New();
    modelScene->AddNode(mhnd);

    mhnd->SetParentNodeID( rnd->GetID() );
    mhnd->SetModelNodeID( mnd->GetID() );
    
    // clean up
    dnd2->Delete();
    snd->Delete();
    mnd->Delete();
    mhnd->Delete();
    }
  if (NumberOfModels > 2)
    {
    vtkCubeSource *cube = vtkCubeSource::New();
    cube->SetCenter(2*size, 0.0, 0.0);
    cube->SetXLength( size );
    cube->SetYLength( size );
    cube->SetZLength( size );
    cube->Update();

    vtkMRMLModelNode *mnd = vtkMRMLModelNode::New();
    mnd->SetScene( modelScene );
    mnd->SetName( "cube" );
    mnd->SetAndObservePolyData( cube->GetOutput() );
    mnd->SetModifiedSinceRead(1);
    modelScene->AddNode(mnd);
    
    vtkMRMLModelStorageNode *snd = vtkMRMLModelStorageNode::New();
    snd->SetFileName((rootDir + "/cube.vtp").c_str());
    modelScene->AddNode(snd);

    vtkMRMLModelDisplayNode *dnd3 = vtkMRMLModelDisplayNode::New();
    dnd3->SetPolyData(mnd->GetPolyData());
    dnd3->SetColor(0.0, 0.0, 0.8);
    // over ride from the color node
    if (colorNode != NULL)
      {
      double *rgba = colorNode->GetLookupTable()->GetTableValue(3);
      if (rgba != NULL)
        {
        dnd3->SetColor(rgba[0], rgba[1], rgba[2]);
        }
      }
    dnd3->SetVisibility(1);
    modelScene->AddNode(dnd3);

    mnd->SetAndObserveStorageNodeID(snd->GetID());        
    mnd->SetAndObserveDisplayNodeID(dnd3->GetID());

    // force the storage node to write out its data
    snd->WriteData(mnd);
    
    vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::New();
    modelScene->AddNode(mhnd);

    mhnd->SetParentNodeID( rnd->GetID() );
    mhnd->SetModelNodeID( mnd->GetID() );

    // clean up
    dnd3->Delete();
    snd->Delete();
    mnd->Delete();
    mhnd->Delete();
    }
  
  modelScene->Commit();

  modelScene->Clear(1);
  modelScene->Delete();

  return EXIT_SUCCESS;
}

