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
  void PrintSelf ( ostream& os, vtkIndent indent ) override;

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance () override;

  // Description:
  // Read node attributes from XML (MRML) file
  void ReadXMLAttributes ( const char** atts ) override;

  // Description:
  // Write this node's information to a MRML file in XML format.
  void WriteXML ( ostream& of, int indent ) override;


  // Description:
  // Copy the node's attributes to this object
  void Copy ( vtkMRMLNode *node ) override;

  // Description:
  // Get node XML tag name (like Volume, Annotation)
  const char* GetNodeTagName() override {return "AnnotationLineDisplay";}

  // Description:
  // Finds the storage node and read the data
  void UpdateScene(vtkMRMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

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

  /// Set SliceProjection to Dashed
  inline void SliceProjectionDashedOn();

  /// Set SliceProjection to Plain
  inline void SliceProjectionDashedOff();

  /// Set line colored when parallel to slice plane
  inline void SliceProjectionColoredWhenParallelOn();

  /// Set line color unchanged when parallel to slice plane
  inline void SliceProjectionColoredWhenParallelOff();

  /// Set line thicker when on top of the plane, thiner when under
  inline void SliceProjectionThickerOnTopOn();

  /// Set line thickness uniform
  inline void SliceProjectionThickerOnTopOff();

  /// Set projection color as ruler color
  ///\sa SetProjectedColor
  inline void SliceProjectionUseRulerColorOn();

  /// Manually set projection color
  ///\sa SetProjectedColor
  inline void SliceProjectionUseRulerColorOff();

  /// ProjectionDashed : Set projected line dashed
  /// ProjectionColoredWhenParallel : Set projected line
  /// colored when parallel to slice plane
  /// ProjectionThickerOnTop : Set projected line thicker
  /// on top of the plane, thiner when under
  /// Projection Off, Dashed, ColoredWhenParallel,
  /// ThickerOnTop and UseRulerColor by default
  /// \enum ProjectionFlag
  enum ProjectionFlag
  {
  ProjectionDashed = 0x02,
  ProjectionColoredWhenParallel = 0x04,
  ProjectionThickerOnTop = 0x08,
  ProjectionUseRulerColor = 0x10
  };

  /// Get/Set the thickness of the line under the plane
  /// Default: 1.0
  vtkSetMacro(UnderLineThickness, double);
  vtkGetMacro(UnderLineThickness, double);

  /// Get/Set the thickness of the line over the plane
  /// Default: 3.0
  vtkSetMacro(OverLineThickness, double);
  vtkGetMacro(OverLineThickness, double);

  /// Create a backup of this node and attach it.
  void CreateBackup() override;
  /// Restore an attached backup of this node.
  void RestoreBackup() override;

protected:
  vtkMRMLAnnotationLineDisplayNode();
  ~vtkMRMLAnnotationLineDisplayNode() override  = default;
  vtkMRMLAnnotationLineDisplayNode( const vtkMRMLAnnotationLineDisplayNode& );
  void operator= ( const vtkMRMLAnnotationLineDisplayNode& );

  double LineThickness;
  double LabelPosition;
  int LabelVisibility;
  double TickSpacing;
  int MaxTicks;

  double UnderLineThickness;
  double OverLineThickness;
};

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionDashedOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLAnnotationLineDisplayNode::ProjectionDashed);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionDashedOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLAnnotationLineDisplayNode::ProjectionDashed);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionColoredWhenParallelOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionColoredWhenParallelOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLAnnotationLineDisplayNode::ProjectionColoredWhenParallel);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionThickerOnTopOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLAnnotationLineDisplayNode::ProjectionThickerOnTop);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionThickerOnTopOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLAnnotationLineDisplayNode::ProjectionThickerOnTop);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionUseRulerColorOn()
{
  this->SetSliceProjection( this->GetSliceProjection() |
                            vtkMRMLAnnotationLineDisplayNode::ProjectionUseRulerColor);
}

//----------------------------------------------------------------------------
void vtkMRMLAnnotationLineDisplayNode
::SliceProjectionUseRulerColorOff()
{
  this->SetSliceProjection( this->GetSliceProjection() &
                            ~vtkMRMLAnnotationLineDisplayNode::ProjectionUseRulerColor);
}

#endif
