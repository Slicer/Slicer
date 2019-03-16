/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerDICOMExportable_h
#define __qSlicerDICOMExportable_h

// Qt includes
#include <QObject>
#include <QStringList>
#include <QMap>

// MRML includes
#include <vtkType.h>

// DICOMLib includes
#include "qSlicerDICOMLibModuleWidgetsExport.h"

class qSlicerDICOMExportablePrivate;
class vtkSlicerDICOMExportable;

/// Container class for data that can be exported to DICOM files from Slicer.
/// Each plugin returns a list of instances from its examineForExport method and accepts
/// a list of these in its export method corresponding to the things the user has
/// selected for loading
class Q_SLICER_MODULE_DICOMLIB_WIDGETS_EXPORT qSlicerDICOMExportable : public QObject
{
  Q_OBJECT

  /// Name exposed to the user for the export method
  Q_PROPERTY(QString name READ name WRITE setName)
  /// Extra information the user sees on mouse over of the export option
  Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip)
  /// ID of the subject hierarchy item to be exported
  Q_PROPERTY(vtkIdType subjectHierarchyItemID READ subjectHierarchyItemID WRITE setSubjectHierarchyItemID)
  /// Class of the plugin that created this exportable
  Q_PROPERTY(QString pluginClass READ pluginClass WRITE setPluginClass)
  /// Target directory to export this exportable
  Q_PROPERTY(QString directory READ directory WRITE setDirectory)
  /// Confidence - from 0 to 1 where 0 means that the plugin
  /// cannot export the given node, up to 1 that means that the
  /// plugin considers itself the best plugin to export the node
  /// (in case of specialized objects, e.g. RT dose volume)
  Q_PROPERTY(double confidence READ confidence WRITE setConfidence)
  /// Pseudo-tags offered by the plugin that are to be filled out for export.
  /// The pseudo-tags are translated into real DICOM tags at the time of export.
  /// It tag is a pair of strings (name, value). When the exportable is created
  /// by the DICOM plugin, value is the default value that is set in the editor widget
  typedef QMap<QString,QString> TagsMap;
  Q_PROPERTY(TagsMap tags READ tags WRITE setTags);

public:
  typedef QObject Superclass;
  qSlicerDICOMExportable(QObject *parent = nullptr);
  ~qSlicerDICOMExportable() override;

  virtual QString name()const;
  void setName(const QString& newName);

  virtual QString tooltip()const;
  void setTooltip(const QString& newTooltip);

  virtual vtkIdType subjectHierarchyItemID()const;
  void setSubjectHierarchyItemID(const vtkIdType& newItemID);

  virtual QString directory()const;
  void setDirectory(const QString& newDirectory);

  virtual QString pluginClass()const;
  void setPluginClass(const QString& newPluginClass);

  virtual double confidence()const;
  void setConfidence(const double newConfidence);

  virtual QMap<QString,QString> tags()const;
  void setTags(const QMap<QString,QString>& newTags);

  /// Get one tag from tags list
  /// \return Null QString if tag is absent, otherwise tag value
  Q_INVOKABLE QString tag(QString tagName);
  /// Set one tag to tags list
  Q_INVOKABLE void setTag(QString tagName, QString tagValue);

  /// Copy values to VTK exportable
  Q_INVOKABLE void copyToVtkExportable(vtkSlicerDICOMExportable* vtkExportable);

  /// Copy values from VTK exportable
  Q_INVOKABLE void copyFromVtkExportable(vtkSlicerDICOMExportable* vtkExportable);

protected:
  QScopedPointer<qSlicerDICOMExportablePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDICOMExportable);
  Q_DISABLE_COPY(qSlicerDICOMExportable);
};

#endif
