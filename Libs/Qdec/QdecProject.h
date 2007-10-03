/**
 * @file  QdecProject.h
 * @brief API class containing all qdec subject data and methods
 *
 * Top-level interface class containing all data associated with a users
 * subject group, and potentially mri_glmfit processed data associated with
 * that group.
 */
/*
 * Original Author: Nick Schmansky
 * CVS Revision Info:
 *    $Author: nicks $
 *    $Date: 2007/05/23 21:20:58 $
 *    $Revision: 1.5 $
 *
 * Copyright (C) 2007,
 * The General Hospital Corporation (Boston, MA).
 * All rights reserved.
 *
 * Distribution, usage and copying of this software is covered under the
 * terms found in the License Agreement file named 'COPYING' found in the
 * FreeSurfer source code root directory, and duplicated here:
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
 *
 * General inquiries: freesurfer@nmr.mgh.harvard.edu
 * Bug reports: analysis-bugs@nmr.mgh.harvard.edu
 *
 */

#ifndef QDECPROJECT_H
#define QDECPROJECT_H

#include <QdecConfigure.h>
#include "vtkQdecWin32Header.h"

#include <string>
#include <vector>

#include "QdecDataTable.h"
#include "QdecGlmDesign.h"
#include "QdecGlmFit.h"
#include "QdecGlmFitResults.h"
#include "ProgressUpdateGUI.h"

//#include "vtkQdec.h"

using namespace std;

class VTK_Qdec_EXPORT QdecProject
{
public:

  // Constructors/Destructors
  //

  QdecProject ( );

  virtual ~QdecProject ( );

  /**
   * Load a .qdec project file (containing all necessary info to begin
   * working either on a new project, or to continue working using
   * results from a prior saved work session). isDataDir should be a
   * directory where we can expand the .qdec file (like /tmp).
   * @return int
   * @param  isFileName
   * @param  isDataDir
   */
  int LoadProjectFile ( const char* isFileName,
                        const char* isDataDir = "/tmp" );

  /**
   * Save all necessary information pertaining to this project
   * (all subject data, any results, any user preferences).
   * @return int
   * @param  isFileName
   * @param  isDataDir
   */
  int SaveProjectFile ( const char* isFileName,
                        const char* isDataDir = "/tmp" );

    /**
   *
   * The command format strings to zip and unzip a file. Returns -1 if
   * parameter is invalid. The default is acceptable for Linux systems
   * with unzip and zip installed. The substitutions that are made
   * are:
   *
   * %1 - Full project filename
   * %2 - Expanded project base name
   * %3 - Working dir (ifnDataDir)
   *
   * Default zip format string is:
   * cd %3; zip -r %1 %2 > /dev/null
   * Default unzip format string is:
   * unzip -d %3 %1 > /dev/null
   *
   * @return int
   * @param isFormat
   */
  int SetZipCommandFormat ( const char* isFormat );
  int SetUnzipCommandFormat ( const char* isFormat );


  /**
   * @return int
   * @param  isFileName
   */
  int LoadDataTable ( const char* isFileName );

  /**
   * @return void
   * @param  iFilePointer
   */
  void DumpDataTable ( FILE* iFilePointer );

  /**
   * @return int
   * @param  isFileName
   */
  int SaveDataTable ( const char* isFileName );


  /**
   * @return QdecDataTable*
   */
  QdecDataTable* GetDataTable ( );


  /**
   * @return string
   */
  string GetSubjectsDir ( );


  /**
   * @param  ifnSubjectsDir
   */
  int SetSubjectsDir ( const char* ifnSubjectsDir );


  /**
   * @return string
   */
  string GetAverageSubject ( );


  /**
   * @param  isSubjectName
   */
  void SetAverageSubject ( const char* isSubjectName );

  /**
   * @return string
   */
  string GetDefaultWorkingDir ( );


  /**
   * @return string
   */
  string GetWorkingDir ( );


  /**
   * @return 0 if ok, 1 on error
   * @param  isWorkingDir
   */
  int SetWorkingDir ( const char* isWorkingDir );


  /**
   * @return vector< string >
   */
  vector< string > GetSubjectIDs ( );


  /**
   * @return vector< string >
   */
  vector< string > GetDiscreteFactors ( );


  /**
   * @return vector< string >
   */
  vector< string > GetContinousFactors ( );


  /**
   * @return string
   */
  string GetHemi ( );


  /**
   * @return string
   */
  string GetUnzipCommand();
  string GetZipCommand();
  string GetRmCommand();
  
  /**
   *
   */
  void SetUnzipCommand(const char *cmd);
  void SetZipCommand(const char *cmd);
  void SetRmCommand(const char *cmd);
  
  /**
   * From the given design parameters, this creates the input data required by
   * mri_glmfit:
   *  - the 'y' data (concatenated subject volumes)
   *  - the FSGD file
   *  - the contrast vectors, as .mat files
   * and writes this data to the specified working directory.
   * @return int
   * @param  isName
   * @param  isFirstDiscreteFactor
   * @param  isSecondDiscreteFactor
   * @param  isFirstContinuousFactor
   * @param  isSecondContinuousFactor
   * @param  isMeasure
   * @param  isHemi
   * @param  iSmoothnessLevel
   * @param  iProgressUpdateGUI (optional)
   */
  int CreateGlmDesign ( const char* isName,
                        const char* isFirstDiscreteFactor,
                        const char* isSecondDiscreteFactor,
                        const char* isFirstContinuousFactor,
                        const char* isSecondContinuousFactor,
                        const char* isMeasure,
                        const char* isHemi,
                        int iSmoothnessLevel,
                        ProgressUpdateGUI* iProgressUpdateGUI=NULL );


  /**
   * @return int
   */
  int RunGlmFit ( );

  /**
   * @return int
   */
  //int LoadGlmDesign(const char *fileName);

  /**
   * @return QdecGlmFitResults
   */
  QdecGlmFitResults* GetGlmFitResults ( );

  /**
   * Run mri_label2label on each subject, mapping the label that was drawn on 
   * the average surface to each subject. Optionally supply a window manager
   * to allow posting progress info
   * @return int
   * @param  ifnLabel
   * @param  iProgressUpdateGUI (optional)
   */
  int GenerateMappedLabelForAllSubjects 
    ( const char* ifnLabel,
      ProgressUpdateGUI* iProgressUpdateGUI=NULL );


  /**
   * @return QdecGlmDesign
   */
  QdecGlmDesign* GetGlmDesign ( );

  /**
   * The file name of our metadata file, for the project file archive.
   * @return const char*
   */
  const char* GetMetadataFileName () const;

  /**
   * Perform substitutions for command format strings. See
   * documentation for Set(Un)ZipCommandFormat. This will perform the
   * substitutions on isFormat and write the command to iosCommand
   * (overwriting the contents of iosCommand).
   *
   */
  void FormatCommandString ( const char* ifnProject,
                             const char* isExpandedProjectBaseName,
                             const char* isWorkingDir,
                             const char* isFormat,
                             string& iosCommand ) const;

private:

  // private attributes
  //

  string mfnProjectFile;
  QdecDataTable* mDataTable;
  QdecGlmDesign* mGlmDesign;
  QdecGlmFit* mGlmFitter;

  // The command format to run to zip and unzip a file.
  string msBinaryPath;
  string msZipCommandFormat;
  string msUnzipCommandFormat;

  string msZipCommand;
  string msUnzipCommand;
  string msRmCommand;

};

#endif // QDECPROJECT_H
