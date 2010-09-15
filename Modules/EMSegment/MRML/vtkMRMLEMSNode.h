#ifndef __vtkMRMLEMSNode_h
#define __vtkMRMLEMSNode_h

#include "vtkMRML.h"
#include "vtkMRMLNode.h"

#include "vtkMRMLEMS.h"
#include "vtkMRMLEMSSegmenterNode.h"

/// The scripted module node is simply a MRMLNode container for
/// an arbitrary keyword value pair map

class VTK_MRMLEMS_EXPORT vtkMRMLEMSNode : 
  public vtkMRMLNode
{
public:
  static vtkMRMLEMSNode *New();
  vtkTypeMacro(vtkMRMLEMSNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "EMS";}

  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);
  
  vtkGetStringMacro (SegmenterNodeID);
  vtkSetReferenceStringMacro (SegmenterNodeID);
  vtkMRMLEMSSegmenterNode* GetSegmenterNode();

  vtkGetStringMacro(TemplateFilename);
  vtkSetStringMacro(TemplateFilename);

  vtkGetMacro(SaveTemplateAfterSegmentation, int);
  vtkSetMacro(SaveTemplateAfterSegmentation, int);

  /// \deprecated { These methods are kept for backward compatiblity with
  /// EMSegment KWWidget module and will be removed in SlicerQt}
  vtkGetStringMacro(TclTaskFilename);
  vtkSetStringMacro(TclTaskFilename);
  static const char* GetDefaultTclTaskFilename() { return "GenericTask.tcl"; }

  /// \brief Get the task file associated with this node
  vtkGetStringMacro(PyTaskFilename);
  vtkSetStringMacro(PyTaskFilename);

  vtkGetStringMacro(TaskPreprocessingSetting);
  vtkSetStringMacro(TaskPreprocessingSetting);

protected:
  vtkMRMLEMSNode();
  virtual ~vtkMRMLEMSNode();

  char *SegmenterNodeID;

  int   SaveTemplateAfterSegmentation;

  char* TemplateFilename;

  char* TclTaskFilename; // deprecated

  char* PyTaskFilename;

  char* TaskPreprocessingSetting; 

private:
  vtkMRMLEMSNode(const vtkMRMLEMSNode&); // Not implemented
  void operator=(const vtkMRMLEMSNode&); // Not implemented

};

#endif
