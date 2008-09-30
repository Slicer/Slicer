#include "vtkTclUtil.h"
#include "vtkVersion.h"
#define VTK_TCL_TO_STRING(x) VTK_TCL_TO_STRING0(x)
#define VTK_TCL_TO_STRING0(x) #x
extern "C"
{
#if (TCL_MAJOR_VERSION == 8) && (TCL_MINOR_VERSION >= 4) && (TCL_RELEASE_LEVEL >= TCL_FINAL_RELEASE)
  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,int, CONST84 char *[]);
#else
  typedef int (*vtkTclCommandType)(ClientData, Tcl_Interp *,int, char *[]);
#endif
}

int vtkCudaVolumeMapperCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkCudaVolumeMapperNewCommand();
int vtkCudaVolumePropertyCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkCudaVolumePropertyNewCommand();
int vtkCudaVolumeInformationHandlerCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkCudaVolumeInformationHandlerNewCommand();
int vtkCudaRendererInformationHandlerCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkCudaRendererInformationHandlerNewCommand();
int vtkVolumeRenderingCudaFactoryCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkVolumeRenderingCudaFactoryNewCommand();
int vtkVolumeRenderingCudaGUICommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkVolumeRenderingCudaGUINewCommand();
int vtkVolumeRenderingCudaLogicCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkVolumeRenderingCudaLogicNewCommand();
int vtkCudaImageDataFilterCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkCudaImageDataFilterNewCommand();
int vtkCudaImageDataCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkCudaImageDataNewCommand();
int vtkCudaMemoryTextureCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkCudaMemoryTextureNewCommand();
int vtkKWTypeChooserBoxCommand(ClientData cd, Tcl_Interp *interp,
             int argc, char *argv[]);
ClientData vtkKWTypeChooserBoxNewCommand();

extern Tcl_HashTable vtkInstanceLookup;
extern Tcl_HashTable vtkPointerLookup;
extern Tcl_HashTable vtkCommandLookup;
extern void vtkTclDeleteObjectFromHash(void *);
extern void vtkTclListInstances(Tcl_Interp *interp, ClientData arg);


extern "C" {int VTK_EXPORT Volumerenderingcudamodule_SafeInit(Tcl_Interp *interp);}

extern "C" {int VTK_EXPORT Volumerenderingcudamodule_Init(Tcl_Interp *interp);}

extern void vtkTclGenericDeleteObject(ClientData cd);


int VTK_EXPORT Volumerenderingcudamodule_SafeInit(Tcl_Interp *interp)
{
  return Volumerenderingcudamodule_Init(interp);
}


int VTK_EXPORT Volumerenderingcudamodule_Init(Tcl_Interp *interp)
{

  vtkTclCreateNew(interp,(char *) "vtkCudaVolumeMapper", vtkCudaVolumeMapperNewCommand,
                  vtkCudaVolumeMapperCommand);
  vtkTclCreateNew(interp,(char *) "vtkCudaVolumeProperty", vtkCudaVolumePropertyNewCommand,
                  vtkCudaVolumePropertyCommand);
  vtkTclCreateNew(interp,(char *) "vtkCudaVolumeInformationHandler", vtkCudaVolumeInformationHandlerNewCommand,
                  vtkCudaVolumeInformationHandlerCommand);
  vtkTclCreateNew(interp,(char *) "vtkCudaRendererInformationHandler", vtkCudaRendererInformationHandlerNewCommand,
                  vtkCudaRendererInformationHandlerCommand);
  vtkTclCreateNew(interp,(char *) "vtkVolumeRenderingCudaFactory", vtkVolumeRenderingCudaFactoryNewCommand,
                  vtkVolumeRenderingCudaFactoryCommand);
  vtkTclCreateNew(interp,(char *) "vtkVolumeRenderingCudaGUI", vtkVolumeRenderingCudaGUINewCommand,
                  vtkVolumeRenderingCudaGUICommand);
  vtkTclCreateNew(interp,(char *) "vtkVolumeRenderingCudaLogic", vtkVolumeRenderingCudaLogicNewCommand,
                  vtkVolumeRenderingCudaLogicCommand);
  vtkTclCreateNew(interp,(char *) "vtkCudaImageDataFilter", vtkCudaImageDataFilterNewCommand,
                  vtkCudaImageDataFilterCommand);
  vtkTclCreateNew(interp,(char *) "vtkCudaImageData", vtkCudaImageDataNewCommand,
                  vtkCudaImageDataCommand);
  vtkTclCreateNew(interp,(char *) "vtkCudaMemoryTexture", vtkCudaMemoryTextureNewCommand,
                  vtkCudaMemoryTextureCommand);
  vtkTclCreateNew(interp,(char *) "vtkKWTypeChooserBox", vtkKWTypeChooserBoxNewCommand,
                  vtkKWTypeChooserBoxCommand);
  char pkgName[]="VolumeRenderingCuda";
  char pkgVers[]=VTK_TCL_TO_STRING(VTK_MAJOR_VERSION) "." VTK_TCL_TO_STRING(VTK_MINOR_VERSION);
  Tcl_PkgProvide(interp, pkgName, pkgVers);
  return TCL_OK;
}
