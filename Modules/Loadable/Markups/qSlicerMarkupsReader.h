/*==============================================================================

  Program: 3D Slicer

  Copyright (c) BWH

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef qSlicerMarkupsReader_h
#define qSlicerMarkupsReader_h

// Slicer includes
#include "qSlicerFileReader.h"

class qSlicerMarkupsReaderPrivate;
class vtkSlicerMarkupsLogic;

//----------------------------------------------------------------------------
class qSlicerMarkupsReader : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerMarkupsReader(QObject* parent = nullptr);
  qSlicerMarkupsReader(vtkSlicerMarkupsLogic* logic, QObject* parent = nullptr);
  ~qSlicerMarkupsReader() override;

  vtkSlicerMarkupsLogic* markupsLogic() const;
  void setMarkupsLogic(vtkSlicerMarkupsLogic* logic);

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
  QScopedPointer<qSlicerMarkupsReaderPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerMarkupsReader);
  Q_DISABLE_COPY(qSlicerMarkupsReader);
};

#endif
