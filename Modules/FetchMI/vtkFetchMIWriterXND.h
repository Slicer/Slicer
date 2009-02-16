#ifndef __vtkFetchMIWriterXND_h
#define __vtkFetchMIWriterXND_h

#include "vtkObject.h"
#include "vtkFetchMIWriter.h"
#include "vtkFetchMIWin32Header.h"

class VTK_FETCHMI_EXPORT vtkFetchMIWriterXND : public vtkFetchMIWriter
{
  
public:
  static vtkFetchMIWriterXND* New();
  vtkTypeRevisionMacro(vtkFetchMIWriterXND, vtkFetchMIWriter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Method that writes the document declaration for a resource post.
  virtual void WriteDocumentDeclaration();

  // Description:
  // Method that writes the XML header document.
  virtual void WriteXMLHeader ( const char *dataFilename);

  // Description:
  // Method that writes the metadata for a node.
  virtual void WriteMetadataForNode ( const char *nodeID, vtkMRMLScene *scene );

  // Description:
  // Method that writes the metadata for the scene.
  virtual void WriteMetadataForScene ( vtkMRMLScene *scene );
    
 protected:
  vtkFetchMIWriterXND();
  virtual ~vtkFetchMIWriterXND();

  vtkFetchMIWriterXND(const vtkFetchMIWriterXND&); // Not implemented
  void operator=(const vtkFetchMIWriterXND&); // Not Implemented
};

#endif

