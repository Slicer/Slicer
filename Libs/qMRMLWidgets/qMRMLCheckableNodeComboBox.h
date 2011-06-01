/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qMRMLCheckableNodeComboBox_h
#define __qMRMLCheckableNodeComboBox_h

// qMRML includes
#include "qMRMLNodeComboBox.h"

class qMRMLCheckableNodeComboBoxPrivate;

class QMRML_WIDGETS_EXPORT qMRMLCheckableNodeComboBox
  : public qMRMLNodeComboBox
{
  Q_OBJECT
  /*
  Q_PROPERTY(bool selectNodeUponCreation READ selectNodeUponCreation WRITE setSelectNodeUponCreation)
  Q_PROPERTY(bool noneEnabled READ noneEnabled WRITE setNoneEnabled)
  Q_PROPERTY(bool addEnabled READ addEnabled WRITE setAddEnabled)
  Q_PROPERTY(bool removeEnabled READ removeEnabled WRITE setRemoveEnabled)
  Q_PROPERTY(bool editEnabled READ editEnabled WRITE setEditEnabled)
  Q_PROPERTY(bool renameEnabled READ renameEnabled WRITE setRenameEnabled)
  Q_PROPERTY(QString currentNodeId READ currentNodeId WRITE setCurrentNode DESIGNABLE false)
  Q_PROPERTY(QComboBox::SizeAdjustPolicy sizeAdjustPolicy READ sizeAdjustPolicy WRITE setSizeAdjustPolicy)
  */
public:
  /// Superclass typedef
  typedef qMRMLNodeComboBox Superclass;

  /// Construct an empty qMRMLCheckableNodeComboBox with a null scene,
  /// no nodeType, where the hidden nodes are not forced on display.
  explicit qMRMLCheckableNodeComboBox(QWidget* parent = 0);
  virtual ~qMRMLCheckableNodeComboBox();

private:
  Q_DECLARE_PRIVATE(qMRMLCheckableNodeComboBox);
  Q_DISABLE_COPY(qMRMLCheckableNodeComboBox);
};

#endif
