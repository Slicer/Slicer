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
#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkSlicerAnnotationsModuleMRMLExport.h"

#define vtkSetAndPropagateVector3Macro(name,type) \
virtual void SetAndPropagateSuper##name (type _arg1, type _arg2, type _arg3) \
  { \
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Super" << #name " to (" << _arg1 << "," << _arg2 << "," << _arg3 << ")"); \
  if ((this->Super##name[0] != _arg1)||(this->Super##name[1] != _arg2)||(this->Super##name[2] != _arg3)) \
    { \
    this->name[0] = _arg1; \
    this->name[1] = _arg2; \
    this->name[2] = _arg3; \
    this->Super##name[0] = _arg1; \
    this->Super##name[1] = _arg2; \
    this->Super##name[2] = _arg3; \
    vtkMRMLDisplayableNode *displayableNode = this->GetDisplayableNode(); \
    if (!displayableNode) \
      { \
      return; \
      } \
    for (int i=0; i<displayableNode->GetNumberOfDisplayNodes(); i++) \
      { \
      vtkMRMLDisplayNode *displayNode = displayableNode->GetNthDisplayNode(i); \
      if (!displayNode || displayNode==this) \
        { \
        continue; \
        } \
      displayNode->Set##name(this->Super##name); \
      } \
    this->Modified(); \
    } \
  }; \
virtual void SetAndPropagateSuper##name (type _arg[3]) \
  { \
  this->SetAndPropagateSuper##name (_arg[0], _arg[1], _arg[2]);\
  }

#define vtkSetAndPropagateMacro(name,type) \
virtual void SetAndPropagateSuper##name (type _arg) \
  { \
  vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting Super" << #name " to (" << _arg << ")"); \
  if (this->Super##name != _arg) \
    { \
    this->name = _arg; \
    this->Super##name = _arg; \
    vtkMRMLDisplayableNode *displayableNode = this->GetDisplayableNode(); \
    if (!displayableNode) \
      { \
      return; \
      } \
    for (int i=0; i<displayableNode->GetNumberOfDisplayNodes(); i++) \
      { \
      vtkMRMLDisplayNode *displayNode = displayableNode->GetNthDisplayNode(i); \
      if (!displayNode || displayNode==this) \
        { \
        continue; \
        } \
      displayNode->Set##name(this->Super##name); \
      } \
    this->Modified(); \
    } \
  }; \


/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationDisplayNode : public vtkMRMLModelDisplayNode
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
  virtual const char* GetNodeTagName() {return "AnnotationDisplay";};

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
  virtual void CreateBackup(){};
  virtual void RestoreBackup(){};
  void ClearBackup();
  /// Returns the associated backup of this node
  vtkMRMLAnnotationDisplayNode * GetBackup();

  //
  // Set and propagate attributes to all other displayNodes
  //

  vtkSetAndPropagateVector3Macro(Color, double);
  vtkGetVector3Macro(SuperColor, double);

  vtkSetAndPropagateVector3Macro(SelectedColor, double);
  vtkGetVector3Macro(SuperSelectedColor, double);

  vtkSetAndPropagateMacro(Opacity, double);
  vtkGetMacro(SuperOpacity, double);

  vtkSetAndPropagateMacro(Ambient, double);
  vtkGetMacro(SuperAmbient, double);

  vtkSetAndPropagateMacro(Diffuse, double);
  vtkGetMacro(SuperDiffuse, double);

  vtkSetAndPropagateMacro(Specular, double);
  vtkGetMacro(SuperSpecular, double);

  vtkSetAndPropagateMacro(Power, double);
  vtkGetMacro(SuperPower, double);

  vtkSetAndPropagateMacro(SelectedAmbient, double);
  vtkGetMacro(SuperSelectedAmbient, double);

  vtkSetAndPropagateMacro(SelectedSpecular, double);
  vtkGetMacro(SuperSelectedSpecular, double);

 protected:
  vtkMRMLAnnotationDisplayNode();
  ~vtkMRMLAnnotationDisplayNode();
  vtkMRMLAnnotationDisplayNode( const vtkMRMLAnnotationDisplayNode& );
  void operator= ( const vtkMRMLAnnotationDisplayNode& );

  vtkMRMLAnnotationDisplayNode* m_Backup;

  double SuperColor[3];
  double SuperSelectedColor[3];

  double SuperOpacity;
  double SuperAmbient;
  double SuperDiffuse;
  double SuperSpecular;
  double SuperPower;
  double SuperSelectedAmbient;
  double SuperSelectedSpecular;


};

#endif
