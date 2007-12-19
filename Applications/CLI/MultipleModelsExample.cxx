/*=auto=========================================================================

Portions (c) Copyright 2006 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile$
Date:      $Date: 2007-12-11 18:48:55 -0500 (Tue, 11 Dec 2007) $
Version:   $Revision: 5168 $

=========================================================================auto=*/

#include "MultipleModelsExampleCLP.h"

#include "vtkPluginFilterWatcher.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLColorTableNode.h"
#include "vtkMRMLModelHierarchyNode.h"

#include "vtkConeSource.h"
#include "vtkCubeSource.h"
#include "vtkSphereSource.h"
#include "vtkPolyData.h"

#include <fstream>
#include <string>
#include <map>


int main(int argc, char * argv[])
{
  PARSE_ARGS;

  std::cout << "Label image file: " << InputVolume << std::endl;
  std::cout << "Color table file: " << ColorTable << std::endl;
  std::cout << "Number of models: " << NumberOfModels << std::endl;

  // put in code to read the color table file
  //
  
  
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
    
    vtkMRMLModelDisplayNode *dnd = vtkMRMLModelDisplayNode::New();
    dnd->SetPolyData(mnd->GetPolyData());
    dnd->SetColor(0.8, 0.0, 0.0);
    dnd->SetVisibility(1);
    modelScene->AddNode(dnd);

    mnd->SetStorageNodeID(snd->GetID());        
    mnd->SetAndObserveDisplayNodeID(dnd->GetID());

    // force the storage node to write out its data
    snd->WriteData(mnd);
    
    vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::New();
    modelScene->AddNode(mhnd);

    mhnd->SetParentNodeID( rnd->GetID() );
    mhnd->SetModelNodeID( mnd->GetID() );

    // clean up
    dnd->Delete();
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

    vtkMRMLModelDisplayNode *dnd = vtkMRMLModelDisplayNode::New();
    dnd->SetPolyData(mnd->GetPolyData());
    dnd->SetColor(0.0, 0.8, 0.0);
    dnd->SetVisibility(1);
    modelScene->AddNode(dnd);

    mnd->SetStorageNodeID(snd->GetID());        
    mnd->SetAndObserveDisplayNodeID(dnd->GetID());

    // force the storage node to write out its data
    snd->WriteData(mnd);
    
    vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::New();
    modelScene->AddNode(mhnd);

    mhnd->SetParentNodeID( rnd->GetID() );
    mhnd->SetModelNodeID( mnd->GetID() );
    
    // clean up
    dnd->Delete();
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

    vtkMRMLModelDisplayNode *dnd = vtkMRMLModelDisplayNode::New();
    dnd->SetPolyData(mnd->GetPolyData());
    dnd->SetColor(0.0, 0.0, 0.8);
    dnd->SetVisibility(1);
    modelScene->AddNode(dnd);

    mnd->SetStorageNodeID(snd->GetID());        
    mnd->SetAndObserveDisplayNodeID(dnd->GetID());

    // force the storage node to write out its data
    snd->WriteData(mnd);
    
    vtkMRMLModelHierarchyNode *mhnd = vtkMRMLModelHierarchyNode::New();
    modelScene->AddNode(mhnd);

    mhnd->SetParentNodeID( rnd->GetID() );
    mhnd->SetModelNodeID( mnd->GetID() );

    // clean up
    dnd->Delete();
    snd->Delete();
    mnd->Delete();
    mhnd->Delete();
    }
  
  modelScene->Commit();

  modelScene->Clear(1);
  modelScene->Delete();

  return EXIT_SUCCESS;
}

