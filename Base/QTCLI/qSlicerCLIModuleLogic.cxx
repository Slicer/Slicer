#include "qSlicerCLIModuleLogic.h" 

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerCLIModuleLogic::qInternal
{
public:
  qInternal()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerCxxInternalConstructor1Macro(qSlicerCLIModuleLogic, QObject*);
qSlicerCxxDestructorMacro(qSlicerCLIModuleLogic);

//-----------------------------------------------------------------------------
void qSlicerCLIModuleLogic::printAdditionalInfo()
{
  this->Superclass::printAdditionalInfo(); 
}

//----------------------------------------------------------------------------
QString qSlicerCLIModuleLogic::constructTemporaryFileName(const QString& tag,
                             const QString& type,
                             const QString& name,
                             const QStringList& extensions/*,
                             CommandLineModuleType commandType*/) const
{
  QString fname; 
//   std::string fname = name;
//   std::string pid;
//   std::ostringstream pidString;
// 
//   // Constructing a temporary filename from a node involves:
//   //
//   // 1. If the consumer of the file can communicate directly with the
//   // MRML scene, then the node is encoded as slicer:%p#%s where the
//   // pointer is the address of the scene which contains the node
//   // and the string is the MRML node ID.
//   //
//   // 2. If the consumer of the file is a Python module, it operates
//   // in the process space of Slicer.  The Python module can be given
//   // MRML node ID's directly.
//   //
//   // 3. If the consumer of the file cannot communicate directly with
//   // the MRML scene, then a real temporary filename is constructed.
//   // The filename will point to the Temporary directory defined for
//   // Slicer. The filename will be unique to the process (multiple
//   // running instances of slicer will not collide).  The filename
//   // will be unique to the node in the process (the same node will be
//   // encoded to the same filename every time within that running
//   // instance of Slicer).  This last point is an optimization to
//   // minimize the number of times a file is written when running a
//   // module.  However, if we change the execution model such that more
//   // than one module can run at the same time within the same Slicer
//   // process, then this encoding will need to be changed to be unique
//   // per module execution.
//   //
// 
//   
//   // Encode process id into a string.  To avoid confusing the
//   // Archetype reader, convert the numbers in pid to characters [0-9]->[A-J]
// #ifdef _WIN32
//   pidString << GetCurrentProcessId();
// #else
//   pidString << getpid();
// #endif
//   pid = pidString.str();
//   std::transform(pid.begin(), pid.end(), pid.begin(), DigitsToCharacters());
// 
//   // Because Python is responsible for looking up the MRML Object,
//   // we can simply return the MRML Id.
//   if ( commandType == PythonModule )
//     {
//     return fname;
//     }
//   
//   // To avoid confusing the Archetype readers, convert any
//   // numbers in the filename to characters [0-9]->[A-J]
//   std::transform(fname.begin(), fname.end(),
//                  fname.begin(), DigitsToCharacters());
// 
//   // By default, the filename is based on the temporary directory and
//   // the pid
//   fname = this->TemporaryDirectory + "/" + pid + "_" + fname;
// 
//   if (tag == "image")
//     {
//     if ( commandType == CommandLineModule )
//       {
//       // If running an executable 
// 
//       // Use default fname construction, tack on extension
//       std::string ext = ".nrrd";
//       if (extensions.size() != 0)
//         {
//         ext = extensions[0];
//         }
//       fname = fname + ext;
//       }
//     else
//       {
//       // If not a command line module then it is a shared object
//       // module for which we can communicated directly with the MRML
//       // tree.
// 
//       // Redefine the filename to be a reference to a slicer node.
//       
//       // Must be large enough to hold slicer:, #, an ascii
//       // representation of the scene pointer and the MRML node ID. 
//       char *tname = new char[name.size() + 100];
//       
//       sprintf(tname, "slicer:%p#%s", this->MRMLScene, name.c_str());
//       
//       fname = tname;
// 
//       delete [] tname;
//       }
//     }
// 
//   if (tag == "geometry")
//     {
//     // geometry is currently always passed via files
// 
//     // Use default fname construction, tack on extension
//     std::string ext = ".vtp";
//     if (extensions.size() != 0)
//       {
//       ext = extensions[0];
//       }
//     fname = fname + ext;
//     }
// 
//   if (tag == "transform")
//     {
//     // transform is currently always passed via files
// 
//     // Use default fname construction, tack on extension
//     std::string ext = ".mrml";
//     if (extensions.size() != 0)
//       {
//       ext = extensions[0];
//       }
//     fname = fname + ext;
//     }
// 
//   if (tag == "table")
//     {
//     // tables are currently always passed via files
// 
//     // Use default fname construction, tack on extension
//     std::string ext = ".ctbl";
//     if (extensions.size() != 0)
//       {
//       ext = extensions[0];
//       }
//     fname = fname + ext;
//     }
  
    
  return fname;
}
