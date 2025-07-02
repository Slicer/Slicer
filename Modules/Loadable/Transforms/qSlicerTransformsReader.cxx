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
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLTransformNode.h>

// ITK includes
#include "itkMetaDataObject.h"
#include "itkNiftiImageIO.h"

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerTransformsReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerTransformLogic> TransformLogic;
};

//-----------------------------------------------------------------------------
qSlicerTransformsReader::qSlicerTransformsReader(vtkSlicerTransformLogic* _transformLogic, QObject* _parent)
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
vtkSlicerTransformLogic* qSlicerTransformsReader::transformLogic() const
{
  Q_D(const qSlicerTransformsReader);
  return d->TransformLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerTransformsReader::description() const
{
  return "Transform";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTransformsReader::fileType() const
{
  return QString("TransformFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformsReader::extensions() const
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

  this->userMessages()->ClearMessages();
  vtkMRMLTransformNode* node =
    d->TransformLogic->AddTransform(fileName.toUtf8(), this->mrmlScene(), this->userMessages());
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

//----------------------------------------------------------------------------
double qSlicerTransformsReader::canLoadFileConfidence(const QString& fileName) const
{
  double confidence = Superclass::canLoadFileConfidence(fileName);
  if (confidence > 0)
  {
    // Set higher confidence for NIFTI files containing displacement field
    QString upperCaseFileName = fileName.toUpper();
    if (upperCaseFileName.endsWith(".NII") || upperCaseFileName.endsWith(".NII.GZ"))
    {
      // Use lower than default confidence value unless it turns out that this file contains a displacement field.
      confidence = 0.4;
      using ImageIOType = itk::NiftiImageIO;
      ImageIOType::Pointer niftiIO = ImageIOType::New();
      niftiIO->SetFileName(fileName.toStdString());
      try
      {
        niftiIO->ReadImageInformation();
        const itk::MetaDataDictionary& metadata = niftiIO->GetMetaDataDictionary();
        std::string niftiIntentCode;
        if (itk::ExposeMetaData<std::string>(metadata, "intent_code", niftiIntentCode))
        {
          // This is a NIFTI file. Verify that it contains a displacement vector image
          // by checking that the "intent code" metadata field equals 1006 (NIFTI_INTENT_DISPVECT).
          if (niftiIntentCode == "1006")
          {
            confidence = 0.6;
          }
        }
      }
      catch (...)
      {
        // Something went wrong, we do not need to know the details, it is enough to know that
        // this does not look like a transform file.
      }
    }
  }
  return confidence;
}
