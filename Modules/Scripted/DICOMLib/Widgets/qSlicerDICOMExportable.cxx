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
#include "qSlicerDICOMExportable.h"
#include "vtkSlicerDICOMExportable.h"

// CTK includes
#include <ctkPimpl.h>

//-----------------------------------------------------------------------------
class qSlicerDICOMExportablePrivate
{
public:
  qSlicerDICOMExportablePrivate();
  virtual ~qSlicerDICOMExportablePrivate();

  /// Name exposed to the user for the export method
  QString Name;
  /// Extra information the user sees on mouse over of the export option
  QString Tooltip;
  /// ID of the subject hierarchy item to be exported
  vtkIdType SubjectHierarchyItemID;
  /// Class of the plugin that created this exportable
  QString PluginClass;
  /// Target directory to export this exportable
  QString Directory;
  /// Confidence - from 0 to 1 where 0 means that the plugin
  /// cannot export the given node, up to 1 that means that the
  /// plugin considers itself the best plugin to export the node
  /// (in case of specialized objects, e.g. RT dose volume)
  double Confidence;
  /// Pseudo-tags offered by the plugin that are to be filled out for export.
  /// The pseudo-tags are translated into real DICOM tags at the time of export.
  /// It tag is a pair of strings (name, value). When the exportable is created
  /// by the DICOM plugin, value is the default value that is set in the editor widget
  QMap<QString,QString> Tags;
};

//-----------------------------------------------------------------------------
// qSlicerDICOMExportablePrivate methods

//-----------------------------------------------------------------------------
qSlicerDICOMExportablePrivate::qSlicerDICOMExportablePrivate()
{
  this->Name = QString("Unknown exporter");
  this->Tooltip = QString("Creates a DICOM file from the selected data");
  this->Confidence = 0.0;
  this->SubjectHierarchyItemID = 0;
}

//-----------------------------------------------------------------------------
qSlicerDICOMExportablePrivate::~qSlicerDICOMExportablePrivate() = default;


//-----------------------------------------------------------------------------
// qSlicerDICOMExportable methods

//-----------------------------------------------------------------------------
qSlicerDICOMExportable::qSlicerDICOMExportable(QObject* parentObject)
  : Superclass(parentObject)
  , d_ptr(new qSlicerDICOMExportablePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDICOMExportable::~qSlicerDICOMExportable() = default;

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMExportable, const QString&, setName, Name)
CTK_GET_CPP(qSlicerDICOMExportable, QString, name, Name)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMExportable, const QString&, setTooltip, Tooltip)
CTK_GET_CPP(qSlicerDICOMExportable, QString, tooltip, Tooltip)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMExportable, const vtkIdType&, setSubjectHierarchyItemID, SubjectHierarchyItemID)
CTK_GET_CPP(qSlicerDICOMExportable, vtkIdType, subjectHierarchyItemID, SubjectHierarchyItemID)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMExportable, const QString&, setPluginClass, PluginClass)
CTK_GET_CPP(qSlicerDICOMExportable, QString, pluginClass, PluginClass)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMExportable, const QString&, setDirectory, Directory)
CTK_GET_CPP(qSlicerDICOMExportable, QString, directory, Directory)

//-----------------------------------------------------------------------------
CTK_SET_CPP(qSlicerDICOMExportable, const double, setConfidence, Confidence)
CTK_GET_CPP(qSlicerDICOMExportable, double, confidence, Confidence)

//-----------------------------------------------------------------------------
QMap<QString,QString> qSlicerDICOMExportable::tags()const
{
  Q_D(const qSlicerDICOMExportable);
  return d->Tags;
}
//-----------------------------------------------------------------------------
void qSlicerDICOMExportable::setTags(const QMap<QString,QString>& var)
{
  Q_D(qSlicerDICOMExportable);
  d->Tags = var;
}

//-----------------------------------------------------------------------------
QString qSlicerDICOMExportable::tag(QString tagName)
{
  Q_D(qSlicerDICOMExportable);
  // Returns QString() if tagName is not in the tags map, which contains Null value for QString
  return d->Tags[tagName];
}
//-----------------------------------------------------------------------------
void qSlicerDICOMExportable::setTag(QString tagName, QString tagValue)
{
  Q_D(qSlicerDICOMExportable);
  d->Tags[tagName] = tagValue;
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportable::copyToVtkExportable(vtkSlicerDICOMExportable* vtkExportable)
{
  Q_D(qSlicerDICOMExportable);

  if (!vtkExportable)
    {
    return;
    }

  vtkExportable->SetName(d->Name.toUtf8().constData());
  vtkExportable->SetTooltip(d->Tooltip.toUtf8().constData());
  vtkExportable->SetSubjectHierarchyItemID(d->SubjectHierarchyItemID);
  vtkExportable->SetPluginClass(d->PluginClass.toUtf8().constData());
  vtkExportable->SetDirectory(d->Directory.toUtf8().constData());
  vtkExportable->SetConfidence(d->Confidence);

  QMapIterator<QString,QString> tagsIt(d->Tags);
  while (tagsIt.hasNext())
    {
    tagsIt.next();
    vtkExportable->SetTag(tagsIt.key().toUtf8().constData(), tagsIt.value().toUtf8().constData());
    }
}

//-----------------------------------------------------------------------------
void qSlicerDICOMExportable::copyFromVtkExportable(vtkSlicerDICOMExportable* vtkExportable)
{
  Q_D(qSlicerDICOMExportable);

  if (!vtkExportable)
    {
    return;
    }

  d->Name = QString(vtkExportable->GetName());
  d->Tooltip = QString(vtkExportable->GetTooltip());
  d->SubjectHierarchyItemID = vtkExportable->GetSubjectHierarchyItemID();
  d->PluginClass = QString(vtkExportable->GetPluginClass());
  d->Directory = QString(vtkExportable->GetDirectory());
  d->Confidence = vtkExportable->GetConfidence();

  std::map<std::string, std::string> vtkTags = vtkExportable->GetTags();
  for ( std::map<std::string, std::string>::iterator it=vtkTags.begin(); it != vtkTags.end(); ++it )
    {
    this->setTag(it->first.c_str(), it->second.c_str());
    }
}
