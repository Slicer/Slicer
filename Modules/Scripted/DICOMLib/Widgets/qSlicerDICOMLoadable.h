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

#ifndef __qSlicerDICOMLoadable_h
#define __qSlicerDICOMLoadable_h

// Qt includes
#include <QObject>
#include <QStringList>

// DICOMLib includes
#include "qSlicerDICOMLibModuleWidgetsExport.h"

class qSlicerDICOMLoadablePrivate;
class vtkSlicerDICOMLoadable;

/// Container class for objects that can be loaded from DICOM files into Slicer.
/// Each plugin returns a list of instances from its evaluate method and accepts
/// a list of these in its load method corresponding to the things the user has
/// selected for loading
class Q_SLICER_MODULE_DICOMLIB_WIDGETS_EXPORT qSlicerDICOMLoadable : public QObject
{
  Q_OBJECT

  /// Name exposed to the user for the node
  Q_PROPERTY(QString name READ name WRITE setName)
  /// Extra information the user sees on mouse over of the thing
  Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip)
  /// Things the user should know before loading this data
  Q_PROPERTY(QString warning READ warning WRITE setWarning)
  /// The file list of the data to be loaded
  Q_PROPERTY(QStringList files READ files WRITE setFiles)
  /// Is the object checked for loading by default
  Q_PROPERTY(bool selected READ selected WRITE setSelected)
  /// Confidence - from 0 to 1 where 0 means low chance
  /// that the user actually wants to load their data this
  /// way up to 1, which means that the plugin is very confident
  /// that this is the best way to load the data.
  /// When more than one plugin marks the same series as
  /// selected, the one with the highest confidence is
  /// actually selected by default.  In the case of a tie,
  /// both series are selected for loading.
  Q_PROPERTY(double confidence READ confidence WRITE setConfidence)
  /// List of UIDs for the DICOM instances that are referenced by this loadable
  Q_PROPERTY(QStringList referencedInstanceUIDs READ referencedInstanceUIDs WRITE setReferencedInstanceUIDs)

public:
  typedef QObject Superclass;
  qSlicerDICOMLoadable(QObject *parent = nullptr);
  ~qSlicerDICOMLoadable() override;

  virtual QString name()const;
  void setName(const QString& newName);

  virtual QString tooltip()const;
  void setTooltip(const QString& newTooltip);

  virtual QString warning()const;
  void setWarning(const QString& newWarning);

  virtual QStringList files()const;
  void setFiles(const QStringList& newFiles);

  virtual bool selected()const;
  void setSelected(const bool newSelected);

  virtual double confidence()const;
  void setConfidence(const double newConfidence);

  virtual QStringList referencedInstanceUIDs()const;
  void setReferencedInstanceUIDs(const QStringList& newReferencedInstanceUIDs);

  /// Copy values to VTK loadable
  Q_INVOKABLE void copyToVtkLoadable(vtkSlicerDICOMLoadable* vtkLoadable);

  /// Copy values from VTK loadable
  Q_INVOKABLE void copyFromVtkLoadable(vtkSlicerDICOMLoadable* vtkLoadable);

protected:
  QScopedPointer<qSlicerDICOMLoadablePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerDICOMLoadable);
  Q_DISABLE_COPY(qSlicerDICOMLoadable);
};

#endif
