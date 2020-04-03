/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSliceCompositeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLSliceCompositeNode_h
#define __vtkMRMLSliceCompositeNode_h

#include "vtkMRMLNode.h"

/// \brief MRML node for storing a slice through RAS space.
///
/// This node stores the information about how to composite two
/// vtkMRMLVolumes into a single display image.
class VTK_MRML_EXPORT vtkMRMLSliceCompositeNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSliceCompositeNode *New();
  vtkTypeMacro(vtkMRMLSliceCompositeNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLSliceCompositeNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "SliceComposite";}

  /// Set the volumes as reference in the scene
  void SetSceneReferences() override;

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateReferences() override;

  ///
  /// Update the stored reference to another node in the scene
  void UpdateReferenceID(const char *oldID, const char *newID) override;

  ///
  /// the ID of a MRMLVolumeNode
  vtkGetStringMacro (BackgroundVolumeID);
  void SetBackgroundVolumeID(const char* id);
  void SetReferenceBackgroundVolumeID(const char *id) { this->SetBackgroundVolumeID(id); }

  ///
  /// the ID of a MRMLVolumeNode
  /// TODO: make this an arbitrary list of layers
  vtkGetStringMacro (ForegroundVolumeID);
  void SetForegroundVolumeID(const char* id);
  void SetReferenceForegroundVolumeID(const char *id) { this->SetForegroundVolumeID(id); }

  ///
  /// the ID of a MRMLVolumeNode
  /// TODO: make this an arbitrary list of layers
  vtkGetStringMacro (LabelVolumeID);
  void SetLabelVolumeID(const char* id);
  void SetReferenceLabelVolumeID(const char *id) { this->SetLabelVolumeID(id); }

  ///
  /// Compositing mode for foreground and background can be alpha
  /// blending, reverse alpha blending, addition, or subtraction
  vtkGetMacro (Compositing, int);
  vtkSetMacro (Compositing, int);

  ///
  /// opacity of the Foreground for rendering over background
  /// TODO: make this an arbitrary list of layers
  /// TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (ForegroundOpacity, double);
  vtkSetMacro (ForegroundOpacity, double);

  ///
  /// opacity of the Label for rendering over background
  /// TODO: make this an arbitrary list of layers
  /// TODO: make different composite types (checkerboard, etc)
  vtkGetMacro (LabelOpacity, double);
  vtkSetMacro (LabelOpacity, double);

  ///
  /// toggle that gangs control of slice viewers
  vtkGetMacro (LinkedControl, int );
  vtkSetMacro (LinkedControl, int );
  vtkBooleanMacro(LinkedControl, int);

  ///
  /// toggle for whether linked behavior is immediate or waits until
  /// an interaction is finished
  vtkGetMacro (HotLinkedControl, int );
  vtkSetMacro (HotLinkedControl, int );
  vtkBooleanMacro(HotLinkedControl, int);

  ///
  /// toggles fiducial visibility in the slice viewer
  vtkGetMacro (FiducialVisibility, int );
  vtkSetMacro (FiducialVisibility, int );
  vtkGetMacro (FiducialLabelVisibility, int );
  vtkSetMacro (FiducialLabelVisibility, int );

  ///
  /// toggles visibility of intersections of other slices in the slice viewer
  vtkGetMacro (SliceIntersectionVisibility, int );
  vtkSetMacro (SliceIntersectionVisibility, int );

  /// Get annotation space.
  vtkGetMacro ( AnnotationSpace, int );

  /// Set annotation space.
  /// Space could be either XYZ, IJK or RAS
  /// \sa GetAnnotationSpace, AnnotationSpace
  vtkSetMacro ( AnnotationSpace, int );

  /// Get annotation mode
  vtkGetMacro ( AnnotationMode, int );

  /// Set annotation mode
  /// Mode could be either NoAnnotation, All, LabelValuesOnly or LabelAndVoxelValuesOnly
  /// \sa GetAnnotationMode, AnnotationMode
  vtkSetMacro ( AnnotationMode, int );

  ///
  /// configures the behavior of PropagateVolumeSelection():
  /// if set to false, the background/label for slice views
  /// will not be reset. Default value is true
  vtkSetMacro (DoPropagateVolumeSelection, bool );
  vtkGetMacro (DoPropagateVolumeSelection, bool );

  ///
  /// Name of the layout. Must be unique between all the slice composite
  /// nodes because it is used as a singleton tag.
  /// Must be the same than the slice node.
  /// No name (i.e. "") by default. Typical names are colors:
  /// "Red", "Green", "Yellow"... to uniquely define the slice node
  /// \sa vtkMRMLSliceNode::SetLayoutName
  void SetLayoutName(const char *layoutName) {
    this->SetSingletonTag(layoutName);
  }
  char *GetLayoutName() {
    return this->GetSingletonTag();
  }

  /// Annotation space
  enum AnnotationSpace
    {
      XYZ = 0,
      IJK,
      RAS,
      IJKAndRAS
    };

  /// Annotation mode
  enum AnnotationMode
    {
      NoAnnotation = 0,
      All,
      LabelValuesOnly,
      LabelAndVoxelValuesOnly
    };

  /// Modes for compositing
  enum
    {
      Alpha = 0,
      ReverseAlpha,
      Add,
      Subtract
    };

  /// Get/Set a flag indicating whether this node is actively being
  /// manipulated (usually) by a user interface. This flag is used by
  /// logic classes to determine whether state changes should be
  /// propagated to other nodes to implement linked controls. Does not
  /// mark the node as Modified.
  void SetInteracting(int);
  vtkGetMacro(Interacting, int);
  vtkBooleanMacro(Interacting, int);

  /// Enum identifying the parameters being manipulated with calls to
  /// InteractionOn() and InteractionOff(). Identifiers are powers of
  /// two so they can be combined into a bitmask to manipulate
  /// multiple parameters.
  /// The meanings for the flags are:
  ///    ForegroundVolumeFlag - broadcast the foreground volume selection
  ///    BackgroundVolumeFlag - broadcast the background volume selection
  ///    LabelVolumeFlag - broadcast the label volume selection
  enum InteractionFlagType
  {
    None = 0,
    ForegroundVolumeFlag = 1,
    BackgroundVolumeFlag = 2,
    LabelVolumeFlag = 4,
    LabelOpacityFlag = 8,
    ForegroundOpacityFlag = 16
    // Next one needs to be 32
  };

  /// Get/Set a flag indicating what parameters are being manipulated
  /// within calls to InteractingOn() and InteractingOff(). These
  /// fields are used to propagate linked behaviors. This flag is a
  /// bitfield, with multiple parameters OR'd to composte the
  /// flag. Does not mark the node as Modified.
  void SetInteractionFlags(unsigned int);
  vtkGetMacro(InteractionFlags, unsigned int);

  /// Get/Set a flag indicating how the linking behavior should be modified.
  /// InteractionFlags modifier uses bits defined by InteractionFlagType enum
  /// that by default are all set and result in the default behavior. If a
  /// bit is not set, this will result in composite slice node interactions
  /// not broadcast.
  void SetInteractionFlagsModifier(unsigned int);
  vtkGetMacro(InteractionFlagsModifier, unsigned int);

  /// Set all bits of the modifier to 1, resulting in the default linking
  /// behavior (selection of foreground, background and label volumes being
  /// broadcast when composite slice nodes are linked).
  void ResetInteractionFlagsModifier();


protected:
  vtkMRMLSliceCompositeNode();
  ~vtkMRMLSliceCompositeNode() override;
  vtkMRMLSliceCompositeNode(const vtkMRMLSliceCompositeNode&);
  void operator=(const vtkMRMLSliceCompositeNode&);

  char *BackgroundVolumeID;
  char *ForegroundVolumeID;
  char *LabelVolumeID;
  double ForegroundOpacity;

  int Compositing;

  double LabelOpacity;
  int LinkedControl;
  int HotLinkedControl;

  int FiducialVisibility;
  int FiducialLabelVisibility;

  int SliceIntersectionVisibility;

  int AnnotationSpace;
  int AnnotationMode;

  bool DoPropagateVolumeSelection;

  int Interacting;
  unsigned int InteractionFlags;
  unsigned int InteractionFlagsModifier;
};

#endif
