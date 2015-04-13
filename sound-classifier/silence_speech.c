//**************************************************************************
//* File: silence_speech.c
//*
//* Created: 20th November 2013
//*
//* Purpose: Top level for classifier
//*             Due to the required memory management to make it generic
//*             this remains specific to the task in hand.
//*
//*          C was chosen for speed.
//*
//* Author: Hector Grebbell
//*
//* Copyright: Copyright (c) Hector Grebbell 2013
//*
//* The contents of this file should not be copied or distributed
//* without prior permission
//*
//* All rights reserved.
//***************************************************************************

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "classifier.h"

int main ( int argc, char *argv[] )
{    //Requires 100 files, first 50 silence, second 50 speech
    if (argc == 101 || ( argc == 103 && strlen(argv[1]) == 2 && argv[1][0] == '-' && argv[1][1] == 'v'))
    {
        char vb = 0;        // verbose level
        if (argc == 103)
        {
            #ifdef NO_VERBOSE
            printf("WARNING: Code was compiled without Verbose support\n");
            #else
            vb = atoi(argv[2]);
            #endif
            argc-=2; argv+=2;
        }
        return classifier(vb, argc-1, argv+1);
    }
    printf("\nUsage: %s [-v n] silence_1..silence_50 speech_1..speech_50\n  -v  n    verbose    Print additional information (n = level)\n\n", argv[0]);
    return 0;
}
