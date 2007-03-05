/*=========================================================================

  Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   vtkITK
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

// .NAME vtkITKImageToImageFilter - Abstract base class for connecting ITK and VTK
// .SECTION Description
// vtkITKImageToImageFilter provides a foo

#ifndef __vtkITKImageToImageFilter_h
#define __vtkITKImageToImageFilter_h


#include "itkCommand.h"
#include "vtkCommand.h"
#include "itkProcessObject.h"
#include "vtkImageImport.h"
#include "vtkImageExport.h"
#include "vtkImageToImageFilter.h"
#include "vtkImageCast.h"
#include "vtkImageData.h"

#include "vtkITK.h"

#undef itkExceptionMacro  
#define itkExceptionMacro(x) \
  { \
  ::itk::OStringStream message; \
  message << "itk::ERROR: " << this->GetNameOfClass() \
          << "(" << this << "): "; \
  std::cout << message.str().c_str() << std::endl; \
  }

#undef itkGenericExceptionMacro  
#define itkGenericExceptionMacro(x) \
  { \
  ::itk::OStringStream message; \
  message << "itk::ERROR: " x; \
  std::cout << message.str() << std::endl; \
  }

class VTK_ITK_EXPORT vtkITKImageToImageFilter : public vtkImageToImageFilter
{
public:
  static vtkITKImageToImageFilter *New()
   {
     return new vtkITKImageToImageFilter;
   };
  
  vtkTypeMacro(vtkITKImageToImageFilter,vtkImageToImageFilter);

  void PrintSelf(ostream& os, vtkIndent indent)
  {
    Superclass::PrintSelf ( os, indent );
    this->vtkExporter->PrintSelf ( os, indent );
    this->vtkImporter->PrintSelf ( os, indent );
  };
  
  // Description:
  // This method considers the sub filters MTimes when computing this objects
  // modified time.
  unsigned long int GetMTime()
  {
    unsigned long int t1, t2;
  
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

  // Description:
  // Pass modified message to itk filter
  void Modified()
  {
    this->Superclass::Modified();
    if (this->m_Process)
      {
      m_Process->Modified();
      }
  };
  
  // Description:
  // Pass DebugOn.
  void DebugOn()
  {
    this->m_Process->DebugOn();
  };
  
  // Description:
  // Pass DebugOff.
  void DebugOff()
  {
    this->m_Process->DebugOff();
  };
  
  // Description:
  // Pass SetNumberOfThreads.
  void SetNumberOfThreads(int val)
  {
    this->m_Process->SetNumberOfThreads(val);
  };
  
  // Description:
  // Pass SetNumberOfThreads.
  int GetNumberOfThreads()
  {
    return this->m_Process->GetNumberOfThreads();
  };
  
  // Description:
  // This method returns the cache to make a connection
  // It justs feeds the request to the sub filter.
  void SetOutput ( vtkImageData* d ) { this->vtkImporter->SetOutput ( d ); };
  virtual vtkImageData *GetOutput() { return this->vtkImporter->GetOutput(); };
  virtual vtkImageData *GetOutput(int idx)
  {
    return (vtkImageData *) this->vtkImporter->GetOutput(idx);
  };

  // Description:
  // Set the Input of the filter.
  virtual void SetInput(vtkImageData *Input)
  {
    this->vtkCast->SetInput(Input);
  };

  // Description: Override vtkSource's Update so that we can access
  // this class's GetOutput(). vtkSource's GetOutput is not virtual.
  void Update()
    {
      if (this->GetOutput(0))
        {
        this->GetOutput(0)->Update();
        if ( this->GetOutput(0)->GetSource() )
          {
          //          this->SetErrorCode( this->GetOutput(0)->GetSource()->GetErrorCode() );
          }
        }
    }
  //BTX
  void HandleProgressEvent ()
  {
    if ( this->m_Process )
      {
      this->UpdateProgress ( m_Process->GetProgress() );
      }
  };
  void HandleStartEvent ()
  {
    this->InvokeEvent(vtkCommand::StartEvent,NULL);
  };
  void HandleEndEvent ()
  {
    this->InvokeEvent(vtkCommand::EndEvent,NULL);
  };
  // ETX  

 protected:

  // BTX
  // Dummy ExecuteData
  void ExecuteData (vtkDataObject *)
  {
    vtkWarningMacro(<< "This filter does not respond to Update(). Doing a GetOutput->Update() instead.");
  }
  // ETX

  vtkITKImageToImageFilter()
  {
    // Need an import, export, and a ITK pipeline
    this->vtkCast = vtkImageCast::New();
    this->vtkExporter = vtkImageExport::New();
    this->vtkImporter = vtkImageImport::New();
    this->vtkExporter->SetInput ( this->vtkCast->GetOutput() );
    this->m_Process = NULL;
    this->m_ProgressCommand = MemberCommand::New();
    this->m_ProgressCommand->SetCallbackFunction ( this, &vtkITKImageToImageFilter::HandleProgressEvent );
    this->m_StartEventCommand = MemberCommand::New();
    this->m_StartEventCommand->SetCallbackFunction ( this, &vtkITKImageToImageFilter::HandleStartEvent );
    this->m_EndEventCommand = MemberCommand::New();
    this->m_EndEventCommand->SetCallbackFunction ( this, &vtkITKImageToImageFilter::HandleEndEvent );
  };
  ~vtkITKImageToImageFilter()
  {
    vtkDebugMacro ("Destructing vtkITKImageToImageFilter");
    this->vtkExporter->Delete();
    this->vtkImporter->Delete();
    this->vtkCast->Delete();
  };

  // BTX  
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
  
  // ITK Progress object
  // To/from VTK
  vtkImageCast* vtkCast;
  vtkImageImport* vtkImporter;
  vtkImageExport* vtkExporter;  
  //ETX
  
private:
  vtkITKImageToImageFilter(const vtkITKImageToImageFilter&);  // Not implemented.
  void operator=(const vtkITKImageToImageFilter&);  // Not implemented.
};

#endif




