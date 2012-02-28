// .NAME vtkMRMLAnnotationHierarchyStorageNode - MRML node for representing a hierarchy storage
// .SECTION Description
// vtkMRMLAnnotationHierarchyStorageNode nodes describe the annotation
// hierarchy storage node that writes multiple annotations to one file

#ifndef __vtkMRMLAnnotationHierarchyStorageNode_h
#define __vtkMRMLAnnotationHierarchyStorageNode_h

#include <fstream>

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLStorageNode.h"

class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationHierarchyStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLAnnotationHierarchyStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationHierarchyStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

   // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual int ReadData(vtkMRMLNode *refNode);

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual int WriteData(vtkMRMLNode *refNode);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

 // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "AnnotationHierarchyStorage";};

  // Description:
  // Check to see if this storage node can handle the file type in the input
  // string. If input string is null, check URI, then check FileName. 
  // Subclasses should implement this method.
  virtual int SupportedFileType(const char *fileName);

  // Description:
  // Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  // Description:
  // Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension()
    {
    return "acsv";
    };

protected:


  vtkMRMLAnnotationHierarchyStorageNode();
  ~vtkMRMLAnnotationHierarchyStorageNode();
  vtkMRMLAnnotationHierarchyStorageNode(const vtkMRMLAnnotationHierarchyStorageNode&);
  void operator=(const vtkMRMLAnnotationHierarchyStorageNode&);

};

#endif



