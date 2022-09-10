// .NAME vtkMRMLAnnotationNode - MRML node to represent a fiber bundle from tractography in DTI data.
// .SECTION Description
// Annotation nodes contains control points, internally represented as vtkPolyData.
// A Annotation node contains many control points  and forms the smallest logical unit of tractography
// that MRML will manage/read/write. Each control point has accompanying data.
// Visualization parameters for these nodes are controlled by the vtkMRMLAnnotationTextDisplayNode class.
//

#ifndef __vtkMRMLAnnotationNode_h
#define __vtkMRMLAnnotationNode_h

// MRML includes
#include "vtkMRMLModelNode.h"
class vtkMRMLCameraNode;
class vtkMRMLSliceNode;

// Annotations includes
#include "vtkSlicerAnnotationsModuleMRMLExport.h"
class vtkMRMLAnnotationTextDisplayNode;

// VTK includes
#include <vtkSmartPointer.h>
class vtkCellArray;
class vtkPoints;
class vtkStringArray;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationNode : public vtkMRMLModelNode
{
public:
  static vtkMRMLAnnotationNode *New();
  vtkTypeMacro(vtkMRMLAnnotationNode,vtkMRMLModelNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  // Description:
  // Just prints short summary
  virtual void PrintAnnotationInfo(ostream& os, vtkIndent indent, int titleFlag = 1);

  virtual const char* GetIcon() {return "";}

  //--------------------------------------------------------------------------
  // MRMLNode methods
  //--------------------------------------------------------------------------

  vtkMRMLNode* CreateNodeInstance() override;
  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Annotation";}

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;


  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLAnnotationNode);

  void UpdateScene(vtkMRMLScene *scene) override;

  // Description:
  // alternative method to propagate events generated in Display nodes
  void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ ) override;

  // vtkMRMLModelNode overrides it and it handles models only, while in annotations
  // we have all kinds of nodes (e.g., screenshot), so we need to revert to the generic
  // storable node implementation.
  std::string GetDefaultStorageNodeClassName(const char* filename /* =nullptr */) override;

  // Description:
  // Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  /// Returns false since in general cannot apply non linear transforms
  /// \sa ApplyTransformMatrix, ApplyTransform
  bool CanApplyNonLinearTransforms()const override {return false;}

  int AddText(const char *newText,int selectedFlag, int visibleFlag);
  void SetText(int id, const char *newText,int selectedFlag, int visibleFlag);
  vtkStdString GetText(int id);
  int DeleteText(int id);

  int GetNumberOfTexts();

  enum
  {
    TEXT_SELECTED = 0,
    TEXT_VISIBLE,
    NUM_TEXT_ATTRIBUTE_TYPES,
     LockModifiedEvent,
  };
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
  void SetTextScale(double textScale);
  /// Get the text scale of the associated text.
  double GetTextScale();


  // Description:
  // Reference of this annotation - can be an image, model, scene ,  ...
  vtkGetStringMacro (ReferenceNodeID);
  vtkSetStringMacro (ReferenceNodeID);

  /// If set to 1 then parameters should not be changed.
  /// Fires vtkMRMLAnnotationNode::LockModifiedEvent if changed except if
  /// GetDisableModifiedEvent() is true.
  vtkGetMacro(Locked, int);
  void SetLocked(int init);

  virtual void Initialize(vtkMRMLScene* mrmlScene);

protected:
  vtkMRMLAnnotationNode();
  ~vtkMRMLAnnotationNode() override;
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
  vtkPoints* GetPoints();
  vtkCellArray* GetLines();

  vtkStringArray *TextList;
  char *ReferenceNodeID;

  int Locked;

  vtkMRMLAnnotationNode * m_Backup;

  vtkSmartPointer<vtkMRMLSliceNode> m_RedSliceNode;
  vtkSmartPointer<vtkMRMLSliceNode> m_YellowSliceNode;
  vtkSmartPointer<vtkMRMLSliceNode> m_GreenSliceNode;
  vtkSmartPointer<vtkMRMLCameraNode> m_CameraNode;

};

#endif
