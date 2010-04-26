extern int BRAINSDemonWarpPrimary(int argc, char *argv[]);

// main function built in BRAINSDemonWarpPrimary.cxx so that testing only builds
// templates once.
int main(int argc, char *argv[])
{
  return BRAINSDemonWarpPrimary(argc, argv);
}
