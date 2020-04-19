/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkMRMLLinearTransformSequenceStorageNode - MRML node that can read/write
///  a Sequence node containing linear transforms in a single nrrd or mha file
///

#ifndef __vtkMRMLLinearTransformSequenceStorageNode_h
#define __vtkMRMLLinearTransformSequenceStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLNRRDStorageNode.h"

#include <deque>

class vtkMRMLSequenceNode;

enum SequenceFileType
  {
  INVALID_SEQUENCE_FILE,
  METAIMAGE_SEQUENCE_FILE,
  NRRD_SEQUENCE_FILE
  };

/// \ingroup Slicer_QtModules_Sequences
class VTK_MRML_EXPORT vtkMRMLLinearTransformSequenceStorageNode : public vtkMRMLNRRDStorageNode
{
  public:

  static vtkMRMLLinearTransformSequenceStorageNode *New();
  vtkTypeMacro(vtkMRMLLinearTransformSequenceStorageNode,vtkMRMLNRRDStorageNode);

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "LinearTransformSequenceStorage";};

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

  /// Return true if the node can be written by using thie writer.
  bool CanWriteFromReferenceNode(vtkMRMLNode* refNode) override;
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  ///
  /// Return a default file extension for writting
  const char* GetDefaultWriteFileExtension() override;

  /// Read all the fields in the metaimage file header.
  /// If sequence nodes are passed in createdNodes then they will be reused. New sequence nodes will be created if there are more transforms
  /// in the sequence metafile than pointers in createdNodes. The caller is responsible for deleting all nodes in createdNodes.
  /// Return number of created transform nodes.
  static int ReadSequenceFileTransforms(const std::string& fileName, vtkMRMLScene *scene,
    std::deque< vtkSmartPointer<vtkMRMLSequenceNode> > &createdNodes, std::map< int, std::string >& frameNumberToIndexValueMap,
    std::map< std::string, std::string > &imageMetaData, SequenceFileType fileType = METAIMAGE_SEQUENCE_FILE);

  /// Write the transform fields to the metaimage header
  static bool WriteSequenceMetafileTransforms(const std::string& fileName, std::deque< vtkMRMLSequenceNode* > &transformNodes,
    std::deque< std::string > &transformNames, vtkMRMLSequenceNode* masterNode, vtkMRMLSequenceNode* imageNode);

protected:
  vtkMRMLLinearTransformSequenceStorageNode();
  ~vtkMRMLLinearTransformSequenceStorageNode() override;
  vtkMRMLLinearTransformSequenceStorageNode(const vtkMRMLLinearTransformSequenceStorageNode&);
  void operator=(const vtkMRMLLinearTransformSequenceStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;
};

#endif
