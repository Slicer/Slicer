/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/


#include "slicerio.h"
#include <string.h>

int main(int argc, char **argv)
{
    char buffer[1024];
    size_t objSize = sizeof(char);
    size_t objCount = 100;
    size_t opRet;

    FILE *sp = NULL;
    char *str = NULL;
    int count;
    int i;
 
    if (argc != 2)
    {
        printf("Usage: \n1. testSlicerIO path/fileName\n2. testSlicerIO \"command pipe\" (e.g. \"| date\")\n");
        return 1; 
    }



    /* ------------------------------------------------------ */
    /* Testing fgets                                          */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fgets .........\n\n");
    printf("Case 1: Get a line from a file stream or command pipe\n");

    sp = fopen(argv[1], "r");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    memset(buffer, 0, 1024);
    fgets(buffer, 1024, sp);
    if (strlen(buffer) > 0)
    {
        printf(".........passed\n\n");
        printf("Output: %s \n\n", buffer);
    }
    else
    {
        printf(".........failed\n\n");
    }
    fclose(sp);

    printf("Case 2: Get a line from stdin (please type and hit return)\n");
    str = fgets(buffer, 1024, stdin);
    if (str != NULL)
    {
        printf(".........passed\n\n");
        printf("Output: %s \n\n", str);
    }
    else
    {
        printf(".........failed\n\n");
    }



    /* ------------------------------------------------------ */
    /* Testing fread                                          */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fread .........\n\n");
    printf("Case 1: Read from a file stream or command pipe\n");

    sp = fopen(argv[1], "r");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    memset(buffer, 0, 1024);
    opRet = fread(buffer, objSize, objCount, sp);

    if (opRet > 0)
    {
        printf(".........passed\n\n");
    }
    else
    {
        printf(".........failed\n\n");
    }
    printf("Output: %s \n\n", buffer);

    printf("Case 2: Read from stdin (please type (> 20 chars) and hit return)\n");
    memset(buffer, 0, 1024);
    objCount = 20;
    opRet = fread(buffer, objSize, objCount, stdin);
    if (opRet > 0)
    {
        printf(".........passed\n\n");
    }
    else
    {
        printf(".........failed\n\n");
    }
    printf("Output: %s \n\n", buffer);

    fclose(sp);



    /* ------------------------------------------------------ */
    /* Test fwrite                                            */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fwrite .........\n\n");
    printf("Case 1: Write to a file\n");

    sp = fopen("testFwrite.txt", "w");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    memset(buffer, 0, 1024);
    objCount = 200;
    opRet = fwrite(buffer, objSize, objCount, sp);
    fclose(sp);
    if (opRet != objCount)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    printf("Case 2: Write to stdout\n");
    opRet = fwrite(buffer, objSize, objCount, stdout);
    if (opRet != objCount)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }


    printf("Case 3: Write to stderr\n");
    opRet = fwrite(buffer, objSize, objCount, stderr);
    if (opRet != objCount)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    printf("Case 4: Write to a command pipe\n");

    sp = fopen("| cal", "w");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    strcpy(buffer, "Hello, I'm here in Boston Ma.");
    /* memset(buffer, 38, 1024); */
    opRet = fwrite(buffer, objSize, objCount, sp);
    fclose(sp);
    if (opRet != objCount)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }



    /* ------------------------------------------------------ */
    /* Testing fprintf                                           */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fprintf .........\n\n");
 
    printf("Case 1: Write to stdout\n");
    opRet = fprintf(stdout, "%s\n", "Boston MA");
    if (opRet <= 0)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    printf("Case 2: Write to stderr\n");
    opRet = fprintf(stderr, "%s %d\n", "Boston Ma", 2006);
    if (opRet <= 0)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    printf("Case 3: Write to a file\n");
    sp = fopen("testFprintf.txt", "w");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    strcpy(buffer, "Hello, I'm here in Boston Ma.");
    /* memset(buffer, 38, 1024); */
    opRet = fprintf(sp, "%s %d\n", buffer, 2006);
    fclose(sp);

    if (opRet <= 0)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    printf("Case 4: Write to command pipe\n");
    sp = fopen("| cal", "w");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    strcpy(buffer, "Hello, I'm here in Boston Ma.");
    /* memset(buffer, 38, 1024); */
    opRet = fprintf(sp, "%s %d\n", buffer, 2006);
    fclose(sp);

    if (opRet <= 0)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }


    /* ------------------------------------------------------ */
    /* Testing fflush                                           */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fflush .........\n\n");
 
    printf("Case 1: Flush stdout\n");
    opRet = fflush(stdout);
    if (opRet != 0)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    printf("Case 2: Flush stderr\n");
    opRet = fflush(stderr);
    if (opRet != 0)
    {
        printf(".........failed\n\n");
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    printf("Case 3: Flush a file stream or command pipe\n");

    sp = fopen("testFflush.txt", "w");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    opRet = fflush(sp);
    if (opRet != 0)
    {
        printf(".........failed\n\n");
        if (sp->interp != NULL)
        {
            int errno = Tcl_GetErrno();
            printf("%s", Tcl_ErrnoMsg(errno));
        }
        return 1;
    }
    else
    {
        printf(".........passed\n\n");
    }

    fclose(sp);



    /* ------------------------------------------------------ */
    /* Testing fgetc                                          */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fgetc .........\n\n");
 
    printf("Case 1: fgetc from stdin (type > 10 chars)\n");
    i = 0;
    while (i++ < 10) {
        printf("%c", fgetc(stdin));
    }
    printf("\n");
    printf(".........passed\n\n");


    printf("Case 2: fgetc from a file or command pipe\n");
    sp = fopen(argv[1], "r");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    count = 1; 
    while ((i = fgetc(sp)) != EOF && count < 30)
    {
        printf("%c", i);
        count++;
    }
    fclose(sp);
    printf("\n");
    printf(".........passed\n\n");



    /* ------------------------------------------------------ */
    /* Testing fputc                                          */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fputc .........\n\n");
 
    printf("Case 1: fputc to stdout\n");
    strcpy(buffer, "Hello, I'm here in Boston Ma.");
    count = 0; 
    fflush(stdout);
    while (count < 30) {
        fputc(buffer[count], stdout);
        count++;
    }
    printf("\n");
    printf(".........passed\n\n");


    printf("Case 2: fputc to stderr\n");
    count = 0; 
    fflush(stderr);
    while (count < 30) {
        fputc(buffer[count], stderr);
        count++;
    }
    printf("\n");
    printf(".........passed\n\n");


    printf("Case 3: fputc to a file\n");
    sp = fopen("testFputc.txt", "w");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    count = 0; 
    while (count < 30)
    {
        printf("%c", fputc(buffer[count], sp));
        count++;
    }
    fclose(sp);
    printf("\n");
    printf(".........passed\n\n");


    printf("Case 4: fputc to a command pipe\n");
    sp = fopen("| cal", "w");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    count = 0; 
    while (count < 30)
    {
        printf("%c", fputc(buffer[count], sp));
        count++;
    }
    fclose(sp);
    printf("\n");
    printf(".........passed\n\n");



    /* ------------------------------------------------------ */
    /* Testing feof                                           */
    /* ------------------------------------------------------ */
    printf("\n......... Testing feof .........\n\n");
 
    printf("Case 1: feof to a file or command pipe\n");
    sp = fopen(argv[1], "r");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    while (!feof(sp))
    {
        fgets(buffer, 1024, sp);
        printf("%s\n", buffer);
    } 
    fclose(sp);
    printf("\n");
    printf(".........passed\n\n");


    printf("Case 2: feof to stdin (use control-d to finish input)\n");
    while (!feof(stdin))
    {
        fgets(buffer, 1024, stdin);
        printf("%s\n", buffer);
    }
    printf("\n");
    printf(".........passed\n\n");



    /* ------------------------------------------------------ */
    /* Testing fseek                                          */
    /* ------------------------------------------------------ */
    printf("\n......... Testing fseek .........\n\n");
 
    printf("Case 1: fseek a file or command pipe\n");
    sp = fopen(argv[1], "r");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    fgets(buffer, 1024, sp);
    printf("%s\n", buffer);

    opRet = fseek(sp, 5, SEEK_SET);

    fgets(buffer, 1024, sp);
    printf("%s\n", buffer);

    fclose(sp);
    if (opRet == -1)
    {
        printf(".........failed (Tcl_Seek on pipes: Not supported\n\n");
        /* return 1; */
    }
    else
    {
        printf(".........passed\n\n");
    }



    /* ------------------------------------------------------ */
    /* Testing ftell                                          */
    /* ------------------------------------------------------ */
    printf("\n......... Testing ftell .........\n\n");
 
    printf("Case 1: ftell a file or command pipe\n");
    sp = fopen(argv[1], "r");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    fseek(sp, 5, SEEK_SET);
    opRet = ftell(sp);

    fclose(sp);
    if (opRet == -1)
    {
        printf(".........failed (Tcl_Tell on pipes: Not supported)\n\n");
        /* return 1; */
    }
    else
    {
        printf("Current access point = %d\n", opRet);
        printf(".........passed\n\n");
    }



    /* ------------------------------------------------------ */
    /* Testing ungetc                                          */
    /* ------------------------------------------------------ */
    printf("\n......... Testing ungetc .........\n\n");
 
    printf("Case 1: ungetc onto a file stream or command pipe\n");
    sp = fopen(argv[1], "r");
    if (sp == NULL)
    {
        printf("\n\nfopen failed\n\n");
        return 1;
    }

    count = 1; 
    while ((i = fgetc(sp)) != EOF && count < 5)
    {
        printf("%c", i);
        count++;
    }
 
    ungetc(i, sp);
    opRet = fgetc(sp);

    fclose(sp);

    if (opRet != i)
    {
        printf(".........failed");
        return 1;
    }
    else
    {
        printf("\nchar = %c\n", i);
        printf(".........passed\n\n");
    }



    printf("\nAll testing cases passed\n\n");
 
    return 0;
}

