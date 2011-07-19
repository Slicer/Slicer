#ifndef __vtkMRMLInteractionNode_h
#define __vtkMRMLInteractionNode_h

#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLInteractionNode : public vtkMRMLNode
{
public:
  static vtkMRMLInteractionNode *New();
  vtkTypeMacro(vtkMRMLInteractionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------
  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Interaction";};
  
  /// 
  /// Get/Set Current and Last mouse mode.
  vtkGetMacro (CurrentInteractionMode, int );
  void SetCurrentInteractionMode ( int mode );
  vtkGetMacro (LastInteractionMode, int );
  void SetLastInteractionMode (int mode );

  vtkGetMacro (PickModePersistence, int );
  vtkGetMacro (PlaceModePersistence, int);
  vtkGetMacro (TransformModePersistence, int );

  //--- workaround for mouse modes.
  //--- put on when fiducials are placed.
  //--- turned off by callback to window/level.
  vtkGetMacro (WindowLevelLock, int );
  vtkSetMacro (WindowLevelLock, int );
  
  //--- workaround for mouse modes.
  //--- put on when an object is mid-place
  //--- (between mouse-press and mouse-release)
  //--- to prevent PickAndManipulate operations
  //--- from being processed while placing.
  vtkGetMacro (PlaceOperationLock, int );
  vtkSetMacro (PlaceOperationLock, int );

  // Description:
  // Convenience methods for setting modes
  // without triggering events on the node.
  // These methods assist in distinguishing between
  // persistent and transient mouse modes.
  // TransformMode is persistent by default,
  // and Pick or Place are transient by default.
  virtual void SetPickModePersistence ( int val );
  virtual void SetPlaceModePersistence (int val );
  virtual void SetTransformModePersistence ( int val );
  virtual void NormalizeAllMouseModes();
  
  /// mouse modes
  enum
    {
      PickManipulate = 0,
      SelectRegion,
      LassoRegion,
      Place,
      ViewPan,
      ViewZoom,
      ViewRotate,
      ViewTransform,
      CustomTag,
    };
  /// events
  enum
    {
      InteractionModeChangedEvent = 19001,
      TransientTimeoutEvent,
      InteractionModePersistenceChangedEvent,
      EndPlacementEvent,
    };

  //BTX
  int PickModePersistence;
  int PlaceModePersistence;
  int TransformModePersistence;
  //ETX

  /// 
  /// Return a text string describing the mode
  const char *GetInteractionModeAsString(int mode);
  const char *GetInteractionModeAsString() { return this->GetInteractionModeAsString(this->CurrentInteractionMode); } ;
  ///
  /// Return the mode given a text string.
  int GetInteractionModeByString ( const char * modeString );

  int WindowLevelLock;
  int PlaceOperationLock;

  ///
  /// Convenience methods for the mouse mode tool bar to call
  void SwitchToPersistentPickMode();
  void SwitchToSinglePickMode();
  void SwitchToPersistentPlaceMode();
  void SwitchToSinglePlaceMode();
  void SwitchToViewTransformMode();
  
protected:
  vtkMRMLInteractionNode();
  ~vtkMRMLInteractionNode();
  vtkMRMLInteractionNode(const vtkMRMLInteractionNode&);
  void operator=(const vtkMRMLInteractionNode&);

  /// may be used if temporary overrides are possible.
  int LastInteractionMode;
  /// current mouse mode
  int CurrentInteractionMode;
};

#endif
