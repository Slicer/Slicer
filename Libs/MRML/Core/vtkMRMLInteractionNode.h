#ifndef __vtkMRMLInteractionNode_h
#define __vtkMRMLInteractionNode_h

#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLInteractionNode : public vtkMRMLNode
{
public:
  static vtkMRMLInteractionNode *New();
  vtkTypeMacro(vtkMRMLInteractionNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Interaction";};

  /// Get/Set Current and Last mouse mode.
  vtkGetMacro(CurrentInteractionMode, int);
  void SetCurrentInteractionMode(int mode);
  vtkGetMacro(LastInteractionMode, int);
  void SetLastInteractionMode(int mode);

  vtkGetMacro(PlaceModePersistence, int);
  vtkGetMacro(TransformModePersistence, int);

  /// Convenience methods for setting modes
  /// without triggering events on the node.
  /// These methods assist in distinguishing between
  /// persistent and transient mouse modes.
  /// TransformMode is persistent by default.
  virtual void SetPlaceModePersistence(int val);
  virtual void SetTransformModePersistence(int val);
  virtual void NormalizeAllMouseModes();

  /// mouse modes
  /// note: any new modes should be powers of 2 to allow
  /// bit field operations
  enum
    {
      //SelectRegion,
      //LassoRegion,
      Place = 0x1,
      ViewTransform = 0x2,
      Select = 0x4,
    };

  /// events
  enum
    {
      InteractionModeChangedEvent = 19001,
      InteractionModePersistenceChangedEvent,
      EndPlacementEvent,
    };

  /// Return a text string describing the mode
  const char *GetInteractionModeAsString(int mode);
  const char *GetInteractionModeAsString() { return this->GetInteractionModeAsString(this->CurrentInteractionMode); } ;

  /// Return the mode given a text string.
  int GetInteractionModeByString ( const char * modeString );

  /// Convenience methods for the mouse mode tool bar to call
  void SwitchToPersistentPlaceMode();
  void SwitchToSinglePlaceMode();
  void SwitchToViewTransformMode();

/// Enable/Disable Editing of Fibers
  vtkGetMacro(EnableFiberEdit, int);
  vtkSetMacro(EnableFiberEdit, int);

protected:
  vtkMRMLInteractionNode();
  ~vtkMRMLInteractionNode();

  vtkMRMLInteractionNode(const vtkMRMLInteractionNode&);
  void operator=(const vtkMRMLInteractionNode&);

  int LastInteractionMode;
  int CurrentInteractionMode;

  int PlaceModePersistence;
  int TransformModePersistence;

  int EnableFiberEdit;
};

#endif
