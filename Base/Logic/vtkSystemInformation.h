#ifndef __vtkSystemInformation_h
#define __vtkSystemInformation_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>

class vtkRenderWindow;

// VTKSYS includes
#include <vtksys/SystemInformation.hxx>

// STD includes
#include <string>

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSystemInformation : public vtkObject
{
public:
  static vtkSystemInformation *New();
  vtkTypeMacro(vtkSystemInformation,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  const char * GetVendorString();
  const char * GetVendorID();
  const char * GetTypeID();
  const char * GetFamilyID();
  const char * GetModelName();
  const char * GetModelID();
  const char * GetSteppingCode();
  const char * GetExtendedProcessorName();
  const char * GetProcessorSerialNumber();
  int GetProcessorCacheSize();
  int GetLogicalProcessorsPerPhysical();
  float GetProcessorClockFrequency();
  int GetProcessorAPICID();
  int GetProcessorCacheXSize(long int);
  int DoesCPUSupportFeature(long int);

  const char * GetOSName();
  const char * GetHostname();
  const char * GetOSRelease();
  const char * GetOSVersion();
  const char * GetOSPlatform();

  int Is64Bits();

  unsigned int GetNumberOfLogicalCPU(); // per physical cpu
  unsigned int GetNumberOfPhysicalCPU();

  int DoesCPUSupportCPUID();

  // Retrieve memory information in megabyte.
  unsigned long GetTotalVirtualMemory();
  unsigned long GetAvailableVirtualMemory();
  unsigned long GetTotalPhysicalMemory();
  unsigned long GetAvailablePhysicalMemory();

  // Run the different checks
  void RunCPUCheck();
  void RunOSCheck();
  void RunMemoryCheck();
  void RunRenderingCheck();

  enum RenderingCapabilitiesMask
  {
    NONE = 0,
    /// Indicates if onscreen rendering is possible.
    ONSCREEN_RENDERING = 0x01,
    /// Indicates if headless rendering using OSMesa is possible.
    HEADLESS_RENDERING_USES_OSMESA = 0x04,
    /// Indicates if headless rendering using EGL is possible.
    HEADLESS_RENDERING_USES_EGL = 0x08,
    /// Indicates if any headless rendering is possible.
    HEADLESS_RENDERING = HEADLESS_RENDERING_USES_OSMESA | HEADLESS_RENDERING_USES_EGL,
    /// Indicates if any rendering is possible.
    RENDERING = ONSCREEN_RENDERING | HEADLESS_RENDERING,
    /// If rendering is possible, this indicates that that OpenGL version
    /// is adequate for basic rendering requirements.
    /// This flag will only be set if `RENDERING` is set too.
    OPENGL = 0x10,
  };

  /// Returns rendering capabilities as bitfield of RenderingCapabilitiesMask values.
  /// Value is set by calling RunRenderingCheck().
  vtkGetMacro(RenderingCapabilities, vtkTypeUInt32);

  /// Returns string describing rendering capabilities.
  /// Value is set by calling RunRenderingCheck().
  vtkGetMacro(RenderingCapabilitiesDetails, std::string);

protected:

  vtkSmartPointer<vtkRenderWindow> NewOffscreenRenderWindow();

  vtkSystemInformation();
  ~vtkSystemInformation() override;
  vtkSystemInformation(const vtkSystemInformation&);
  void operator=(const vtkSystemInformation&);

  //private:

  vtksys::SystemInformation SystemInformation;
  std::string StringHolder;
  vtkTypeUInt32 RenderingCapabilities;
  std::string RenderingCapabilitiesDetails;
};
#endif
