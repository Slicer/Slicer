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

// Slicer includes
#include "qSlicerTransformsReader.h"

// Logic includes
#include "vtkSlicerTransformLogic.h"

// MRML includes
#include <vtkMRMLTransformNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerTransformsReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerTransformLogic> TransformLogic;
};

//-----------------------------------------------------------------------------
qSlicerTransformsReader::qSlicerTransformsReader(
  vtkSlicerTransformLogic* _transformLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTransformsReaderPrivate)
{
  this->setTransformLogic(_transformLogic);
}


//-----------------------------------------------------------------------------
qSlicerTransformsReader::~qSlicerTransformsReader() = default;

//-----------------------------------------------------------------------------
void qSlicerTransformsReader::setTransformLogic(vtkSlicerTransformLogic* newTransformLogic)
{
  Q_D(qSlicerTransformsReader);
  d->TransformLogic = newTransformLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerTransformLogic* qSlicerTransformsReader::transformLogic()const
{
  Q_D(const qSlicerTransformsReader);
  return d->TransformLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsReader::description()const
{
  return "Transform";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTransformsReader::fileType()const
{
  return QString("TransformFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformsReader::extensions()const
{
  return QStringList() << "Transform (*.h5 *.tfm *.mat *.nrrd *.nhdr *.mha *.mhd *.nii *.nii.gz *.txt *.hdf5 *.he5)";
}

//-----------------------------------------------------------------------------
bool qSlicerTransformsReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTransformsReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  if (d->TransformLogic.GetPointer() == nullptr)
    {
    return false;
    }
  vtkMRMLTransformNode* node = d->TransformLogic->AddTransform(
    fileName.toUtf8(), this->mrmlScene());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != nullptr;
}

// TODO: add the save() method. Use vtkSlicerTransformLogic::SaveTransform()
