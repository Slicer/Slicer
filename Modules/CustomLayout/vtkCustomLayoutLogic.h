#ifndef __vtkCustomLayoutLogic_h
#define __vtkCustomLayoutLogic_h

#include "vtkCustomLayoutWin32Header.h"
#include <vtksys/SystemTools.hxx>
#include "vtkObject.h"

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLScene.h"

#include "vtkCustomLayout.h"
#include "vtkMRMLCustomLayoutNode.h"

#include <string>
#include <vector>
#include <map>
#include <iterator>

class vtkIntArray;
class VTK_CUSTOMLAYOUT_EXPORT vtkCustomLayoutLogic : public vtkSlicerModuleLogic
{
  public:
  static vtkCustomLayoutLogic *New();
  vtkTypeMacro(vtkCustomLayoutLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // These methods are used to turn observers on/off when module is entered/exited.
  virtual void Enter ( );
  virtual void Exit ( ) ;
  
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                   void *callData );

  // Description:
  // Get/Set MRML node storing parameter values
  vtkGetObjectMacro (CustomLayoutNode, vtkMRMLCustomLayoutNode);
  virtual void SetCustomLayoutNode( vtkMRMLCustomLayoutNode *node );
  
  // Description:
  // Get/Set MRML node storing parameter values
  void SetAndObserveCustomLayoutNode(vtkMRMLCustomLayoutNode *n) 
    {
    vtkSetAndObserveMRMLNodeMacro( this->CustomLayoutNode, n);
    }

  virtual vtkIntArray* NewObservableEvents();

  // Description:
  // Creates a layout grid populated with numRows rows
  // and numColumns columns of panes.
  virtual void PopulateGrid( int numRows, int numColumns );

  // Description:
  // The entire layout dimensions are specified in a 1.0 x 1.0 parcel.
  // Individual panes occupy some sub-parcel within.
  // What coordinates will a mouse-move generate? dunno. this
  // will eventually convert. Maybe method belongs in GUI.
  virtual float MouseCoordinateToPaneCoordinate(float pos);
  
  
  // Description:
  // Get the panes that touch the Left, Right, Top, Bottom
  // Edges of a pane. Make sure to delete the collection after
  // these methods are called.
  virtual vtkCollection* GetRightNeighborPanes( vtkIdType paneID);
  virtual vtkCollection* GetLeftNeighborPanes(vtkIdType paneID);
  virtual vtkCollection* GetTopNeighborPanes(vtkIdType paneID);
  virtual vtkCollection* GetBottomNeighborPanes(vtkIdType paneID);
    
  virtual void SplitPaneHorizontally(double y);
  virtual void SplitPaneVertically(double x);

  virtual void DeletePaneAndExpandLeftNeighbor(vtkIdType paneID);
  virtual void DeletePaneAndExpandRightNeighbor(vtkIdType paneID);
  virtual void DeletePaneAndShareParcelHorizontally(vtkIdType paneID);

  virtual void DeletePaneAndExpandTopNeighbor(vtkIdType paneID);
  virtual void DeletePaneAndExpandBottomNeighbor(vtkIdType paneID);
  virtual void DeletePaneAndShareParcelVertically(vtkIdType paneID);
  
 protected:
  vtkCustomLayoutLogic();
  ~vtkCustomLayoutLogic();
  vtkCustomLayoutLogic(const vtkCustomLayoutLogic&);
  void operator=(const vtkCustomLayoutLogic&);

  bool Visited;
  bool Raised;
  vtkMRMLCustomLayoutNode *CustomLayoutNode;

};


#endif

