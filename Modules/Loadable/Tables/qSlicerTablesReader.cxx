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

// Qt includes
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerTablesReader.h"

// Logic includes
#include "vtkSlicerTablesLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLSelectionNode.h>
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerTablesReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerTablesLogic> Logic;
};

//-----------------------------------------------------------------------------
qSlicerTablesReader::qSlicerTablesReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTablesReaderPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTablesReader
::qSlicerTablesReader(vtkSlicerTablesLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTablesReaderPrivate)
{
  this->setLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerTablesReader::~qSlicerTablesReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerTablesReader::setLogic(vtkSlicerTablesLogic* logic)
{
  Q_D(qSlicerTablesReader);
  d->Logic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerTablesLogic* qSlicerTablesReader::logic()const
{
  Q_D(const qSlicerTablesReader);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerTablesReader::description()const
{
  return "Table";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTablesReader::fileType()const
{
  return QString("TableFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesReader::extensions()const
{
  return QStringList()
    << "Table (*.tsv)"
    << "Table (*.csv)"
    << "Table (*.txt)"
    ;
}

//-----------------------------------------------------------------------------
bool qSlicerTablesReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTablesReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }
  std::string uname = this->mrmlScene()->GetUniqueNameByString(name.toLatin1());
  vtkMRMLTableNode* node = NULL;
  if (d->Logic!=NULL)
    {
    node = d->Logic->AddTable(fileName.toLatin1(),uname.c_str());
    }
  if (node)
    {
    // Show table in viewers
    vtkSlicerApplicationLogic* appLogic = d->Logic->GetApplicationLogic();
    vtkMRMLSelectionNode* selectionNode = appLogic ? appLogic->GetSelectionNode() : 0;
    if (selectionNode)
      {
      selectionNode->SetReferenceActiveTableID(node->GetID());
      }
    if (appLogic)
      {
      appLogic->PropagateTableSelection();
      }
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    qCritical("Failed to read table from %s", qPrintable(fileName));
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}
