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


 protected:
  vtkMRMLAnnotationTextDisplayNode();
  ~vtkMRMLAnnotationTextDisplayNode() { }
  vtkMRMLAnnotationTextDisplayNode( const vtkMRMLAnnotationTextDisplayNode& );
  void operator= ( const vtkMRMLAnnotationTextDisplayNode& );

  double TextScale;

};

#endif
