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

==============================================================================*/

#ifndef __qSlicerGenericIOOptionsWidget_h
#define __qSlicerGenericIOOptionsWidget_h

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"
#include "qSlicerFileWriterOptionsWidget.h"

// Slicer includes
#include <vtkMRMLFileIOHandler.h>

// VTK includes
#include <vtkSmartPointer.h>

class qSlicerGenericIOOptionsWidgetPrivate;

/// Options widget that displays the options of a VTK-based reader or writer (see vtkMRMLIOOptionsDescription).
///
/// See https://slicer.readthedocs.io/en/latest/developer_guide/mrml_overview.html#reader-and-writer-options
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerGenericIOOptionsWidget : public qSlicerFileWriterOptionsWidget
{
  Q_OBJECT
public:
  typedef qSlicerFileWriterOptionsWidget Superclass;
  explicit qSlicerGenericIOOptionsWidget(QWidget* parent = nullptr);
  qSlicerGenericIOOptionsWidget(vtkMRMLFileIOHandler* ioHandler, QWidget* parent = nullptr);
  ~qSlicerGenericIOOptionsWidget() override;

  /// Reader or writer that describes the options.
  Q_INVOKABLE void setIOHandler(vtkMRMLFileIOHandler* ioHandler);
  Q_INVOKABLE vtkMRMLFileIOHandler* ioHandler() const;

  /// Set values of options (for example, to set default options in a file dialog).
  void updateGUI(const qSlicerIO::IOProperties& ioProperties) override;

  /// Find a registered reader or writer of the specified class (exact class name match) in the
  /// file IO manager of the application logic. Returns nullptr if not found.
  static vtkMRMLFileIOHandler* findIOHandler(const char* className);

  /// Return registered reader or writer of the specified class. If not found then a new instance is created
  /// (it is not registered, but it can access the scene and the application logic).
  template <class T>
  static vtkSmartPointer<vtkMRMLFileIOHandler> findOrCreateIOHandler()
  {
    vtkSmartPointer<vtkMRMLFileIOHandler> newHandler = vtkSmartPointer<T>::New();
    vtkMRMLFileIOHandler* registeredHandler = qSlicerGenericIOOptionsWidget::findIOHandler(newHandler->GetClassName());
    if (registeredHandler)
    {
      return registeredHandler;
    }
    qSlicerGenericIOOptionsWidget::initializeUnregisteredIOHandler(newHandler);
    return newHandler;
  }

public slots:
  void setFileName(const QString& fileName) override;
  void setFileNames(const QStringList& fileNames) override;
  void setObject(vtkObject* object) override;
  void setMRMLScene(vtkMRMLScene* scene) override;

protected slots:
  /// Called when the user changes an option
  void onOptionChanged();

  /// Called when the application is about to quit.
  /// Node selectors stop observing the scene, because nodes may be removed from the scene after the application
  /// object is deleted (if this widget still exists at that time, for example because it was created from Python
  /// without a parent), and node selectors must not update their GUI then.
  void onApplicationAboutToQuit();

protected:
  /// Request the description of options from the reader or writer and update the widgets.
  void updateOptions();

  /// Set the scene and the application logic's file IO manager in a reader or writer that is not registered.
  static void initializeUnregisteredIOHandler(vtkMRMLFileIOHandler* handler);

private:
  Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr), qSlicerGenericIOOptionsWidget);
  Q_DISABLE_COPY(qSlicerGenericIOOptionsWidget);
};

#endif
