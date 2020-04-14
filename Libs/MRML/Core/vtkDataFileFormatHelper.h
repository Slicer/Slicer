#ifndef __vtkDataFileFormatHelper_h
#define __vtkDataFileFormatHelper_h

// MRML includes
#include "vtkMRML.h"

// VTK includes
#include <vtkObject.h>
class vtkStringArray;

struct ITKImageFileFormatStruct
{
  const char *ClassName;
  const char *Description;
  const char *GenericName;
  const char *Extension;
};
using ITKImageFileFormat = struct ITKImageFileFormatStruct;

class VTK_MRML_EXPORT vtkDataFileFormatHelper : public vtkObject
{
  public:

  /// The Usual vtk class functions
  static vtkDataFileFormatHelper *New();
  vtkTypeMacro(vtkDataFileFormatHelper, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static std::string GetFileExtensionFromFormatString(
   const char* fileformat);
  const char* GetClassNameFromFormatString(
   const char* fileformat);

  ///
  /// Get the itkimageio supported file formats.
  //vtkGetObjectMacro ( ITKSupportedWriteFileFormats, vtkStringArray);
  virtual vtkStringArray* GetITKSupportedWriteFileFormats();
  virtual vtkStringArray* GetITKSupportedReadFileFormats()
    {
    return nullptr;
    }
  virtual vtkStringArray* GetITKSupportedWriteFileExtensions();
  virtual const char* GetITKSupportedExtensionGenericNameByIndex(int idx);
  virtual const char* GetITKSupportedExtensionClassNameByIndex(int idx);

 protected:
  vtkDataFileFormatHelper();
  ~vtkDataFileFormatHelper() override;

  vtkStringArray* SupportedWriteFileExtensions;
  vtkStringArray* SupportedWriteFileGenericNames;
  vtkStringArray* SupportedWriteFileClassNames;
  vtkStringArray* ITKSupportedWriteFileFormats;

  /// This will initialize the supported file formats
  virtual void InitializeITKSupportedFileFormats();
  virtual void PopulateITKSupportedWriteFileTypes();
  virtual void AddSupportedWriterFileFormat(
     ITKImageFileFormat& structFileFormat);

private:
  vtkDataFileFormatHelper(const vtkDataFileFormatHelper&) = delete;
  void operator=(const vtkDataFileFormatHelper&) = delete;
};

#endif
