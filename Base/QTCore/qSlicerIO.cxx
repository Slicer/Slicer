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

/// QtCore includes
#include "qSlicerIO.h"

// MRML includes
#include <vtkMRMLMessageCollection.h>

// CTK includes
#include <ctkPimpl.h>

//-----------------------------------------------------------------------------
class qSlicerIOPrivate
{
  Q_DECLARE_PUBLIC(qSlicerIO);

protected:
  qSlicerIO* q_ptr;

public:
  qSlicerIOPrivate(qSlicerIO& object);
  virtual ~qSlicerIOPrivate();

  vtkMRMLMessageCollection* UserMessages;
};

//-----------------------------------------------------------------------------
// qSlicerIOPrivate methods

//-----------------------------------------------------------------------------
qSlicerIOPrivate::qSlicerIOPrivate(qSlicerIO& object)
  : q_ptr(&object)
{
  this->UserMessages = vtkMRMLMessageCollection::New();
}

//-----------------------------------------------------------------------------
qSlicerIOPrivate::~qSlicerIOPrivate()
{
  this->UserMessages->Delete();
  this->UserMessages = nullptr;
}

//-----------------------------------------------------------------------------
// qSlicerIO methods

CTK_GET_CPP(qSlicerIO, vtkMRMLMessageCollection*, userMessages, UserMessages);

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
