/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

/// Qt includes
#include <QDebug>
#include <QDir>

/// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerSlicer2SceneReader.h"
#include "vtkSlicerApplicationLogic.h"

/// MRML includes
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLFiducialListNode.h>
#include <vtkMRMLFolderDisplayNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLSubjectHierarchyNode.h>

/// VTK includes
#include <vtkImageReader.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkXMLDataParser.h>

//-----------------------------------------------------------------------------
namespace
{
int dataType(QString type)
{
  type.toLower().remove(' ');
  if (type == "void")
    {
    return VTK_VOID;
    }
  else if (type == "bit")
    {
    return VTK_BIT;
    }
  else if (type == "char")
    {
    return VTK_CHAR;
    }
  else if (type == "signedchar")
    {
    return VTK_SIGNED_CHAR;
    }
  else if (type == "unsignedchar")
    {
    return VTK_UNSIGNED_CHAR;
    }
  else if (type == "short")
    {
    return VTK_SHORT;
    }
  else if (type == "unsignedshort")
    {
    return VTK_UNSIGNED_SHORT;
    }
  else if (type == "int")
    {
    return VTK_INT;
    }
  else if (type == "unsignedint")
    {
    return VTK_UNSIGNED_INT;
    }
  else if (type == "long")
    {
    return VTK_LONG;
    }
  else if (type == "unsignedlong")
    {
    return VTK_UNSIGNED_LONG;
    }
  else if (type == "longlong")
    {
    return VTK_LONG_LONG;
    }
  else if (type == "unsignedlonglong")
    {
    return VTK_UNSIGNED_LONG_LONG;
    }
  else if (type == "float")
    {
    return VTK_FLOAT;
    }
  else if (type == "double")
    {
    return VTK_DOUBLE;
    }
  return VTK_VOID;
}

}

//-----------------------------------------------------------------------------
class qSlicerSlicer2SceneReaderPrivate
{
  Q_DECLARE_PUBLIC(qSlicerSlicer2SceneReader);
protected:
  qSlicerSlicer2SceneReader* const q_ptr;
public:
  qSlicerSlicer2SceneReaderPrivate(qSlicerSlicer2SceneReader& object);

  vtkSmartPointer<vtkSlicerApplicationLogic> ApplicationLogic;
  ///
  typedef QMap<QString, QString> NodeType;

  void importElement(vtkXMLDataElement* element);
  void importNode(vtkXMLDataElement* node);
  void importMRMLNode(NodeType& node);
  void importTransformNode(NodeType& node);
  void importMatrixNode(NodeType& node);
  void importVolumeNode(NodeType& node);
  void importModelNode(NodeType& node);
  void importHierarchyNode(NodeType& node);
  void importModelGroupNode(NodeType& node);
  void importModelRefNode(NodeType& node);
  void importFiducialsNode(NodeType& node);
  void importPointNode(NodeType& node);
  void importColorNode(NodeType& node);
  void importOptionsNode(NodeType& node);

  QString                  Directory;
  vtkMRMLFiducialListNode* FiducialListNode;
  QMap<int, QString>       ModelIDs;
  QString                  ParentID;
  QStringList              TransformIDStack;
  QStringList              LoadedNodes;
protected:
  qSlicerCoreIOManager* ioManager()const;
};

//-----------------------------------------------------------------------------
qSlicerSlicer2SceneReaderPrivate::qSlicerSlicer2SceneReaderPrivate(qSlicerSlicer2SceneReader& object)
  :q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager* qSlicerSlicer2SceneReaderPrivate::ioManager()const
{
  return qSlicerCoreApplication::application()->coreIOManager();
}

//-----------------------------------------------------------------------------
// recursive routine to import all elements and their
// nested parts
void qSlicerSlicer2SceneReaderPrivate::importElement(vtkXMLDataElement* element)
{
  // save current parent locally
  // set parent $::S2_HParent_ID
  QString parent = this->ParentID;

  // import this element
  //ImportNode $element
  this->importNode(element);

  // leave a place holder in case we are a group (transform) node
  //lappend ::S2(transformIDStack) "NestingMarker"
  this->TransformIDStack << "NestingMarker";

  // process all the sub nodes, which may include a sequence of matrices
  // and/or nested transforms
  //set nNested [$element GetNumberOfNestedElements]
  int childrenCount = element->GetNumberOfNestedElements();
  //for {set i 0} {$i < $nNested} {incr i} {
  for (int i = 0; i < childrenCount; ++i)
    {
    // set nestElement [$element GetNestedElement $i]
    vtkXMLDataElement* child = element->GetNestedElement(i);
    //ImportElement $nestElement
    this->importElement(child);
    }

  // strip away any accumulated transform ids
  //while { $::S2(transformIDStack) != "" && [lindex $::S2(transformIDStack) end] != "NestingMarker" } {
  //  set ::S2(transformIDStack) [lrange $::S2(transformIDStack) 0 end-1]
  //}
  // strip away the nesting marker
  //set ::S2(transformIDStack) [lrange $::S2(transformIDStack) 0 end-1]
  int nestingMarkerIndex = this->TransformIDStack.lastIndexOf("NestingMarker");
  if (nestingMarkerIndex > 0)
    {
    QStringList::iterator nestingMarkerIterator = this->TransformIDStack.begin();
    nestingMarkerIterator += nestingMarkerIndex;
    this->TransformIDStack.erase(nestingMarkerIterator, this->TransformIDStack.end());
    }

  // restore parent locally
  //set ::S2_HParent_ID $parent
  this->ParentID = parent;
}

//
// parse the attributes of a node into a map
// and then invoke the type-specific handler
//
//-----------------------------------------------------------------------------
void qSlicerSlicer2SceneReaderPrivate::importNode(vtkXMLDataElement* element)
{
  NodeType node;
  //set nAtts [$element GetNumberOfAttributes]
  int nAtts = element->GetNumberOfAttributes();
  //for {set i 0} {$i < $nAtts} {incr i} {
  for (int i = 0; i < nAtts; ++i)
    {
    //set attName [$element GetAttributeName $i]
    QString attName = element->GetAttributeName(i);
    //set node($attName) [$element GetAttributeValue $i]
    node[attName] = element->GetAttributeValue(i);
  }

  // call the handler for this element
  //$handler node
  QString name = element->GetName();
  if (name == "MRML")
    {
    this->importMRMLNode(node);
    }
  else if (name == "Transform")
    {
    this->importTransformNode(node);
    }
  else if (name == "Matrix")
    {
    this->importMatrixNode(node);
    }
  else if (name == "Volume")
    {
    this->importVolumeNode(node);
    }
  else if (name == "Model")
    {
    this->importModelNode(node);
    }
  else if (name == "Hierarchy")
    {
    this->importHierarchyNode(node);
    }
  else if (name == "ModelGroup")
    {
    this->importModelGroupNode(node);
    }
  else if (name == "ModelRef")
    {
    this->importModelRefNode(node);
    }
  else if (name == "Fiducials")
    {
    this->importFiducialsNode(node);
    }
  else if (name == "Point")
    {
    this->importPointNode(node);
    }
  else if (name == "Color")
    {
    this->importColorNode(node);
    }
  else if (name == "Options")
    {
    this->importOptionsNode(node);
    }
  else
    {
    qDebug() << "Node type unsupported: " << name;
    }
}

//-----------------------------------------------------------------------------
//proc ImportNodeMRML {node} {
void qSlicerSlicer2SceneReaderPrivate::importMRMLNode(NodeType& node)
{
  Q_UNUSED(node);
  // no op, just a marker
}

//-----------------------------------------------------------------------------
//proc ImportNodeTransform {node} {
void qSlicerSlicer2SceneReaderPrivate::importTransformNode(NodeType& node)
{
  Q_UNUSED(node);
  // no op - handled by ImportElement
}

//-----------------------------------------------------------------------------
//
// a slicer2 matrix corresponds to a slicer3 Transform
//
//proc ImportNodeMatrix {node} {
void qSlicerSlicer2SceneReaderPrivate::importMatrixNode(NodeType& node)
{
  Q_Q(qSlicerSlicer2SceneReader);
  //upvar $node n
  //set transformNode [vtkMRMLLinearTransformNode New]
  vtkNew<vtkMRMLLinearTransformNode> transformNode;

  //set matrix [$transformNode GetMatrixTransformToParent]
  //if { [info exists n(name)] } {
  //  $transformNode SetName $n(name)
  //} else {
  //  $transformNode SetName "Imported Transform"
  //}
  if (node.contains("name"))
    {
    transformNode->SetName(node["name"].toUtf8());
    }
  else
    {
    transformNode->SetName("Imported Transform");
    }
  //eval $matrix DeepCopy $n(matrix)
  double elements[16];
  int i = 0;
  foreach(QString element, node["matrix"].split(' '))
    {
    elements[i++] = element.toDouble();
    }
  vtkNew<vtkMatrix4x4> matrix;
  matrix->DeepCopy(elements);
  transformNode->SetMatrixTransformToParent(matrix.GetPointer());


  //$::slicer3::MRMLScene AddNode $transformNode
  q->mrmlScene()->AddNode(transformNode.GetPointer());
  this->LoadedNodes << transformNode->GetID();

  //set parentTransform ""
  QString parentTransform;
  //set index [expr [llength $::S2(transformIDStack)] - 1]
  //for {} { $index > 0 } { incr index -1 } {
  //  set element [lindex $::S2(transformIDStack) $index]
  //  if { $element != "NestingMarker" } {
  //    set parentTransform $element
  //    break
  //  }
  //}
  for (int i = this->TransformIDStack.count() -1; i >= 0; --i)
    {
    if (this->TransformIDStack[i] != "NestingMarker")
      {
      parentTransform = this->TransformIDStack[i];
      break;
      }
    }
  //$transformNode SetAndObserveTransformNodeID $parentTransform
  transformNode->SetAndObserveTransformNodeID(parentTransform.toUtf8());

  //lappend ::S2(transformIDStack) [$transformNode GetID]
  this->TransformIDStack << transformNode->GetID();

  //$transformNode Delete
}

//-----------------------------------------------------------------------------
//proc ImportNodeVolume {node} {
void qSlicerSlicer2SceneReaderPrivate::importVolumeNode(NodeType& node)
{
  Q_Q(qSlicerSlicer2SceneReader);
  //upvar $node n

  //if { ![info exists n(fileType)] } {
  if (!node.contains("fileType"))
    {
    //set n(fileType) "Basic"
    node["fileType"] = "Basic";
    }

  //if { ![info exists n(name)] } {
  if (!node.contains("name"))
    {
    //set n(name) "Imported Volume"
    node["name"] = "Imported Volume";
    }

  //if { ![info exists n(description)] } {
  if (!node.contains("description"))
    {
    //set n(description) ""
    node["description"] = "";
    }

  vtkSmartPointer<vtkMRMLVolumeNode> volumeNode;
  QString volumeNodeID;
  vtkSmartPointer<vtkMRMLVolumeDisplayNode> volumeDisplayNode;

  // set labelMap 0
  //   if { [info exists n(labelMap)] && ($n(labelMap) == "yes"  || $n(labelMap) == "true") } {
  //       set labelMap 1
  //   }
  bool labelMap = node.contains("labelMap") &&
    (node["labelMap"] == "yes" ||
     node["labelMap"] == "true");

  //switch [string tolower $n(fileType)] {
  //  "nrrd" -
  //  "generic" {
  if (node["fileType"].toLower() == "nrrd" ||
      node["fileType"].toLower() == "generic")
    {
    //if { ![info exists n(fileName)] } {
    if (!node.contains("fileName"))
      {
      //set n(fileName) [format $n(filePattern) $n(filePrefix)]
      node["fileName"].sprintf(
        node["filePattern"].toUtf8(),
        node["filePrefix"].toUtf8().data());
      }

    //if { [file pathtype $n(fileName)] == "relative" } {
    //set fileName $::S2(dir)/$n(fileName)
    //  } else {
    //    set fileName $n(fileName)
    //  }
    QString fileName;
    QFileInfo fileInfo(node["fileName"]);
    if (fileInfo.isRelative())
      {
      fileInfo = QFileInfo(QDir(this->Directory), node["fileName"]);
      fileName = fileInfo.absoluteFilePath();
      }
    else
      {
      fileName = node["fileName"];
      }

    //set logic [$::slicer3::VolumesGUI GetLogic]
    //set loadingOptions $labelMap
    //set volumeNode [$logic AddArchetypeVolume $fileName $n(name) $loadingOptions ""]
    //set volumeNodeID [$volumeNode GetID]
    //int loadingOptions = labelMap ? 1 : 0;
    // volumeNode = volumesLogic()->AddArchetypeVolume(
    //   fileName.toUtf8(),
    //   node["name"].toUtf8(), loadingOptions, 0);
    qSlicerIO::IOProperties properties;
    properties["name"] = node["name"];
    properties["fileName"] = fileName;
    properties["labelMap"] = labelMap;
    volumeNode = vtkMRMLVolumeNode::SafeDownCast(
      this->ioManager()->loadNodesAndGetFirst(QString("VolumeFile"), properties));
    volumeNodeID = volumeNode->GetID();
    this->LoadedNodes << volumeNode->GetID();
    }
  // "dicom" {
  else if( node["fileType"].toLower() == "dicom")
    {
    //set fileList [vtkStringArray New]
    QStringList fileNames;
    vtkNew<vtkStringArray> fileList;
    //foreach f $n(dicomFileNameList) {
    foreach(QString file, node["dicomFileNameList"].split(' '))
      {
      //if { [file pathtype $f] == "relative" } {
      //   set fileName $::S2(dir)/$f
      // } else {
      //   set fileName $f
      // }
      QString fileName;
      QFileInfo fileInfo(file);
      if (fileInfo.isRelative())
        {
        fileInfo = QFileInfo(this->Directory, file);
        fileName = fileInfo.absoluteFilePath();
        }
      else
        {
        fileName = file;
        }
      //$fileList InsertNextValue $fileName
      fileList->InsertNextValue(fileName.toUtf8());
      fileNames << fileName;
      }
    //set fileName [$fileList GetValue 0]
    //set logic [$::slicer3::VolumesGUI GetLogic]
    //set loadingOptions 0
    //set volumeNode [$logic AddArchetypeVolume $fileName $n(name) $loadingOptions $fileList]
    //set volumeNodeID [$volumeNode GetID]
    //$fileList Delete
    // volumeNode = volumesLogic()->AddArchetypeVolume(
    //   fileList->GetValue(0).c_str(),
    //   node["name"].toUtf8(),
    //   0, fileList);
    qSlicerIO::IOProperties properties;
    properties["name"] = node["name"];
    properties["fileName"] = fileNames[0];
    properties["fileNames"] = fileNames;
    volumeNode = vtkMRMLVolumeNode::SafeDownCast(
      this->ioManager()->loadNodesAndGetFirst(QString("VolumeFile"), properties));
    Q_ASSERT(volumeNode.GetPointer());
    volumeNodeID = volumeNode->GetID();
    this->LoadedNodes << volumeNode->GetID();
    }
  //  "headerless" -
  //  "basic" {
  else if (node["fileType"].toLower() == "headerless" ||
           node["fileType"].toLower() == "basic")
    {
    //
    // first, parse the slicer2 node
    //
    //if { ![info exists n(dimensions)] } {
    if (!node.contains("dimensions"))
      {
      //set n(dimensions) "256 256"
      node["dimensions"] = "256 256";
      }

    //if { ![info exists n(scalarType)] } {
    if (!node.contains("scalarType"))
      {
      //set n(scalarType) "Short"
      node["scalarType"] = "Short";
      }

    //if { ![info exists n(littleEndian)] } {
    if (!node.contains("littleEndian"))
      {
      //set n(littleEndian) "false"
      node["littleEndian"] = "false";
      }

    //if { $n(littleEndian) } {
    //  set fileLittleEndian 1
    //} else {
    //  set fileLittleEndian 0
    //}
    bool fileLittleEndian = node["littleEndian"] == "true";

    // if { $::tcl_platform(byteOrder) == "littleEndian" } {
    //     set platformLittleEndian 1
    //   } else {
    //     set platformLittleEndian 0
    //   }
#if Q_BYTE_ORDER == Q_LITTLE_ENDIAN
    bool platformLittleEndian = true;
#else
    bool platformLittleEndian = false;
#endif

    //if { $fileLittleEndian != $platformLittleEndian } {
    //   set swap 1
    // } else {
    //   set swap 0
    // }
    bool swap = fileLittleEndian != platformLittleEndian;

    //
    // next, read the image data
    //
    //set imageReader [vtkImageReader New]
    vtkNew<vtkImageReader> imageReader;

    // if { [file pathtype $n(filePrefix)] == "relative" } {
    //   $imageReader SetFilePrefix  $::S2(dir)/$n(filePrefix)
    //   } else {
    //   $imageReader SetFilePrefix  $n(filePrefix)
    //   }
    QFileInfo fileInfo(node["filePrefix"]);
    if (fileInfo.isRelative())
      {
      fileInfo = QFileInfo(this->Directory, node["filePrefix"]);
      imageReader->SetFilePrefix(fileInfo.absoluteFilePath().toUtf8());
      }
    else
      {
      imageReader->SetFilePrefix(node["filePrefix"].toUtf8());
      }
    //$imageReader SetFilePattern  $n(filePattern)
    imageReader->SetFilePattern(node["filePrefix"].toUtf8());

    //foreach {w h} $n(dimensions) {}
    QStringList dimensions = node["dimensions"].split(' ');
    int width = dimensions[0].toInt();
    int height = dimensions[1].toInt();
    //  foreach {zlo zhi} $n(imageRange) {}
    QStringList imageRange = node["imageRange"].split(' ');
    int zlo = imageRange[0].toInt();
    int zhi = imageRange[1].toInt();
    //set d [expr $zhi - $zlo]
    int d = zhi - zlo;

    //$imageReader SetDataExtent 0 [expr $w -1] 0 [expr $h - 1] 0 [expr $d -1]
    //$imageReader SetFileNameSliceOffset $zlo
    //$imageReader SetDataScalarTypeTo$n(scalarType)
    //$imageReader SetSwapBytes $swap
    //$imageReader Update
    imageReader->SetDataExtent(0, width-1, 0, height-1, 0, d-1);
    imageReader->SetFileNameSliceOffset(zlo);
    imageReader->SetDataScalarType(dataType(node["scalarType"]));
    imageReader->SetSwapBytes(swap);
    imageReader->Update();

    //
    // now, construct the slicer3 node
    // - volume
    // - transform
    // - display
    //

    //set volumeNode [vtkMRMLScalarVolumeNode New]
      // $volumeNode SetAndObserveImageData [$imageReader GetOutput]
      // $volumeNode SetName $n(name)
      // $volumeNode SetDescription $n(description)
      // $imageReader Delete
    volumeNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
    volumeNode->SetAndObserveImageData(imageReader->GetOutput());
    volumeNode->SetName(node["name"].toUtf8());
    volumeNode->SetDescription(node["description"].toUtf8());

    // if { [info exists n(labelMap)] && ($n(labelMap) == "yes"  || $n(labelMap) == "true") } {
    //       $volumeNode SetLabelMap 1
    //       set volumeDisplayNode [vtkMRMLLabelMapVolumeDisplayNode New]
    //   } else {
    //       set volumeDisplayNode [vtkMRMLScalarVolumeDisplayNode New]
    //   }
    if (labelMap)
      {
      volumeDisplayNode = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
      }
    else
      {
      volumeDisplayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
      }

    //
    // add nodes to the scene
    //

    //$::slicer3::MRMLScene AddNode $volumeDisplayNode
    //$::slicer3::MRMLScene AddNode $volumeNode
    //$volumeNode SetAndObserveDisplayNodeID [$volumeDisplayNode GetID]
    //$volumeNode SetModifiedSinceRead 1
    q->mrmlScene()->AddNode(volumeNode.GetPointer());
    q->mrmlScene()->AddNode(volumeDisplayNode);
    volumeNode->SetAndObserveDisplayNodeID(volumeDisplayNode->GetID());
    this->LoadedNodes << volumeNode->GetID() << volumeDisplayNode->GetID();

    // use the RASToIJK information from the file, to override what the
    // archetype reader might have set
    //set rasToVTK [vtkMatrix4x4 New]
    //  eval $rasToVTK DeepCopy $n(rasToVtkMatrix)
    //  $volumeNode SetRASToIJKMatrix $rasToVTK
    //  $rasToVTK Delete
    vtkNew<vtkMatrix4x4> rasToVTK;
    double elements[16];
    int i = 0;
    foreach(QString element, node["rasToVtkMatrix"].split(' '))
      {
      elements[i++] = element.toDouble();
      }
    rasToVTK->DeepCopy(elements);
    volumeNode->SetRASToIJKMatrix(rasToVTK.GetPointer());

    //
    // clean up
    //
    //set volumeNodeID [$volumeNode GetID]
    volumeNodeID = volumeNode->GetID();
    }

  //set volumeNode [$::slicer3::MRMLScene GetNodeByID $volumeNodeID]
  Q_ASSERT(volumeNode.GetPointer() == q->mrmlScene()->GetNodeByID(volumeNodeID.toUtf8()));
  volumeNode = vtkMRMLVolumeNode::SafeDownCast(q->mrmlScene()->GetNodeByID(volumeNodeID.toUtf8()));
  Q_ASSERT(volumeNode.GetPointer());

  // use the current top of stack (might be "" if empty, but that's okay)
  //set transformID [lindex $::S2(transformIDStack) end]
  //$volumeNode SetAndObserveTransformNodeID $transformID
  int count = this->TransformIDStack.count();
  QString transformID =  count? this->TransformIDStack[count -1] : QString();
  volumeNode->SetAndObserveTransformNodeID(transformID.toUtf8());

  //set volumeDisplayNode [$volumeNode GetDisplayNode]
  volumeDisplayNode = vtkMRMLVolumeDisplayNode::SafeDownCast(volumeNode->GetDisplayNode());
  Q_ASSERT(volumeDisplayNode.GetPointer());

  // switch -- $n(colorLUT) {
  //   "0" {
  //     $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
  //   }
  //   "-1" {
  //     $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeLabels"
  //   }
  //   default {
  //     $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
  //   }
  // }
  if (node["colorLUT"] == "-1")
    {
    volumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeLabels");
    }
  else
    {
    volumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
    }

  //if { [info exists n(applyThreshold)] && ( $n(applyThreshold) == "yes" || $n(applyThreshold) == "true" ) } {
  //  if { [$volumeDisplayNode IsA "vtkMRMLScalarVolumeDisplayNode"] } {
  //    $volumeDisplayNode SetApplyThreshold 1
  //  } else {
  //    puts "Slicer2Import.tcl: ImportNodeVolume Can't set apply threshold, $volumeDisplayNode is a [$volumeDisplayNode GetClassName] rather than a vtkMRMLScalarVolumeDisplayNode"
  //  }
  //}
  if (node.contains("applyThreshold") &&
      (node["applyThreshold"] == "yes" ||
       node["applyThreshold"] == "true"))
    {
    if (volumeDisplayNode->IsA("vtkMRMLScalarVolumeDisplayNode"))
      {
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeDisplayNode)->SetApplyThreshold(1);
      }
    else
      {
      qDebug() << "qSlicerSlicer2SceneReader: Can't set apply threshold, "
               << "volumeDisplayNode is a " << volumeDisplayNode->GetClassName()
               << " rather than a vtkMRMLScalarVolumeDisplayNode";
      }
    }

  // if { [$volumeDisplayNode IsA "vtkMRMLScalarVolumeDisplayNode"] } {
  //   $volumeDisplayNode SetWindow $n(window)
  //   $volumeDisplayNode SetLevel $n(level)
  //   $volumeDisplayNode SetLowerThreshold $n(lowerThreshold)
  //   $volumeDisplayNode SetUpperThreshold $n(upperThreshold)
  //   }
  if (volumeDisplayNode->IsA("vtkMRMLScalarVolumeDisplayNode"))
    {
    vtkMRMLScalarVolumeDisplayNode* scalarDisplayNode=
      vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeDisplayNode);
    scalarDisplayNode->SetWindow(node["window"].toDouble());
    scalarDisplayNode->SetLevel(node["level"].toDouble());
    scalarDisplayNode->SetLowerThreshold(node["lowerThreshold"].toDouble());
    scalarDisplayNode->SetUpperThreshold(node["upperThreshold"].toDouble());
    }

  //set logic [$::slicer3::VolumesGUI GetLogic]
  //$logic SetActiveVolumeNode $volumeNode
  //volumesLogic()->SetActiveVolumeNode(volumeNode);

  //[[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetActiveVolumeID [$volumeNode GetID]
  //[$::slicer3::VolumesGUI GetApplicationLogic] PropagateVolumeSelection
  this->ApplicationLogic->GetSelectionNode()
    ->SetActiveVolumeID(volumeNodeID.toUtf8());
  this->ApplicationLogic->PropagateVolumeSelection();
}

//-----------------------------------------------------------------------------
//proc ImportNodeModel {node} {
void qSlicerSlicer2SceneReaderPrivate::importModelNode(NodeType& node)
{
  //upvar $node n

  // if { [file pathtype $n(fileName)] == "relative" } {
  //   set fileName $::S2(dir)/$n(fileName)
  // } else {
  //   set fileName $n(fileName)
  // }
  QString fileName;
  QFileInfo fileInfo(node["fileName"]);
  if (fileInfo.isRelative())
    {
    fileInfo = QFileInfo(this->Directory, node["fileName"]);
    fileName = fileInfo.absoluteFilePath();
    }
  else
    {
    fileName = node["fileName"];
    }

  // set logic [$::slicer3::ModelsGUI GetLogic]
  // set mnode [$logic AddModel $fileName]
  // set dnode [$mnode GetDisplayNode]
  qSlicerIO::IOProperties properties;
  properties["name"] = node["name"];
  properties["fileName"] = fileName;
  vtkMRMLModelNode* mnode =
    //modelsLogic()->AddModel(fileName.toUtf8());
    vtkMRMLModelNode::SafeDownCast(this->ioManager()->loadNodesAndGetFirst(QString("ModelFile"), properties));
  Q_ASSERT(mnode);
  vtkMRMLDisplayNode* dnode =
    mnode->GetDisplayNode();
  Q_ASSERT(dnode);
  this->LoadedNodes << mnode->GetID() << dnode->GetID();

  // if { ![info exists n(id)] } {
  //   // model node has no id, so create one
  //   // - try to get a high number that isn't already used
  //   for {set i 1000} {$i < 1000000} {incr i} {
  //     if { [lsearch [array names ::S2_Model_ID] $i] == -1 } {
  //       set n(id) $i
  //       break
  //     }
  //   }
  // }
  if (!node.contains("id"))
    {
    //model node has no id, so create one
    // try to get a high number that isn't already used
    for (int i = 1000; i < 1000000; ++i)
      {
      if (!this->ModelIDs.contains(i))
        {
        node["id"] = QString::number(i);
        }
      }
    }
  //set ::S2_Model_ID($n(id)) [$mnode GetID]
  this->ModelIDs[node["id"].toInt()] = mnode->GetID();

  // if { [info exists n(visibility)] } {
  //   if {$n(visibility) == "false"} {
  //     $dnode SetVisibility 0
  //   } else {
  //     $dnode SetVisibility 1
  //   }
  // }
  if (node.contains("visibility"))
    {
    bool visibility = node["visibility"] != "false";
    dnode->SetVisibility(visibility);
    }

  //if { [info exists n(color)] } {
  if (node.contains("color"))
    {

    // if { [string tolower $n(color)] == "skin" } {
    //   // workaround slicer2 ethnocentrism
    //   set n(color) "peach"
    // }
    if (node["color"].toLower() == "skin")
      {
      // workaround slicer2 ethnocentrism
      node["color"] = "peach";
      }

    //set cnode [vtkMRMLColorTableNode New]
    vtkNew<vtkMRMLColorTableNode> cnode;

    //foreach colorType "SPLBrainAtlas Labels" {
    QStringList colorTypes;
    colorTypes << "SPLBrainAtlas" << "Labels";
    foreach(const QString& colorType, colorTypes)
      {

      //$cnode SetTypeTo$colorType
      if (colorType == "SPLBrainAtlas")
        {
        qDebug() << "FIXME: fix the color type";
        //cnode->SetTypeToSPLBrainAtlas();
        cnode->SetTypeToLabels();
        }
      else
        {
        cnode->SetTypeToLabels();
        }
      //puts "looking for $n(color) in $colorType node $cnode"
      //set saveColor 0
      bool saveColor = false;
      //for {set i 0} {$i < [$cnode GetNumberOfColors]} {incr i} {
      for (int i = 0; i < cnode->GetNumberOfColors(); ++i)
        {
        //set name [$cnode GetColorName $i]
        QString name = cnode->GetColorName(i);
        //if {[string tolower $name] == [string tolower $n(color)]} {
        if (name.toLower() == node["color"].toLower())
          {
          //eval $dnode SetColor [lrange [[$cnode GetLookupTable] GetTableValue $i] 0 2]
          dnode->SetColor( cnode->GetLookupTable()->GetTableValue(i));
          //set saveColor 1
          saveColor = true;
          //puts " found color $i $name"
          //break
          break;
          }
        }
      //if {$saveColor == 0} {
      //  $dnode SetAttribute colorid $n(color)
      //} else {
      //  break
      //}
      if (!saveColor)
        {
        dnode->SetAttribute("colorid", node["color"].toUtf8());
        }
      else
        {
        break;
        }
      }
    //$cnode Delete
  }
}

//-----------------------------------------------------------------------------
//proc ImportNodeHierarchy {node} {
void qSlicerSlicer2SceneReaderPrivate::importHierarchyNode(NodeType& node)
{
  Q_UNUSED(node);
  //set ::S2_HParent_ID ""
  this->ParentID = "";
}

//-----------------------------------------------------------------------------
//proc ImportNodeModelGroup {node} {
void qSlicerSlicer2SceneReaderPrivate::importModelGroupNode(NodeType& node)
{
  Q_Q(qSlicerSlicer2SceneReader);
  //upvar $node n
  //set hnode [vtkMRMLModelHierarchyNode New]
  //set dnode [vtkMRMLModelDisplayNode New]
  //$hnode SetScene $::slicer3::MRMLScene
  //$dnode SetScene $::slicer3::MRMLScene
  // Folder display node belongs to the folder item, which replaced the model hierarchy
  vtkSmartPointer<vtkMRMLFolderDisplayNode> dnode =
    vtkSmartPointer<vtkMRMLFolderDisplayNode>::New();
  dnode->SetScene(q->mrmlScene());
  dnode->SetHideFromEditors(0); // Need to set this so that the folder shows up in SH as folder

  // if { [info exists n(visibility)] } {
  //   if {$n(visibility) == "false"} {
  //     $dnode SetVisibility 0
  //   } else {
  //     $dnode SetVisibility 1
  //   }
  // }
  if (node.contains("visibility"))
    {
    bool visibility = node["visibility"] != "false";
    dnode->SetVisibility(visibility);
    }

  //if { [info exists n(name)] } {
  //  $hnode SetName $n(name)
  //}
  if (node.contains("name"))
    {
    dnode->SetName(node["name"].toUtf8());
    }

  // if { [info exists n(color)] } {
  //     set cnode [$::slicer3::MRMLScene GetNodeByID vtkMRMLColorTableNodeSPLBrainAtlas]
  //     for {set i 0} {$i < [$cnode GetNumberOfColors]} {incr i} {
  //         if {[$cnode GetColorName $i] == $n(color)} {
  //             eval $dnode SetColor [lrange [[$cnode GetLookupTable] GetTableValue $i] 0 2]
  //         }
  //     }
  // }
  vtkMRMLColorTableNode* cnode = nullptr;
  if (node.contains("color"))
    {
    cnode = vtkMRMLColorTableNode::SafeDownCast(
      q->mrmlScene()->GetNodeByID("vtkMRMLColorTableNodeSPLBrainAtlas"));
    Q_ASSERT(cnode);
    for (int i = 0; i < cnode->GetNumberOfColors(); ++i)
      {
      if (cnode->GetColorName(i) == node["color"])
        {
        dnode->SetColor(cnode->GetLookupTable()->GetTableValue(i));
        }
      }
    }
  //set dnode [$::slicer3::MRMLScene AddNode $dnode]
  //set hnode [$::slicer3::MRMLScene AddNode $hnode]
  dnode = vtkMRMLFolderDisplayNode::SafeDownCast(
    q->mrmlScene()->AddNode(dnode));
  Q_ASSERT(dnode);
  this->LoadedNodes << dnode->GetID();

  //if {$::S2_HParent_ID != ""} {
  //  $hnode SetParentNodeIDReference $::S2_HParent_ID
  //}
  if (!this->ParentID.isEmpty())
    {
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(q->mrmlScene());
    if (!shNode)
      {
      qCritical() << Q_FUNC_INFO << "Unable to access subject hierarchy node";
      return;
      }

    vtkMRMLNode* parentNode = q->mrmlScene()->GetNodeByID(this->ParentID.toUtf8());
    if (parentNode)
      {
      vtkIdType parentItemId = shNode->GetItemByDataNode(parentNode);
      vtkIdType folderItemId = shNode->GetItemByDataNode(dnode);
      if (parentItemId && folderItemId)
        {
        shNode->SetItemParent(folderItemId, parentItemId);
        }
      }
    }

  //set ::S2_HParent_ID [$hnode GetID]
  this->ParentID = dnode->GetID();
}

//-----------------------------------------------------------------------------
//proc ImportNodeModelRef {node} {
void qSlicerSlicer2SceneReaderPrivate::importModelRefNode(NodeType& node)
{
  Q_Q(qSlicerSlicer2SceneReader);
  //upvar $node n
  //set hnode [vtkMRMLModelHierarchyNode New]
  //$hnode SetScene $::slicer3::MRMLScene

  // Important! Leaf hierarchy nodes are not needed any more.
  //   Simply set parent folder to the model node in subject hierarchy

  //set id2 $n(ModelRefID)
  //set id3 $::S2_Model_ID($id2)
  int id2 = node["ModeRefID"].toInt();
  QString id3 = this->ModelIDs[id2];

  //$hnode SetName [[$::slicer3::MRMLScene GetNodeByID $id3] GetName]
  vtkMRMLNode* modelNode = q->mrmlScene()->GetNodeByID(id3.toUtf8());
  if (!modelNode)
    {
    qCritical() << Q_FUNC_INFO << "Unable to access referenced model node";
    return;
    }

  //if {$::S2_HParent_ID != ""} {
  //  $hnode SetParentNodeIDReference $::S2_HParent_ID
  //}
  if (!this->ParentID.isEmpty())
    {
    vtkMRMLSubjectHierarchyNode* shNode = vtkMRMLSubjectHierarchyNode::GetSubjectHierarchyNode(q->mrmlScene());
    if (!shNode)
      {
      qCritical() << Q_FUNC_INFO << "Unable to access subject hierarchy node";
      return;
      }

    vtkMRMLNode* parentNode = q->mrmlScene()->GetNodeByID(this->ParentID.toUtf8());
    if (parentNode)
      {
      vtkIdType parentItemId = shNode->GetItemByDataNode(parentNode);
      vtkIdType modelItemId = shNode->GetItemByDataNode(modelNode);
      if (parentItemId && modelItemId)
        {
        shNode->SetItemParent(modelItemId, parentItemId);
        }
      }
    }
}

//-----------------------------------------------------------------------------
//proc ImportNodeFiducials {node} {
void qSlicerSlicer2SceneReaderPrivate::importFiducialsNode(NodeType& node)
{
  Q_Q(qSlicerSlicer2SceneReader);
  //upvar $node n
  //set fiducialNode [vtkMRMLFiducialListNode New]
  vtkSmartPointer<vtkMRMLFiducialListNode> fiducialNode =
    vtkSmartPointer<vtkMRMLFiducialListNode>::New();

  // if { [info exists n(symbolSize)] } {
  //   $fiducialNode SetSymbolScale $n(symbolSize)
  // }
  if (node.contains("symbolSize"))
    {
    fiducialNode->SetSymbolScale(node["symbolSize"].toDouble());
    }
  // if { [info exists n(textSize)] } {
  //   $fiducialNode SetTextScale $n(textSize)
  // }
  if (node.contains("textSize"))
    {
    fiducialNode->SetTextScale(node["textSize"].toDouble());
    }
  // if { [info exists n(visibility)] } {
  //   $fiducialNode SetVisibility $n(visibility)
  // }
  if (node.contains("visibility"))
    {
    fiducialNode->SetVisibility(node["visibility"].toInt());
    }
  //if { [info exists n(color)] } {
  //  foreach {c0 c1 c2} $n(color) {}
  //  $fiducialNode SetColor $c0 $c1 $c2
  //}
  if (node.contains("color"))
    {
    QStringList colors = node["color"].split(' ');
    fiducialNode->SetColor(colors[0].toDouble(),
                           colors[1].toDouble(),
                           colors[2].toDouble());
    }
  // if { [info exists n(name)] } {
  //   $fiducialNode SetName $n(name)
  // }
  if (node.contains("name"))
    {
    fiducialNode->SetName(node["name"].toUtf8());
    }
  //$::slicer3::MRMLScene AddNode $fiducialNode
  q->mrmlScene()->AddNode(fiducialNode);
  this->LoadedNodes << fiducialNode->GetID();
  //set ::S2(fiducialListNode) $fiducialNode
  this->FiducialListNode = fiducialNode.GetPointer();

  // set it to be the selected one, last one imported will stick
  //set selNode [$::slicer3::ApplicationLogic GetSelectionNode]
  vtkMRMLSelectionNode* selNode =
    this->ApplicationLogic->GetSelectionNode();
  //if { $selNode != "" } {
  //   $selNode SetActiveFiducialListID [$fiducialNode GetID]
  //   $::slicer3::ApplicationLogic PropagateFiducialListSelection
  //}
  if (selNode != nullptr)
    {
    selNode->SetActiveFiducialListID(fiducialNode->GetID());
    this->ApplicationLogic->PropagateFiducialListSelection();
    }
}

//-----------------------------------------------------------------------------
//proc ImportNodePoint {node} {
void qSlicerSlicer2SceneReaderPrivate::importPointNode(NodeType& node)
{
  //upvar $node n
  //set f [$::S2(fiducialListNode) AddFiducial]
  Q_ASSERT(this->FiducialListNode);
  int f = this->FiducialListNode->AddFiducial();

  // if { [info exists n(xyz)] } {
  //   foreach {x y z} $n(xyz) {}
  //   $::S2(fiducialListNode) SetNthFiducialXYZ $f $x $y $z
  // }
  if (node.contains("xyz"))
    {
    QStringList xyz = node["xyz"].split(' ');
    this->FiducialListNode->SetNthFiducialXYZ(f,
                                              xyz[0].toFloat(),
                                              xyz[1].toFloat(),
                                              xyz[2].toFloat());
    }
  //if { [info exists n(name)] } {
  //  $::S2(fiducialListNode) SetNthFiducialLabelText $f $n(name)
  //}
  if (node.contains("name"))
    {
    this->FiducialListNode->SetNthFiducialLabelText(f, node["name"].toUtf8());
    }
}

//-----------------------------------------------------------------------------
//proc ImportNodeColor {node} {
void qSlicerSlicer2SceneReaderPrivate::importColorNode(NodeType& node)
{
  Q_Q(qSlicerSlicer2SceneReader);
  //upvar $node n
  //if { [info exists n(name)] } {
  if (!node.contains("name"))
    {
    return;
    }

  //set id $n(name)
  QString id = node["name"];
  //if { [info exists n(diffuseColor)] } {
  if (!node.contains("diffuseColor"))
    {
    return;
    }
  //foreach {r g b} $n(diffuseColor) {}
  QStringList rgb = node["diffuseColor"].split(' ');
  //$::slicer3::MRMLScene InitTraversal
  std::vector<vtkMRMLNode*> nodes;
  q->mrmlScene()->GetNodesByClass("vtkMRMLModelDisplayNode", nodes);
  for (std::vector< vtkMRMLNode* >::iterator nodeIt = nodes.begin(); nodeIt != nodes.end(); ++nodeIt)
    {
    //set dnode [$::slicer3::MRMLScene GetNthNodeByClass $i vtkMRMLModelDisplayNode]
    vtkMRMLModelDisplayNode* dnode =
      vtkMRMLModelDisplayNode::SafeDownCast(*nodeIt);
    // set cid [$dnode GetAttribute colorid]
    QString cid = dnode->GetAttribute("colorid");
    //if {$id == $cid} {
    if (id == cid)
      {
      //$dnode SetColor $r $g $b
      dnode->SetColor(rgb[0].toFloat(),
                      rgb[1].toFloat(),
                      rgb[2].toFloat());
      }
    }
}

//-----------------------------------------------------------------------------
//proc ImportNodeOptions {node} {
void qSlicerSlicer2SceneReaderPrivate::importOptionsNode(NodeType& node)
{
  Q_UNUSED(node);
  Q_Q(qSlicerSlicer2SceneReader);
  //$::slicer3::MRMLScene SetErrorMessage "warning: option nodes cannot be imported"
  //$::slicer3::MRMLScene SetErrorCode 1
  q->mrmlScene()->SetErrorMessage("Warning: option nodes cannot be imported");
  q->mrmlScene()->SetErrorCode(1);
}

//-----------------------------------------------------------------------------
qSlicerSlicer2SceneReader::qSlicerSlicer2SceneReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSlicer2SceneReaderPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerSlicer2SceneReader::qSlicerSlicer2SceneReader(vtkSlicerApplicationLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSlicer2SceneReaderPrivate(*this))
{
  this->setApplicationLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerSlicer2SceneReader::~qSlicerSlicer2SceneReader() = default;

//-----------------------------------------------------------------------------
vtkSlicerApplicationLogic* qSlicerSlicer2SceneReader::applicationLogic()const
{
  Q_D(const qSlicerSlicer2SceneReader);
  return d->ApplicationLogic.GetPointer();
}

//-----------------------------------------------------------------------------
void qSlicerSlicer2SceneReader::setApplicationLogic(vtkSlicerApplicationLogic* logic)
{
  Q_D(qSlicerSlicer2SceneReader);
  d->ApplicationLogic = logic;
}

//-----------------------------------------------------------------------------
QString qSlicerSlicer2SceneReader::description()const
{
  return "Slicer2 Scene";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerSlicer2SceneReader::fileType()const
{
  return QString("SceneFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSlicer2SceneReader::extensions()const
{
  return QStringList() << "*.xml";
}

//-----------------------------------------------------------------------------
bool qSlicerSlicer2SceneReader::load(const IOProperties& properties)
{
  Q_D(qSlicerSlicer2SceneReader);
  QString file = properties["fileName"].toString();
  Q_ASSERT(!file.isEmpty());

  d->TransformIDStack.clear();
  d->FiducialListNode = nullptr;
  d->ParentID = "";
  d->ModelIDs.clear();
  d->LoadedNodes.clear();
  QFileInfo fileInfo(file);
  d->Directory = fileInfo.absolutePath();

  if (properties.contains("clear") && properties["clear"].toBool())
    {
    this->mrmlScene()->Clear(false);
    }

  vtkNew<vtkXMLDataParser> slicer2Parser;
  slicer2Parser->SetFileName(file.toUtf8());
  slicer2Parser->Parse();
  vtkXMLDataElement* root = slicer2Parser->GetRootElement();
  d->importElement(root);
  this->setLoadedNodes(d->LoadedNodes);

  return true;
}
