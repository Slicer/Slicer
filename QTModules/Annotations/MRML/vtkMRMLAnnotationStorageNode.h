// .NAME vtkMRMLAnnotationStorageNode - MRML node for representing a volume storage
// .SECTION Description
// vtkMRMLAnnotationStorageNode nodes describe the annotation storage
// node that allows to read/write point data from/to file.

#ifndef __vtkMRMLAnnotationStorageNode_h
#define __vtkMRMLAnnotationStorageNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLStorageNode.h"

class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationNode;

/// \ingroup Slicer_QtModules_Annotation
class  VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLAnnotationStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationStorageNode,vtkMRMLStorageNode);
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
  virtual const char* GetNodeTagName()  {return "AnnotationStorage";};

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


  vtkMRMLAnnotationStorageNode();
  ~vtkMRMLAnnotationStorageNode();
  vtkMRMLAnnotationStorageNode(const vtkMRMLAnnotationStorageNode&);
  void operator=(const vtkMRMLAnnotationStorageNode&);
  void WriteAnnotationDisplayProperties(fstream & of, vtkMRMLAnnotationDisplayNode *refNode, std::string preposition);
  void WriteAnnotationTextDisplayProperties(fstream & of, vtkMRMLAnnotationTextDisplayNode *refNode, std::string preposition);

  int WriteAnnotationTextProperties(fstream & of, vtkMRMLAnnotationNode *refNode);
  void WriteAnnotationData(fstream& of, vtkMRMLAnnotationNode *refNode);
  int OpenFileToWrite(fstream& of);

  // Description:
  // Read data related to vtkMRMLAnnotationDisplayNode
  int ReadAnnotationDisplayProperties(vtkMRMLAnnotationDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition);
  int ReadAnnotationTextDisplayProperties(vtkMRMLAnnotationTextDisplayNode *annotationDisplayNode, std::string lineString, std::string preposition);

  void ReadAnnotationTextData(vtkMRMLAnnotationNode *refNode, char line[1024], int typeColumn, int textColumn,  int selColumn, 
              int visColumn, int numColumns);
  int ReadAnnotationTextProperties(vtkMRMLAnnotationNode *annotationNode, char line[1024], int &typeColumn, int& annotationColumn, int& selColumn, int& visColumn, int& columnNumber);
  // Description:
  // assumes that ResetAnnotations is executed 
  int ReadAnnotation(vtkMRMLAnnotationNode *refNode);
  
  int OpenFileToRead(fstream& of, vtkMRMLNode *refNode);
  const char* GetAnnotationStorageType() { return "text"; } 

  int WriteData(vtkMRMLNode *refNode, fstream &of);

};

#endif



