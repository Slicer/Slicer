/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerFiberBundleDisplayLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <itksys/SystemTools.hxx> 
#include <itksys/Directory.hxx> 

#include "vtkSlicerFiberBundleDisplayLogic.h"

#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleDisplayNode.h"

#include "vtkTubeFilter.h"
#include "vtkDiffusionTensorGlyph.h"

#include <sstream>

vtkCxxRevisionMacro(vtkSlicerFiberBundleDisplayLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerFiberBundleDisplayLogic);

//----------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayLogic::vtkSlicerFiberBundleDisplayLogic()
{
  this->LineModelNode = NULL;
  this->TubeModelNode = NULL;
  this->GlyphModelNode = NULL;

  this->LineModelDisplayNode = NULL;
  this->TubeModelDisplayNode = NULL;
  this->GlyphModelDisplayNode = NULL;

  this->FiberBundleNode = NULL;

}

//----------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayLogic::~vtkSlicerFiberBundleDisplayLogic()
{

  if ( this->FiberBundleNode ) 
    {
    // TO DO: Does this undo the observing of events?
    vtkSetAndObserveMRMLNodeMacro( this->FiberBundleNode, NULL );
    }

  this->DeleteTemporaryModelNodeForDisplay ( this->LineModelNode, this->LineModelDisplayNode );
}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::SetAndObserveFiberBundleNode( vtkMRMLFiberBundleNode *fiberBundleNode )
{
  vtkDebugMacro("Setting FiberBundle node in display logic");

  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLFiberBundleNode::DisplayModifiedEvent);
  events->InsertNextValue(vtkMRMLFiberBundleNode::PolyDataModifiedEvent);
  vtkSetAndObserveMRMLNodeEventsMacro(this->FiberBundleNode, fiberBundleNode, events );
  events->Delete();

  vtkDebugMacro("Creating display model(s)");
  // Now that we have a fiber bundle node, display it.
  this->UpdateModelDisplay();
  vtkDebugMacro("Done creating display model(s)");
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateTemporaryModelNodeForDisplay ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode)

{
  
  vtkDebugMacro("Creating temporary display model");

  modelNode = vtkMRMLModelNode::New();
  modelNode->SetScene(this->GetMRMLScene());
  modelNode->SetHideFromEditors(1);
  modelNode->SetSaveWithScene(0);
  
  // create display node
  displayNode = vtkMRMLModelDisplayNode::New();
  displayNode->SetScene(this->GetMRMLScene());
  displayNode->SetSaveWithScene(0);
  
  // give it a name
  std::stringstream ss;
  char name[256];
  ss << this->FiberBundleNode->GetName() << " " << "Line Model Node";
  ss.getline(name,256);
  modelNode->SetName(name);
  
  
  //modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  vtkDebugMacro("Done creating temporary display model");
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::AddTemporaryModelNodeToScene ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode)
{
  vtkDebugMacro("Adding temporary display model to scene");

  // check that the model and display node exist, and that the model
  // is not already in the scene.
  if (modelNode != NULL && displayNode != NULL && this->MRMLScene->GetNodeByID( modelNode->GetID() ) == NULL )
    {
    this->MRMLScene->AddNode(displayNode);
    this->MRMLScene->AddNode(modelNode);
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    }

  vtkDebugMacro("Done adding temporary display model to scene");
}


// TO DO: How to remove from the scene?
//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::DeleteTemporaryModelNodeForDisplay ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode)
{
  vtkDebugMacro("Deleting temporary display model");
  
  if (modelNode != NULL)
    {
    modelNode->SetAndObserveDisplayNodeID(NULL);
    modelNode->SetAndObservePolyData(NULL);
    modelNode->Delete();
    modelNode = NULL;
    }
  if (displayNode != NULL)
    {
    displayNode->Delete();
    displayNode = NULL;
    }

  vtkDebugMacro("Done deleting temporary display model");

} 

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::UpdateModelDisplay ( )
{

  // if we have a valid fiber bundle node
  // TO DO: this is all supposed to use IDs instead?
  if ( this->FiberBundleNode != NULL ) 
    {

    // if we are supposed to be displaying something do it
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = this->FiberBundleNode->GetDisplayNode();
    if ( fiberBundleDisplayNode != NULL )
      {

      // see which kind of model we are displaying
      if (fiberBundleDisplayNode->GetFiberLineVisibility() == 1)
        {
        this->CreateLineModel();
        }
      else
        {
        this->DeleteLineModelNodes();
        }

      // see which kind of model we are displaying
      if (fiberBundleDisplayNode->GetFiberTubeVisibility() == 1)
        {
        this->CreateTubeModel();
        }
      else
        {
        this->DeleteTubeModelNodes();
        }

      // see which kind of model we are displaying
      if (fiberBundleDisplayNode->GetFiberGlyphVisibility() == 1)
        {
        this->CreateGlyphModel();
        }
      else
        {
        this->DeleteGlyphModelNodes();
        }


      }

    }

}


void vtkSlicerFiberBundleDisplayLogic::DeleteLineModelNodes ( )
{
  this->DeleteTemporaryModelNodeForDisplay ( this->LineModelNode, this->LineModelDisplayNode );
}

void vtkSlicerFiberBundleDisplayLogic::CreateLineModelNodes ( )
{
    this->CreateTemporaryModelNodeForDisplay ( this->LineModelNode, this->LineModelDisplayNode );
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateLineModel ( )
{
  vtkDebugMacro("Creating line model");

  // if we have no MRML scene yet 
  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("Cannot create line model, no MRMLScene set yet.");
    return;
    }

  // if the modelNode exists and is not in the scene, delete it (and then redo it)
  if (this->LineModelNode != NULL && this->MRMLScene->GetNodeByID( this->LineModelNode->GetID() ) == NULL )
    {
    this->DeleteLineModelNodes();
    }

  // if the modelNode does not exist set it up again totally
  if ( this->LineModelNode == NULL) 
    {
    this->CreateLineModelNodes();
    }

  vtkDebugMacro("Updating line model according to fiber bundle nodes");

  // update the polydata and display parameters:
  if (this->FiberBundleNode != NULL) 
    {

    vtkDebugMacro("Getting poly data from FB node");

    // get polylines from the fiber bundle node
    this->LineModelNode->SetAndObservePolyData(this->FiberBundleNode->GetPolyData());

    vtkDebugMacro("Done getting poly data from FB node");

    // update the polydata and display parameters:
    // set properties according to the fiber bundle's display node
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = this->FiberBundleNode->GetDisplayNode();
    if (fiberBundleDisplayNode != NULL)
      {

      vtkDebugMacro("Updating line model according to FB display node");

      this->LineModelDisplayNode->SetVisibility( fiberBundleDisplayNode->GetVisibility ( ) );
      this->LineModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetFiberLineOpacity ( ) );
      this->LineModelDisplayNode->SetColor( fiberBundleDisplayNode->GetColor ( ) );
      this->LineModelDisplayNode->SetAmbient( fiberBundleDisplayNode->GetAmbient ( ) );
      this->LineModelDisplayNode->SetDiffuse( fiberBundleDisplayNode->GetDiffuse ( ) );

      vtkDebugMacro("Updating line model according to DT display node");

      //this->LineModelDisplayNode->GetColorModeForFiberLines();
      // set display properties according to the tensor-specific display properties node
      vtkMRMLDiffusionTensorDisplayPropertiesNode * DTDisplayNode = fiberBundleDisplayNode->GetFiberLineDTDisplayPropertiesNode( );

      // TO DO: need filter to calculate FA, average FA, etc. as requested

      }


    }

  vtkDebugMacro("Adding model to scene");

  this->AddTemporaryModelNodeToScene ( this->LineModelNode, this->LineModelDisplayNode );

  vtkDebugMacro("Done creating line model");

} 


void vtkSlicerFiberBundleDisplayLogic::DeleteTubeModelNodes ( )
{
  this->DeleteTemporaryModelNodeForDisplay ( this->TubeModelNode, this->TubeModelDisplayNode );
}

void vtkSlicerFiberBundleDisplayLogic::CreateTubeModelNodes ( )
{
    this->CreateTemporaryModelNodeForDisplay ( this->TubeModelNode, this->TubeModelDisplayNode );
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateTubeModel ( )
{
  vtkDebugMacro("Creating line model");

  // if we have no MRML scene yet 
  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("Cannot create line model, no MRMLScene set yet.");
    return;
    }

  // if the modelNode exists and is not in the scene, delete it and then redo it
  if (this->TubeModelNode != NULL && this->MRMLScene->GetNodeByID( this->TubeModelNode->GetID() ) == NULL )
    {
    this->DeleteTubeModelNodes();
    }

  // if the modelNode does not exist set it up again totally
  if ( this->TubeModelNode == NULL) 
    {
    this->CreateTubeModelNodes();
    }

  vtkDebugMacro("Updating line model according to fiber bundle nodes");

  // update the polydata and display parameters:
  if (this->FiberBundleNode != NULL) 
    {

    // update the polydata and display parameters:
    // set properties according to the fiber bundle's display node
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = this->FiberBundleNode->GetDisplayNode();
    if (fiberBundleDisplayNode != NULL)
      {

      vtkDebugMacro("Updating line model according to FB display node");

      this->TubeModelDisplayNode->SetVisibility( fiberBundleDisplayNode->GetVisibility ( ) );
      this->TubeModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetFiberTubeOpacity ( ) );
      this->TubeModelDisplayNode->SetColor( fiberBundleDisplayNode->GetColor ( ) );
      this->TubeModelDisplayNode->SetAmbient( fiberBundleDisplayNode->GetAmbient ( ) );
      this->TubeModelDisplayNode->SetDiffuse( fiberBundleDisplayNode->GetDiffuse ( ) );

      vtkDebugMacro("Updating line model according to DT display node");

      vtkDebugMacro("Getting poly data from FB node");
      
      // get polylines from the fiber bundle node and tube them
      vtkTubeFilter *tubeFilter = vtkTubeFilter::New();
      tubeFilter->SetInput(this->FiberBundleNode->GetPolyData () );
      tubeFilter->SetRadius(fiberBundleDisplayNode->GetFiberTubeRadius ( ) );
      tubeFilter->SetNumberOfSides(fiberBundleDisplayNode->GetFiberTubeNumberOfSides ( ) );
      tubeFilter->Update ( );
      this->TubeModelNode->SetAndObservePolyData(tubeFilter->GetOutput( ) );
      tubeFilter->Delete ( );
      vtkDebugMacro("Done getting poly data from FB node");


      //this->TubeModelDisplayNode->GetColorModeForFiberTubes();
      // set display properties according to the tensor-specific display properties node
      vtkMRMLDiffusionTensorDisplayPropertiesNode * DTDisplayNode = fiberBundleDisplayNode->GetFiberTubeDTDisplayPropertiesNode( );

      if (DTDisplayNode != NULL)
        {
        // TO DO: need filter to calculate FA, average FA, etc. as requested
        }

      }


    }

  vtkDebugMacro("Adding model to scene");

  // if the modelNode exists but is not in the scene, put it there
  if (this->TubeModelNode != NULL && this->MRMLScene->GetNodeByID( this->TubeModelNode->GetID() ) == NULL )
    {
    this->MRMLScene->AddNode(this->TubeModelDisplayNode);
    this->MRMLScene->AddNode(this->TubeModelNode);
    this->TubeModelNode->SetAndObserveDisplayNodeID(this->TubeModelDisplayNode->GetID());
    }

  vtkDebugMacro("Done creating line model");

} 



void vtkSlicerFiberBundleDisplayLogic::DeleteGlyphModelNodes ( )
{
  this->DeleteTemporaryModelNodeForDisplay ( this->GlyphModelNode, this->GlyphModelDisplayNode );
}

void vtkSlicerFiberBundleDisplayLogic::CreateGlyphModelNodes ( )
{
    this->CreateTemporaryModelNodeForDisplay ( this->GlyphModelNode, this->GlyphModelDisplayNode );
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateGlyphModel ( )
{
  vtkDebugMacro("Creating line model");

  // if we have no MRML scene yet 
  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("Cannot create line model, no MRMLScene set yet.");
    return;
    }

  // if the modelNode exists and is not in the scene, delete it and then redo it
  if (this->GlyphModelNode != NULL && this->MRMLScene->GetNodeByID( this->GlyphModelNode->GetID() ) == NULL )
    {
    this->DeleteGlyphModelNodes();
    }

  // if the modelNode does not exist set it up again totally
  if ( this->GlyphModelNode == NULL) 
    {
    this->CreateGlyphModelNodes();
    }

  vtkDebugMacro("Updating line model according to fiber bundle nodes");

  // update the polydata and display parameters:
  if (this->FiberBundleNode != NULL) 
    {

    // update the polydata and display parameters:
    // set properties according to the fiber bundle's display node
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = this->FiberBundleNode->GetDisplayNode();
    if (fiberBundleDisplayNode != NULL)
      {

      vtkDebugMacro("Updating line model according to FB display node");

      this->GlyphModelDisplayNode->SetVisibility( fiberBundleDisplayNode->GetVisibility ( ) );
      this->GlyphModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetFiberGlyphOpacity ( ) );
      this->GlyphModelDisplayNode->SetColor( fiberBundleDisplayNode->GetColor ( ) );
      this->GlyphModelDisplayNode->SetAmbient( fiberBundleDisplayNode->GetAmbient ( ) );
      this->GlyphModelDisplayNode->SetDiffuse( fiberBundleDisplayNode->GetDiffuse ( ) );

      vtkDebugMacro("Updating line model according to DT display node");

      vtkDebugMacro("Getting poly data from FB node");
      
      // get polylines from the fiber bundle node and glyph them
      vtkDiffusionTensorGlyph *glyphFilter = vtkDiffusionTensorGlyph::New();
      glyphFilter->SetInput(this->FiberBundleNode->GetPolyData () );

      glyphFilter->Update ( );
      this->GlyphModelNode->SetAndObservePolyData(glyphFilter->GetOutput( ) );
      glyphFilter->Delete ( );

      vtkDebugMacro("Done getting poly data from FB node");


      //this->GlyphModelDisplayNode->GetColorModeForFiberGlyphs();
      // set display properties according to the tensor-specific display properties node
      vtkMRMLDiffusionTensorDisplayPropertiesNode * DTDisplayNode = fiberBundleDisplayNode->GetFiberGlyphDTDisplayPropertiesNode( );

      if (DTDisplayNode != NULL)
        {
        // TO DO: need filter to calculate FA, average FA, etc. as requested
        }

      }


    }

  vtkDebugMacro("Adding model to scene");

  // if the modelNode exists but is not in the scene, put it there
  if (this->GlyphModelNode != NULL && this->MRMLScene->GetNodeByID( this->GlyphModelNode->GetID() ) == NULL )
    {
    this->MRMLScene->AddNode(this->GlyphModelDisplayNode);
    this->MRMLScene->AddNode(this->GlyphModelNode);
    this->GlyphModelNode->SetAndObserveDisplayNodeID(this->GlyphModelDisplayNode->GetID());
    }

  vtkDebugMacro("Done creating line model");

} 


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::ProcessMRMLEvents( vtkObject * caller, 
                                            unsigned long event, 
                                            void * callData )
{


  // if we have no MRML scene yet 
  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("Cannot process MRML Events, no MRMLScene set yet.");
    return;
    }

  if (vtkMRMLFiberBundleNode::SafeDownCast(caller) != NULL
      && (event == vtkMRMLFiberBundleNode::DisplayModifiedEvent || event == vtkMRMLFiberBundleNode::PolyDataModifiedEvent))
    {
    
    vtkDebugWithObjectMacro(this,"Hooray caught a display modified event");
    vtkErrorWithObjectMacro(this,"Hooray caught a display modified event from fiber bundle node");
    this->UpdateModelDisplay();
    vtkErrorWithObjectMacro(this,"Done creating line model!!!!");
    }

  vtkDebugWithObjectMacro(this,"Process MRML Events " << event );
  vtkErrorWithObjectMacro(this,"Process MRML Events " << event );
}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerFiberBundleDisplayLogic:             " << this->GetClassName() << "\n";

}


