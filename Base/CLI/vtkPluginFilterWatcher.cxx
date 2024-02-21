
#include <vtkPluginFilterWatcher.h>

// VTK includes

//-----------------------------------------------------------------------------
class vtkPluginWatcherStart : public vtkCommand
{
public:
  // generic new method
  static vtkPluginWatcherStart *New()
  { return new vtkPluginWatcherStart; }

  void Execute(vtkObject *vtkNotUsed(caller),
                       unsigned long event,
                       void* vtkNotUsed(v)) override
  {
    if (event == vtkCommand::StartEvent && this->Watcher)
    {
      this->Watcher->SetSteps(0);
      if (this->Watcher->GetProcessInformation())
      {
        this->Watcher->GetProcessInformation()->Progress = 0;
        this->Watcher->GetProcessInformation()->StageProgress = 0;
        strncpy(this->Watcher->GetProcessInformation()->ProgressMessage,
                this->Watcher->GetComment().c_str(), 1023);

        if (this->Watcher->GetProcessInformation()->ProgressCallbackFunction
            && this->Watcher->GetProcessInformation()->ProgressCallbackClientData)
        {
          (*(this->Watcher->GetProcessInformation()->ProgressCallbackFunction))(this->Watcher->GetProcessInformation()->ProgressCallbackClientData);
        }
      }
      else
      {
        std::cout << "<filter-start>"
                  << std::endl;
        std::cout << "<filter-name>"
                  << (this->Watcher->GetProcess()
                      ? this->Watcher->GetProcess()->GetClassName() : "None")
                  << "</filter-name>"
                  << std::endl;
        std::cout << "<filter-comment>"
                  << " \"" << this->Watcher->GetComment() << "\" "
                  << "</filter-comment>"
                  << std::endl;
        std::cout << "</filter-start>"
                  << std::endl;
        std::cout << std::flush;
      }
    }
  }
  void SetWatcher(vtkPluginFilterWatcher *w)
  {
    this->Watcher = w;
  }

private:
  vtkPluginFilterWatcher *Watcher;

};

//-----------------------------------------------------------------------------
class vtkPluginWatcherEnd : public vtkCommand
{
public:
  // generic new method
  static vtkPluginWatcherEnd *New()
  {
    return new vtkPluginWatcherEnd;
  }

  void Execute(vtkObject *vtkNotUsed(caller),
                       unsigned long event,
                       void* vtkNotUsed(v)) override
  {
    if (event == vtkCommand::EndEvent && this->Watcher)
    {
      if (this->Watcher->GetProcessInformation())
      {
        this->Watcher->GetProcessInformation()->Progress = 0;
        this->Watcher->GetProcessInformation()->StageProgress = 0;
        if (this->Watcher->GetProcessInformation()->ProgressCallbackFunction
            && this->Watcher->GetProcessInformation()->ProgressCallbackClientData)
        {
          (*(this->Watcher->GetProcessInformation()->ProgressCallbackFunction))(this->Watcher->GetProcessInformation()->ProgressCallbackClientData);
        }
      }
      else
      {
        std::cout << "<filter-end>"
                  << std::endl;
        std::cout << "<filter-name>"
                  << (this->Watcher->GetProcess()
                      ? this->Watcher->GetProcess()->GetClassName() : "None")
                  << "</filter-name>"
                  << std::endl;
        std::cout << "</filter-end>";
        std::cout << std::flush;
      }
    }
  }
  void SetWatcher(vtkPluginFilterWatcher *w)
  {
    this->Watcher = w;
  }
private:
  vtkPluginFilterWatcher *Watcher;

};

//-----------------------------------------------------------------------------
class vtkPluginWatcherProgress : public vtkCommand
{
public:
  // generic new method
  static vtkPluginWatcherProgress *New()
  {
    return new vtkPluginWatcherProgress;
  }

  void Execute(vtkObject *vtkNotUsed(caller),
                       unsigned long event,
                       void* vtkNotUsed(v)) override
  {
    if (event == vtkCommand::ProgressEvent && this->Watcher)
    {
      this->Watcher->SetSteps( this->Watcher->GetSteps()+1 );
      if (this->Watcher->GetProcessInformation())
      {
        strncpy(this->Watcher->GetProcessInformation()->ProgressMessage,
                this->Watcher->GetComment().c_str(), 1023);
        this->Watcher->GetProcessInformation()->Progress =
          (this->Watcher->GetProcess()->GetProgress() *
           this->Watcher->GetFraction() + this->Watcher->GetStart());
        if (this->Watcher->GetFraction() != 1.0)
        {
          this->Watcher->GetProcessInformation()->StageProgress =
            this->Watcher->GetProcess()->GetProgress();
        }

        if (this->Watcher->GetProcessInformation()->Abort)
        {
          this->Watcher->GetProcessInformation()->Progress = 0;
          this->Watcher->GetProcessInformation()->StageProgress = 0;
        }

        if (this->Watcher->GetProcessInformation()->ProgressCallbackFunction
            && this->Watcher->GetProcessInformation()->ProgressCallbackClientData)
        {
          (*(this->Watcher->GetProcessInformation()->ProgressCallbackFunction))(this->Watcher->GetProcessInformation()->ProgressCallbackClientData);
        }
      }
      else
      {
        if (!this->Quiet)
        {
          std::cout << "<filter-progress>"
                    << (this->Watcher->GetProcess()->GetProgress() *
                        this->Watcher->GetFraction()) + this->Watcher->GetStart()
                    << "</filter-progress>"
                    << std::endl;
          if (this->Watcher->GetFraction() != 1.0)
          {
            std::cout << "<filter-stage-progress>"
                      << this->Watcher->GetProcess()->GetProgress()
                      << "</filter-stage-progress>"
                      << std::endl;
          }
          std::cout << std::flush;
        }
      }
    }
  }

  //-----------------------------------------------------------------------------
  void SetWatcher(vtkPluginFilterWatcher *w)
  {
    this->Watcher = w;
  }
  /** Set/Get the quiet mode boolean. If false, verbose progress is
   * reported. */
  void SetQuiet(bool val) {Quiet=val;};
  bool GetQuiet() {return Quiet;};
private:
  vtkPluginFilterWatcher *Watcher;
  bool Quiet;

};

//-----------------------------------------------------------------------------
vtkPluginFilterWatcher
::vtkPluginFilterWatcher(vtkAlgorithm* o,
                         const char *comment,
                         ModuleProcessInformation *inf,
                         double fraction,
                         double start)
{
  // Initialize state
  this->Process = o;
  this->Process->Register(nullptr);

  this->Steps = 0;
  this->Comment = comment;
#if defined(_COMPILER_VERSION) && (_COMPILER_VERSION == 730)
  this->Quiet = true;
#else
  this->Quiet = false;
#endif

  this->ProcessInformation = inf;
  this->Fraction = fraction;
  this->Start = start;

  // Create a series of commands
  this->StartFilterCommand = vtkPluginWatcherStart::New();
  this->StartFilterCommand->SetWatcher(this);

  this->EndFilterCommand = vtkPluginWatcherEnd::New();;
  this->EndFilterCommand->SetWatcher(this);

  this->ProgressFilterCommand = vtkPluginWatcherProgress::New();;
  this->ProgressFilterCommand->SetWatcher(this);
  this->ProgressFilterCommand->SetQuiet(this->GetQuiet());

  // Add the commands as observers
  this->StartTag = this->Process->AddObserver(vtkCommand::StartEvent,
                                              this->StartFilterCommand);
  this->EndTag = this->Process->AddObserver(vtkCommand::EndEvent,
                                            this->EndFilterCommand);
  this->ProgressTag = this->Process->AddObserver(vtkCommand::ProgressEvent,
                                                 this->ProgressFilterCommand);
}

//-----------------------------------------------------------------------------
vtkPluginFilterWatcher::~vtkPluginFilterWatcher()
{
  // Remove any observers we have on the old process object
  if (this->Process)
  {
    if (this->StartFilterCommand)
    {
      this->Process->RemoveObserver(this->StartTag);
    }
    if (this->EndFilterCommand)
    {
      this->Process->RemoveObserver(this->EndTag);
    }
    if (this->ProgressFilterCommand)
    {
      this->Process->RemoveObserver(this->ProgressTag);
    }

    this->Process->UnRegister(nullptr);
  }
  this->StartFilterCommand->Delete();
  this->EndFilterCommand->Delete();
  this->ProgressFilterCommand->Delete();
}

//-----------------------------------------------------------------------------
void vtkPluginFilterWatcher::SetQuiet(bool val)
{
  Quiet=val;
  if (this->ProgressFilterCommand)
  {
    this->ProgressFilterCommand->SetQuiet(val);
  }
}
