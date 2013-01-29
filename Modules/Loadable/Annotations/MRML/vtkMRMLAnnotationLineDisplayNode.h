// .NAME vtkMRMLAnnotationLineDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLAnnotationLineDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLAnnotationLineDisplayNode_h
#define __vtkMRMLAnnotationLineDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLAnnotationDisplayNode.h"
#include "vtkSlicerAnnotationsModuleMRMLExport.h"

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationLineDisplayNode : public vtkMRMLAnnotationDisplayNode
{
 public:
  static vtkMRMLAnnotationLineDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLAnnotationLineDisplayNode,vtkMRMLAnnotationDisplayNode );
  void PrintSelf ( ostream& os, vtkIndent indent );
  
  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance (  );

  // Description:
  // Read node attributes from XML (MRML) file
  virtual void ReadXMLAttributes ( const char** atts );

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML ( ostream& of, int indent );


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy ( vtkMRMLNode *node );
  
  // Description:
  // Get node XML tag name (like Volume, Annotation)
  virtual const char* GetNodeTagName() {return "AnnotationLineDisplay";};

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  /// Get/Set for Symbol scale
  ///  vtkSetMacro(GlyphScale,double);
  void SetLineThickness(double thickness);
  vtkGetMacro(LineThickness,double);

  /// Get/Set for LabelPosition
  vtkSetClampMacro(LabelPosition, double, 0.0, 1.0);
  vtkGetMacro(LabelPosition, double);

  /// Get/Set for LabelVisibility
  vtkBooleanMacro(LabelVisibility, int);
  vtkSetMacro(LabelVisibility, int);
  vtkGetMacro(LabelVisibility, int);

  /// Get/Set for TickSpacing
  vtkSetMacro(TickSpacing, double);
  vtkGetMacro(TickSpacing, double);

  /// Get/Set for maximum number of ticks
  vtkSetMacro(MaxTicks, int);
  vtkGetMacro(MaxTicks, int);
  
  /// Set SliceProjection flag 
  /// ProjectionOff, Dotted and ColoredWhenParallel by default.
  /// ProjectionOff : No projection
  /// ProjectionOn : Projection with plain line
  /// ProjectionDotted : Projection with dotted line when
  ///                          not in the plane
  /// ProjectionColoredWhenParallel : Color line if parallel
  ///                                            to slice plane
  /// \sa SliceIntersectionVisibilty, ProjectedLineColor
  vtkGetMacro(SliceProjection, int);
  vtkSetMacro(SliceProjection, int);

  /// Set SliceProjection to On
  inline void SetSliceProjectionOn();
 
  /// Set SliceProjection to Off
  inline void SetSliceProjectionOff();

  /// Set SliceProjection to Dotted
  inline void SetSliceProjectionDottedOn();

  /// Set SliceProjection to Plain
  inline void SetSliceProjectionDottedOff();

  /// Set line colored when parallel to slice plane
  inline void SetSliceProjectionColoredWhenParallelOn();

  /// Set line color unchanged when parallel to slice plane
  inline void SetSliceProjectionColoredWhenParallelOff();

  /// \enumthis->SliceProjection ProjectionFlag
  enum ProjectionFlag
  {
  ProjectionOff = 0x00,
  ProjectionOn = 0x01,
  ProjectionDotted = 0x02,
  ProjectionColoredWhenParallel = 0x04,
  //ProjectionSparseLineStipplePattern = 0x08
  };

  /// Set color of the projected line on the 2D viewers
  /// when parallel to the slice plane
  /// White (1.0, 1.0, 1.0, 1.0) by default.
  vtkGetVector4Macro(SliceProjectionLineColor, double);
  vtkSetVector4Macro(SliceProjectionLineColor, double);

  /// Create a backup of this node and attach it.
  void CreateBackup();
  /// Restore an attached backup of this node.
  void RestoreBackup();

protected:
  vtkMRMLAnnotationLineDisplayNode();
  ~vtkMRMLAnnotationLineDisplayNode() { };
  vtkMRMLAnnotationLineDisplayNode( const vtkMRMLAnnotationLineDisplayNode& );
  void operator= ( const vtkMRMLAnnotationLineDisplayNode& );
  
  double LineThickness;
  double LabelPosition;
  int LabelVisibility;
  double TickSpacing;
  int MaxTicks;
  int SliceProjection;
  double SliceProjectionLineColor[4];
};

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SetSliceProjectionOn()
{
  this->SetSliceProjection( this->GetSliceProjection() | 
                            vtkMRMLAnnotationLineDisplayNode::ProjectionOn);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SetSliceProjectionOff()
{
  this->SetSliceProjection( this->GetSliceProjection() & 
                            ~vtkMRMLAnnotationLineDisplayNode::ProjectionOn);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SetSliceProjectionDottedOn()
{
  this->SetSliceProjection( this->GetSliceProjection() | 
                            vtkMRMLAnnotationLineDisplayNode::ProjectionDotted);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SetSliceProjectionDottedOff()
{
  this->SetSliceProjection( this->GetSliceProjection() & 
                            ~vtkMRMLAnnotationLineDisplayNode::ProjectionDotted);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SetSliceProjectionColoredWhenParallelOn()
{
  this->SetSliceProjection( this->GetSliceProjection() | 
                            vtkMRMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SetSliceProjectionColoredWhenParallelOff()
{
  this->SetSliceProjection( this->GetSliceProjection() & 
                            ~vtkMRMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel);
}


#endif
