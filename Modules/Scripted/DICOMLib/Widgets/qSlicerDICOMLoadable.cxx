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

// DICOMLib includes
#include "qSlicerDICOMLoadable.h"
#include "vtkSlicerDICOMLoadable.h"

// CTK includes
#include <ctkPimpl.h>

//-----------------------------------------------------------------------------
class qSlicerDICOMLoadablePrivate
{
public:
  qSlicerDICOMLoadablePrivate();
  virtual ~qSlicerDICOMLoadablePrivate();

  /// Name exposed to the user for the node
  QString Name;
  /// Extra information the user sees on mouse over of the thing
  QString Tooltip;
  /// Things the user should know before loading this data
  QString Warning;
  /// The file list of the data to be loaded
  QStringList Files;
  /// Is the object checked for loading by default
  bool Selected;
  /// Confidence - from 0 to 1 where 0 means low chance
  /// that the user actually wants to load their data this
  /// way up to 1, which means that the plugin is very confident
  /// that this is the best way to load the data.
  /// When more than one plugin marks the same series as
  /// selected, the one with the highest confidence is
  /// actually selected by default.  In the case of a tie,
  /// both series are selected for loading.
  double Confidence;
  /// List of UIDs for the DICOM instances that are referenced by this loadable
  QStringList ReferencedInstanceUIDs;
};

//-----------------------------------------------------------------------------
// qSlicerDICOMLoadablePrivate methods

//-----------------------------------------------------------------------------
qSlicerDICOMLoadablePrivate::qSlicerDICOMLoadablePrivate()
{
  this->Name = QString("Unknown");
  this->Tooltip = QString("No further information available");
  this->Warning = QString("");
  this->Selected = false;
  this->Confidence = 0.5;
}

//-----------------------------------------------------------------------------
qSlicerDICOMLoadablePrivate::~qSlicerDICOMLoadablePrivate() = default;


//-----------------------------------------------------------------------------
// qSlicerDICOMLoadable methods

//-----------------------------------------------------------------------------
qSlicerDICOMLoadable::qSlicerDICOMLoadable(QObject* parentObject)
  : Superclass(parentObject)
  , d_ptr(new qSlicerDICOMLoadablePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDICOMLoadable::~qSlicerDICOMLoadable() = default;

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMLoadable, const QString&, setName, Name)
CTK_GET_CPP(qSlicerDICOMLoadable, QString, name, Name)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMLoadable, const QString&, setTooltip, Tooltip)
CTK_GET_CPP(qSlicerDICOMLoadable, QString, tooltip, Tooltip)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMLoadable, const QString&, setWarning, Warning)
CTK_GET_CPP(qSlicerDICOMLoadable, QString, warning, Warning)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMLoadable, const QStringList&, setFiles, Files)
CTK_GET_CPP(qSlicerDICOMLoadable, QStringList, files, Files)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMLoadable, const bool, setSelected, Selected)
CTK_GET_CPP(qSlicerDICOMLoadable, bool, selected, Selected)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMLoadable, const double, setConfidence, Confidence)
CTK_GET_CPP(qSlicerDICOMLoadable, double, confidence, Confidence)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMLoadable, const QStringList&, setReferencedInstanceUIDs, ReferencedInstanceUIDs)
CTK_GET_CPP(qSlicerDICOMLoadable, QStringList, referencedInstanceUIDs, ReferencedInstanceUIDs)

//-----------------------------------------------------------------------------
void qSlicerDICOMLoadable::copyToVtkLoadable(vtkSlicerDICOMLoadable* vtkLoadable)
{
  Q_D(qSlicerDICOMLoadable);

  if (!vtkLoadable)
    {
    return;
    }

  vtkLoadable->SetName(d->Name.toUtf8().constData());
  vtkLoadable->SetTooltip(d->Tooltip.toUtf8().constData());
  vtkLoadable->SetWarning(d->Warning.toUtf8().constData());
  vtkLoadable->SetSelected(d->Selected);
  vtkLoadable->SetConfidence(d->Confidence);

  foreach(QString file, d->Files)
    {
    vtkLoadable->AddFile(file.toUtf8().constData());
    }

  foreach(QString referencedInstanceUID, d->ReferencedInstanceUIDs)
    {
    vtkLoadable->AddReferencedInstanceUID(referencedInstanceUID.toUtf8().constData());
    }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMLoadable::copyFromVtkLoadable(vtkSlicerDICOMLoadable* vtkLoadable)
{
  Q_D(qSlicerDICOMLoadable);

  if (!vtkLoadable)
    {
    return;
    }

  d->Name = QString(vtkLoadable->GetName());
  d->Tooltip = QString(vtkLoadable->GetTooltip());
  d->Warning = QString(vtkLoadable->GetWarning());
  d->Selected = vtkLoadable->GetSelected();
  d->Confidence = vtkLoadable->GetConfidence();

  vtkStringArray* filesArray = vtkLoadable->GetFiles();
  if (filesArray)
    {
    for (int fileIndex = 0; fileIndex < filesArray->GetNumberOfValues(); ++fileIndex)
      {
      d->Files.append(QString(filesArray->GetValue(fileIndex)));
      }
    }

  vtkStringArray* referencedInstanceUIDsArray = vtkLoadable->GetReferencedInstanceUIDs();
  if (referencedInstanceUIDsArray)
    {
    for (int fileIndex = 0; fileIndex < referencedInstanceUIDsArray->GetNumberOfValues(); ++fileIndex)
      {
      d->ReferencedInstanceUIDs.append(QString(referencedInstanceUIDsArray->GetValue(fileIndex)));
      }
    }
}
