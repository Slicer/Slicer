// .NAME vtkMRMLAnnotationNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography 
// that MRML will manage/read/write. Each control point has accompanying data.  
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationTextDisplayNode class.
//

#ifndef __vtkMRMLAnnotationNode_h
#define __vtkMRMLAnnotationNode_h

#include "vtkMRMLModelNode.h" 
#include "vtkMRMLAnnotationTextDisplayNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkSlicerAnnotationModuleMRMLExport.h"

class vtkStringArray;
class vtkMRMLStorageNode;
class vtkMRMLAnnotationTextDisplayNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATION_MODULE_MRML_EXPORT vtkMRMLAnnotationNode : public vtkMRMLModelNode
{
public:
  static vtkMRMLAnnotationNode *New();
  vtkTypeMacro(vtkMRMLAnnotationNode,vtkMRMLModelNode);
  void PrintSelf(ostream& os, vtkIndent indent);
  // Description:
  // Just prints short summary 
  virtual void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  virtual const char* GetIcon() {return "";};

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();
  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Annotation";};

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);
  
  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);


  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);


  void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );


  // Description:
  // Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();  

  int AddText(const char *newText,int selectedFlag, int visibleFlag); 
  void SetText(int id, const char *newText,int selectedFlag, int visibleFlag);
  vtkStdString GetText(int id); 
  int DeleteText(int id); 

  int GetNumberOfTexts(); 

//BTX
  enum 
  {
    TEXT_SELECTED = 0,
    TEXT_VISIBLE,
    NUM_TEXT_ATTRIBUTE_TYPES,
     LockModifiedEvent,
     CancelPlacementEvent,
  };
//ETX
  virtual const char *GetAttributeTypesEnumAsString(int val);
  vtkDataArray* GetAnnotationAttributes(int att); 
  int GetAnnotationAttribute(vtkIdType id, int att);
  void SetAnnotationAttribute(vtkIdType id, int att, double value);
  int DeleteAttribute(vtkIdType idAttEntry,  vtkIdType idAttType);

  // Description:
  // Initializes all variables associated with annotations
  virtual void ResetAnnotations();

  // Description:
  // add display node if not already present
  void CreateAnnotationTextDisplayNode();

  vtkMRMLAnnotationTextDisplayNode* GetAnnotationTextDisplayNode();

  /// Set the text scale of the associated text.
  void SetTextScale(double textScale) {this->GetAnnotationTextDisplayNode()->SetTextScale(textScale); this->InvokeEvent(vtkCommand::ModifiedEvent);}
  /// Get the text scale of the associated text.
  double GetTextScale() {return this->GetAnnotationTextDisplayNode()->GetTextScale();}


  // Description:
  // Reference of this annotation - can be an image, model, scene ,  ... 
  vtkGetStringMacro (ReferenceNodeID);
  vtkSetStringMacro (ReferenceNodeID);

  // Description:
  // Select all elements defining an annotation 
  // overwrites *_SELECTED and *_VISIBLE when set to 0
  // do not have to define for Selected as already defined by Superclass 
  vtkGetMacro(Visible, int);
  vtkSetMacro(Visible, int);
  vtkBooleanMacro(Visible, int);

  // Description:
  // If set to 1 then parameters should not be changed 
  vtkGetMacro(Locked, int);
  void SetLocked(int init);

  void Initialize(vtkMRMLScene* mrmlScene);

  // Functionality for backups of this node
  /// Creates a backup of the current MRML state of this node and keeps a reference
  void CreateBackup();
  /// Returns the associated backup of this node
  vtkMRMLAnnotationNode * GetBackup();

  // Functionality to save the current view
  /// Saves the current view.
  void SaveView();
  /// Restores a previously saved view.
  void RestoreView();

protected:
  vtkMRMLAnnotationNode();
  ~vtkMRMLAnnotationNode();
  vtkMRMLAnnotationNode(const vtkMRMLAnnotationNode&);
  void operator=(const vtkMRMLAnnotationNode&);

  // Description:
  // Initializes Text as  well as attributes
  // void ResetAnnotations(); 

  // Description:
  // Only initializes attributes with ID
  void ResetAttributes(int id);
  // Description:
  // Initializes all attributes 
  void ResetTextAttributesAll(); 
  void SetAttributeSize(vtkIdType  id, vtkIdType n);

  void CreatePolyData();


  vtkStringArray *TextList;
  char *ReferenceNodeID; 

  int Visible;
  int Locked;

  vtkMRMLAnnotationNode * m_Backup;

  vtkMRMLSliceNode* m_RedSliceNode;
  vtkMRMLSliceNode* m_YellowSliceNode;
  vtkMRMLSliceNode* m_GreenSliceNode;
  vtkMRMLCameraNode* m_CameraNode;

};

#endif
