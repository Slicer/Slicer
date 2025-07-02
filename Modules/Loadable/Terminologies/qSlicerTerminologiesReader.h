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

#ifndef __qSlicerTerminologiesReader_h
#define __qSlicerTerminologiesReader_h

// Slicer includes
#include "qSlicerFileReader.h"
class qSlicerTerminologiesReaderPrivate;

// Slicer includes
class vtkSlicerTerminologiesModuleLogic;

//-----------------------------------------------------------------------------
class qSlicerTerminologiesReader : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerTerminologiesReader(vtkSlicerTerminologiesModuleLogic* terminologiesLogic = nullptr,
                             QObject* parent = nullptr);
  ~qSlicerTerminologiesReader() override;

  void setTerminologiesLogic(vtkSlicerTerminologiesModuleLogic* terminologiesLogic);
  vtkSlicerTerminologiesModuleLogic* terminologiesLogic() const;

  QString description() const override;
  IOFileType fileType() const override;
  QStringList extensions() const override;

  /// Returns a positive number (>0) if the reader can load this file.
  /// In case the file uses a generic file extension (such as .json) then the confidence value is adjusted based on
  /// the file content: if the file contains markups information then confidence is increased to 0.6,
  /// otherwise the confidence is decreased to 0.4.
  double canLoadFileConfidence(const QString& file) const override;

  bool load(const IOProperties& properties) override;

protected:
  QScopedPointer<qSlicerTerminologiesReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTerminologiesReader);
  Q_DISABLE_COPY(qSlicerTerminologiesReader);
};

#endif
