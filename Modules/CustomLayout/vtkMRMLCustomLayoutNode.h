#ifndef __vtkMRMLCustomLayoutNode_h
#define __vtkMRMLCustomLayoutNode_h

#include "vtkStringArray.h"

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkMutableDirectedGraph.h"
#include "vtkTree.h"

#include "vtkCustomLayoutWin32Header.h"
#include <string>
#include <vector>


class VTK_CUSTOMLAYOUT_EXPORT vtkMRMLCustomLayoutNode : public vtkMRMLNode
{
  public:
  static vtkMRMLCustomLayoutNode *New();
  vtkTypeRevisionMacro(vtkMRMLCustomLayoutNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Create instance 
  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes from name/value pairs
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);
  
  // Description:
  // Get unique node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "CustomLayoutNode"; };

  vtkGetObjectMacro ( PaneGraph, vtkMutableDirectedGraph );
  vtkSetObjectMacro ( PaneGraph, vtkMutableDirectedGraph );

  vtkGetObjectMacro ( PaneTree, vtkTree);
  vtkSetObjectMacro ( PaneTree, vtkTree);

  vtkGetMacro ( DefaultNumberOfRows, int );
  vtkSetMacro ( DefaultNumberOfRows, int );
  
  vtkGetMacro ( DefaultNumberOfColumns, int );
  vtkSetMacro ( DefaultNumberOfColumns, int );
  
  vtkGetMacro ( NumberOfRows, int );
  vtkSetMacro ( NumberOfRows, int );
  
  vtkGetMacro ( NumberOfColumns, int );
  vtkSetMacro ( NumberOfColumns, int );

  vtkGetMacro ( LayoutRoot, vtkIdType );
  vtkSetMacro ( LayoutRoot, vtkIdType );
  
  vtkGetStringMacro ( LayoutName );
  vtkSetStringMacro ( LayoutName );

  // Description:
  // Methods that operate on the layout representation
  virtual void UpdateLayoutFromSpecifications( int numberOfRows, int numberOfColumns );

  // Description:
  // Methods that probe the layout representation
  virtual vtkIdType GetIndexForPane ( vtkIdType vertexID );
  virtual double *GetPaneBounds ( vtkIdType vertexID );
  virtual vtkIdType GetPaneIDByPosition (float x, float y );
  virtual int IsPaneInTree ( vtkIdType id );

  virtual double GetPaneXmin ( vtkIdType id );
  virtual double GetPaneXmax ( vtkIdType id );
  virtual double GetPaneYmin ( vtkIdType id );
  virtual double GetPaneYmax ( vtkIdType id );
  
  // Description:
  // Used for creating the Pane ID Tree.
  vtkMutableDirectedGraph *PaneGraph;

  // Description:
  // Used for read-out -- encapsulates layout of
  // individual panes by ID.
  vtkTree *PaneTree;

  // Description:
  // ID of the tree root.
  vtkIdType LayoutRoot;
  
  // Description:
  // Geometry parameters.
  float PaneGap;
  int DefaultNumberOfRows;
  int DefaultNumberOfColumns;
  int NumberOfRows;
  int NumberOfColumns;
  char *LayoutName;
  
  //BTX
  enum
    {
      LayoutNameModified  = 11000,
      LayoutModified,
      LayoutInitialized,
    };
  //ETX

 protected:
  vtkMRMLCustomLayoutNode();
  ~vtkMRMLCustomLayoutNode();
  vtkMRMLCustomLayoutNode(const vtkMRMLCustomLayoutNode&);
  void operator=(const vtkMRMLCustomLayoutNode&);

  virtual int IsPaneInSubTree (vtkIdType targetIDID, vtkIdType subTreeRootID );

};

#endif

