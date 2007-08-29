// .NAME vtkIGTDataManager - Central registry to provide control and I/O for
//  trackers and imagers
// .SECTION Description
// vtkIGTDataManager registers arbitary number of trackers and imagers, created MRML nodes in the MRML secene. Designed and Coded by Nobuhiko Hata and Haiying Liu, Jan 12, 2007 @ NA-MIC All Hands Meeting, Salt Lake City, UT

#ifndef IGTDATAMANAGER_H
#define IGTDATAMANAGER_H


#include "vtkIGTWin32Header.h" 
#include "vtkObject.h"
#include "vtkMRMLScene.h"

#include <string>


#define IGT_MATRIX_STREAM 0
#define IGT_IMAGE_STREAM 1


class VTK_IGT_EXPORT vtkIGTDataManager : public vtkObject
{
public:


    static vtkIGTDataManager *New();
    vtkTypeRevisionMacro(vtkIGTDataManager,vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);


    //Description:
    // Set MRML scene from the Slicer main routine
    vtkSetObjectMacro(MRMLScene,vtkMRMLScene);

    //Description:
    // Get MRML scene stored
    vtkGetObjectMacro(MRMLScene,vtkMRMLScene);


    /**
     * Constructor
     */
    vtkIGTDataManager();


    //Description:
    //Destructor
    virtual ~vtkIGTDataManager ( );


    //Descripton:
    //Reigster stream.  
    const char *RegisterStream(int streamType);


private:

    //Description.
    //MRML scene passed from the Module instantiating this class
    vtkMRMLScene *MRMLScene;
    static int index;
    //BTX
    std::string StreamID;
    //ETX

};


#endif // IGTDATAMANAGER_H
