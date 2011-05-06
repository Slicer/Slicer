// .NAME vtkMRMLAnnotationDisplayNode - MRML node to represent display properties for tractography.
// .SECTION Description
// vtkMRMLAnnotationDisplayNode nodes store display properties of trajectories 
// from tractography in diffusion MRI data, including color type (by bundle, by fiber, 
// or by scalar invariants), display on/off for tensor glyphs and display of 
// trajectory as a line or tube.
//

#ifndef __vtkMRMLAnnotationDisplayNode_h
#define __vtkMRMLAnnotationDisplayNode_h

#include "vtkMRML.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkSlicerAnnotationModuleMRMLExport.h"

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATION_MODULE_MRML_EXPORT vtkMRMLAnnotationDisplayNode : public vtkMRMLModelDisplayNode
{
 public:
  static vtkMRMLAnnotationDisplayNode *New (  );
  vtkTypeMacro ( vtkMRMLAnnotationDisplayNode,vtkMRMLModelDisplayNode );
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
  virtual const char* GetNodeTagName() {return "AnnotationTextDisplay";};

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );

  // Functionality for backups of this node
  /// Creates a backup of the current MRML state of this node and keeps a reference
  void CreateBackup();
  /// Returns the associated backup of this node
  vtkMRMLAnnotationDisplayNode * GetBackup();

 protected:
  vtkMRMLAnnotationDisplayNode();
  ~vtkMRMLAnnotationDisplayNode();
  vtkMRMLAnnotationDisplayNode( const vtkMRMLAnnotationDisplayNode& );
  void operator= ( const vtkMRMLAnnotationDisplayNode& );

  vtkMRMLAnnotationDisplayNode* m_Backup;

};

#endif
