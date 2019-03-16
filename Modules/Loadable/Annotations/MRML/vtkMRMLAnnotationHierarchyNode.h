// .NAME vtkMRMLAnnotationHierarchyNode - MRML node to represent hierarchy of Annotations.
// .SECTION Description
// n/a
//

#ifndef __vtkMRMLAnnotationHierarchyNode_h
#define __vtkMRMLAnnotationHierarchyNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLDisplayableHierarchyNode.h"

class vtkAbstractTransform;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationHierarchyNode : public vtkMRMLDisplayableHierarchyNode
{
public:
  static vtkMRMLAnnotationHierarchyNode *New();
  vtkTypeMacro(vtkMRMLAnnotationHierarchyNode,vtkMRMLDisplayableHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  virtual const char* GetIcon() {return ":/Icons/Medium/SlicerHierarchy.png";};

  // Description:
  // Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  // Description:
  // Get node XML tag name (like Volume, Annotation)
  const char* GetNodeTagName() override;

  // Description:
  // Get all top level children associated to this node.
  virtual void GetDirectChildren(vtkCollection *children);

  /// Add into \a children all the children of the hierarchy node.
  /// \sa GetDirectChildren(), GetChildren()
  virtual void GetAllChildren(vtkCollection *children);

  /// Add into \a children all children of the hierarchy node of the first
  /// \a level nodes.
  /// If \a level is <0, then all levels are added.
  /// \sa GetAllChildren(), GetDirectChildren()
  virtual void GetChildren(vtkCollection *children, int level);

  // Description:
  // Delete all children of this node
  // If a child is another hierarchyNode, the parent of it gets set to this' parent
  virtual void DeleteDirectChildren();

  /// From Transformable superclass
  virtual bool CanApplyNonLinearTransforms()const;
  virtual void ApplyTransform(vtkAbstractTransform* transform);

protected:
  vtkMRMLAnnotationHierarchyNode();
  ~vtkMRMLAnnotationHierarchyNode() override;
  vtkMRMLAnnotationHierarchyNode(const vtkMRMLAnnotationHierarchyNode&);
  void operator=(const vtkMRMLAnnotationHierarchyNode&);

};

#endif
