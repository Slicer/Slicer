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

#include <string.h> // strcmp

void writeFile(FILE *in, FILE *out)
{
    int c;
    while (! feof(in))
    {
        c = fgetc(in); 
        fputc(c, out);
    }
    fclose(in);
    fflush(out);
}


int main(int argc, char **argv)
{
    FILE *in;
    FILE *out;
    char **ptr;
    int count = 1;
    int index;
    int help = 0;

    in = stdin;
    out = stdout;

    index = argc;
    ptr = argv;
    while (count < argc)
    {
        /* print usage info if requested */
        if (strcmp(*(++ptr), "--help") == 0)
        {
            printf("Usage: \nscat [<filename> ...] [-o <filename>]\n");
            help = 1;
            break;
        }

        /* find if we have an output file */
        if (strcmp(*(ptr), "-o") == 0 && count <= (argc-1))
        {
            out = fopen(*(++ptr), "w");
            break;
        }

        count++;
    }        

    if (!help)
    {
        if (argc == 1)
        {
            writeFile(in, out);
        }
        else
        {
            count = 1;
            while (count < argc)
            {
                if (strcmp(*(++argv), "-o") == 0)
                {
                    if (count == 1)
                    {
                        writeFile(in, out);
                    }
                    break;
                }    
                else
                {
                    in = fopen(*(argv), "r");
                    writeFile(in, out);
                }

                count++;
            }
        }
    }


    if (out != NULL && out != stdout)
    {
        fclose(out);
    }

    return 0;
}
