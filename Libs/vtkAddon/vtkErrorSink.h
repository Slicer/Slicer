/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// \brief vtkErrorSink - class to capture error and warning events
/// reported by a VTK object. This is useful for detecting that a VTK object
/// encountered problems and act accordingly (instead of just logging the event).
///

#ifndef __vtkErrorSink_h
#define __vtkErrorSink_h

#include "vtkAddon.h"

//#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkSmartPointer.h"

#include <vector>

class VTK_ADDON_EXPORT vtkErrorSink : public vtkObject
{
public:
  static vtkErrorSink *New();
  vtkTypeMacro(vtkErrorSink,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;
  typedef vtkErrorSink Self;

  /// Observe error and warnings reported by observedObject.
  virtual void SetObservedObject(vtkObject* observedObject);

  /// Return true if the observed object reported errors.
  virtual bool HasErrors();

  /// Return true if the observed object reported errors.
  virtual bool HasWarnings();

  /// Return number of reported error or warning messages.
  virtual int GetNumberOfMessages();

  /// Display errors using vtkOutputWindowDisplayErrorText
  /// \sa vtkOutputWindowDisplayErrorText
  void DisplayMessages();

protected:
  vtkErrorSink();
  ~vtkErrorSink() override;

  struct Message
    {
    long unsigned int EventId;
    std::string MessageText;
    };

  std::vector<Message> MessageList;
  vtkSmartPointer<vtkObject> ObservedObject;
  vtkSmartPointer<vtkCallbackCommand> CallbackCommand;

private:
  vtkErrorSink(const vtkErrorSink&) = delete;
  void operator=(const vtkErrorSink&) = delete;

  static void CallbackFunction(vtkObject*, long unsigned int,
    void* clientData, void* callData);
};

#endif
