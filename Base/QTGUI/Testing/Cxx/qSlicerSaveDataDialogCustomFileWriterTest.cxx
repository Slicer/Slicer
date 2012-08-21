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

// Qt includes
#include <QDebug>
#include <QTimer>

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerSaveDataDialog.h"
#include "qSlicerFileWriter.h"

// MRML includes
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLTransformStorageNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkNew.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Dummy
class qSlicerDummyFileWriter: public qSlicerFileWriter
{
public:
  qSlicerDummyFileWriter(qSlicerIO::IOFileType fileType, QObject* parent = 0)
    : qSlicerFileWriter(parent)
    , FileType(fileType)
    {}
  virtual ~qSlicerDummyFileWriter(){}
  virtual QStringList nodeTags()const {return QStringList() << "LinearTransform";}
  virtual QString description()const{return "Dummy";}
  virtual qSlicerIO::IOFileType fileType()const{return this->FileType;}
  virtual QStringList extensions(vtkObject*)const{return QStringList(QString("MyType(*.mhd *.vtk)"));}

  virtual bool write(const IOProperties& properties);

  qSlicerIO::IOFileType FileType;
};

//-----------------------------------------------------------------------------
bool qSlicerDummyFileWriter::write(const IOProperties& properties)
{
  QStringList nodeIDs = QStringList() << properties["nodeID"].toString();
  qDebug() << "write" << properties["nodeID"].toString();
  this->setWrittenNodes(nodeIDs);
  return true;
}

//-----------------------------------------------------------------------------
int qSlicerSaveDataDialogCustomFileWriterTest(int argc, char * argv[] )
{
  qSlicerApplication app(argc, argv);
  app.coreIOManager()->registerIO(new qSlicerDummyFileWriter(QString("TransformFile"), 0));

  vtkNew<vtkMRMLTransformStorageNode> storageNode;
  app.mrmlScene()->AddNode(storageNode.GetPointer());
  vtkNew<vtkMRMLLinearTransformNode> transformNode;
  app.mrmlScene()->AddNode(transformNode.GetPointer());
  transformNode->SetAndObserveStorageNodeID(storageNode->GetID());

  qSlicerSaveDataDialog saveDataDialog;

  if (argc < 2 || QString(argv[1]) != "-I")
    {
    // Quit the dialog
    QTimer::singleShot(100, &app, SLOT(quit()));
    // Quit the app
    QTimer::singleShot(120, &app, SLOT(quit()));
    }

  return saveDataDialog.exec();
}

