#pragma once

// LayerDM includes
#include "vtkMRMLLayerDMObjectEventObserver.h"
#include "vtkSlicerLayerDMModuleMRMLExport.h"

// VTK includes
#include <vtkPython.h>
#include <vtkSmartPyObject.h>

class vtkCallbackCommand;

/// Python lambda implementation of \sa vtkMRMLLayerDMObjectEventObserver
/// Delegates full callback to underlying Python callable object.
class VTK_SLICER_LAYERDM_MODULE_MRML_EXPORT vtkMRMLLayerDMObjectEventObserverScripted : public vtkMRMLLayerDMObjectEventObserver
{
public:
  static vtkMRMLLayerDMObjectEventObserverScripted* New();
  vtkTypeMacro(vtkMRMLLayerDMObjectEventObserverScripted, vtkMRMLLayerDMObjectEventObserver);

  /// \brief Sets the Python callable object to be invoked on events.
  ///
  /// The provided Python object should be callable (e.g., a function, lambda,
  /// or an object with a __call__ method). This callable will be invoked
  /// whenever the observed event is triggered.
  ///
  /// The python callable will be invoked with the following args:
  ///   (vtkObject* node, unsigned long eventId, void* callData)
  ///
  /// If callData is not nullptr, the callData can be converted to the right
  /// Python type using the \see CastCallData method.
  void SetPythonCallback(PyObject* object);

  /// \brief Cast call data to appropriate Python type based on VTK type
  /// \param object Python object to cast
  /// \param vtkType VTK type identifier to cast to
  /// \return PyObject* Casted Python object
  static PyObject* CastCallData(PyObject* object, int vtkType);

protected:
  vtkMRMLLayerDMObjectEventObserverScripted();
  ~vtkMRMLLayerDMObjectEventObserverScripted() override;

private:
  PyObject* m_object;
};
