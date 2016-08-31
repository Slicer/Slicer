/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __vtkMRMLTableStorageNode_h
#define __vtkMRMLTableStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for handling Table node storage
///
/// vtkMRMLTableStorageNode allows reading/writing of table node from
/// to comma or tab-separated files.
///
/// If the file extension is .tsv or .txt then it is assumed to be tab-separated.
/// Values in tab-separated files may not contain tabs but may contain any other
/// characters (including commas and quotaion marks).
///
/// If the file extension is .csv then it is assumed to be comma-separated.
/// Values in comma-separated files may not contain quotation marks but may contain
/// any other characters (including commas and tabs).
///
class VTK_MRML_EXPORT vtkMRMLTableStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLTableStorageNode *New();
  vtkTypeMacro(vtkMRMLTableStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "TableStorage";};

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

protected:
  vtkMRMLTableStorageNode();
  ~vtkMRMLTableStorageNode();
  vtkMRMLTableStorageNode(const vtkMRMLTableStorageNode&);
  void operator=(const vtkMRMLTableStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node. Returns 0 on failure.
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node. Returns 0 on failure.
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

};

#endif
