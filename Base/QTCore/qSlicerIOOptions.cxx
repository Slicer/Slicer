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

#include "qSlicerIOOptions.h"
#include "qSlicerIOOptions_p.h"

//------------------------------------------------------------------------------
qSlicerIOOptionsPrivate::~qSlicerIOOptionsPrivate() = default;

//------------------------------------------------------------------------------
qSlicerIOOptions::qSlicerIOOptions()
  : d_ptr(new qSlicerIOOptionsPrivate)
{
}

//------------------------------------------------------------------------------
qSlicerIOOptions::qSlicerIOOptions(qSlicerIOOptionsPrivate* pimpl)
  : d_ptr(pimpl)
{
}

//------------------------------------------------------------------------------
qSlicerIOOptions::~qSlicerIOOptions() = default;

//------------------------------------------------------------------------------
bool qSlicerIOOptions::isValid()const
{
  Q_D(const qSlicerIOOptions);
  return d->Properties.size() > 0;
}

//------------------------------------------------------------------------------
const qSlicerIO::IOProperties& qSlicerIOOptions::properties()const
{
  Q_D(const qSlicerIOOptions);
  return d->Properties;
}

//------------------------------------------------------------------------------
void qSlicerIOOptions::updateValid()
{
  Q_D(qSlicerIOOptions);
  d->ArePropertiesValid = this->isValid();
}
