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
#include <QPixmap>

#include "qSlicerIO.h"

// Slicer includes
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLMessageCollection.h>

// VTK includes
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkVariantArray.h>

// CTK includes
#include <ctkPimpl.h>

// STD includes
#include <cstring>

//-----------------------------------------------------------------------------
class qSlicerIOPrivate
{
  Q_DECLARE_PUBLIC(qSlicerIO);

protected:
  qSlicerIO* q_ptr;

public:
  qSlicerIOPrivate(qSlicerIO& object);
  virtual ~qSlicerIOPrivate();

  vtkSmartPointer<vtkMRMLMessageCollection> UserMessages;
};

//-----------------------------------------------------------------------------
// qSlicerIOPrivate methods

//-----------------------------------------------------------------------------
qSlicerIOPrivate::qSlicerIOPrivate(qSlicerIO& object)
  : q_ptr(&object)
{
  this->UserMessages = vtkSmartPointer<vtkMRMLMessageCollection>::New();
}

//-----------------------------------------------------------------------------
qSlicerIOPrivate::~qSlicerIOPrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerIO methods

//----------------------------------------------------------------------------
qSlicerIO::qSlicerIO(QObject* parentObject)
  : Superclass(parentObject)
  , d_ptr(new qSlicerIOPrivate(*this))
{
  qRegisterMetaType<qSlicerIO::IOFileType>("qSlicerIO::IOFileType");
  qRegisterMetaType<qSlicerIO::IOProperties>("qSlicerIO::IOProperties");
}

//----------------------------------------------------------------------------
qSlicerIO::~qSlicerIO() = default;

//----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerIO::options() const
{
  return nullptr;
}

//----------------------------------------------------------------------------
vtkMRMLMessageCollection* qSlicerIO::userMessages() const
{
  Q_D(const qSlicerIO);
  return d->UserMessages;
}

//----------------------------------------------------------------------------
vtkMRMLFileIOHandler* qSlicerIO::ioHandler() const
{
  return nullptr;
}

//----------------------------------------------------------------------------
void qSlicerIO::setMRMLScene(vtkMRMLScene* scene)
{
  this->qSlicerObject::setMRMLScene(scene);
}

namespace
{
//----------------------------------------------------------------------------
int qVariantType(const QVariant& value)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  return value.typeId();
#else
  return value.userType();
#endif
}

//----------------------------------------------------------------------------
/// Convert a Qt value to a VTK value (used for items of lists).
/// Boolean values are stored as vtkVariant of char type (see vtkVariant(bool)).
/// Returns false if the value type is not supported.
bool qVariantToVTKVariant(const QVariant& value, vtkVariant& result)
{
  switch (qVariantType(value))
  {
    case QMetaType::Bool: result = vtkVariant(value.toBool()); return true;
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::UChar: result = vtkVariant(value.toInt()); return true;
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Long:
    case QMetaType::ULong: result = vtkVariant(value.toLongLong()); return true;
    case QMetaType::Float:
    case QMetaType::Double: result = vtkVariant(value.toDouble()); return true;
    case QMetaType::QString: result = vtkVariant(value.toString().toStdString()); return true;
    default: return false;
  }
}

//----------------------------------------------------------------------------
/// Convert a VTK value to a Qt value (used for items of lists and for single values).
QVariant vtkVariantToQVariant(const vtkVariant& value)
{
  if (value.IsChar())
  {
    // boolean values are stored as char (see vtkVariant(bool))
    return QVariant(value.ToChar() != 0);
  }
  if (value.IsString())
  {
    return QVariant(QString::fromStdString(value.ToString()));
  }
  if (value.IsFloat() || value.IsDouble())
  {
    return QVariant(value.ToDouble());
  }
  if (value.IsLongLong() || value.IsUnsignedLongLong() || value.IsLong() || value.IsUnsignedLong())
  {
    return QVariant(static_cast<qlonglong>(value.ToTypeInt64()));
  }
  if (value.IsNumeric())
  {
    return QVariant(value.ToInt());
  }
  return QVariant(QString::fromStdString(value.ToString()));
}
} // namespace

//----------------------------------------------------------------------------
void qSlicerIO::toVTKProperties(const IOProperties& properties, vtkMRMLIOProperties* vtkProperties)
{
  if (!vtkProperties)
  {
    return;
  }
  vtkProperties->RemoveAllProperties();
  for (IOProperties::const_iterator it = properties.constBegin(); it != properties.constEnd(); ++it)
  {
    const std::string name = it.key().toStdString();
    const QVariant& value = it.value();
    if (!value.isValid())
    {
      // None value
      continue;
    }
    const int type = qVariantType(value);
    switch (type)
    {
      case QMetaType::Bool: vtkProperties->SetBoolProperty(name, value.toBool()); break;
      case QMetaType::Int:
      case QMetaType::UInt:
      case QMetaType::Short:
      case QMetaType::UShort:
      case QMetaType::Char:
      case QMetaType::UChar: vtkProperties->SetIntProperty(name, value.toInt()); break;
      case QMetaType::LongLong:
      case QMetaType::ULongLong:
      case QMetaType::Long:
      case QMetaType::ULong: vtkProperties->SetProperty(name, vtkVariant(value.toLongLong())); break;
      case QMetaType::Float:
      case QMetaType::Double: vtkProperties->SetDoubleProperty(name, value.toDouble()); break;
      case QMetaType::QString: vtkProperties->SetStringProperty(name, value.toString().toStdString()); break;
      case QMetaType::QStringList:
      {
        std::vector<std::string> values;
        for (const QString& item : value.toStringList())
        {
          values.push_back(item.toStdString());
        }
        vtkProperties->SetStringListProperty(name, values);
        break;
      }
      case QMetaType::QVariantList:
      {
        vtkNew<vtkVariantArray> values;
        bool valid = true;
        for (const QVariant& item : value.toList())
        {
          vtkVariant vtkItem;
          if (!qVariantToVTKVariant(item, vtkItem))
          {
            qCritical() << Q_FUNC_INFO << "Property" << it.key() << "is ignored, its list contains a value of unsupported type:" << item.typeName();
            valid = false;
            break;
          }
          values->InsertNextValue(vtkItem);
        }
        if (valid)
        {
          vtkProperties->SetListProperty(name, values);
        }
        break;
      }
      case QMetaType::QVariantMap:
      case QMetaType::QVariantHash:
      {
        IOProperties nestedProperties;
        if (type == QMetaType::QVariantMap)
        {
          nestedProperties = value.toMap();
        }
        else
        {
          const QVariantHash hash = value.toHash();
          for (QVariantHash::const_iterator hashIt = hash.constBegin(); hashIt != hash.constEnd(); ++hashIt)
          {
            nestedProperties[hashIt.key()] = hashIt.value();
          }
        }
        vtkNew<vtkMRMLIOProperties> nestedVTKProperties;
        qSlicerIO::toVTKProperties(nestedProperties, nestedVTKProperties);
        vtkProperties->SetMapProperty(name, nestedVTKProperties);
        break;
      }
      case QMetaType::QImage:
      case QMetaType::QPixmap:
      {
        QImage image = value.value<QImage>();
        if (type == QMetaType::QPixmap)
        {
          image = value.value<QPixmap>().toImage();
        }
        vtkNew<vtkImageData> imageData;
        if (qSlicerIO::qImageToVTKImageData(image, imageData))
        {
          vtkProperties->SetObjectProperty(name, imageData);
        }
        break;
      }
      default: qCritical() << Q_FUNC_INFO << "Property" << it.key() << "is ignored, its type is not supported:" << value.typeName();
    }
  }
}

//----------------------------------------------------------------------------
qSlicerIO::IOProperties qSlicerIO::fromVTKProperties(vtkMRMLIOProperties* vtkProperties)
{
  IOProperties properties;
  if (!vtkProperties)
  {
    return properties;
  }
  for (const std::string& name : vtkProperties->GetPropertyNames())
  {
    const QString key = QString::fromStdString(name);
    if (vtkProperties->IsBoolProperty(name))
    {
      properties[key] = vtkProperties->GetBoolProperty(name);
    }
    else if (vtkProperties->IsStringListProperty(name))
    {
      QStringList values;
      for (const std::string& value : vtkProperties->GetStringListProperty(name))
      {
        values << QString::fromStdString(value);
      }
      properties[key] = values;
    }
    else if (vtkProperties->IsListProperty(name))
    {
      vtkNew<vtkVariantArray> vtkValues;
      vtkProperties->GetListProperty(name, vtkValues);
      QVariantList values;
      for (vtkIdType i = 0; i < vtkValues->GetNumberOfValues(); ++i)
      {
        values << vtkVariantToQVariant(vtkValues->GetValue(i));
      }
      properties[key] = values;
    }
    else if (vtkProperties->IsMapProperty(name))
    {
      properties[key] = qSlicerIO::fromVTKProperties(vtkProperties->GetMapProperty(name));
    }
    else if (vtkProperties->IsObjectProperty(name))
    {
      vtkImageData* imageData = vtkImageData::SafeDownCast(vtkProperties->GetObjectProperty(name));
      if (imageData)
      {
        properties[key] = qSlicerIO::vtkImageDataToQImage(imageData);
      }
      else
      {
        vtkObject* object = vtkProperties->GetObjectProperty(name);
        qCritical() << Q_FUNC_INFO << "Property" << key << "is ignored, it cannot be converted to Qt type:" << (object ? object->GetClassName() : "(null)");
      }
    }
    else
    {
      properties[key] = vtkVariantToQVariant(vtkProperties->GetProperty(name));
    }
  }
  return properties;
}

//----------------------------------------------------------------------------
bool qSlicerIO::qImageToVTKImageData(const QImage& image, vtkImageData* imageData)
{
  if (!imageData || image.isNull())
  {
    return false;
  }
  const int width = image.width();
  const int height = image.height();
  // Qt image is upside-down compared to VTK
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
  QImage normalizedImage = image.convertToFormat(QImage::Format_RGBA8888).flipped();
#else
  QImage normalizedImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
#endif
  const int numberOfScalarComponents = 4;
  imageData->SetExtent(0, width - 1, 0, height - 1, 0, 0);
  imageData->AllocateScalars(VTK_UNSIGNED_CHAR, numberOfScalarComponents);
  unsigned char* imageBuffer = static_cast<unsigned char*>(imageData->GetScalarPointer());
  const size_t lineSize = static_cast<size_t>(numberOfScalarComponents) * width;
  for (int y = 0; y < height; ++y)
  {
    memcpy(imageBuffer + y * lineSize, normalizedImage.constScanLine(y), lineSize);
  }
  imageData->GetPointData()->GetScalars()->Modified();
  return true;
}

//----------------------------------------------------------------------------
QImage qSlicerIO::vtkImageDataToQImage(vtkImageData* imageData)
{
  if (!imageData || !imageData->GetPointData() || !imageData->GetPointData()->GetScalars() || imageData->GetScalarType() != VTK_UNSIGNED_CHAR)
  {
    return QImage();
  }
  const int width = imageData->GetDimensions()[0];
  const int height = imageData->GetDimensions()[1];
  const int numberOfScalarComponents = imageData->GetNumberOfScalarComponents();
  QImage::Format pixelFormat;
  switch (numberOfScalarComponents)
  {
    case 1: pixelFormat = QImage::Format_Grayscale8; break;
    case 3: pixelFormat = QImage::Format_RGB888; break;
    case 4: pixelFormat = QImage::Format_RGBA8888; break;
    default: return QImage(); // unsupported pixel format
  }
  const int bytesPerLine = static_cast<int>(imageData->GetIncrements()[1]);
  // The QImage object does not take ownership of the voxel buffer.
  QImage image(static_cast<const uchar*>(imageData->GetScalarPointer()), width, height, bytesPerLine, pixelFormat);
  // Qt image is upside-down compared to VTK. Mirroring deep-copies the pixel buffer.
#if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
  return image.flipped();
#else
  return image.mirrored();
#endif
}
