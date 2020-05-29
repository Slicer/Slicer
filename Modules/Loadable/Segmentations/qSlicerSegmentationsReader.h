/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Adam Rankin, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __qSlicerSegmentationsReader_h
#define __qSlicerSegmentationsReader_h

// Slicer includes
#include "qSlicerFileReader.h"
class qSlicerSegmentationsReaderPrivate;

// Slicer includes
class vtkSlicerSegmentationsModuleLogic;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Segmentations
class qSlicerSegmentationsReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerSegmentationsReader(vtkSlicerSegmentationsModuleLogic* segmentationsLogic = nullptr, QObject* parent = nullptr);
  ~qSlicerSegmentationsReader() override;

  void setSegmentationsLogic(vtkSlicerSegmentationsModuleLogic* segmentationsLogic);
  vtkSlicerSegmentationsModuleLogic* segmentationsLogic()const;

  QString description()const override;
  IOFileType fileType()const override;
  QStringList extensions()const override;
  qSlicerIOOptions* options()const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerSegmentationsReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerSegmentationsReader);
  Q_DISABLE_COPY(qSlicerSegmentationsReader);
};

#endif
