#ifndef __vtkSystemInformation_h
#define __vtkSystemInformation_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// VTK includes
#include <vtkObject.h>

#include <vtksys/SystemInformation.hxx>
#include <vtksys/stl/string>

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSystemInformation : public vtkObject
{
public:
  static vtkSystemInformation *New();
  vtkTypeRevisionMacro(vtkSystemInformation,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  const char * GetVendorString();
  const char * GetVendorID();
  const char * GetTypeID();
  const char * GetFamilyID();
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

  protected:

  vtkSystemInformation();
  virtual ~vtkSystemInformation();
  vtkSystemInformation(const vtkSystemInformation&);
  void operator=(const vtkSystemInformation&);
  
  //private:

  vtksys::SystemInformation SystemInformation;
  vtksys_stl::string StringHolder;

};
#endif
