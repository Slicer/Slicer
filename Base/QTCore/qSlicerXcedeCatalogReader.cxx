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

#include "qSlicerXcedeCatalogReader.h"

/// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLColorLogic.h"

/// MRML includes
#include <vtkCacheManager.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLStorageNode.h>
#include <vtkURIHandler.h>
#include <vtkMRMLVolumeNode.h>

/// FreeSurfer includes
#include <vtkFSSurfaceHelper.h>

/// VTK includes
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkXMLDataParser.h>

class qSlicerXcedeCatalogReaderPrivate
{
  Q_DECLARE_PUBLIC(qSlicerXcedeCatalogReader);
protected:
  qSlicerXcedeCatalogReader* const q_ptr;
public:
  qSlicerXcedeCatalogReaderPrivate(qSlicerXcedeCatalogReader& object);

  typedef QMap<QString, QString> NodeType;
  int     GetNumberOfElements(vtkXMLDataElement* parent);
  QString nodeType(const QString& format)const;
  int     checkFormat(const QString& format)const;
  void    importVolumeNode(NodeType node);
  void    importModelNode(NodeType node);
  void    importTransformNode(NodeType node);
  void    importOverlayNode(NodeType node);
  void    importElement(vtkXMLDataElement* element);
  void    importEntry(vtkXMLDataElement* element);
  bool    computeFIPS2SlicerTransformCorrection();
  void    applyFIPS2SlicerTransformCorrection();

  vtkSmartPointer<vtkMRMLColorLogic> ColorLogic;
  QStringList TransformIDStack;
  QString     Directory;
  //QString     ParentId;
  QString     MRMLIdFSBrain;
  QString     MRMLIdExampleFunc;
  QStringList MRMLIdStatFileList;
  QString     MRMLIdLHModel;
  QString     MRMLIdRHModel;
  QString     MRMLIdAnat2Exf;
  QString     MRMLIdStatisticsToBrainXform;
  //QString     MRMLId;
  //int         NumberOfElements;
  QStringList AnnotationFiles;
  QStringList LoadedNodes;
protected:
  qSlicerCoreIOManager* ioManager()const;
};

//-----------------------------------------------------------------------------
qSlicerXcedeCatalogReaderPrivate::qSlicerXcedeCatalogReaderPrivate(qSlicerXcedeCatalogReader& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerCoreIOManager* qSlicerXcedeCatalogReaderPrivate::ioManager()const
{
  return qSlicerCoreApplication::application()->coreIOManager();
}

//------------------------------------------------------------------------------
//proc XcedeCatalogImportGetNumberOfElements { element } {
int qSlicerXcedeCatalogReaderPrivate::GetNumberOfElements(vtkXMLDataElement* parent)
{
  ////--- save current parent locally
  //set parent $::XcedeCatalog_HParent_ID

  ////--- increment count
  //incr ::XcedeCatalog_NumberOfElements

  ////---TODO: probably don't need this...
  //// leave a place holder in case we are a group node
  //lappend ::XcedeCatalog(transformIDStack) "NestingMarker"

  //// process all the sub nodes, which may include a sequence of matrices
  //// and/or nested transforms
  //set nNested [$element GetNumberOfNestedElements]
  //for {set i 0} {$i < $nNested} {incr i} {
  //  set nestElement [$element GetNestedElement $i]
  //  XcedeCatalogImportGetNumberOfElements $nestElement
  //}

  ////---TODO: probably don't need this...
  //// strip away any accumulated transform ids
  //while { $::XcedeCatalog(transformIDStack) != "" && [lindex $::XcedeCatalog(transformIDStack) end] != "NestingMarker" } {
  //  set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]
  //}
  //// strip away the nesting marker
  //set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]

  //// restore parent locally
  //set ::XcedeCatalog_HParent_ID $parent

  int numberOfElements = 1;
  for(int i = 0; i < parent->GetNumberOfNestedElements(); ++i)
    {
    numberOfElements += this->GetNumberOfElements(parent->GetNestedElement(i));
    }
  return numberOfElements;
}

//------------------------------------------------------------------------------
// recursive routine to import all elements and their
// nested parts
//------------------------------------------------------------------------------
//proc XcedeCatalogImportGetElement { element } {
void qSlicerXcedeCatalogReaderPrivate::importElement(vtkXMLDataElement* element)
{
  // save current parent locally
  //set parent $::XcedeCatalog_HParent_ID
  //QString parent = this->ParentID;

  //--- update progress bar
  //set elementType [$element GetName]
  //$::XcedeCatalog_mainWindow SetStatusText "Parsing $elementType..."

  //incr ::XcedeCatalog_WhichElement
  //$::XcedeCatalog_progressGauge SetValue [expr 100 * $::XcedeCatalog_WhichElement / (1. * $::XcedeCatalog_NumberOfElements)]

  // import this element if it contains an entry of the correct type
  //XcedeCatalogImportGetEntry $element
  this->importEntry(element);

  //---TODO: probably don't need this...
  // leave a place holder in case we are a group node
  //lappend ::XcedeCatalog(transformIDStack) "NestingMarker"
  this->TransformIDStack << "NestingMarker";

  // process all the sub nodes, which may include a sequence of matrices
  // and/or nested transforms
  //set nNested [$element GetNumberOfNestedElements]
  //for {set i 0} {$i < $nNested} {incr i} {
  //  set nestElement [$element GetNestedElement $i]
  //  XcedeCatalogImportGetElement $nestElement
  //}
  int nNester = element->GetNumberOfNestedElements();
  for (int i = 0; i < nNester; ++i)
    {
    this->importElement(element->GetNestedElement(i));
    }

  //---TODO: probably don't need this...
  // strip away any accumulated transform ids
  //while { $::XcedeCatalog(transformIDStack) != "" && [lindex $::XcedeCatalog(transformIDStack) end] != "NestingMarker" } {
  //  set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]
  //}
  // strip away the nesting marker
  //set ::XcedeCatalog(transformIDStack) [lrange $::XcedeCatalog(transformIDStack) 0 end-1]
  int nestingMarkerIndex = this->TransformIDStack.lastIndexOf("NestingMarker");
  if (nestingMarkerIndex > 0)
    {
    QStringList::iterator nestingMarkerIterator = this->TransformIDStack.begin();
    nestingMarkerIterator += nestingMarkerIndex;
    this->TransformIDStack.erase(nestingMarkerIterator, this->TransformIDStack.end());
    }
  // restore parent locally
  //set ::XcedeCatalog_HParent_ID $parent
  //this->ParentID = parent;
}

//------------------------------------------------------------------------------
// and returns the nodeType associated with that format
//------------------------------------------------------------------------------
//proc XcedeCatalogImportGetNodeType { format } {
QString qSlicerXcedeCatalogReaderPrivate::nodeType(const QString& format)const
{
  //if {$format == "FreeSurfer:mgz-1" } {
  //    return "Volume"
  //} elseif {$format == "nrrd:nrrd-1" } {
  //    return "Volume"
  //} elseif {$format == "nifti:nii-1" } {
  //    return "Volume"
  //} elseif { $format == "FreeSurfer:w-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:thickness-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:curv-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:avg_curv-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:sulc-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:area-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:annot-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:mgh-1" } {
  //    return "Volume"
  //} elseif { $format == "FreeSufer:mat-1" } {
  //    return "Transform"
  //} elseif { $format == "FreeSurfer:surface-1" } {
  //    return "Model"
  //} elseif { $format == "FreeSurfer:overlay-1" } {
  //    return "Overlay"
  //} elseif { $format == "FreeSurfer:matrix-1" } {
  //    return "Transform"
  //}  else {
  //    return "Unknown"
  //}
  if (format == "FreeSurfer:mgz-1" ||
      format == "nrrd:nrrd-1" ||
      format == "nifti:nii-1" ||
      format == "FreeSurfer:mgh-1")
    {
    return "Volume";
    }
  else if ( format == "FreeSurfer:w-1" ||
            format == "FreeSurfer:thickness-1" ||
            format == "FreeSurfer:curv-1" ||
            format == "FreeSurfer:avg_curv-1" ||
            format == "FreeSurfer:sulc-1" ||
            format == "FreeSurfer:area-1" ||
            format == "FreeSurfer:annot-1" ||
            format == "FreeSurfer:overlay-1")
    {
    return "Overlay";
    }
  else if (format == "FreeSufer:mat-1" ||
           format == "FreeSurfer:matrix-1")
    {
    return "Transform";
    }
  else if (format == "FreeSurfer:surface-1")
    {
    return "Model";
    }
  return "Unknown";
}

//------------------------------------------------------------------------------
// checking to see if Slicer can read this file format
//------------------------------------------------------------------------------
//proc XcedeCatalogImportFormatCheck { format } {
int qSlicerXcedeCatalogReaderPrivate::checkFormat(const QString& format)const
{
    //--- check format against known formats
    //--- TODO: Once these values are formally defined for
    //--- all freesurfer data, we will have to change.
    //--- matrix-1 and overlay-1 are made up!
    //--- TODO: Add more as we know what their
    //--- XCEDE definitions are (analyze, etc.)

// return 1 if have a valid storage node that can deal with remote uri's, return 2 if need to synch download
  if (format == "FreeSurfer:mgz-1" ||
      format == "nifti:nii-1" ||
      format == "nrrd:nrrd-1" ||
      format == "FreeSurfer:w-1" ||
      format == "FreeSurfer:thickness-1" ||
      format == "FreeSurfer:curv-1" ||
      format == "FreeSurfer:avg_curv-1" ||
      format == "FreeSurfer:sulc-1"  ||
      format == "FreeSurfer:area-1" ||
      format == "FreeSurfer:annot-1" ||
      format == "FreeSurfer:surface-1" ||
      format == "FreeSurfer:overlay-1")
    {
    return 1;
    }
  else if (format == "FreeSurfer:mgh-1" ||
           format == "FreeSurfer:matrix-1")
    {
    return 2;
    }
  else
    {
    return 0;
    }
}

//------------------------------------------------------------------------------
// if the element is of a certain type of data
// (Transform, Volume, Model, etc.)
// parse the attributes of a node into a tcl array
// and then invoke the type-specific handler
//------------------------------------------------------------------------------
//proc XcedeCatalogImportGetEntry {element } {
void qSlicerXcedeCatalogReaderPrivate::importEntry(vtkXMLDataElement* element)
{
  Q_Q(qSlicerXcedeCatalogReader);
  //--- is this a catalog entry that contains a file or reference?
  //set elementType [$element GetName]
  //if { $elementType != "entry" && $elementType != "Entry" } {
  //    //--- only process catalog entry tags
  //    return
  //}
  QString elementType = element->GetName();
  if (elementType.toLower() != "entry")
    {
    //--- only process catalog entry tags
    return;
    }

  NodeType node;
  //--- get attributes
  //set nAtts [$element GetNumberOfAttributes]
  //for {set i 0} {$i < $nAtts} {incr i} {
  //    set attName [$element GetAttributeName $i]
  //    set node($attName) [$element GetAttributeValue $i]
  //}
  int nAtts = element->GetNumberOfAttributes();
  for (int i=0; i < nAtts; ++i)
    {
    QString attributeName = element->GetAttributeName(i);
    node[attributeName] = element->GetAttributeValue(i);
    }

  //--- make sure the entry has a "uri" attribute by searching
  //--- all attributes to find one with a name that matches "uri"
  //set hasuri 0
  //set uriAttName ""
  //for {set i 0} {$i < $nAtts} {incr i} {
  //    set attName [$element GetAttributeName $i]
  //    if { $attName == "uri" || $attName == "URI" } {
  //        //--- mark as found and capture its case (upper or lower)
  //        set hasuri 1
  //        set uriAttName $attName
  //    }
  //}
  //
  //if { $hasuri == 0 } {
  //    puts "can't find an attribute called URI in $element"
  //    return
  //}
  QString uriAttName;
  foreach(QString key, node.keys())
    {
    if (key.toLower() == "uri")
      {
      uriAttName = key;
      break;
      }
    }
  if (uriAttName.isEmpty())
    {
    qDebug() << "can't find an attribute called URI in element";
    return;
    }

  //$::XcedeCatalog_mainWindow SetStatusText "Loading $node(uri)..."

  //--- strip off the entry's relative path, and add the
  //--- absolute path of the Xcede file to it.
  //set fname [ file normalize $node($uriAttName) ]
  //set plist [ file split $fname ]
  //set len [ llength $plist ]
  //set fname [ lindex $plist [ expr $len - 1 ] ]

  ////set node($uriAttName) $::XcedeCatalog_Dir/$fname
  //set node(localFileName)  $::XcedeCatalog_Dir/$fname
  QString fname = QFileInfo(node[uriAttName]).fileName();
  node["localFileName"] = QFileInfo(QDir(this->Directory), fname).absoluteFilePath();

  //--- check to see if it's a remote file
  //set cacheManager [$::slicer3::MRMLScene GetCacheManager]
  vtkCacheManager* cacheManager = q->mrmlScene()->GetCacheManager();

  //--- get the file format
  //set gotformat 0
  //set formatAttName ""
  //for {set i 0} {$i < $nAtts} {incr i} {
  //    set attName [$element GetAttributeName $i]
  //    if { $attName == "format" || $attName == "Format" } {
  //        //--- mark as found and capture its case (upper or lower)
  //        set gotformat 1
  //        set formatAttName $attName
  //    }
  //}
  //if { $gotformat == 0 } {
  //    puts "description for entry contains no format information. Cannot import entry."
  //    return
  //}
  QString formatAttName;
  foreach(QString key, node.keys())
    {
    if (key.toLower() == "format")
      {
      formatAttName = key;
      break;
      }
    }
  if (formatAttName.isEmpty())
    {
    qDebug() << "Description for entry contains no format information. Cannot import entry.";
    return;
    }

  //--- what kind of node is it?
  //set nodeType [ XcedeCatalogImportGetNodeType $node($formatAttName) ]
  //if { $nodeType == "Unknown" } {
  //    puts "$node($formatAttName) is an unsupported format. Cannot import entry."
  //    return
  //}
  QString nodeType = this->nodeType(node[formatAttName]);
  //--- make sure the file is a supported format
  //set fileformat [ XcedeCatalogImportFormatCheck $node($formatAttName) ]
  int fileFormat = this->checkFormat(node[formatAttName]);
  bool isRemote = false;
  vtkURIHandler* uriHandler = nullptr;
  QFileInfo file;
  switch (fileFormat)
    {
//  if { $fileformat == 0 } {
//      puts "$node($formatAttName) is an unsupported format. Cannot import entry."
//      return
    case 0:
      qDebug() << node[formatAttName] << "is an unsupported format. Cannot import entry.";
      return;
//  } elseif { $fileformat == 1 } {
//        puts "$node($formatAttName) can handle downloads automatically"
    case 1:
//      if {$cacheManager != ""} {
      if (cacheManager == nullptr)
        {
        break;
        }
//      set isRemote [$cacheManager IsRemoteReference $node($uriAttName)]
//      if {$isRemote == 0} {
      isRemote = cacheManager->IsRemoteReference(
        node[uriAttName].toUtf8());
      if (isRemote)
        {
        break;
        }
//          //--- make sure the local file exists
//          set node(localFileName) [ file normalize $node(localFileName) ]
//          if {![ file exists $node(localFileName) ] } {
//              puts "can't find file $node(localFileName)."
//              return
//          }
      file = QFileInfo(node["localFileName"]);
      if (!file.exists())
        {
        qDebug() << "Can't find file" << node["localFileName"];
        return;
        }

//          //--- make sure the local file is a file (and not a directory)
//          if { ![file isfile $node(localFileName) ] } {
//              puts "$node(localFileName) doesn't appear to be a file. Not trying to import."
//              return
//          }
      if (!file.isFile())
        {
        qDebug() << node["localFileName"] << "doesn't appear to be a file. Not trying to import.";
        return;
        }
//          // it's a local file, so reset the uri
//          set node($uriAttName) $node(localFileName)
      node[uriAttName] = node["localFileName"];
//      }
//      }
      break;
//  } elseif { $fileformat == 2 } {
    case 2:
// //        puts "$node($formatAttName) is something we have to download manually if it has a remote uri"
//      if {$cacheManager != ""} {
      if (cacheManager == nullptr)
        {
        break;
        }
//            puts "Asynch Enabled = [[$::slicer3::MRMLScene GetDataIOManager] GetEnableAsynchronousIO]"
//      set isRemote [$cacheManager IsRemoteReference $node($uriAttName)]
//      if {$isRemote == 1} {
//        $::XcedeCatalog_mainWindow SetStatusText "Loading remote $node($uriAttName)..."
      isRemote = cacheManager->IsRemoteReference(node[uriAttName].toUtf8());
      if (isRemote)
        {
        break;
        }
      qDebug() << "Loading remote" << node[uriAttName];
//                puts "Trying to find URI handler for $node($uriAttName)"
//      set uriHandler [$::slicer3::MRMLScene FindURIHandler $node($uriAttName)]
//      if {$uriHandler != ""} {
//          // for now, do a synchronous download
//          // puts "Found a file handler, doing a synchronous download from $node($uriAttName) to $node(localFileName)"
 //         $uriHandler StageFileRead $node($uriAttName) $node(localFileName)
//      } else {
//          puts "Unable to find a file handler for $node($uriAttName)"
//      }
//          }
//      }
      uriHandler = q->mrmlScene()->FindURIHandler(node[uriAttName].toUtf8());
      if (uriHandler)
        {
        uriHandler->StageFileRead(node[uriAttName].toUtf8(), node["localFileName"].toUtf8());
        }
      else
        {
        qDebug() << "Unable to find a file handler for " << node[uriAttName];
        }
//      // puts "\tNow resetting uri $node($uriAttName) to local file name $node(localFileName) so can read from disk"
//      set node($uriAttName) $node(localFileName)
      node[uriAttName] = node["localFileName"];
    }

  // //--- finally, create the node
  // set handler XcedeCatalogImportEntry$nodeType

  // // call the handler for this element
  // puts "Importing $nodeType"
  // $handler node
  if (nodeType == "Overlay")
    {
    this->importOverlayNode(node);
    }
  else if (nodeType == "Transform")
    {
    this->importTransformNode(node);
    }
  else if (nodeType == "Volume")
    {
    this->importVolumeNode(node);
    }
  else if (nodeType == "Model")
    {
    this->importModelNode(node);
    }
  else
    {
    qDebug() << "Node type unsupported: " << nodeType;
    }
}


//----------------------------------------------------------------------------
//proc XcedeCatalogImportEntryVolume {node} {
void qSlicerXcedeCatalogReaderPrivate::importVolumeNode(NodeType node)
{
  //upvar $node n


  //--- ditch if there's no file in the uri
  //if { ![info exists n(uri) ] } {
  //  puts "XcedeCatalogImportEntryVolume: no uri specified for node $n(uri)"
  //  return
  //}
  if (!node.contains("uri"))
    {
    qDebug() << "QSlicerXCedeCatalogIO::importVolumeNode: no uri specified for node";
    return;
    }


  //set centered 1
  //set labelmap 0
  //set singleFile 0
  //set autoLevel 1
  bool centered = true;
  bool labelMap = false;
  bool singleFile = false;
  bool autoLevel = true;

  // if { [info exists n(labelmap) ] } {
  //     set labelmap 1
  //     // don't do auto level calc if it's a label map
  //     set autoLevel 0
  // }
  if (node.contains("labelMap") && node["labelMap"].toInt())
    {
    labelMap = true;
    autoLevel = false;
    }

  qSlicerIO::IOProperties properties;
  //if { [ string first "stat" $n(uri) ] >= 0 } {
  //  // set autoLevel 0
  //}
  //set loadingOptions [expr $labelmap * 1 + $centered * 2 + $singleFile * 4 + $autoLevel * 8]
  properties["labelMap"] = QVariant(labelMap);
  properties["centered"] = QVariant(centered);
  properties["singleFile"] = QVariant(singleFile);
  properties["autoLevel"] = QVariant(autoLevel);

  //set logic [$::slicer3::VolumesGUI GetLogic]
  //if { $logic == "" } {
  //  puts "XcedeCatalogImportEntryVolume: Unable to access Volumes Logic. $n(uri) not imported."
  //  return
  //}
//    puts "Calling volumes logic add archetype scalar volume with uri = $n(uri) and name = $n(name)"
//    set volumeNode [$logic AddArchetypeVolume $n(uri) $n(name) $loadingOptions]
  //set volumeNode [$logic AddArchetypeScalarVolume $n(uri) $n(name) $loadingOptions]
  properties["name"] = node["name"];
  properties["fileName"] = node["uri"];
  vtkMRMLVolumeNode* volumeNode = vtkMRMLVolumeNode::SafeDownCast(
    this->ioManager()->loadNodesAndGetFirst(QString("VolumeFile"), properties));
  //if { $volumeNode == "" } {
  //  puts "XcedeCatalogImportEntryVolume: Unable to add Volume Node for $n(uri)."
  //  return
  //}
  if (volumeNode == nullptr)
    {
    qDebug() << "Unable to add Volume node for" << node["uri"];
    return;
    }

  //set volumeNodeID [$volumeNode GetID]
  QString volumeNodeID = volumeNode->GetID();
  this->LoadedNodes << volumeNodeID;

  //if { [info exists n(description) ] } {
  //  $volumeNode SetDescription $n(description)
  //}
  if (node.contains("description"))
    {
    volumeNode->SetDescription(node["description"].toUtf8());
    }

  //--- try using xcede differently than the slicer2 xform description
  // use the current top of stack (might be "" if empty, but that's okay)
  //set transformID [lindex $::XcedeCatalog(transformIDStack) end]
  //$volumeNode SetAndObserveTransformNodeID $transformID

  //set volumeDisplayNode [$volumeNode GetDisplayNode]
  //if { $volumeDisplayNode == ""  } {
  //  puts "XcedeCatalogImportEntryVolume: Unable to access Volume Display Node for  $n(uri). Volume display not configured."
  //  return
  //}
  vtkMRMLVolumeDisplayNode* volumeDisplayNode =
    vtkMRMLVolumeDisplayNode::SafeDownCast(volumeNode->GetDisplayNode());

  //--- make some good guesses about what color node to set
  //--- and what window/level/threshold properties to set
  //if { [ string first "stat" $n(uri) ] >= 0 } {
  ////--- this is likely a statistical volume.
  //  $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodefMRIPA"
  ////$volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeIron"
  ////        $volumeDisplayNode SetAutoWindowLevel 0
  //  $volumeDisplayNode SetAutoWindowLevel 1
  ////$volumeDisplayNode SetThresholdType 1
  vtkMRMLScalarVolumeDisplayNode* scalarDisplayNode =
    vtkMRMLScalarVolumeDisplayNode::SafeDownCast(volumeDisplayNode);
  if (node["uri"].startsWith("stat"))
    {
    volumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeIron");
    scalarDisplayNode->SetAutoWindowLevel(1);
    }
  //} elseif { [ string first "aseg" $n(uri) ] >= 0 } {
  //      //--- this is likely a freesurfer label map volume
  //      set colorLogic [ $::slicer3::ColorGUI GetLogic ]
  //      if { $colorLogic != "" } {
  //          $volumeDisplayNode SetAndObserveColorNodeID [$colorLogic GetDefaultFreeSurferLabelMapColorNodeID ]
  //      } else {
  //          $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
  //      }
  //      //$volumeDisplayNode SetAutoWindowLevel 0
  //      //$volumeDisplayNode SetAutoThreshold 0
  else if (node["uri"].startsWith("aseg"))
    {
    // if (colorLogic) {
    //volumeDisplayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
    // }else {
    // volumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");}
    volumeDisplayNode->SetAndObserveColorNodeID(
      this->ColorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
    }

    // } else {
    //     //--- assume this is a greyscale volume
    //     $volumeDisplayNode SetAndObserveColorNodeID "vtkMRMLColorTableNodeGrey"
    //     if { $labelmap == 0 } {
    //         $volumeDisplayNode SetAutoWindowLevel 1
    //         $volumeDisplayNode SetAutoThreshold 1
    //     }
    // }
  else
    {
    volumeDisplayNode->SetAndObserveColorNodeID("vtkMRMLColorTableNodeGrey");
    if (!labelMap)
      {
      scalarDisplayNode->SetAutoWindowLevel(true);
      scalarDisplayNode->SetAutoThreshold(true);
      }
    }
  //// puts "\tFor volume [$volumeNode GetName], on volume display node [$volumeDisplayNode GetID], observing colour node [$volumeDisplayNode GetColorNodeID]. Display node window = [$volumeDisplayNode GetWindow], level = [$volumeDisplayNode GetLevel]"
  //$logic SetActiveVolumeNode $volumeNode

  // //--- If volume freesurfer brain.mgz, set a global
  // //--- This global is used as a reference volume for any
  // //--- potential functional or statistical volumes
  // //--- that may need to be registered to the brain
  // //--- image via the anat2exf.register.dat xform.
  // if { [ string first "brain.mgz" $n(uri) ] >= 0 } {
  //   set ::XcedeCatalog_MrmlID(FSBrain) $volumeNodeID
  // }
  if (node["uri"].startsWith("brain.mgz"))
    {
    this->MRMLIdFSBrain = volumeNodeID;
    }

  // //--- If volume is an example_func image (used for
  // //--- registration with the anatomical), set a global.
  // if { [ string first "example_func" $n(uri) ] >= 0 } {
  //   set ::XcedeCatalog_MrmlID(ExampleFunc) $volumeNodeID
  // }
  if (node["uri"].startsWith("example_func"))
    {
    this->MRMLIdExampleFunc = volumeNodeID;
    }

  // //--- If volume is a statistics volume, add to a
  // //--- global list: these volumes will be put inside
  // //--- a transform to register them to brain.mgz
  // //--- if that transform is created.
  // //--- this is weak; need a better test.
  // if { [ string first "stat" $n(uri) ] >= 0 } {
  //     lappend ::XcedeCatalog_MrmlID(StatFileList) $volumeNodeID
  // }
  if (node["uri"].startsWith("stat"))
    {
    MRMLIdStatFileList << volumeNodeID;
    }

  // if { $labelmap } {
  //   [[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetActiveLabelVolumeID [$volumeNode GetID]
  // } else {
  //   [[$::slicer3::VolumesGUI GetApplicationLogic] GetSelectionNode] SetActiveVolumeID [$volumeNode GetID]
  // }
  // [$::slicer3::VolumesGUI GetApplicationLogic] PropagateVolumeSelection
  if (labelMap)
    {
    this->ColorLogic->GetMRMLApplicationLogic()->GetSelectionNode()
      ->SetActiveLabelVolumeID(volumeNodeID.toUtf8());
    }
  else
    {
    this->ColorLogic->GetMRMLApplicationLogic()->GetSelectionNode()
      ->SetActiveVolumeID(volumeNodeID.toUtf8());
    }
  this->ColorLogic->GetMRMLApplicationLogic()->PropagateVolumeSelection();
}

//----------------------------------------------------------------------------
//proc XcedeCatalogImportEntryModel {node} {
void qSlicerXcedeCatalogReaderPrivate::importModelNode(NodeType node)
{
  //upvar $node n

  // //--- ditch if there's no file in the uri
  // if { ! [info exists n(uri) ] } {
  //     puts "XcedeCatalogImportEntryModel: no uri specified for node $n(uri). No model imported."
  //     return
  // }
  if (!node.contains("uri"))
    {
    qDebug() << "QSlicerXCedeCatalogIO::importModelNode: no uri specified for node";
    return;
    }

    // set logic [$::slicer3::ModelsGUI GetLogic]
    // if {$logic == "" } {
    //     puts "XcedeCatalogImportEntryModel: couldn't retrieve Models Logic. Model $n(name) not imported."
    //     return
    // }
    // puts "importing model for $n(uri)"
    // set mnode [$logic AddModel $n(uri)]
    // //--- maybe don't need this?
    // //set snode [ $mnode GetModelStorageNode ]
    // //set type [ $snode IsA ]
    // //if { $type == "vtkMRMLFreeSurferModelStorageNode" } {
    // //    $snode SetUseStripper 0
    // //}
  qSlicerIO::IOProperties properties;
  properties["fileName"] = node["uri"];
  vtkMRMLModelNode* mnode = vtkMRMLModelNode::SafeDownCast(
    ioManager()->loadNodesAndGetFirst(QString("ModelFile"), properties));

  // if { $mnode == "" } {
  //   puts "XcedeCatalogImportEntryModel: couldn't created Model Node. Model $n(name) not imported."
  //   return
  // }
  if (mnode == nullptr)
    {
    qDebug() << "qSlicerXCedeCatalogIO::importModelNode: couldn't created Model Node. Model " << node["name"] << " not imported.";
    return;
    }
  this->LoadedNodes << mnode->GetID();

  //--- set name and description
  //if { [info exists n(description) ] } {
  //  $mnode SetDescription $n(description)
  //}
  if (node.contains("description"))
    {
    mnode->SetDescription(node["description"].toUtf8());
    }

  //if { [info exists n(name) ] } {
  //  $mnode SetName $n(name)
  //}
  if (node.contains("name"))
    {
    mnode->SetName(node["name"].toUtf8());
    }

  // //--- we assume catalogs will contain a single LH model
  // //--- with which all LHoverlays will be associated.
  // //--- and/or a single RH model with which RH overlays are associated.
  // //--- left hemisphere models
  // if { [ string first "lh." $n(uri) ] >= 0 } {
  //     if { $::XcedeCatalogMrmlID(LHmodel) == "" } {
  //         set ::XcedeCatalogMrmlID(LHmodel) [ $mnode GetID ]
  //     } else {
  //         puts "Warning: Xcede catalogs for slicer should contain at single LH model to which LH scalar overlays will be associated. This xcede file appears to contain multiple left hemisphere models: all scalar overlays will be associated with the first LH model."
  //     }
  // }
  if (node["uri"].startsWith("lh."))
    {
    if (this->MRMLIdLHModel.isEmpty())
      {
      this->MRMLIdLHModel = mnode->GetID();
      }
    else
      {
      qDebug() << "Warning: Xcede catalogs for slicer should contain at single LH model to which LH scalar overlays will be associated. This xcede file appears to contain multiple left hemisphere models: all scalar overlays will be associated with the first LH model.";
      }
    }
    // //--- right hemisphere models
    // if { [ string first "rh." $n(uri) ] >= 0 } {
    //     if { $::XcedeCatalogMrmlID(RHmodel) == "" } {
    //         set ::XcedeCatalogMrmlID(RHmodel) [ $mnode GetID ]
    //     } else {
    //         puts "Warning: Xcede catalogs for slicer should contain at single RH model to which RH scalar overlays will be associated. This xcede file appears to contain multiple right hemisphere models: all scalar overlays will be associated with the first RH model."
    //     }
    // }
  if (node["uri"].startsWith("rh."))
    {
    if (MRMLIdRHModel.isEmpty())
      {
      MRMLIdRHModel = mnode->GetID();
      }
    else
      {
      qDebug() << "Warning: Xcede catalogs for slicer should contain at single RH model to which RH scalar overlays will be associated. This xcede file appears to contain multiple right hemisphere models: all scalar overlays will be associated with the first RH model.";
      }
    }
}



//------------------------------------------------------------------------------
//-- TODO: something in this proc is causing debug leaks. WHAT?
//------------------------------------------------------------------------------
//proc XcedeCatalogImportEntryTransform {node} {
void qSlicerXcedeCatalogReaderPrivate::importTransformNode(NodeType node)
{
  Q_Q(qSlicerXcedeCatalogReader);
  //upvar $node n


  //--- ditch if there's no file in the uri
  // if { ! [info exists n(uri) ] } {
  //     puts "XcedeCatalogImportEntryTransform: no uri specified for node $n(uri). No transform imported."
  //     return
  // }
  if (!node.contains("uri"))
    {
    qDebug() << "QSlicerXCedeCatalogIO::importTransformNode: no uri specified for node";
    return;
    }

  // //--- if filename contains ".register.dat" then we know what to do
  // set check [ string first "register.dat" $n(uri) ]
  //   if { $check < 0 } {
  // puts "XcedeCatalogImportSetMatrixFromURI: $filename is unknown filetype, No transform imported."
  //   tk_messageBox -message "XcedeCatalogImportSetMatrixFromURI: $filename is unknown filetype, No transform imported."
  //   return
  //   }
  if (!node["uri"].startsWith("register.dat"))
    {
    qDebug() << "Unknown file type from name:" << node["uri"]
             << "No transform imported";
    return;
    }

  // //--- add the node
  // set tnode [ vtkMRMLLinearTransformNode New ]
  // $tnode SetName $n(name)
  // $::slicer3::MRMLScene AddNode $tnode
  // set tid [ $tnode GetID ]

  // if { $tid == "" } {
  //     puts "XcedeCatalogImportEntryTransform: unable to add Transform Node. No transform imported."
  //    tk_messageBox -message "XcedeCatalogImportEntryTransform: unable to add Transform Node. No transform imported."
  //     return
  // }
  vtkNew<vtkMRMLLinearTransformNode> tnode;
  tnode->SetName(node["name"].toUtf8());
  q->mrmlScene()->AddNode(tnode.GetPointer());
  QString tid = tnode->GetID();
  if (tid.isNull())
    {
    qDebug() << "qSlicerXcedeCatalogReader::importTransformNode: unable to add Transform Node. No transform imported.";
    return;
    }
  this->LoadedNodes << tid;

  //if { [info exists n(description) ] } {
  //  $tnode SetDescription $n(description)
  //}
  if (node.contains("description"))
    {
    tnode->SetDescription(node["description"].toUtf8());
    }

  // //--- open register.dat file and read
  // set matrix [ vtkMatrix4x4 New ]
  // set fid [ open $n(uri) r ]
  // set row 0
  // set col 0
  // while { ! [ eof $fid ] } {
  //     gets $fid line
  //     set llen [ llength $line ]
  //     //--- grab only lines that have matrix elements
  //     if { $llen == 4 } {
  //         set element [ expr [ lindex $line 0 ] ]
  //         $matrix SetElement $row $col $element
  //         incr col
  //         set element [ expr [ lindex $line 1 ] ]
  //         $matrix SetElement $row $col $element
  //         incr col
  //         set element [ expr [ lindex $line 2 ] ]
  //         $matrix SetElement $row $col $element
  //         incr col
  //         set element [ expr [ lindex $line 3 ] ]
  //         $matrix SetElement $row $col $element
  //         incr row
  //         set col 0
  //     }
  // }
  // close $fid
  // TODO: use IOManager to read the file...
  vtkNew<vtkMatrix4x4> matrix;
  QFile fid(node["uri"]);
  if (fid.open(QFile::ReadOnly | QIODevice::Text))
    {
    int row = 0;
    while (!fid.atEnd())
      {
      QString line = fid.readLine();
      QStringList elements = line.simplified().split(" ");
      if (elements.size() == 4)
        {
        matrix->SetElement(row, 0, elements[0].toDouble());
        matrix->SetElement(row, 1, elements[1].toDouble());
        matrix->SetElement(row, 2, elements[2].toDouble());
        matrix->SetElement(row, 3, elements[3].toDouble());
        ++row;
        }
      }
    }

  // //--- read the uri and translate matrix element values into place.
  // set M [ $tnode GetMatrixTransformToParent ]
  // if { $M == "" } {
  //     puts "XcedeCatalogImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
  //    tk_messageBox -message "XcedeCatalogImportSetMatrixFromURI: matrix for transform ID=$id not found. No elements set."
  //     return
  // }

  // $M DeepCopy $matrix
  // $matrix Delete
  // $tnode Delete
  tnode->SetMatrixTransformToParent(matrix.GetPointer());

  // //--- this is for help with FIPS registration correction
  // if { $n(name) == "anat2exf" } {
  //       set ::XcedeCatalog_MrmlID(anat2exf) $tid
  //   }
  if (node["name"] == "anat2exf")
    {
    this->MRMLIdAnat2Exf = tid;
    }
}

//----------------------------------------------------------------------------
//proc XcedeCatalogImportEntryOverlay {node} {
void qSlicerXcedeCatalogReaderPrivate::importOverlayNode(NodeType node)
{
  Q_Q(qSlicerXcedeCatalogReader);
  //upvar $node n

  // //--- not really a node, per se...
  // //--- ditch if there's no file in the uri

  // if { ! [info exists n(uri) ] } {
  //     puts "XcedeCatalogImportEntryOverlay: no uri specified for node $n(name). No overlay imported."
  //     return
  // }
  if (!node.contains("uri"))
    {
    qDebug() << "qSlicerXCedeCatalogIO::importOverlayNode: no uri specified for node";
    return;
    }

  // //--- what model node should these scalars be applied to?
  //   if { [ string first "lh." $n(uri) ] >= 0 } {
  //       if { ![info exists ::XcedeCatalogMrmlID(LHmodel) ] } {
  //           puts "XcedeCatalogImportEntryOverlay: no model ID specified for overlay $n(uri). No overlay imported."
  //           return
  //       }
  //       set mid $::XcedeCatalogMrmlID(LHmodel)
  //       set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
  //   }
  vtkMRMLModelNode* mnode = nullptr;
  QString mid;
  if (node["uri"].startsWith("lh."))
    {
    if (this->MRMLIdLHModel.isEmpty())
      {
      qDebug() << "qSlicerXCedeCatalogIO::importOverlayNode: no model ID specified for overlay" << node["uri"] << "No overlay imported.";
      return;
      }
    mid = this->MRMLIdLHModel;
    mnode = vtkMRMLModelNode::SafeDownCast(
      q->mrmlScene()->GetNodeByID(mid.toUtf8()));
    }

  // if { [ string first "rh." $n(uri) ] >= 0 } {
  //     if { ![info exists ::XcedeCatalogMrmlID(RHmodel) ] } {
  //         puts "XcedeCatalogImportEntryOverlay: no model ID specified for overlay $n(uri). No overlay imported."
  //         return
  //     }
  //     set mid $::XcedeCatalogMrmlID(RHmodel)
  //     set mnode [$::slicer3::MRMLScene GetNodeByID $mid]
  // }
  if (node["uri"].startsWith("rh."))
    {
    if (this->MRMLIdRHModel.isEmpty())
      {
      qDebug() << "qSlicerXCedeCatalogIO::importOverlayNode: no model ID specified for overlay" << node["uri"] << "No overlay imported.";
      return;
      }
    mid = this->MRMLIdRHModel;
    mnode = vtkMRMLModelNode::SafeDownCast(
      q->mrmlScene()->GetNodeByID(mid.toUtf8()));
    }

  // if { $mnode == "" } {
  //     puts "XcedeCatalogImportEntryOverlay: Model MRML Node corresponding to ID=$mid not found. No overlay imported."
  //     return
  // }
  if (mnode == nullptr)
    {
    qDebug() << "qSlicerXCedeCatalogIO::importOverlayNode: Model MRML node corresponding to ID: " << mid << "not found. No overlay imported.";
    return;
    }

  // set logic [$::slicer3::ModelsGUI GetLogic]
  // if { $logic == "" } {
  //     puts "XcedeCatalogImportEntryOverlay: cannot access Models Logic class. No overlay imported."
  //     return
  // }

  // //--- add the scalar to the node
  // $logic AddScalar $n(uri) $mnode
  //vtkSlicerModelsLogic* logic = this->modelsLogic();
  //logic->AddScalar(node["uri"].toUtf8(), mnode);
  qSlicerIO::IOProperties properties;
  properties["fileName"] = node["uri"];
  properties["modelNodeId"] = mnode->GetID();
  vtkMRMLStorageNode* scalarNode = vtkMRMLStorageNode::SafeDownCast(
    ioManager()->loadNodesAndGetFirst(QString("ScalarOverlayFile"), properties));
  this->LoadedNodes << scalarNode->GetID();
  // //--- keep track of all annotation files loaded.
  // if { [ string first "annot" $n(uri) ] >= 0 } {
  //     lappend ::XcedeCatalog_AnnotationFiles $n(uri)
  // }
  if (node["uri"].startsWith("annot"))
    {
    this->AnnotationFiles << node["uri"];
    }
}

//------------------------------------------------------------------------------
//proc XcedeCatalogImportComputeFIPS2SlicerTransformCorrection { } {
bool qSlicerXcedeCatalogReaderPrivate::computeFIPS2SlicerTransformCorrection()
{
  Q_Q(qSlicerXcedeCatalogReader);
  // if { $::XcedeCatalog_MrmlID(anat2exf) == "" } {
  //     return
  // }
  // if { $::XcedeCatalog_MrmlID(FSBrain) == "" } {
  //     return
  // }
  // if { $::XcedeCatalog_MrmlID(ExampleFunc) == "" } {
  //     return
  // }
  if (this->MRMLIdAnat2Exf.isNull() ||
      this->MRMLIdFSBrain.isNull() ||
      this->MRMLIdExampleFunc.isNull())
    {
    return false;
    }

  //--- find a brain.mgz, an example_func.nii, and an anat2exf.register.dat.
  //$::XcedeCatalog_mainWindow SetStatusText "Computing corrected registration matrix."
  //--- get required nodes from scene
  //set v1 [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(FSBrain) ]
  //set v2 [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(ExampleFunc) ]
  //set anat2exfT [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(anat2exf) ]
  vtkMRMLVolumeNode* v1 = vtkMRMLVolumeNode::SafeDownCast(
    q->mrmlScene()->GetNodeByID(this->MRMLIdFSBrain.toUtf8()));
  vtkMRMLVolumeNode* v2 = vtkMRMLVolumeNode::SafeDownCast(
    q->mrmlScene()->GetNodeByID(this->MRMLIdExampleFunc.toUtf8()));
  vtkMRMLLinearTransformNode* anat2exfT =
    vtkMRMLLinearTransformNode::SafeDownCast(
      q->mrmlScene()->GetNodeByID(this->MRMLIdAnat2Exf.toUtf8()));

  //--- get FSregistration matrix from node
  //set anat2exf [ $anat2exfT GetMatrixTransformToParent ]
  vtkNew<vtkMatrix4x4> anat2exf;
  anat2exfT->GetMatrixTransformToParent(anat2exf.GetPointer());

  // //--- create a new node to hold the transform
  // set ras2rasT [ vtkMRMLLinearTransformNode New ]
  // $ras2rasT SetName StatisticsToBrainXform
  // $::slicer3::MRMLScene AddNode $ras2rasT
  vtkNew<vtkMRMLLinearTransformNode> ras2rasT;
  ras2rasT->SetName("StatisticsToBrainXform");
  q->mrmlScene()->AddNode(ras2rasT.GetPointer());
  this->LoadedNodes << ras2rasT->GetID();

  //set ::XcedeCatalog_MrmlID(StatisticsToBrainXform) [ $ras2rasT GetID ]
  this->MRMLIdStatisticsToBrainXform = ras2rasT->GetID();

  //--- get access to methods we need through logic
  //set volumesLogic [ $::slicer3::VolumesGUI GetLogic ]

  //--- compute some matrices.
  //--- todo: deal with delayed read due to remote storage of data
  //set mat [ vtkMatrix4x4 New]
  //$volumesLogic ComputeTkRegVox2RASMatrix $v1 $mat
  //$volumesLogic TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix $v1 $v2 $anat2exf $mat
  vtkNew<vtkMatrix4x4> mat;
  vtkFSSurfaceHelper::ComputeTkRegVox2RASMatrix(
    v1->GetSpacing(), v1->GetImageData()->GetDimensions(), mat.GetPointer());
  vtkNew<vtkMatrix4x4> ijkToRas;
  vtkNew<vtkMatrix4x4> rasToIjk;
  v1->GetIJKToRASMatrix(ijkToRas.GetPointer());
  v2->GetRASToIJKMatrix(rasToIjk.GetPointer());
  vtkFSSurfaceHelper::TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix(
    v1->GetSpacing(), v1->GetImageData()->GetDimensions(), ijkToRas.GetPointer(),
    v2->GetSpacing(), v2->GetImageData()->GetDimensions(), rasToIjk.GetPointer(),
    anat2exf.GetPointer(), mat.GetPointer());

  //--- this inverse will register statistics to the brain.mgz
  //$mat Invert
  mat->Invert();

  //--- now have matrix. put it in transform.
  //[ $ras2rasT GetMatrixTransformToParent ] DeepCopy $mat
  ras2rasT->SetMatrixTransformToParent(mat.GetPointer());

  //--- ok -- now manually put your volume in the ras2rasT transform node.
  //$mat Delete
  //$ras2rasT Delete

  //--- mark the transform as created
  //set ::XcedeCatalog_RAS2RASTransformCreated 1
  return true;
}

//------------------------------------------------------------------------------
//proc XcedeCatalogImportApplyFIPS2SlicerTransformCorrection { } {
void qSlicerXcedeCatalogReaderPrivate::applyFIPS2SlicerTransformCorrection()
{
  Q_Q(qSlicerXcedeCatalogReader);
  //if { $::XcedeCatalog_RAS2RASTransformCreated == 1 } {
  //$::XcedeCatalog_mainWindow SetStatusText "Applying registration matrix to statistics volumes"
  //--- move all the detected stats files under the new registration xform
  // foreach id  $::XcedeCatalog_MrmlID(StatFileList) {
  //     set vnode [ $::slicer3::MRMLScene GetNodeByID $id ]
  //     $vnode SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(StatisticsToBrainXform)
  //     $vnode Modified
  // }
  foreach(QString id, this->MRMLIdStatFileList)
    {
    vtkMRMLVolumeNode* vnode = vtkMRMLVolumeNode::SafeDownCast(
      q->mrmlScene()->GetNodeByID(id.toUtf8()));
    vnode->SetAndObserveTransformNodeID(this->MRMLIdStatisticsToBrainXform.toUtf8());
    vnode->Modified();
    }
  //--- move the example func also into the new registration xform
  // set vnode [ $::slicer3::MRMLScene GetNodeByID $::XcedeCatalog_MrmlID(ExampleFunc) ]
  // $vnode SetAndObserveTransformNodeID $::XcedeCatalog_MrmlID(StatisticsToBrainXform)
  // $vnode Modified
  vtkMRMLVolumeNode* vnode = vtkMRMLVolumeNode::SafeDownCast(
    q->mrmlScene()->GetNodeByID(this->MRMLIdExampleFunc.toUtf8()));
  vnode->SetAndObserveTransformNodeID(this->MRMLIdStatisticsToBrainXform.toUtf8());
  vnode->Modified();
}

//------------------------------------------------------------------------------
qSlicerXcedeCatalogReader::qSlicerXcedeCatalogReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerXcedeCatalogReaderPrivate(*this))
{
}

//------------------------------------------------------------------------------
qSlicerXcedeCatalogReader::qSlicerXcedeCatalogReader(vtkMRMLColorLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerXcedeCatalogReaderPrivate(*this))
{
  this->setColorLogic(logic);
}

//------------------------------------------------------------------------------
qSlicerXcedeCatalogReader::~qSlicerXcedeCatalogReader() = default;

//------------------------------------------------------------------------------
vtkMRMLColorLogic* qSlicerXcedeCatalogReader::colorLogic()const
{
  Q_D(const qSlicerXcedeCatalogReader);
  return d->ColorLogic.GetPointer();
}

//------------------------------------------------------------------------------
void qSlicerXcedeCatalogReader::setColorLogic(vtkMRMLColorLogic* logic)
{
  Q_D(qSlicerXcedeCatalogReader);
  d->ColorLogic = logic;
}

//------------------------------------------------------------------------------
QString qSlicerXcedeCatalogReader::description()const
{
  return "Xcede Catalog";
}

//------------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerXcedeCatalogReader::fileType()const
{
  return QString("SceneFile");
}

//------------------------------------------------------------------------------
QStringList qSlicerXcedeCatalogReader::extensions()const
{
  return QStringList() << "*.xcat";
}

//------------------------------------------------------------------------------
bool qSlicerXcedeCatalogReader::load(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerXcedeCatalogReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  d->LoadedNodes.clear();
  this->setLoadedNodes(d->LoadedNodes);
  //--- get all current slice nodes and save their orientations.
  //--- these things are saved and restored with a MRML scene,
  //--- but are not represented in an xcat. So we can at least
  //--- restore the current state of each slice viewer after the
  //--- the MRML scene is cleared, and the node->Reset() call
  //--- has been made (which sets all orientations to Axial).


  //--- create a parser and parse the file
  //set parser [vtkXMLDataParser New]
  //$parser SetFileName $xcedeFile
  //set retval [ $parser Parse ]
  //if { $retval == 0 } {
  //  $parser Delete
  //  return $retval
  //} else {
  vtkNew<vtkXMLDataParser> parser;
  parser->SetFileName(fileName.toUtf8());
  if (parser->Parse() == 0)
    {
    return false;
    }

  //--- display to progress gauge and status bar.
  //set ::XcedeCatalog_mainWindow [$::slicer3::ApplicationGUI GetMainSlicerWindow]
  //set ::XcedeCatalog_progressGauge [$::XcedeCatalog_mainWindow GetProgressGauge]
  //$::XcedeCatalog_progressGauge SetValue 0
  //$::XcedeCatalog_mainWindow SetStatusText "Parsing $xcedeFile"

  //--- get the XCEDE root
  //set root [$parser GetRootElement]
  vtkXMLDataElement* root = parser->GetRootElement();
  if (QString(root->GetName()).toLower() != "xcede")
    {
    return false;
    }

  //--- get the directory of the normalized xcede file.
  //set ::XcedeCatalog_Dir [file dirname [file normalize $xcedeFile]]
  //puts "Reading file $xcedeFile from $::XcedeCatalog_Dir..."
  d->Directory = QFileInfo(fileName).absolutePath();

  //--- initialize some globals
  //set ::XcedeCatalog(transformIDStack) ""
  //set ::XcedeCatalog_HParent_ID ""
  //set ::XcedeCatalogMrmlID(LHmodel) ""
  //set ::XcedeCatalogMrmlID(RHmodel) ""
  //set ::XcedeCatalog_MrmlID(anat2exf) ""
  //set ::XcedeCatalog_MrmlID(FSBrain) ""
  //set ::XcedeCatalog_MrmlID(ExampleFunc) ""
  //set ::XcedeCatalog_MrmlID(StatisticsToBrainXform) ""
  //set ::XcedeCatalog_MrmlID(StatFileList) ""
  //set ::XcedeCatalog_AnnotationFiles ""
  //set ::XcedeCatalog_NumberOfElements 0
  //set ::XcedeCatalog_WhichElement 0
  //set ::XcedeCatalog_RAS2RASTransformCreated 0
  //array unset ::XcedeCatalog_MrmlID ""
  //set ::XcedeCatalog(transformIDStack) ""
  //set ::XcedeCatalog_HParent_ID ""
  d->TransformIDStack.clear();
  //d->ParentId.clear();
  d->MRMLIdLHModel.clear();
  d->MRMLIdRHModel.clear();
  d->MRMLIdAnat2Exf.clear();
  d->MRMLIdFSBrain.clear();
  d->MRMLIdExampleFunc.clear();
  d->MRMLIdStatisticsToBrainXform.clear();
  d->MRMLIdStatFileList.clear();
  d->AnnotationFiles.clear();
  // number of elements and whichElement are for progress report only
  //d->NumberOfElements = 0;
  //d->WhichElement = 0;
  //d->MRMLId.clear();

  //--- recursively import cataloged datasets
  //XcedeCatalogImportGetNumberOfElements $root
  //d->NumberOfElements = d->GetNumberOfElements(root);

  //--- recursively import cataloged datasets
  //set ::XcedeCatalog(transformIDStack) ""
  //set ::XcedeCatalog_HParent_ID ""
  //set root [$parser GetRootElement]

  //XcedeCatalogImportGetElement $root
  d->importElement(root);

  //--- if the catalog includes a brain.mgz, example_func.nii and
  //--- anat2exf.dat, we assume this is a FreeSurfer/FIPS catalog
  //--- and convert FreeSurfer tkRegister2's registration matrix
  //--- to a Slicer RAS2RAS registration matrix.
  //XcedeCatalogImportComputeFIPS2SlicerTransformCorrection
  bool created = d->computeFIPS2SlicerTransformCorrection();

  //--- if the Correction transform node is created,
  //--- place all statistics volumes inside that.
  //XcedeCatalogImportApplyFIPS2SlicerTransformCorrection
  if (created)
    {
    d->applyFIPS2SlicerTransformCorrection();
    }

  //--- reset the feedback things
  //$::XcedeCatalog_progressGauge SetValue 0
  //$::XcedeCatalog_mainWindow SetStatusText ""

  //--- update main viewer and slice viewers.
  //$::slicer3::MRMLScene Modified
  this->mrmlScene()->Modified();
  // TODO: request a render...

  //[$::slicer3::ApplicationGUI GetActiveViewerWidget ] RequestRender
  // set ns [[$::slicer3::ApplicationGUI GetSlicesGUI] GetNumberOfSliceGUI]
  // set slicesGUI [$::slicer3::ApplicationGUI GetSlicesGUI]
  // for { set ss 0 } { $ss < $ns } { incr ss } {
  //     if { $ss == 0 } {
  //         [[$slicesGUI GetFirstSliceGUI] GetSliceViewer ] RequestRender
  //         set layoutname [$slicesGUI  GetFirstSliceGUILayoutName ]
  //     } else {
  //         [[$slicesGUI GetNextSliceGUI $layoutname] GetSliceViewer ] RequestRender
  //         set layoutname [$slicesGUI  GetNextSliceGUILayoutName $layoutname ]
  //     }
  // }

  //--- clean up.
  //$parser Delete
  //$::slicer3::MRMLScene SetErrorCode 0
  this->mrmlScene()->SetErrorCode(0);
  this->setLoadedNodes(d->LoadedNodes);
  //puts "...done reading $xcedeFile."
  qDebug() << "... done reading" << fileName;
  return true;
}
