// .NAME vtkMRMLAnnotationTextDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLAnnotationTextDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLAnnotationTextDisplayNode_h
#define __vtkMRMLAnnotationTextDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLAnnotationDisplayNode.h"
#include "vtkSlicerAnnotationModuleMRMLExport.h"

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATION_MODULE_MRML_EXPORT vtkMRMLAnnotationTextDisplayNode : public vtkMRMLAnnotationDisplayNode
{
 public:
  static vtkMRMLAnnotationTextDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLAnnotationTextDisplayNode,vtkMRMLAnnotationDisplayNode )
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
  virtual const char* GetNodeTagName() {return "AnnotationTextDisplay";}

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  // Description:
  // Get/Set for Text scale
  void SetTextScale(double scale);
  vtkGetMacro(TextScale,double)

  /// Enable/disable using automatic line wrap on text lines that are longer than MaxCharactersPerLine
  vtkSetMacro(UseLineWrap,int);
  vtkGetMacro(UseLineWrap,int);
  vtkBooleanMacro(UseLineWrap,int);

  /// Set teh maximum number of characters in a text line, used if UseLineWrap is true
  vtkSetMacro(MaxCharactersPerLine, unsigned int);
  vtkGetMacro(MaxCharactersPerLine, unsigned int);

    /// based on vtkCaptionActor2D.h

  /// Enable/disable the placement of a border around the text.
  vtkSetMacro(ShowBorder,int);
  vtkGetMacro(ShowBorder,int);
  vtkBooleanMacro(ShowBorder,int);

  /// Enable/disable drawing a "line" from the caption to the
  /// attachment point.
  vtkSetMacro(ShowLeader,int);
  vtkGetMacro(ShowLeader,int);
  vtkBooleanMacro(ShowLeader,int);

  /// Indicate whether the leader is 2D (no hidden line) or 3D (z-buffered).
  /// Only used in the 3d displayable managers, 2d ones set it off
  vtkSetMacro(UseThreeDimensionalLeader,int);
  vtkGetMacro(UseThreeDimensionalLeader,int);
  vtkBooleanMacro(UseThreeDimensionalLeader,int);

  /// Enable/disable showing the arrow head glyph at the end of the leader. Only used if ShowLeader is true
  vtkSetMacro(ShowArrowHead, int);
  vtkGetMacro(ShowArrowHead, int);
  vtkBooleanMacro(ShowArrowHead, int);

  /// Specify the relative size of the leader head. This is expressed as a
  /// fraction of the size (diagonal length) of the renderer. The leader
  /// head is automatically scaled so that window resize, zooming or other
  /// camera motion results in proportional changes in size to the leader
  /// glyph. Only used if ShowArrow is true.
  vtkSetClampMacro(LeaderGlyphSize,double,0.0,0.1);
  vtkGetMacro(LeaderGlyphSize,double);

  /// Specify the maximum size of the leader head (if any) in pixels. This
  /// is used in conjunction with LeaderGlyphSize to cap the maximum size of
  /// the LeaderGlyph.
  vtkSetClampMacro(MaximumLeaderGlyphSize,int,1,1000);
  vtkGetMacro(MaximumLeaderGlyphSize,int);

  /// Set/Get the padding between the caption and the border. The value
  /// is specified in pixels.
  vtkSetClampMacro(Padding, int, 0, 50);
  vtkGetMacro(Padding, int);

  /// Enable/disable whether to attach the arrow only to the edge,
  /// NOT the vertices of the caption border.
  vtkSetMacro(AttachEdgeOnly,int);
  vtkGetMacro(AttachEdgeOnly,int);
  vtkBooleanMacro(AttachEdgeOnly,int);
 
  /// Create a backup of this node and attach it.
  void CreateBackup();
  /// Restore an attached backup of this node.
  void RestoreBackup();

protected:
  vtkMRMLAnnotationTextDisplayNode();
  ~vtkMRMLAnnotationTextDisplayNode() { }
  vtkMRMLAnnotationTextDisplayNode( const vtkMRMLAnnotationTextDisplayNode& );
  void operator= ( const vtkMRMLAnnotationTextDisplayNode& );

  double TextScale;

  int UseLineWrap;
  unsigned int MaxCharactersPerLine;

  int ShowBorder;
  int ShowLeader;
  int ShowArrowHead;
  int UseThreeDimensionalLeader;
  double LeaderGlyphSize;
  int MaximumLeaderGlyphSize;
  int Padding;
  int AttachEdgeOnly;

};

#endif
