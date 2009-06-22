#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>

#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkDoubleArray.h"
#include "vtkAdjacentVertexIterator.h"
#include "vtkTree.h"

#include "vtkMRMLScene.h"
#include "vtkCustomLayoutLogic.h"
#include "vtkCustomLayout.h"


vtkCustomLayoutLogic* vtkCustomLayoutLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCustomLayoutLogic");
  if(ret)
    {
      return (vtkCustomLayoutLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCustomLayoutLogic;
}



//----------------------------------------------------------------------------
vtkCustomLayoutLogic::vtkCustomLayoutLogic()
{
   
   if ( this->CustomLayoutNode != NULL )
     {
     }
   this->Visited = false;
   this->Raised = false;
}



//----------------------------------------------------------------------------
vtkCustomLayoutLogic::~vtkCustomLayoutLogic()
{ 
  this->SetCustomLayoutNode(NULL);
  this->SetAndObserveMRMLScene ( NULL );
  this->Visited = false;  
  this->Raised = false;
}


//----------------------------------------------------------------------------
void vtkCustomLayoutLogic::SetCustomLayoutNode ( vtkMRMLCustomLayoutNode *node)
{
  vtkSetMRMLNodeMacro ( this->CustomLayoutNode, node );
}




//----------------------------------------------------------------------------
void vtkCustomLayoutLogic::Enter()
{
  this->Visited = true;
  this->Raised = true;
  //---
  //--- Set up Logic observers on enter, and released on exit.
  vtkIntArray *logicEvents = this->NewObservableEvents();
  if ( logicEvents != NULL )
    {
    this->SetAndObserveMRMLSceneEvents ( this->MRMLScene, logicEvents );
    logicEvents->Delete();
    }

}


//----------------------------------------------------------------------------
void vtkCustomLayoutLogic::Exit()
{
  this->Raised = false;
}



//----------------------------------------------------------------------------
vtkIntArray* vtkCustomLayoutLogic::NewObservableEvents()
{

  if ( !this->Visited )
    {
    return (NULL);
    }

 vtkIntArray *events = vtkIntArray::New();
 events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  // Slicer3.cxx calls delete on events
  return events;
}




//----------------------------------------------------------------------------
void vtkCustomLayoutLogic::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{

  if ( !this->Visited )
    {
    return;
    }
  if ( this->CustomLayoutNode == NULL )
    {
    vtkErrorMacro ( "CustomLayoutLogic::ProcessMRMLEvents: got null CustomLayoutNode." );
    return;
    }

  vtkMRMLScene *scene = vtkMRMLScene::SafeDownCast ( caller );
  if ( scene == this->MRMLScene && event == vtkMRMLScene::NodeAddedEvent )
    {
    }

  // only do what we need it to do if module is hidden.
  if ( !this->Raised )
    {
    return;
    }

  vtkMRMLCustomLayoutNode* node = vtkMRMLCustomLayoutNode::SafeDownCast ( caller );
  if ( node == this->CustomLayoutNode )
    {
    }
}



//----------------------------------------------------------------------------
void vtkCustomLayoutLogic::PopulateGrid( int rows, int columns )
{

  if ( this->GetCustomLayoutNode() == NULL )
    {
    vtkMRMLCustomLayoutNode *node = vtkMRMLCustomLayoutNode::New();
    this->SetAndObserveCustomLayoutNode ( node );
    node->Delete();
    }

  vtkMRMLCustomLayoutNode *n =  this->GetCustomLayoutNode();
  if ( n == NULL )
    {
    vtkErrorMacro ( "" );
    return;
    }

  if ( n->NumberOfRows == 0 || n->NumberOfRows == 0 )
    {
    vtkErrorMacro ( "InitializeLayout: Number of Rows or Number of Columns specified is zero, which is an invalid number." );
    return;
    }

  //---
  //--- Initialize all layout and parameter containers
  //---
  if ( n->PaneGraph == NULL )
    {
    n->PaneGraph = vtkMutableDirectedGraph::New();
    }
  else
    {
    n->PaneGraph->Initialize();
    }

  if ( n->PaneTree == NULL )
    {
    n->PaneTree = vtkTree::New();
    }
  else
    {
    n->PaneTree->Initialize();
    }

  //--- Now I don't think we need paneIDs -- the position in the
  //--- tree and their bounds are enough to distinguish them.
  //--- todo: test this in tcl.
  
  //--- number of vertices in the tree (+ 1 for the root).
  int numPanes = n->NumberOfRows * n->NumberOfColumns + 1;

  //--- add root, which corresponds to the overall container pane.
  n->LayoutRoot = n->PaneGraph->AddVertex();

  //--- store xmin, xmax, ymin and ymax for the whole grid container.
  vtkDoubleArray *rootPaneData = vtkDoubleArray::New();
  rootPaneData->SetName( "PaneBounds");
  rootPaneData->SetNumberOfComponents ( 4 );
  rootPaneData->SetNumberOfTuples ( numPanes );
  rootPaneData->InsertTuple4 ( 0, 0.0, 1.0, 0.0, 1.0 );

  //--- store the ID of the vertex at the same index for crossref to bounds
  vtkIntArray *paneIDData = vtkIntArray::New();
  paneIDData->SetName ( "PaneIDs");
  paneIDData->SetNumberOfComponents (1);
  paneIDData->SetNumberOfTuples ( numPanes );
  paneIDData->InsertTuple1 ( 0, (int)(n->LayoutRoot) );
  
  //---
  //--- Specify default grid of 4x4 panes inside the root container.
  //---
  vtkIdType child;

  //--- Create the pane layout (graph and tree)
  //--- and parameterize each pane within the layout.
  double yMin = 0.0;
  double paneHeight = 1.0 / n->NumberOfRows;
  double paneWidth = 1.0 / n->NumberOfColumns;
  int index = 1;
  for ( int row=0; row < n->NumberOfRows; row++)
    {
    double xMin = 0.0;
    for ( int col=0; col < n->NumberOfColumns; col++)
      {
      //---a pane's place in the layout and its parameterization
      child = n->PaneGraph->AddChild ( n->LayoutRoot );
      rootPaneData->InsertTuple4(index, xMin, xMin + paneWidth, yMin, yMin + paneHeight );
      paneIDData->InsertTuple1(index, (int)child );
      xMin += paneWidth;
      index++;
      }
      yMin += paneHeight;
    }

  //--- attach layout parameters, indexed by paneID
  n->PaneGraph->GetVertexData()->AddArray ( rootPaneData );
  n->PaneGraph->GetVertexData()->AddArray ( paneIDData );
  //--- copy layout to tree
  n->PaneTree->ShallowCopy ( n->PaneGraph);
  rootPaneData->Delete();
  paneIDData->Delete();
  
}

//----------------------------------------------------------------------------
float vtkCustomLayoutLogic::MouseCoordinateToPaneCoordinate(float pos)
{
  float newpos = 0.0;

  //--- convert input value to a value between 0.0 and 1.0;
  return (newpos);
}
  

//----------------------------------------------------------------------------
vtkCollection* vtkCustomLayoutLogic::GetRightNeighborPanes( vtkIdType paneID)
{


  vtkTree *t = this->CustomLayoutNode->GetPaneTree();
  if ( t == NULL )
    {
    vtkErrorMacro ( "TODO" );
    return (NULL );
    }
  if ( !this->CustomLayoutNode->IsPaneInTree ( paneID ) )
    {
    vtkErrorMacro ( "TODO" );
    return (NULL );
    }

  vtkCollection *c = vtkCollection::New();
  
  
  if ( c->GetNumberOfItems() == 0 )
    {
    c->Delete();
    c = NULL;
    }
  return ( c );
  
}


//----------------------------------------------------------------------------
vtkCollection* vtkCustomLayoutLogic::GetLeftNeighborPanes(vtkIdType paneID)
{
  vtkCollection *c = vtkCollection::New();

  if ( c->GetNumberOfItems() == 0 )
    {
    c->Delete();
    c = NULL;
    }
  return ( c );
}


//----------------------------------------------------------------------------
vtkCollection* vtkCustomLayoutLogic::GetTopNeighborPanes(vtkIdType paneID)
{
  vtkCollection *c = vtkCollection::New();

  if ( c->GetNumberOfItems() == 0 )
    {
    c->Delete();
    c = NULL;
    }
  return ( c );
}


//----------------------------------------------------------------------------
vtkCollection* vtkCustomLayoutLogic::GetBottomNeighborPanes(vtkIdType paneID)
{
  vtkCollection *c = vtkCollection::New();

  if ( c->GetNumberOfItems() == 0 )
    {
    c->Delete();
    c = NULL;
    }
  return ( c );
}



//----------------------------------------------------------------------------
void vtkCustomLayoutLogic::SplitPaneHorizontally(double y)
{
}


//----------------------------------------------------------------------------
 void vtkCustomLayoutLogic::SplitPaneVertically(double x)
{
}


//----------------------------------------------------------------------------
 void vtkCustomLayoutLogic::DeletePaneAndExpandLeftNeighbor(vtkIdType paneID)
{
}


//----------------------------------------------------------------------------
 void vtkCustomLayoutLogic::DeletePaneAndExpandRightNeighbor(vtkIdType paneID)
{
}


//----------------------------------------------------------------------------
 void vtkCustomLayoutLogic::DeletePaneAndShareParcelHorizontally(vtkIdType paneID)
{
}


//----------------------------------------------------------------------------
 void vtkCustomLayoutLogic::DeletePaneAndExpandTopNeighbor(vtkIdType paneID)
{
}


//----------------------------------------------------------------------------
 void vtkCustomLayoutLogic::DeletePaneAndExpandBottomNeighbor(vtkIdType paneID)
{
}


//----------------------------------------------------------------------------
 void vtkCustomLayoutLogic::DeletePaneAndShareParcelVertically(vtkIdType paneID)
{
}

//----------------------------------------------------------------------------
void vtkCustomLayoutLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

}


