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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerAbstractModuleRepresentation_h
#define __qSlicerAbstractModuleRepresentation_h

// Qt includes
#include <QString>

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerObject.h"

#include "qSlicerBaseQTCoreExport.h"

/// class vtkSlicerApplicationLogic;
class vtkMRMLAbstractLogic;
class vtkMRMLNode;
class qSlicerAbstractCoreModule;
class QAction;
class qSlicerAbstractModuleRepresentationPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerAbstractModuleRepresentation : virtual public qSlicerObject
{
public:

  typedef qSlicerObject Superclass;
  qSlicerAbstractModuleRepresentation();
  ~qSlicerAbstractModuleRepresentation() override;

  /// Set/Get module name
  QString moduleName()const;

  /// Returns the module the representation belongs to.
  /// The module is set right before setup() is called.
  qSlicerAbstractCoreModule* module()const;

  /// Allows other modules to select input and output nodes in the module's GUI.
  /// There may be multiple node selectors in a module widget, you can select between them
  /// using the role argument.
  /// Context can be specified to make a selection within that node (for example, a markup list
  /// node may contain multiple markups; context can be used to select a specific item).
  /// Returns true if the selection was successful.
  /// This node has to be overridden in child classes that support editing certain node types.
  /// Preferably each module that defines a new MRML node class should also make sure there
  /// is a module widget that can edit that node.
  /// If setEditedNode method is overridden then nodeEditable method can be overridden to
  /// specify a non-default confidence value that the module can handle a specific node instance.
  virtual bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString());

  /// Returns a confidence value (between 0.0 and 1.0) that defines how much this widget is
  /// suitable for editing the provided node. In general value of 0.5 should be used.
  /// If the returned value is 0 then it means the node should not be attempted to set as edited node.
  /// This node has to be overridden in child classes that want to use a non-default confidence value
  /// (for example, they look into the node contents and decide based on that if the node belongs to
  /// this module).
  virtual double nodeEditable(vtkMRMLNode* node);

protected:
  /// All initialization code (typically setupUi()) must be done in setup()
  /// The module and the logic are accessible.
  virtual void setup() = 0;

  /// Return the logic of the module
  /// The logic is available (not null) when setup() is called. So you can't
  /// access it in the constructor of your module widget.
  vtkMRMLAbstractLogic* logic() const;

protected:
  QScopedPointer<qSlicerAbstractModuleRepresentationPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAbstractModuleRepresentation);
  Q_DISABLE_COPY(qSlicerAbstractModuleRepresentation);

  /// Give access to qSlicerAbstractModule to the method
  /// qSlicerAbstractModuleRepresentation::setModule
  friend class qSlicerAbstractCoreModule;
  void setModule(qSlicerAbstractCoreModule* module);
};

#endif
