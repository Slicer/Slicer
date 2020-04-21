#include "vtkSystemInformation.h"
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>

vtkStandardNewMacro(vtkSystemInformation);

vtkSystemInformation::vtkSystemInformation()
{
  this->RenderingCapabilities = 0;
}

vtkSystemInformation::~vtkSystemInformation() = default;

const char * vtkSystemInformation::GetVendorString()
{
  return this->SystemInformation.GetVendorString();
}

const char * vtkSystemInformation::GetVendorID()
{
  return this->SystemInformation.GetVendorID();
}

const char * vtkSystemInformation::GetTypeID()
{
  this->StringHolder = this->SystemInformation.GetTypeID();
  return this->StringHolder.c_str();
}

const char * vtkSystemInformation::GetFamilyID()
{
  this->StringHolder = this->SystemInformation.GetFamilyID();
  return this->StringHolder.c_str();
}

const char * vtkSystemInformation::GetModelName()
{
  this->StringHolder = this->SystemInformation.GetModelName();
  return this->StringHolder.c_str();
}

const char * vtkSystemInformation::GetModelID()
{
  this->StringHolder = this->SystemInformation.GetModelID();
  return this->StringHolder.c_str();
}

const char * vtkSystemInformation::GetSteppingCode()
{
  this->StringHolder = this->SystemInformation.GetSteppingCode();
  return this->StringHolder.c_str();
}

const char * vtkSystemInformation::GetExtendedProcessorName()
{
  return this->SystemInformation.GetExtendedProcessorName();
}

const char * vtkSystemInformation::GetProcessorSerialNumber()
{
  return this->SystemInformation.GetProcessorSerialNumber();
}

int vtkSystemInformation::GetProcessorCacheSize()
{
  return this->SystemInformation.GetProcessorCacheSize();
}

int vtkSystemInformation::GetLogicalProcessorsPerPhysical()
{
  return this->SystemInformation.GetLogicalProcessorsPerPhysical();
}

float vtkSystemInformation::GetProcessorClockFrequency()
{
  return this->SystemInformation.GetProcessorClockFrequency();
}

int vtkSystemInformation::GetProcessorAPICID()
{
  return this->SystemInformation.GetProcessorAPICID();
}

int vtkSystemInformation::GetProcessorCacheXSize(long int x)
{
  return this->SystemInformation.GetProcessorCacheXSize(x);
}

int vtkSystemInformation::DoesCPUSupportFeature(long int x)
{
  return this->SystemInformation.DoesCPUSupportFeature(x);
}


const char * vtkSystemInformation::GetOSName()
{
  return this->SystemInformation.GetOSName();
}

const char * vtkSystemInformation::GetHostname()
{
  return this->SystemInformation.GetHostname();
}

const char * vtkSystemInformation::GetOSRelease()
{
  return this->SystemInformation.GetOSRelease();
}

const char * vtkSystemInformation::GetOSVersion()
{
  return this->SystemInformation.GetOSVersion();
}

const char * vtkSystemInformation::GetOSPlatform()
{
  return this->SystemInformation.GetOSPlatform();
}


int vtkSystemInformation::Is64Bits()
{
  return this->SystemInformation.Is64Bits();
}


unsigned int vtkSystemInformation::GetNumberOfLogicalCPU() // per physical cpu
{
  return this->SystemInformation.GetNumberOfLogicalCPU();
}

unsigned int vtkSystemInformation::GetNumberOfPhysicalCPU()
{
  return this->SystemInformation.GetNumberOfPhysicalCPU();
}


int vtkSystemInformation::DoesCPUSupportCPUID()
{
  return this->SystemInformation.DoesCPUSupportCPUID();
}


// Retrieve memory information in megabyte.
unsigned long vtkSystemInformation::GetTotalVirtualMemory()
{
  return this->SystemInformation.GetTotalVirtualMemory();
}

unsigned long vtkSystemInformation::GetAvailableVirtualMemory()
{
  return this->SystemInformation.GetAvailableVirtualMemory();
}

unsigned long vtkSystemInformation::GetAvailablePhysicalMemory()
{
  return this->SystemInformation.GetAvailablePhysicalMemory();
}

unsigned long vtkSystemInformation::GetTotalPhysicalMemory()
{
  return this->SystemInformation.GetTotalPhysicalMemory();
}


// Run the different checks
void vtkSystemInformation::RunCPUCheck()
{
  this->SystemInformation.RunCPUCheck();
}

void vtkSystemInformation::RunOSCheck()
{
  this->SystemInformation.RunOSCheck();
}

void vtkSystemInformation::RunMemoryCheck()
{
  this->SystemInformation.RunMemoryCheck();
}

void
vtkSystemInformation::PrintSelf(ostream& os, vtkIndent indent)
{
  // update all values before printing
  this->RunCPUCheck();
  this->RunOSCheck();
  this->RunMemoryCheck();

  Superclass::PrintSelf(os, indent);

  os << indent << "SystemInformation:             " << this->GetClassName() << "\n";

  os << indent << "VendorString: " << this->GetVendorString() << "\n";
  os << indent << "TypeID: " << this->GetTypeID() << "\n";
  os << indent << "FamilyID: " << this->GetFamilyID() << "\n";
  os << indent << "ModelName: " << this->GetModelName() << "\n";
  os << indent << "ModelID: " << this->GetModelID() << "\n";
  os << indent << "SteppingCode: " << this->GetSteppingCode() << "\n";
  os << indent << "ExtendedProcessorName: " << this->GetExtendedProcessorName() << "\n";
  os << indent << "ProcessorSerialNumber: " << this->GetProcessorSerialNumber() << "\n";

  os << indent << "ProcessorCacheSize: " << this->GetProcessorCacheSize() << "\n";
  os << indent << "LogicalProcessorsPerPhysical: " << this->GetLogicalProcessorsPerPhysical() << "\n";
  os << indent << "ProcessorClockFrequency: " << this->GetProcessorClockFrequency() << "\n";
  os << indent << "ProcessorAPICID: " << this->GetProcessorAPICID() << "\n";

  os << indent << "OSName: " << this->GetOSName() << "\n";
  os << indent << "Hostname: " << this->GetHostname() << "\n";
  os << indent << "OSRelease: " << this->GetOSRelease() << "\n";
  os << indent << "OSVersion: " << this->GetOSVersion() << "\n";
  os << indent << "OSPlatform: " << this->GetOSPlatform() << "\n";
  os << indent << "Is64Bits: " << this->Is64Bits() << "\n";


  os << indent << "NumberOfLogicalCPU: " << this->GetNumberOfLogicalCPU() << "\n";
  os << indent << "NumberOfPhysicalCPU: " << this->GetNumberOfPhysicalCPU() << "\n";
  os << indent << "DoesCPUSupportCPUID: " << this->DoesCPUSupportCPUID() << "\n";

  os << indent << "GetTotalVirtualMemory: " << this->GetTotalVirtualMemory() << "\n";
  os << indent << "GetAvailableVirtualMemory: " << this->GetAvailableVirtualMemory() << "\n";
  os << indent << "GetTotalPhysicalMemory: " << this->GetTotalPhysicalMemory() << "\n";
  os << indent << "GetAvailablePhysicalMemory: " << this->GetAvailablePhysicalMemory() << "\n";
}

//----------------------------------------------------------------------------
void vtkSystemInformation::RunRenderingCheck()
{
  this->RenderingCapabilities = 0;
  this->RenderingCapabilitiesDetails.clear();

  // This method is based on ParaView's capabilities check method in
  // ParaViewCore\ClientServerCore\Rendering\vtkPVRenderingCapabilitiesInformation.cxx

#if defined(VTK_USE_COCOA) || defined(_WIN32)
  this->RenderingCapabilities |= ONSCREEN_RENDERING;
#elif defined(VTK_USE_X)
  // if using X, need to check if display is accessible.
  Display* dId = XOpenDisplay(nullptr);
  if (dId)
  {
    XCloseDisplay(dId);
    this->RenderingCapabilities |= ONSCREEN_RENDERING;
  }
#endif

#if defined(VTK_OPENGL_HAS_OSMESA)
  this->RenderingCapabilities |= HEADLESS_RENDERING_USES_OSMESA;
#endif

#if defined(VTK_OPENGL_HAS_EGL)
  this->RenderingCapabilities |= HEADLESS_RENDERING_USES_EGL;
#endif

  if (!(this->RenderingCapabilities & RENDERING))
    {
    this->RenderingCapabilitiesDetails = "No suitable renderer found.";
    return;
    }

  // now test OpenGL capabilities.

  vtkSmartPointer<vtkRenderWindow> window = vtkSystemInformation::NewOffscreenRenderWindow();
  if (!window)
    {
    this->RenderingCapabilitiesDetails = "Failed to create render window.";
    return;
    }
  if (!window->SupportsOpenGL())
    {
    this->RenderingCapabilitiesDetails = "Renderer does not support required OpenGL capabilities.";
    return;
    }
  this->RenderingCapabilities |= OPENGL;
  const char* opengl_capabilities = window->ReportCapabilities();
  this->RenderingCapabilitiesDetails = (opengl_capabilities ? opengl_capabilities : "");
}

//----------------------------------------------------------------------------
vtkSmartPointer<vtkRenderWindow> vtkSystemInformation::NewOffscreenRenderWindow()
{
  // This method is based on ParaView's
  // vtkPVRenderingCapabilitiesInformation::NewOffscreenRenderWindow() method in
  // ParaViewCore\ClientServerCore\Rendering\vtkPVRenderingCapabilitiesInformation.cxx

  vtkSmartPointer<vtkRenderWindow> window;

  // if headless rendering is supported, let's create the headless render
  // window.
#if defined(VTK_OPENGL_HAS_EGL)
  window.TakeReference(vtkEGLRenderWindow::New());
  // vtkEGLRenderWindow gets initialized with `VTK_DEFAULT_EGL_DEVICE_INDEX`
  // CMake variable. If the command line options overrode it, change it.
  if (window)
  {
    int deviceIndex = GetEGLDeviceIndex();
    if (deviceIndex >= 0)
    {
      window->SetDeviceIndex(deviceIndex);
    }
  }
#elif defined(VTK_OPENGL_HAS_OSMESA)
  window.TakeReference(vtkOSOpenGLRenderWindow::New());
#endif
  if (!window)
  {
    // if not, let VTK create a default based on CMake flags specified.
    window = vtkSmartPointer<vtkRenderWindow>::New();
  }

  window->SetOffScreenRendering(1); // we want to keep the window unmapped.
                                    // this should be largely unnecessary, but vtkRenderWindow subclasses
                                    // are fairly inconsistent about this so let's just set it always.

  return window;
}
