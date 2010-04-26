extern int VBRAINSDemonWarpPrimary(int argc, char *argv[]);

// main function built in BRAINSDemonWarpPrimary.cxx so that testing only builds
// templates once.
int main(int argc, char *argv[])
{
  return VBRAINSDemonWarpPrimary(argc, argv);
}
