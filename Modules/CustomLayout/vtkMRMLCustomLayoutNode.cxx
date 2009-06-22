#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkDoubleArray.h"
#include "vtkAdjacentVertexIterator.h"
#include "vtkDataSetAttributes.h"

#include "vtkMRMLCustomLayoutNode.h"
#include "vtkMRMLScene.h"

//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkMRMLCustomLayoutNode, "$Revision: 1.0 $");


//------------------------------------------------------------------------------
vtkMRMLCustomLayoutNode* vtkMRMLCustomLayoutNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCustomLayoutNode");
  if(ret)
    {
      return (vtkMRMLCustomLayoutNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCustomLayoutNode;
}

//----------------------------------------------------------------------------

vtkMRMLNode* vtkMRMLCustomLayoutNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLCustomLayoutNode");
  if(ret)
    {
      return (vtkMRMLCustomLayoutNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLCustomLayoutNode;
}

//----------------------------------------------------------------------------
vtkMRMLCustomLayoutNode::vtkMRMLCustomLayoutNode()
{
  this->HideFromEditors = true;
  this->PaneGap = 0.01;
  this->DefaultNumberOfRows = 4;
  this->DefaultNumberOfColumns = 4;
  this->NumberOfRows = this->DefaultNumberOfRows;
  this->NumberOfColumns = this->DefaultNumberOfColumns;
  this->PaneGraph = NULL;
  this->PaneTree = NULL;
  this->LayoutRoot = -1;
  this->LayoutName = NULL;
}




//----------------------------------------------------------------------------
vtkMRMLCustomLayoutNode::~vtkMRMLCustomLayoutNode()
{
  this->SetLayoutName(NULL);
  this->DefaultNumberOfRows = 4;
  this->DefaultNumberOfColumns = 4;
  this->NumberOfRows = this->DefaultNumberOfRows;
  this->NumberOfColumns = this->DefaultNumberOfColumns;
  this->PaneGap = 0.01;
  
  if ( this->PaneTree )
    {
    this->PaneTree->Delete();
    this->PaneTree = NULL;
    }
  if ( this->PaneGraph)
    {
    this->PaneGraph->Delete();
    this->PaneGraph = NULL;
    }
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, VolumeID
void vtkMRMLCustomLayoutNode::Copy(vtkMRMLNode *anode)
{
  //TODO
  Superclass::Copy(anode);
  vtkMRMLCustomLayoutNode *node = (vtkMRMLCustomLayoutNode *) anode;
}


//----------------------------------------------------------------------------
void vtkMRMLCustomLayoutNode::PrintSelf(ostream& os, vtkIndent indent)
{
  //TODO  
  vtkMRMLNode::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkMRMLCustomLayoutNode::WriteXML(ostream& of, int nIndent)
{
  //TODO
}


//----------------------------------------------------------------------------
void vtkMRMLCustomLayoutNode::ReadXMLAttributes(const char** atts)
{
  //TODO
}


//----------------------------------------------------------------------------
double vtkMRMLCustomLayoutNode::GetPaneXmin ( vtkIdType id )
{
  double min = 0.0;
  return ( min );
}

//----------------------------------------------------------------------------
double vtkMRMLCustomLayoutNode::GetPaneXmax ( vtkIdType id )
{
  double max = 0.0;
  return ( max );
}

//----------------------------------------------------------------------------
double vtkMRMLCustomLayoutNode::GetPaneYmin ( vtkIdType id )
{
  double min = 0.0;
  return ( min );
}

double vtkMRMLCustomLayoutNode::GetPaneYmax ( vtkIdType id )
{
  double max = 0.0;
  return ( max );
}

//----------------------------------------------------------------------------
void vtkMRMLCustomLayoutNode::UpdateLayoutFromSpecifications( int numberOfRows, int numberOfColumns )
{
}

//----------------------------------------------------------------------------
vtkIdType vtkMRMLCustomLayoutNode::GetIndexForPane ( vtkIdType vertexID )
{
  if ( ID < 0 )
    {
    vtkErrorMacro ( "GetIndexForPane: invalid Pane ID." );
    return ( -1 );
    }

  //--- get the array that contains all vertex IDs
  vtkIntArray *ids = vtkIntArray::SafeDownCast(this->PaneTree->GetVertexData()->GetArray ( "PaneIDs"));
  int numTuples = ids->GetNumberOfTuples();
  
  for ( int i=0; i < numTuples; i++ )
    {
    vtkIdType testID = ids->GetTuple1 ((vtkIdType) i );
    if ( testID == vertexID )
      {
      //---
      //--- found the target vertexID in the array.
      //--- return its index
      //---
      return ((vtkIdType) i );
      }
    }
  return (-1);

}



//----------------------------------------------------------------------------
double *vtkMRMLCustomLayoutNode::GetPaneBounds ( vtkIdType vertexID )
{
  //---
  //--- index for get Pane ID
  //---
  vtkIntArray *ids = vtkIntArray::SafeDownCast(this->PaneTree->GetVertexData()->GetArray ( "PaneIDs"));
  if (ids == NULL )
    {
    vtkErrorMacro ( "GetPaneBounds: unable to get array of ID data" );
    return ( NULL );
    }
  vtkIdType paneIndex = GetIndexForPane ( vertexID );
  if ( paneIndex < 0 )
    {
    vtkErrorMacro ( "GetPaneBounds: got an invalid index for pane." );
    return ( NULL );
    }
  
  //---
  //--- get corresponding bbox at index for Pane of same ID
  //---
  vtkDoubleArray *bounds = vtkDoubleArray::SafeDownCast(this->PaneTree->GetVertexData()->GetArray ( "PaneBounds" ));
  if ( bounds == NULL )
    {
    vtkErrorMacro ( "GetPaneBounds: unable to get array of vertex data" );
    return ( NULL );
    }
  double *paneBounds = bounds->GetTuple4 ( paneIndex );
  if ( paneBounds == NULL )
    {
    vtkErrorMacro ( "GetPaneBounds: got NULL pane bounds data." );    
    return ( NULL );
    }

  //---
  //--- return the info
  //---
  return ( paneBounds );
}



//----------------------------------------------------------------------------
vtkIdType vtkMRMLCustomLayoutNode::GetPaneIDByPosition( float x, float y)
{
  if ( this->PaneTree == NULL )
    {
    vtkErrorMacro ( "GetPaneIDByPosition: got NULL layout tree.");
    return (-1);
    }
  
  vtkIdType vertexID = this->PaneTree->GetRoot();;
  if ( vertexID <= 0 )
    {
    vtkErrorMacro ( "GetPaneIDByPosition: got invalid root for layout tree." );
    return (-1);
    }

  double * paneBounds = this->GetPaneBounds ( vertexID );
  if ( paneBounds == NULL )
    {
    vtkErrorMacro ( "TODO: got NULL bounds for pane..");
    return (-1);
    }

  //-- test
  if (( (double)x < paneBounds[0]) || ((double)x > paneBounds[1]) ||
      ( (double)y < paneBounds[2]) || ((double)y > paneBounds[3]) )
    {
    vtkErrorMacro ( "GetPaneIDByPosition: invalid point; outside entire bounds." );
    return (-1);
    }

  //---
  //--- search thru tree
  //---
  vtkAdjacentVertexIterator *it = vtkAdjacentVertexIterator::New();
  this->PaneTree->GetChildren(vertexID, it );
  vtkIdType child;
  vtkIdType paneID = -1;
  while ( it->HasNext() )
    {
    child = it->Next();
    if ( child <= 0 )
      {
      vtkErrorMacro ( "GetPaneIDByPosition: got invalid child in layout tree." );
      return (-1);
      }

    //--- save the ID in case this is the target pane.
    paneID = child;
    paneBounds = this->GetPaneBounds ( child );
    if ( paneBounds == NULL )
      {
      vtkErrorMacro ( "TODO: got NULL bounds for pane..");
      return (-1);
      }

    //-- test
    if (( x < paneBounds[0]) || (x > paneBounds[1]) ||
        ( y < paneBounds[2]) || (y > paneBounds[3]) )
      {
      // position is not inside this pane.
      continue;
      }

    //-- position is inside this pane; check the pane's children.
    vertexID = child;
    this->PaneTree->GetChildren(vertexID, it );
    }

  //--- clean up
  it->Delete();

  //--- return result
  if ( paneID >= 0 )
    {
    return ( paneID );
    }
  else
    {
    //--- negative ID value for error checking.
    return (-1);
    }
}


//----------------------------------------------------------------------------
int vtkMRMLCustomLayoutNode::IsPaneInSubTree (vtkIdType targetID, vtkIdType subTreeRootID )
{
  vtkIntArray *ids = vtkIntArray::SafeDownCast (this->PaneTree->GetVertexData()->GetArray ( "PaneIDs"));

  vtkIdType count = this->PaneTree->GetNumberOfChildren( subTreeRootID );
  for ( vtkIdType child = 0; child != count; ++child )
    {
    if (child == targetID )
      {
      return (1);
      }
    else
      {
      if ( IsPaneInSubTree ( targetID, child ) )
        {
        return ( 1 );
        }
    }
    return ( 0 );
    }
}
  



//----------------------------------------------------------------------------
int vtkMRMLCustomLayoutNode::IsPaneInTree (vtkIdType targetID  )
{
  if ( targetID < 0 )
    {
    vtkErrorMacro ( "IsPaneInPainIDTree: invalid Pane ID." );
    return ( 0 );
    }
  if ( this->PaneTree == NULL )
    {
    vtkErrorMacro ( "IsPaneInPainTree: got NULL PaneTree" );
    return ( 0 );
    }
  
  vtkIntArray *ids = vtkIntArray::SafeDownCast(this->PaneTree->GetVertexData()->GetArray ( "PaneIDs"));
  vtkAdjacentVertexIterator *it = vtkAdjacentVertexIterator::New();
  vtkIdType vertexID = this->PaneTree->GetRoot();;
  
  //--- look at root...
  if ( vertexID == targetID )
    {
    return ( 1 );
    }

  if ( this->IsPaneInSubTree ( targetID, vertexID ) )
    {
    return ( 1 );
    }
  else
    {
    return ( 0 );
    }
}








