//**************************************************************************
//* File: classifier.h
//*
//* Created: 20th November 2013
//*
//* Purpose: 10-fold Naive Bayers Classification
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

#ifndef __CLASSIFIER__
#define __CLASSIFIER__

//#define NO_VERBOSE
//#define GEN_ADD_DATA         // If defined compiled code -
                              //  Generates averages.csv
                              //  Generates graphs to [graph].png

#define WINDOW_SIZE 240
#define SAMPLE_LEN 2400

int classifier(char vb, int numFiles, char *files[]);

#endif
