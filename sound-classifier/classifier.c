//**************************************************************************
//* File: classifier.c
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

#include "classifier.h"
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>

#ifdef NO_VERBOSE
#define VERBOSE_PRINT(n,s,a) vb=vb;
#else
#define VERBOSE_PRINT(n,s,a) if(vb>n)printf(s, a)
#endif

//**********************************************************************
//* Reads in a file and generates Log(Mean(E)), Log(Mean(M)), Mean(Z)//*
//* Where E is energy, M maginitude & Z the zero crossing rate
//**********************************************************************
char file_average(char file[], long double *energy, long double *mag, long double *zcr)
{
    *energy = *mag = *zcr = 0;                    // Zero these initially
    int pos = 0, zct = 0;                         // position in temp array, sum of zcr's ish
    long double tempArr[WINDOW_SIZE][3] = {{0}};  // We need to store a window worth of data. Three values - one for each metric.
    long double lastTemp[3] = {0}, last[2] = {0}, tempR = 0, tempR2; // Value being replaced, prev total value, reading, prev reading
    unsigned short line=0;
    
    FILE* f;
    if (!(f = fopen(file, "r")))
    {
        printf("File (%s) could not be opened for reading\n", file);
        return -1;
    }
    for (; line < SAMPLE_LEN; line++)            // There should be sample_len lines
    {
        pos = line%WINDOW_SIZE;

        tempR2 = tempR;                         // Store the old val
        if (fscanf(f,"%Lf\n",&tempR)<=0)        // Read a value
        {
            printf("Read error on file %s\n", file);
            if (fclose(f))
                printf("File (%s) could not be closed\n", file);
            return -1;
        }
        //store previous values
        lastTemp[0] = tempArr[pos][0];
        lastTemp[1] = tempArr[pos][1];
        lastTemp[2] = tempArr[pos][2];
        
        // Calculate new values
		// Energy is amplitude squared.
        tempArr[pos][0] = tempR*tempR;
		// Magnitude is the absolute value
        tempArr[pos][1] = (tempR >= 0) ? tempR : -tempR;
		// Zero crossing rate increases if zero has been crossed
        zct += (tempArr[pos][2] = !((tempR >= 0) ^ (tempR2 < 0))) - lastTemp[2];
        
        //store new window sum
        last[0] =  last[0] + tempArr[pos][0] - lastTemp[0];
        last[1] =  last[1] + tempArr[pos][1] - lastTemp[1];
        
        //add new window sum
        *energy += last[0];
        *mag += last[1];
        *zcr += (long double) zct / (2*WINDOW_SIZE);
        
    }
    //calculate [log] mean from sum
    *energy = log((*energy)/SAMPLE_LEN);
    *mag = log((*mag)/SAMPLE_LEN);
    *zcr = (*zcr)/SAMPLE_LEN;
    
    if (fclose(f))
    {
        printf("File (%s) could not be closed\n", file);
        //return -1;  Meh, whats the worst that could happen!
    }
    return 0;
}

// I'd suggest againt looking into this function... The pointer arithmatic
// gets a bit confusing, but it was the most efficient way I could see to do things
//**********************************************************************
//* Generates the Means and Standard deviations for the two data classes
//**********************************************************************
// TODO(HG): Remove hardcoded data set constants
char train(long double *data[3], long double *silence, long double *speech, char vb)
{
    int i, j;                // dataset, value
    //Make it all 0 to be safe
    *silence = *(silence+1) = *(silence+2) = *(silence+3) = *(silence+4) = *(silence+5) = 0;
    *speech = *(speech+1) = *(speech+2) = *(speech+3) = *(speech+4) = *(speech+5) = 0;
    for (i=0;i<9;i++)        // 9 Datasets (the 10th is test data)
    {
        VERBOSE_PRINT(2, "      Train Summing training set %d of 9\n", i);
        for (j = 0; j < 15;)    // So... I know... But it works.. Basically we have 10 lots of 3 values (30 total) in one big block of memory.
        {                        // First half will be silence data
            *(silence+3) += (*(*(data+i)+j))*(*(*(data+i)+j));        // We use sum of squares for VAR. silence+3 is the Var E
            *(silence+0) += *(*(data+i)+j++);                        // Sum of values for Mean. silence is Mean E. Increment J to get to M
            *(silence+4) += (*(*(data+i)+j))*(*(*(data+i)+j));        // Var M
            *(silence+1) += *(*(data+i)+j++);                        // Mean M, Increment J to get to Z
            *(silence+5) += (*(*(data+i)+j))*(*(*(data+i)+j));        // Var Z
            *(silence+2) += *(*(data+i)+j++);                        // Mean Z, Increment J to get to next E val
        }
        for (;j<30;)            // Second Half is speech data
        {
            *(speech+3) += (*(*(data+i)+j))*(*(*(data+i)+j));        // Var E
            *(speech+0) += *(*(data+i)+j++);                        // Mean E
            *(speech+4) += (*(*(data+i)+j))*(*(*(data+i)+j));        // Var M
            *(speech+1) += *(*(data+i)+j++);                        // Mean M
            *(speech+5) += (*(*(data+i)+j))*(*(*(data+i)+j));        // Var Z
            *(speech+2) += *(*(data+i)+j++);                        // Mean Z
        }
    }
    // This May look messy, but all we are really doing is dividing every total by 45 (the number of files in each set)
    *silence /= 45; *(silence+1) /= 45; *(silence+2) /= 45; *speech /= 45; *(speech+1) /= 45; *(speech+2) /= 45;        //Means
    *(silence+3) /= 45; *(silence+4) /= 45; *(silence+5) /= 45;                                                            //Vars
    *(speech+3) /= 45; *(speech+4) /= 45; *(speech+5) /= 45;                                                            //Vars
    
    //Vars have to have the Mean^2 subtracted
    *(silence+3) -= (*silence)*(*silence); *(silence+4) -= (*(silence+1))*(*(silence+1)); *(silence+5) -= (*(silence+2))*(*(silence+2));
    *(speech+3) -= *speech**speech; *(speech+4) -= (*(speech+1))*(*(speech+1)); *(speech+5) -= (*(speech+2))*(*(speech+2));
#ifndef NO_VERBOSE
    if (vb>2) printf("  Silence ME = %.4Lf\tMM= %.4Lf\tMZ= %.4Lf\tDE = %.4Lf, DM= %.4Lf\tDZ= %.4Lf\n  Speech  ME = %.4Lf\tMM= %.4Lf\tMZ= %.4Lf\tDE = %.4Lf, DM= %.4Lf\tDZ= %.4Lf\n", *silence, *(silence+1), *(silence+2), *(silence+3), *(silence+4), *(silence+5), *speech, *(speech+1), *(speech+2), *(speech+3), *(speech+4), *(speech+5));
#endif
    return 0;
}

//**********************************************************************
//* Works out the probability the value is in each group
//**********************************************************************
char classify(long double data[3], long double *silence, long double *speech, char vb)
{
    long double probSilence, probSpeech;
    // Now I stripped out some of the values (2*PI) since they really act as multipliers and won't effect the result.
    
    probSilence = exp(-((data[0]-*silence)*(data[0]-*silence))/(2*(*(silence+3))))/sqrt(*(silence+3));
    probSilence *= exp(-((data[1]-(*(silence+1)))*(data[1]-(*(silence+1))))/(2*(*(silence+4))))/sqrt(*(silence+4));
    probSilence *= exp(-((data[2]-(*(silence+2)))*(data[2]-(*(silence+2))))/(2*(*(silence+5))))/sqrt(*(silence+5));
    
    probSpeech =  exp(-((data[0]-*speech)*(data[0]-*speech))/(2*(*(speech+3))))/sqrt(*(speech+3));
    probSpeech *= exp(-((data[1]-(*(speech+1)))*(data[1]-(*(speech+1))))/(2*(*(speech+4))))/sqrt(*(speech+4));
    probSpeech *= exp(-((data[2]-(*(speech+2)))*(data[2]-(*(speech+2))))/(2*(*(speech+5))))/sqrt(*(speech+5));
    
    VERBOSE_PRINT(2, "      Silence Weighting for value: %Lf\n", probSilence);
    VERBOSE_PRINT(2, "      Speech  Weighting for value: %Lf\n", probSpeech);
    
    return (probSpeech > probSilence) ? 1 : 0;
}


//**********************************************************************
//* Runs 10-Fold Validation using a Naive Bayers on each of the files
//* vb - verbose level
//**********************************************************************
int classifier(char vb, int numFiles, char *files[])
{    
    //10 Folds of 10 Samples with 3 vals (E, M, Z)
    long double folds[10][10][3] = {{{0}}};
    //Total Accuracy
    long double accuracy = 0.0f;
    
    //k-fold ///////////////////////////////////////////////////////////
    VERBOSE_PRINT(1,"Running 10-Fold Grouping\n%s", "");
    int fill[10] = {0};                // Count of how full each fold is
    int n = 0;                        // Current File
    srand(time(NULL)*getpid());        // For our needs a time based seed is fine
    
    VERBOSE_PRINT(1," Reading In Silence Files%s\n", "");
    VERBOSE_PRINT(3,"  File,           G, E,         M,         Z\n%s", "");
    
#ifdef GEN_ADD_DATA
	FILE* f;
    if (!(f = fopen("averages.csv", "w")))
	{
		printf("File \"averages.csv\" could not be opened for writing\n");
		return -1;
	}
#endif
    
    for (;n < numFiles/2;n++)
    {
        int r = rand()%10;                             // choose a group
        while (fill[r] == 5) r = rand()%10;             // TODO: Inefficient
                                                     // Calculates the Log(Mean E), Log(Mean M), Mean Z
        if (file_average(files[n], &folds[r][fill[r]][0], &folds[r][fill[r]][1], &folds[r][fill[r]][2]))
        {
            printf ("Aborting\n");
            return -1;
        }
#ifdef GEN_ADD_DATA
		fprintf(f, "%s, %Lf, %Lf, %Lf\n", basename(files[n]), folds[r][fill[r]][0], folds[r][fill[r]][1], folds[r][fill[r]][2]);
#endif
#ifndef NO_VERBOSE
        if (vb > 3) printf ("  %s, %d, %Lf, %Lf, %Lf\n", basename(files[n]), r, folds[r][fill[r]][0], folds[r][fill[r]][1], folds[r][fill[r]][2]);
#endif
        fill[r]++;                                     // Add one to full count
    }
    VERBOSE_PRINT(1," Reading In Speech Files%s\n", "");
    VERBOSE_PRINT(3,"  File,           G, E,         M,         Z\n%s", "");
    for (;n < numFiles;n++)                             // See Above
    {
        int r = rand()%10;
        while (fill[r] == 10) r = rand()%10;
        if (file_average(files[n], &folds[r][fill[r]][0], &folds[r][fill[r]][1], &folds[r][fill[r]][2]))
        {
            printf ("Aborting\n");
            return -1;
        }
#ifdef GEN_ADD_DATA
		fprintf(f, "%s, %Lf, %Lf, %Lf\n", basename(files[n]), folds[r][fill[r]][0], folds[r][fill[r]][1], folds[r][fill[r]][2]);
#endif
#ifndef NO_VERBOSE
        if (vb > 3) printf ("  %s,  %d, %Lf, %Lf, %Lf\n", basename(files[n]), r, folds[r][fill[r]][0], folds[r][fill[r]][1], folds[r][fill[r]][2]);
#endif
        fill[r]++;
    }
#ifdef GEN_ADD_DATA
	if (fclose(f))
    {
        printf("File \"averages.csv\" could not be closed\n");
        //return -1;  Meh, whats the worst that could happen!
    }// Use case is so small I haven't checked return codes. You have to recompile anyways... if you want it fix it yourself :P
    f = popen("gnuplot", "w");
    fprintf(f, "set output 'EvM.png'\nset terminal png size 1200,600\nset xlabel 'Short Term Energy'\nset ylabel 'Short Term Magnitude'\nplot 'averages.csv' every ::0::50 u 2:3 t 'silence', 'averages.csv' every ::50::100 u 2:3 t 'speech'\n");
    fprintf(f, "set output 'EvZ.png'\nset terminal png size 1200,600\nset xlabel 'Short Term Energy'\nset ylabel 'Zero Crossing Rate'\nplot 'averages.csv' every ::0::50 u 2:4 t 'silence', 'averages.csv' every ::50::100 u 2:4 t 'speech'\n");
    fprintf(f, "set output 'MvZ.png'\nset terminal png size 1200,600\nset xlabel 'Short Term Magnitude'\nset ylabel 'Zero Crossing Rate'\nplot 'averages.csv' every ::0::50 u 3:4 t 'silence', 'averages.csv' every ::50::100 u 3:4 t 'speech'\n");
    fflush(f);
    pclose(f);
#endif
    VERBOSE_PRINT(1,"Completed 10-Fold Grouping\n%s", "");
    //k-fold end ///////////////////////////////////////////////////////
    
    //cross-validation /////////////////////////////////////////////////
    if (vb > 1) printf("Running Cross Validation (Naive Bayes)\n");
    int i, j, res;                    // test set, loopItr, holder for classify result
    long double *train_data[9];        // pointers to organise training data
    long double silence[6];            // Mean E, Mean M, Mean Z, Var E, Var M, Var Z
    long double speech[6];            // Mean E, Mean M, Mean Z, Var E, Var M, Var Z
    long double cycle_accuracy;        // Count of correct
    
    for (i = 0; i < 10; i++)
    {
        VERBOSE_PRINT(1,"  Cross Validation Cycle %d of 10\n", i+1);
        n=0; cycle_accuracy = 0.0f;
        for (j=0;j<10;j++)
        {
            if (j==i) continue;        //Can't train with the test data
            train_data[n] = *folds[j];
            n++;
        }
        
        VERBOSE_PRINT(1,"    Training...\n%s", "");
        train(train_data, silence, speech, vb);
        VERBOSE_PRINT(1,"    Training Complete\n%s","");
        
        for (j=0;j<10;j++)        //Test each sample in test set
        {
            VERBOSE_PRINT(2,"     Testing TestSample %d -\n", j+1);
            res = classify(folds[i][j], silence, speech, vb);
            if (j > 4) res+=2;
            switch (res)
            {
                case 0:
                    VERBOSE_PRINT(2,"       Detected: SILNCE; Actual: SILNCE\n%s", "");
                    cycle_accuracy+=1;
                    break;
                case 1:
                    VERBOSE_PRINT(2,"       Detected: SPEECH; Actual: SILNCE\n%s", "");
                    break;
                case 2:
                    VERBOSE_PRINT(2,"       Detected: SILNCE; Actual: SPEECH\n%s", "");
                    break;
                case 3:
                    VERBOSE_PRINT(2,"       Detected: SPEECH; Actual: SPEECH\n%s", "");
                    cycle_accuracy+=1;
                    break;
                default:
                    printf("You have entered me in ways I never imagined possible!\n");
                    return -1;
            }
        }
        VERBOSE_PRINT(0,"    Cycle %2d ", i+1);
        VERBOSE_PRINT(0,"Complete - Cycle Accuracy %.0Lf%%\n", cycle_accuracy*10);
        accuracy+=cycle_accuracy;
    }
    printf("Tests Complete - Test Accuracy  %.2Lf%%\n", accuracy);
    //cross-validation end ///////////////////////////////////////////// 
    return 0;
}




