/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
  All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer

=========================================================================auto=*/

// SlicerLib includes
#include "vtkArchive.h"

// VTK includes

// VTKSYS includes
#include <vtksys/SystemTools.hxx>
#include <vtksys/Directory.hxx>


// STD includes

#include "vtkMRMLCoreTestingMacros.h"

bool validFile(const char *fileName)
{
  if (!strcmp(fileName, ".")) return true;
  if (!strcmp(fileName, "..")) return true;
  if (!strcmp(fileName, "vol.mrml")) return true;
  if (!strcmp(fileName, "vol_and_cube.mrml")) return true;
  return false;
}

int vtkArchiveTest1(int argc, char * argv[] )
{
  if (argc < 2)
    {
    std::cerr << "Usage: vtkArchiveTest1 archive.zip" << std::endl;
    return EXIT_FAILURE;
    }

  //
  // list files in the archive
  //
  std::vector<std::string> files;
  if (!vtkArchive::ListArchive(argv[1], files) || files.size() != 2)
    {
    std::cerr << "failed to list archive: " << argv[1] << std::endl;
    return EXIT_FAILURE;
    }
  for (size_t n = 0; n < files.size(); ++n)
    {
    if (!validFile(files[n].c_str()))
      {
      std::cerr << "unexpected file in archive listing: " << files[n]
                << std::endl;
      return EXIT_FAILURE;
      }
    }

  //
  // unzip test file to local directory
  //
  if ( vtksys::SystemTools::FileExists("archiveTest") )
    {
    if ( !vtksys::SystemTools::RemoveADirectory("archiveTest") )
      {
      std::cerr << "Error: could not remove archiveTest directory" << std::endl;
      return EXIT_FAILURE;
      }
    }

  vtksys::SystemTools::MakeDirectory("archiveTest");
  vtksys::SystemTools::ChangeDirectory("archiveTest");
  bool res = vtkArchive::ExtractTar(argv[1], true, true);
  if (!res)
    {
    std::cerr << "failed to extract archive :" << argc << std::endl;
    return EXIT_FAILURE;
    }

  //
  // check that correct files were extracted
  //
  vtksys::Directory cwd;
  cwd.Load(".");
  unsigned int numberOfFiles = cwd.GetNumberOfFiles();
  unsigned int validFiles = 0;
  for (unsigned int i = 0; i < numberOfFiles; i++)
    {
    std::cout << "Extracted file: " << cwd.GetFile(i) << std::endl;
    if ( validFile(cwd.GetFile(i)) ) validFiles++;
    }

  if (validFiles != 4)
    {
    std::cerr << "failed to extract archive :" << argv[1] << std::endl;
    return EXIT_FAILURE;
    }
  vtksys::SystemTools::ChangeDirectory("..");


  //
  // Create a new zip file of the archive
  //

  if ( vtksys::SystemTools::FileExists("archiveTest.zip") )
    {
    if ( !vtksys::SystemTools::RemoveFile("archiveTest.zip") )
      {
      std::cerr << "Error: could not remove archiveTest.zip file" << std::endl;
      return EXIT_FAILURE;
      }
    }

  std::cout << "creating archiveTest.zip" << std::endl;
  std::string zipFilePath = vtksys::SystemTools::GetCurrentWorkingDirectory() +
                                                    std::string("/archiveTest.zip");
  std::string zipDirPath = vtksys::SystemTools::GetCurrentWorkingDirectory() +
                                                    std::string("/archiveTest");
  res = vtkArchive::Zip(zipFilePath.c_str(), zipDirPath.c_str());
  if (!res)
    {
    std::cerr << "failed to create new archive" << std::endl;
    return EXIT_FAILURE;
    }

  // make fresh output directory
  std::cout << "creating extractedArchiveTest" << std::endl;
  if ( vtksys::SystemTools::FileExists("extractedArchiveTest") )
    {
    if ( !vtksys::SystemTools::RemoveADirectory("extractedArchiveTest") )
      {
      std::cerr << "Error: could not remove extractedArchiveTest directory" << std::endl;
      return EXIT_FAILURE;
      }
    }
  vtksys::SystemTools::MakeDirectory("extractedArchiveTest");

  std::cout << "unzipping archiveTest.zip" << std::endl;
  res = vtkArchive::UnZip(zipFilePath.c_str(), "extractedArchiveTest");
  if (!res)
    {
    std::cerr << "failed to extract new archive" << std::endl;
    return EXIT_FAILURE;
    }

  //
  // check that correct files were extracted
  //
  std::cout << "checking files in extractedArchiveTest" << std::endl;
  vtksys::SystemTools::ChangeDirectory("extractedArchiveTest");
  cwd.Load("archiveTest");
  numberOfFiles = cwd.GetNumberOfFiles();
  validFiles = 0;
  for (unsigned int i = 0; i < numberOfFiles; i++)
    {
    std::cerr << "Extracted file: " << cwd.GetFile(i) << std::endl;
    if ( validFile(cwd.GetFile(i)) ) validFiles++;
    }

  if (validFiles != 4)
    {
    std::cerr << "failed to extract archive : " << "extractedArchiveTest" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
