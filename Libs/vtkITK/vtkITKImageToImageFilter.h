/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkITKImageToImageFilter_h
#define __vtkITKImageToImageFilter_h

#include "vtkITK.h"

// ITK includes
#include <itkCommand.h>
#include <itkProcessObject.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkImageAlgorithm.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageExport.h>
#include <vtkImageImport.h>
#include <vtkVersion.h>

#undef itkExceptionMacro
#define itkExceptionMacro(x) \
  { \
  ::std::ostringstream message; \
  message << "itk::ERROR: " << this->GetNameOfClass() \
          << "(" << this << "): "; \
  std::cout << message.str().c_str() << std::endl; \
  }

#undef itkGenericExceptionMacro
#define itkGenericExceptionMacro(x) \
  { \
  ::std::ostringstream message; \
  message << "itk::ERROR: " x; \
  std::cout << message.str() << std::endl; \
  }

/// \brief Abstract base class for connecting ITK and VTK.
///
/// vtkITKImageToImageFilter provides a foo.
class VTK_ITK_EXPORT vtkITKImageToImageFilter
  : public vtkImageAlgorithm
{
public:
  static vtkITKImageToImageFilter *New()
   {
     vtkITKImageToImageFilter* result = new vtkITKImageToImageFilter;
#ifdef VTK_HAS_INITIALIZE_OBJECT_BASE
     result->InitializeObjectBase();
#endif
     return result;
   };

  vtkTypeMacro(vtkITKImageToImageFilter, vtkImageAlgorithm);

  void PrintSelf(ostream& os, vtkIndent indent) override
  {
    Superclass::PrintSelf ( os, indent );
    this->vtkExporter->PrintSelf ( os, indent );
    this->vtkImporter->PrintSelf ( os, indent );
  };

  ///
  /// This method considers the sub filters MTimes when computing this objects
  /// modified time.
  vtkMTimeType GetMTime() override
  {
    vtkMTimeType t1, t2;

    t1 = this->Superclass::GetMTime();
    t2 = this->vtkExporter->GetMTime();
    if (t2 > t1)
      {
      t1 = t2;
      }
    t2 = this->vtkImporter->GetMTime();
    if (t2 > t1)
      {
      t1 = t2;
      }
    return t1;
  };

  ///
  /// Pass modified message to itk filter
  void Modified() override
  {
    this->Superclass::Modified();
    if (this->m_Process)
      {
      m_Process->Modified();
      }
  };

  ///
  /// Pass DebugOn.
  void DebugOn() override
  {
    this->m_Process->DebugOn();
  };

  ///
  /// Pass DebugOff.
  void DebugOff() override
  {
    this->m_Process->DebugOff();
  };

  ///
  /// Pass SetNumberOfThreads.
  void SetNumberOfThreads(int val)
  {
    this->m_Process->SetNumberOfWorkUnits(val);
  };

  ///
  /// Pass SetNumberOfThreads.
  int GetNumberOfThreads()
  {
    return this->m_Process->GetNumberOfWorkUnits();
  };

  ///
  /// This method returns the cache to make a connection
  /// It justs feeds the request to the sub filter.
  void SetOutput ( vtkDataObject* d ) override { this->vtkImporter->SetOutput ( d ); };
  virtual vtkImageData *GetOutput() { return this->vtkImporter->GetOutput(); };
  virtual vtkImageData *GetOutput(int idx)
  {
    return (vtkImageData *) this->vtkImporter->GetOutput(idx);
  };

  ///
  /// Set the Input of the filter.
  virtual void SetInput(vtkImageData *Input)
  {
    this->vtkCast->SetInputData(Input);
  };

  void SetInputConnection(vtkAlgorithmOutput* input) override
  {
    this->vtkCast->SetInputConnection(input);
  };

  void SetInputConnection(int port, vtkAlgorithmOutput* input) override
  {
    this->vtkCast->SetInputConnection(port, input);
  };

  ///
  /// Return the input to the filter
  virtual vtkDataObject* GetInput()
  {
    return (vtkDataObject::SafeDownCast( this->vtkCast->GetInput() ));
  };

  ///  Override vtkSource's Update so that we can access
  /// this class's GetOutput(). vtkSource's GetOutput is not virtual.
  using vtkAlgorithm::Update;
  void Update() override
    {
      this->vtkCast->Update();
      this->vtkImporter->Update();
    }
  void Update(int port) override
    {
      this->vtkCast->Update();
      this->vtkImporter->Update(port);
   }
  void HandleProgressEvent ()
  {
    if ( this->m_Process )
      {
      this->UpdateProgress ( m_Process->GetProgress() );
      }
  };
  void HandleStartEvent ()
  {
    this->InvokeEvent(vtkCommand::StartEvent,nullptr);
  };
  void HandleEndEvent ()
  {
    this->InvokeEvent(vtkCommand::EndEvent,nullptr);
  };
  /// ETX

 protected:

  /// BTX
  /// Dummy ExecuteData
  void ExecuteData (vtkDataObject *) override
  {
    vtkWarningMacro(<< "This filter does not respond to Update(). Doing a GetOutput->Update() instead.");
  }
  /// ETX

  vtkITKImageToImageFilter()
  {
    /// Need an import, export, and a ITK pipeline
    this->vtkCast = vtkImageCast::New();
    this->vtkExporter = vtkImageExport::New();
    this->vtkImporter = vtkImageImport::New();
    this->vtkExporter->SetInputConnection( this->vtkCast->GetOutputPort() );
    this->m_Process = nullptr;
    this->m_ProgressCommand = MemberCommand::New();
    this->m_ProgressCommand->SetCallbackFunction ( this, &vtkITKImageToImageFilter::HandleProgressEvent );
    this->m_StartEventCommand = MemberCommand::New();
    this->m_StartEventCommand->SetCallbackFunction ( this, &vtkITKImageToImageFilter::HandleStartEvent );
    this->m_EndEventCommand = MemberCommand::New();
    this->m_EndEventCommand->SetCallbackFunction ( this, &vtkITKImageToImageFilter::HandleEndEvent );
  };
  ~vtkITKImageToImageFilter() override
  {
    vtkDebugMacro ("Destructing vtkITKImageToImageFilter");
    this->vtkExporter->Delete();
    this->vtkImporter->Delete();
    this->vtkCast->Delete();
  };

  /// BTX
  void LinkITKProgressToVTKProgress ( itk::ProcessObject* process )
  {
    if ( process )
      {
      this->m_Process = process;
      this->m_Process->AddObserver ( itk::ProgressEvent(), this->m_ProgressCommand );
      this->m_Process->AddObserver ( itk::StartEvent(), this->m_StartEventCommand );
      this->m_Process->AddObserver ( itk::EndEvent(), this->m_EndEventCommand );
      }
  };

  typedef itk::SimpleMemberCommand<vtkITKImageToImageFilter> MemberCommand;
  typedef MemberCommand::Pointer MemberCommandPointer;

  itk::ProcessObject::Pointer m_Process;
  MemberCommandPointer m_ProgressCommand;
  MemberCommandPointer m_StartEventCommand;
  MemberCommandPointer m_EndEventCommand;

  /// ITK Progress object
  /// To/from VTK
  vtkImageCast* vtkCast;
  vtkImageImport* vtkImporter;
  vtkImageExport* vtkExporter;

private:
  vtkITKImageToImageFilter(const vtkITKImageToImageFilter&) = delete;
  void operator=(const vtkITKImageToImageFilter&) = delete;
};

#endif
