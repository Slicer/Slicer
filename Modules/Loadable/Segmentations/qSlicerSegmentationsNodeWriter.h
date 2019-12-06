/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women’s Hospital through NIH grant R01MH112748.

==============================================================================*/

#ifndef __qSlicerSegmentationsNodeWriter_h
#define __qSlicerSegmentationsNodeWriter_h

// QtCore includes
#include "qSlicerSegmentationsModuleExport.h"
#include "qSlicerNodeWriter.h"

class qSlicerSegmentationsNodeWriterPrivate;
class vtkMRMLNode;

/// Utility class that is ready to use for most of the nodes.
class Q_SLICER_QTMODULES_SEGMENTATIONS_EXPORT qSlicerSegmentationsNodeWriter
  : public qSlicerNodeWriter
{
  Q_OBJECT
public:
  typedef qSlicerNodeWriter Superclass;
  qSlicerSegmentationsNodeWriter(QObject* parent);
  ~qSlicerSegmentationsNodeWriter() override;

  /// Return a qSlicerIOSegmentationNodeWriterOptionsWidget
  qSlicerIOOptions* options()const override;

  /// Write the node referenced by "nodeID" into the "fileName" file.
  /// Optionally, "useCompression" and "useReferenceGeometry" can be specified.
  /// Return true on success, false otherwise.
  /// Create a storage node if the storable node doesn't have any.
  bool write(const qSlicerIO::IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerSegmentationsNodeWriterPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentationsNodeWriter);
  Q_DISABLE_COPY(qSlicerSegmentationsNodeWriter);
};

#endif
