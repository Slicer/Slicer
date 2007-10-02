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
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"

#include "vtkTubeFilter.h"

//#include "vtkPointData.h"


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


  this->DiffusionTensorGlyphFilter = vtkDiffusionTensorGlyph::New();
}

//----------------------------------------------------------------------------
vtkSlicerFiberBundleDisplayLogic::~vtkSlicerFiberBundleDisplayLogic()
{

  if ( this->FiberBundleNode ) 
    {
    // TO DO: Does this undo the observing of events?
    vtkSetAndObserveMRMLNodeMacro( this->FiberBundleNode, NULL );
    }


  this->DeleteLineModelNodes();
  this->DeleteTubeModelNodes();
  this->DeleteGlyphModelNodes();

  this->DiffusionTensorGlyphFilter->Delete ( );

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

  // Now that we have a fiber bundle node, display it.
  this->UpdateModelDisplay();
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateTemporaryModelNodeForDisplay ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode)

{
  
  //vtkDebugMacro("Creating temporary display model");

  modelNode = vtkMRMLModelNode::New();
  modelNode->SetScene(this->GetMRMLScene());
  modelNode->SetHideFromEditors(1);
  modelNode->SetSaveWithScene(0);
  
  // create display node
  displayNode = vtkMRMLModelDisplayNode::New();
  displayNode->SetScene(this->GetMRMLScene());
  displayNode->SetSaveWithScene(0);
  
  //modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  //vtkDebugMacro("Done creating temporary display model");
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::AddTemporaryModelNodeToScene ( vtkMRMLModelNode * & modelNode, vtkMRMLModelDisplayNode * & displayNode)
{
  //vtkDebugMacro("Adding temporary display model to scene");

  // check that the model and display node exist, and that the model
  // is not already in the scene.
  if (modelNode != NULL && displayNode != NULL && this->MRMLScene->GetNodeByID( modelNode->GetID() ) == NULL )
    {
    this->MRMLScene->AddNode(displayNode);
    this->MRMLScene->AddNode(modelNode);
    modelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    }

  //vtkDebugMacro("Done adding temporary display model to scene");
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

  //vtkDebugMacro("Done deleting temporary display model");

} 

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::VisibilityOffForTemporaryModel ( vtkMRMLModelNode * modelNode,  vtkMRMLModelDisplayNode * displayNode)
{
  vtkDebugMacro("Turning off model visibility");

  if ( displayNode != NULL )
    {
    displayNode->VisibilityOff();
    }

}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::UpdateModelDisplay ( )
{

  // if we have a valid fiber bundle node
  // TO DO: this is all supposed to use IDs instead?
  if ( this->FiberBundleNode != NULL ) 
    {

    // if we are supposed to be displaying something do it
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = vtkMRMLFiberBundleDisplayNode::SafeDownCast(this->FiberBundleNode->GetDisplayNode());
    if ( fiberBundleDisplayNode != NULL )
      {

      // see which kind of model we are displaying
      if (fiberBundleDisplayNode->GetVisibility() == 1)
        {
        this->CreateLineModel();
        }
      else
        {
        this->VisibilityOffForTemporaryModel( this->LineModelNode, this->LineModelDisplayNode );
        }

      // see which kind of model we are displaying
      if (fiberBundleDisplayNode->GetVisibility() == 1)
        {
        this->CreateTubeModel();
        }
      else
        {
        this->VisibilityOffForTemporaryModel( this->TubeModelNode, this->TubeModelDisplayNode );
        }

      // see which kind of model we are displaying
      if (fiberBundleDisplayNode->GetVisibility() == 1)
        {
        this->CreateGlyphModel();
        }
      else
        {
        this->VisibilityOffForTemporaryModel( this->GlyphModelNode, this->GlyphModelDisplayNode );
        }


      }

    }

}


//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::DeleteLineModelNodes ( )
{
  this->DeleteTemporaryModelNodeForDisplay ( this->LineModelNode, this->LineModelDisplayNode );
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateLineModelNodes ( )
{
    this->CreateTemporaryModelNodeForDisplay ( this->LineModelNode, this->LineModelDisplayNode );
  
    // give the nodes names
    std::stringstream ss;
    char name[256];
    ss << this->FiberBundleNode->GetName() << " Line Model Node";
    ss.getline(name,256);
    this->LineModelNode->SetName(name);

    std::stringstream ss2;
    char name2[256];
    ss2 << this->FiberBundleNode->GetName() << " Line Model Display Node";
    ss2.getline(name2,256);
    this->LineModelDisplayNode->SetName(name);
  
}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateLineModel ( )
{
  //vtkDebugMacro("Creating line model");

  // TO DO: don't pass tensors through, just any requested scalars?

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

  //vtkDebugMacro("Updating line model according to fiber bundle nodes");

  // update the polydata and display parameters:
  if (this->FiberBundleNode != NULL) 
    {

    //vtkDebugMacro("Getting poly data from FB node");

    // get polylines from the fiber bundle node
    this->LineModelNode->SetAndObservePolyData(this->FiberBundleNode->GetPolyData());

    // update the polydata and display parameters:
    // set properties according to the fiber bundle's display node
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = vtkMRMLFiberBundleDisplayNode::SafeDownCast(this->FiberBundleNode->GetDisplayNode());
    if (fiberBundleDisplayNode != NULL)
      {
      this->LineModelDisplayNode->SetPolyData(this->LineModelNode->GetPolyData());
      //vtkDebugMacro("Updating line model according to FB display node");

      this->LineModelDisplayNode->SetVisibility( fiberBundleDisplayNode->GetVisibility ( ) );
      this->LineModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetOpacity ( ) );
      this->LineModelDisplayNode->SetColor( fiberBundleDisplayNode->GetColor ( ) );
      this->LineModelDisplayNode->SetAmbient( fiberBundleDisplayNode->GetAmbient ( ) );
      this->LineModelDisplayNode->SetDiffuse( fiberBundleDisplayNode->GetDiffuse ( ) );
      this->LineModelDisplayNode->SetClipping( fiberBundleDisplayNode->GetClipping ( ) );

      //vtkDebugMacro("Updating line model according to DT display node");

      //this->LineModelDisplayNode->GetColorModeForFiberLines();
      // set display properties according to the tensor-specific display properties node
      vtkMRMLDiffusionTensorDisplayPropertiesNode * DTDisplayNode = fiberBundleDisplayNode->GetDTDisplayPropertiesNode( );

      // TO DO: need filter to calculate FA, average FA, etc. as requested

      }


    }

  this->AddTemporaryModelNodeToScene ( this->LineModelNode, this->LineModelDisplayNode );

} 


void vtkSlicerFiberBundleDisplayLogic::DeleteTubeModelNodes ( )
{
  this->DeleteTemporaryModelNodeForDisplay ( this->TubeModelNode, this->TubeModelDisplayNode );
}

void vtkSlicerFiberBundleDisplayLogic::CreateTubeModelNodes ( )
{
    this->CreateTemporaryModelNodeForDisplay ( this->TubeModelNode, this->TubeModelDisplayNode );
  
    // give the nodes names
    std::stringstream ss;
    char name[256];
    ss << this->FiberBundleNode->GetName() << " Tube Model Node";
    ss.getline(name,256);
    this->TubeModelNode->SetName(name);

    std::stringstream ss2;
    char name2[256];
    ss2 << this->FiberBundleNode->GetName() << " Tube Model Display Node";
    ss2.getline(name2,256);
    this->TubeModelDisplayNode->SetName(name);

}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateTubeModel ( )
{
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

  // update the polydata and display parameters:
  if (this->FiberBundleNode != NULL) 
    {

    // update the polydata and display parameters:
    // set properties according to the fiber bundle's display node
    vtkMRMLFiberBundleTubeDisplayNode * fiberBundleDisplayNode = vtkMRMLFiberBundleTubeDisplayNode::SafeDownCast(this->FiberBundleNode->GetDisplayNode());
    if (fiberBundleDisplayNode != NULL)
      {


      this->TubeModelDisplayNode->SetVisibility( fiberBundleDisplayNode->GetVisibility ( ) );
      this->TubeModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetOpacity ( ) );
      this->TubeModelDisplayNode->SetColor( fiberBundleDisplayNode->GetColor ( ) );
      this->TubeModelDisplayNode->SetAmbient( fiberBundleDisplayNode->GetAmbient ( ) );
      this->TubeModelDisplayNode->SetDiffuse( fiberBundleDisplayNode->GetDiffuse ( ) );
      this->TubeModelDisplayNode->SetClipping( fiberBundleDisplayNode->GetClipping ( ) );


      // get polylines from the fiber bundle node and tube them
      vtkTubeFilter *tubeFilter = vtkTubeFilter::New();
      tubeFilter->SetInput(this->FiberBundleNode->GetPolyData () );
      tubeFilter->SetRadius(fiberBundleDisplayNode->GetTubeRadius ( ) );
      tubeFilter->SetNumberOfSides(fiberBundleDisplayNode->GetTubeNumberOfSides ( ) );
      tubeFilter->Update ( );
      this->TubeModelNode->SetAndObservePolyData(tubeFilter->GetOutput( ) );
      tubeFilter->Delete ( );
      this->TubeModelDisplayNode->SetPolyData(this->TubeModelNode->GetPolyData());
      //this->TubeModelDisplayNode->GetColorModeForFiberTubes();
      // set display properties according to the tensor-specific display properties node
      vtkMRMLDiffusionTensorDisplayPropertiesNode * DTDisplayNode = fiberBundleDisplayNode->GetDTDisplayPropertiesNode( );

      if (DTDisplayNode != NULL)
        {
        // TO DO: need filter to calculate FA, average FA, etc. as requested
        }

      }


    }


  // if the modelNode exists but is not in the scene, put it there
  if (this->TubeModelNode != NULL && this->MRMLScene->GetNodeByID( this->TubeModelNode->GetID() ) == NULL )
    {
    this->MRMLScene->AddNode(this->TubeModelDisplayNode);
    this->MRMLScene->AddNode(this->TubeModelNode);
    this->TubeModelNode->SetAndObserveDisplayNodeID(this->TubeModelDisplayNode->GetID());
    }


} 



void vtkSlicerFiberBundleDisplayLogic::DeleteGlyphModelNodes ( )
{
  this->DeleteTemporaryModelNodeForDisplay ( this->GlyphModelNode, this->GlyphModelDisplayNode );
}

void vtkSlicerFiberBundleDisplayLogic::CreateGlyphModelNodes ( )
{
    this->CreateTemporaryModelNodeForDisplay ( this->GlyphModelNode, this->GlyphModelDisplayNode );

    // give the nodes names
    std::stringstream ss;
    char name[256];
    ss << this->FiberBundleNode->GetName() << " Glyph Model Node";
    ss.getline(name,256);
    this->GlyphModelNode->SetName(name);

    std::stringstream ss2;
    char name2[256];
    ss2 << this->FiberBundleNode->GetName() << " Glyph Model Display Node";
    ss2.getline(name2,256);
    this->GlyphModelDisplayNode->SetName(name);

}

//----------------------------------------------------------------------------
void vtkSlicerFiberBundleDisplayLogic::CreateGlyphModel ( )
{

  // if we have no MRML scene yet 
  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("Cannot create glyph model, no MRMLScene set yet.");
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


  // update the polydata and display parameters:
  if (this->FiberBundleNode != NULL) 
    {

    // update the polydata and display parameters:
    // set properties according to the fiber bundle's display node
    vtkMRMLFiberBundleDisplayNode * fiberBundleDisplayNode = vtkMRMLFiberBundleDisplayNode::SafeDownCast(this->FiberBundleNode->GetDisplayNode());
    if (fiberBundleDisplayNode != NULL)
      {

      this->GlyphModelDisplayNode->SetVisibility( fiberBundleDisplayNode->GetVisibility ( ) );
      // TO DO: opacity for all glyph tube line separately
      //this->GlyphModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetFiberGlyphOpacity ( ) );
      this->GlyphModelDisplayNode->SetOpacity( fiberBundleDisplayNode->GetOpacity ( ) );
      this->GlyphModelDisplayNode->SetColor( fiberBundleDisplayNode->GetColor ( ) );
      this->GlyphModelDisplayNode->SetAmbient( fiberBundleDisplayNode->GetAmbient ( ) );
      this->GlyphModelDisplayNode->SetDiffuse( fiberBundleDisplayNode->GetDiffuse ( ) );
      this->GlyphModelDisplayNode->SetClipping( fiberBundleDisplayNode->GetClipping ( ) );

      this->GlyphModelDisplayNode->SetAndObserveColorNodeID( fiberBundleDisplayNode->GetColorNodeID ( ) );

      // set display properties according to the tensor-specific display properties node for glyphs
      vtkMRMLDiffusionTensorDisplayPropertiesNode * DTDisplayNode = fiberBundleDisplayNode->GetDTDisplayPropertiesNode( );

      if (DTDisplayNode != NULL)
        {
        // TO DO: need filter to calculate FA, average FA, etc. as requested


        // get tensors from the fiber bundle node and glyph them
        // TO DO: include superquadrics
        // if glyph type is other than superquadrics, get glyph source
        if (DTDisplayNode->GetGlyphGeometry( ) != vtkMRMLDiffusionTensorDisplayPropertiesNode::Superquadrics)
          {

          this->DiffusionTensorGlyphFilter->SetInput(this->FiberBundleNode->GetPolyData () );
          this->DiffusionTensorGlyphFilter->ClampScalingOff();

          // TO DO: implement max # ellipsoids, random sampling features
          this->DiffusionTensorGlyphFilter->SetResolution(2);
        
          this->DiffusionTensorGlyphFilter->SetScaleFactor( DTDisplayNode->GetGlyphScaleFactor( ) );

          this->DiffusionTensorGlyphFilter->SetSource( DTDisplayNode->GetGlyphSource( ) );

          vtkErrorMacro("setting glyph geometry" << DTDisplayNode->GetGlyphGeometry( ) );

          // set glyph coloring
          if (fiberBundleDisplayNode->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeSolid)
            {
            this->GlyphModelDisplayNode->ScalarVisibilityOff( );
            }
          else
            {
            if (fiberBundleDisplayNode->GetColorMode ( ) == vtkMRMLFiberBundleDisplayNode::colorModeScalar)
              {

              this->GlyphModelDisplayNode->ScalarVisibilityOn( );

              switch ( DTDisplayNode->GetColorGlyphBy( ))
                {
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::FractionalAnisotropy:
                  {
                  vtkErrorMacro("coloring with FA==============================");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByFractionalAnisotropy( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::LinearMeasure:
                  {
                  vtkErrorMacro("coloring with Cl=============================");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByLinearMeasure( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::Trace:
                  {
                  vtkErrorMacro("coloring with trace =================");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByTrace( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::ColorOrientation:
                  {
                  vtkErrorMacro("coloring with direction (re-implement)");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByOrientation( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::PlanarMeasure:
                  {
                  vtkErrorMacro("coloring with planar");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByPlanarMeasure( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::MaxEigenvalue:
                  {
                  vtkErrorMacro("coloring with max eigenval");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByMaxEigenvalue( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::MidEigenvalue:
                  {
                  vtkErrorMacro("coloring with mid eigenval");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByMidEigenvalue( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::MinEigenvalue:
                  {
                  vtkErrorMacro("coloring with min eigenval");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByMinEigenvalue( );
                  }
                  break;
                case vtkMRMLDiffusionTensorDisplayPropertiesNode::RelativeAnisotropy:
                  {
                  vtkErrorMacro("coloring with relative anisotropy");
                  this->DiffusionTensorGlyphFilter->ColorGlyphsByRelativeAnisotropy( );
                  }
                  break;
                  
                }
              }  // if color scalar
            }   // end else


          // this update is needed to get the scalar range correct
          this->DiffusionTensorGlyphFilter->Update ( );
          this->GlyphModelDisplayNode->SetScalarRange( this->DiffusionTensorGlyphFilter->GetOutput()->GetScalarRange() );

          //vtkErrorMacro("TEST NULL PD_________________-------------------___________");
          //this->GlyphModelNode->SetAndObservePolyData( NULL );
          vtkErrorMacro("set and observe PD _________________-------------------___________");
          // this did not change the polydata originally
          this->GlyphModelNode->SetAndObservePolyData( this->DiffusionTensorGlyphFilter->GetOutput( ) );
          this->GlyphModelDisplayNode->SetPolyData(this->GlyphModelNode->GetPolyData());
          vtkErrorMacro("DONE set and observe PD _________________-------------------___________");
          
          // try sending modified event to see if pd updates
          //this->DiffusionTensorGlyphFilter->GetOutput()->Modified();

          }
        else
          {
          // TO DO:
          // Do superquadrics

          }

        } // end if dt display props node not null


      } // end if fb display node not null

    } // end if fb node not null


  // if the modelNode exists but is not in the scene, put it there
  if (this->GlyphModelNode != NULL && this->MRMLScene->GetNodeByID( this->GlyphModelNode->GetID() ) == NULL )
    {
    this->MRMLScene->AddNode(this->GlyphModelDisplayNode);
    this->MRMLScene->AddNode(this->GlyphModelNode);
    this->GlyphModelNode->SetAndObserveDisplayNodeID(this->GlyphModelDisplayNode->GetID());
    }

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
    vtkErrorWithObjectMacro(this,"Done creating models!!!!");
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


