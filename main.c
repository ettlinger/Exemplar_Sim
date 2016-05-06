/****************************************************************
 *
 * Exemplar-based computational model of vowel production
 *
 * by Marc Ettlinger
 *
 * This program simulates B speaker/hearers communicating with each other to observe effects on production and perception as predicted by exemplars.
 * It uses an exemplar-based model similar to the one outlined in Pierrehumbert 2001 and is based on the exchange of vowel tokens with up to 2 fundemental frequencies.
 * Each bot's vowel space is represented by a set of arrays, one for each vowel. Each cell in the array stores the exemplar weighting for that particular 
 * frequency. The frequency range is borken up into discrete values based on the Just Noticable Difference. So, using a MINFREQ of 200 and a MAXFREQ of 700 and
 * a JND of 5 Hz results in 100 SEGMENTS, one each for 200, 205, 210, etc. Hz.
 * The speaker/hearers (bots) exchange tokens of any one of V vowels by using one of 2 production algorithms.
 * In the first, an exemplar is picked at random based on weighting and a normalize variance is added.
 * In the second, an entrenchment model, a window determines the exemlpars included in production and the ultimate frequency is the weighted average of the 
 * exemplars within the window (+error).
 * Perception is acheived by categorizing the frequency by comparing the weighting of all the exemplars within a given window PERCEPT_WINDOW and assigning the new exemplar
 * to the vowel with the greatest activiation. 
 *
 * The are E exchanges for each cycle after which all the exemplars are aged/decayed.
 *
 * The main data structure is a Bot. See vs.h for more details.
 * Each bot sBot has a number (ID - not really used since there is an array of boys and the arrray index is the same thigns), an age, and an array of vowels.
 * 
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "vs.h"

int main(int argc, char *argv[])
{ 
  char strFile[10];
  int i=0;
  int iCycles;
  FILE *fp;

  srand (time (0));
  fp=fopen("outfile", "w");

  if (argc<2) { 
	  printf("Usage: XXX -fFilename -vVOWELS -E -cCYCLES -bBOTS -eEXCHANGES\n"); 
	  printf("\t Filename: Name of file with pre-seeded bots. If blank, bots are randomized.\n");
	  printf("\t Cycles: Number of cycles to iterate through.\n");
	  printf("\t Exchanges: Number of exchanges of vowel tokens per cycle.\n");
	  printf("\t E: Use entrenchment model of production, otherwise use normal model.\n");
	  printf("\t Vowels: Number of vowels for each speaker.\n");
	  printf("\t Bots: Number of bots/speaker-hearers.\n");
	  printf("\t\tNOTE: Bots and Vowels are overridden by the data in Botfile if applicable.\n");
	  exit(1);}


  strcpy(strFile, "0\0");
  for(i=1;i<argc;i++)	{
	if(argv[i][0] == '-') {
	    switch(argv[i][1]) {
	        case 'c' :
		  CYCLES = atoi(argv[i]+2); /* Number of cycles to iterate through */
		  break;
	        case 'b' :
		  IBOTS = atoi(argv[i]+2); /* Number of talker/hearers */
		  break;
	        case 'e' :
		  COMBOS = atoi(argv[i]+2); /* Number of exchange for each cycle */
		  break;
	        case 'E' :
		  TRENCH = 1; /*Use entrenchment algorithm for generating tokens */
		  break;
	        case 'v' :
		  VOWELS = atoi(argv[i]+2); /* Number of vowels used */
		  break;
	        case 'f' :
		  strcpy(strFile, argv[i]+2); /* Filename of file with all initial data on bots and vowels */
		  break;
		default :
		  printf("Unknown option %s\n",argv[i]);
	    }
	}
  }
  printf("___________________________________________________ starting ______________________________________________________\n");   
  printf("File <%s> Cycles <%i> Bots <%i> Exchanges <%i> Vowels <%i>\n", strFile, CYCLES, IBOTS, COMBOS, VOWELS);

  /* Function that loads the data for the bots or randomizes it */
  /* sPop is the main data structure for the program and is an array of bots */
  fLoadBots(sPop, strFile);

  /* Print all the bot info  - this outputs the relevant results for this model */
  fPrintBots(sPop, 1);
  i=0;

  /* Loop to go through the number of cycles specified - within each cycle, there can be more than one exchange */
  while (i++ < CYCLES) {
	printf("************Cycle %i************\n", i);

	/* In each cycle, the bots communicate, exchanging token. Then age. Then the results are printed */
	fTalk(sPop);

	fAge(sPop);

  	fPrintBots(sPop, 0);

  }
  fPrintBots(sPop, 2);
  printf("___________________________________________________ finished  ______________________________________________________\n");
  return 0; 
}
/**
 * This function loads the bots into the sPop structure either from the data file strFile or randomly generates them */
int fLoadBots(struct sBot sPop[], char *strFile) {

	int j,i = 0;
	int iBotcount=10;
	int iSeed;
	double x;
	FILE *fp;
	int iBot, iVowel, iAge;
	double dInfluence;
	int freq, weight;
	char strLine[80];
	char *sTok = NULL;
	int MAX_BOTS = 0;
	int MAX_VOWELS = 0;

	if (!strcmp(strFile, "0")) { /* If there's no data file, randomize all the data for the bots - currently broken */
		printf("Randomizing Bots\n");
		for  (i=0; i<IBOTS; i++) {
			sPop[i].iNum=i;
			x = (double) rand()/RAND_MAX;
			sPop[i].iAge=(100*x);
			sPop[i].dInfluence = 1;
			for (j=0;j<VOWELS;j++) {
				x = (double) rand()/RAND_MAX;
				iSeed = x * SEGMENTS;
				sPop[i].sVowel[j].iF[iSeed] = 10;
/*printf("Loaded %i vowel %i freq %i", i, j, sPop[i].sVowel[j].iF[iSeed]);*/
			}

		}
	}
	else
	{
		/* Load bots from a file */
/*		printf("Loading Bots\n");*/
		if ((fp = fopen(strFile, "r")) == NULL) {
			printf("Can't open file %s\n", strFile);
			return (1);
		} else {
			fgets(strLine, 80, fp);
			while(!feof(fp)) { 
				/* Each datafile line must be tokenize */
				sTok = strtok(strLine, " ");

				/* There are two types of datafile lines indicated by an initial letter */
				switch(sTok[0]) {
					case 'b' : /* First line for each bot is formatted as */
						   /* b X Y Z where X is the bot number, Y is age and Z is influence */
						sTok = strtok(NULL, " ");
						iBot = atoi(sTok);
						sTok = strtok(NULL, " ");
						iAge = atoi(sTok);
						sTok = strtok(NULL, " ");
						dInfluence = atof(sTok);
						sPop[iBot].iAge = iAge;
						sPop[iBot].dInfluence = dInfluence;
						printf("New bot #<%i>Age<%i>Influence<%i>\n", iBot, sPop[iBot].iAge, sPop[iBot].dInfluence);
						if (iBot > MAX_BOTS) MAX_BOTS = iBot;
						break;
					case 'v' : /* Each bot has one or more vowels formatted as */
						   /* v X Y A B where X is the vowel number, Y is the vowel transcription and A B are segment-weight pairs */ 
						sTok = strtok(NULL, " ");
						iVowel = atoi(sTok);
						if (iVowel+1 > MAX_VOWELS) MAX_VOWELS = iVowel+1;
						sTok = strtok(NULL, " ");

						/* Copy the vowel names into the appropriate field */
						strcpy(sPop[iBot].sVowel[iVowel].sIPA, sTok);
						sTok = strtok(NULL, " ");

						/* Load the A B frequency/weight pairs into the vowel. The frequency is based on segments, not absoluate frequency.
						 * So, if the minfreq=200 and the JND=5, then if A=1 and B=100, it will set the weight of the 205Hz exemplar to 100
						 * for the vowel in question */
						while (sTok != NULL) {
							freq = atoi(sTok);
							sTok = strtok(NULL, " ");
							weight = atoi(sTok);
/*							printf("   Vowel Bot<%i>Vow<%i>Freq<%i>Weight<%i>\n", iBot, iVowel, freq, weight);*/
							sPop[iBot].sVowel[iVowel].iF[freq] = weight;
							sTok = strtok(NULL, " ");
						}
						break;
					default :
						printf("Unknown data line type <%s>\n", sTok);

				}	
				fgets(strLine, 80, fp);
					
			}
			fclose(fp);
			/* Override these data vowels if the file data indicates otherwise */
			IBOTS = MAX_BOTS+1; VOWELS = MAX_VOWELS;
/*			printf("Number of bots <%i> vowels <%i>\n", IBOTS, VOWELS);*/
		}
	}
	return(0);
}
/*
 * This function goes through the entire data structure of the population of Bots and prints them out.
 * The first line is the bot's number and age.
 * Then is prints each vowel starting with the label, then all the frequency segments that have weighting followed by a weighted  average frequency for all exemplars 
 */
int fPrintBots(struct sBot sPop[], int iFlag) {
  int i=0, j=0, k=0;
  double sd, sum=0.0, weight=0.0, sumsq=0.0, sdweight=0.0, avg=0.0;
  FILE *fp;

  printf("_______%i flag\n", iFlag);
  if (iFlag==1) {
	  printf("Initial State\n");
	  fp=fopen("outfile", "a");
	  fprintf(fp, "Initial State:\n");
  }
  if (iFlag==2) {
	  printf("Final State\n");
	  fp=fopen("outfile", "a");
	  fprintf(fp, "Final State\n");
  }
  
  for (i=0; i<IBOTS; i++) {
        printf(" Num: %i Age: %i\n", i, sPop[i].iAge);
	for (j=0; j< VOWELS; j++) {
		printf("  %s ", sPop[i].sVowel[j].sIPA);

		for (k=0; k < SEGMENTS; k++) {
		    if (sPop[i].sVowel[j].iF[k] > 0.0) {
		 	    sum = sum + ( k * sPop[i].sVowel[j].iF[k]);
			    weight = weight + sPop[i].sVowel[j].iF[k];
			    if (sPop[i].sVowel[j].iF[k] > 2.0) printf("[%i](%.0f) ", MINFREQ+JND * k, sPop[i].sVowel[j].iF[k]);
		    }
		}

		if (weight > 0.0) {
			avg=sum/weight; }
		else {
			avg = 0; }

		for (k=0; k<SEGMENTS; k++) {
			if (sPop[i].sVowel[j].iF[k]>0) {
				sumsq = sumsq + pow((k-avg),2) * sPop[i].sVowel[j].iF[k];
				sdweight= sdweight + sPop[i].sVowel[j].iF[k];
			}
		}
		if (sdweight > 0) sd = sqrt(sumsq/sdweight); else sd=0.0;

		printf("  avg<%.0f> weight<%.0f> SD<%.1f>\n", MINFREQ + JND * avg, weight, sd);
		if (iFlag>0) fprintf(fp, "Bot %i Vowel %s Avg %.0f SD %.0f\n", i, sPop[i].sVowel[j].sIPA, MINFREQ + JND * avg, sd);

		sum=0.0; sdweight=0.0; weight=0.0; sumsq=0.0;
	}
  }
  if (iFlag>0) fprintf(fp, "--\n");
  return 0;
}
/*
 * This function ages the bots and decays their exemplars.
 * Currently it just decreases the weighting by 10% (*0.9) so over time, the weight of an exemplar is 0.9^(t-T) but exponent models are possible, too.
 * Also, bot age doesn't do anything right now, though it certainly could at some later point.
 */
int fAge(struct sBot sPop[])
{

  int i=0, j=0, k=0;
  for (i=0; i<IBOTS; i++) {
	for (j=0; j < VOWELS; j++) {
	  for (k=0; k < SEGMENTS; k++) {
		if (sPop[i].sVowel[j].iF[k] > 0)
		  /* This is the line that decays the exemplar. Currently, it's set to 0.9 */
		  sPop[i].sVowel[j].iF[k] = sPop[i].sVowel[j].iF[k]*AGERATE;
	  }
	}
	sPop[i].iAge++;
  }
  return 0;

}
/*
 * This function does the bulk of the work and manages the exchange of tokens between bots
 */
int fTalk(struct sBot sPop[]) {
	int i=0, k=0, iSpeaker=0, iHearer=0, sum=0, weight=0;
	int iVowel, iVowelFreq, iNewVowelFreq, iNewVowelCat;
	double x;
	int E;
	int iBias = 0;

	/* Currently the exchanges are completely randomized, An exchange consists of a speaker, a hearer and a vowel */
	for (i=0; i<COMBOS; i++) {
		x = (double) rand()/RAND_MAX;
		iSpeaker = IBOTS*x;
		x = (double) rand()/RAND_MAX;
		iHearer = IBOTS*x;
		x = (double) rand()/RAND_MAX;
		iVowel = x * VOWELS;

		printf("Speaker %i saying vowel %s to hearer %i\n", iSpeaker, sPop[iSpeaker].sVowel[iVowel].sIPA, iHearer);
		/* This calles the function that determines the vowel frequency produced by the speaker for the vowel in question */
		iVowelFreq = fProduceVowel(sPop[iSpeaker], iVowel);

		/* Variance is calculated here (instead of in fProduce */
		E = VARIANCE * random_normal();

		/* The iNewVowelFreq is the vowel frequency after including error and articulatory bias subject to MIN/MAX */
		iNewVowelFreq = iVowelFreq + E + iBias;
		if (iNewVowelFreq*JND + MINFREQ < MINFREQ) { 
			iNewVowelFreq = 0;
			printf("    Floor effect\n"); }
		if (iNewVowelFreq*JND + MINFREQ > MAXFREQ) {
			iNewVowelFreq = SEGMENTS;
			printf("    Ceiling effect\n");}

		printf("     %i (%.0f) saying %s to %i at <%i> + E<%i>=<%i>\n", iSpeaker, sPop[iSpeaker].dInfluence, sPop[iHearer].sVowel[iVowel].sIPA, iHearer, iVowelFreq*JND+MINFREQ, JND*E, iNewVowelFreq*JND+MINFREQ);
		/* Now the heaer must categorize the vowel based on its frequency. The output of this function is the vowel number the vowel has 
		 * been categorized as */
		iNewVowelCat = fCategorizeVowel(sPop[iHearer], iNewVowelFreq);

		/* UNCATEGORIZED is the error code for an uncategorizable vowel either because the vowel is out of range or because the weighting is equal for the competing categories*/
		if (iNewVowelCat < UNCATEGORIZED)

			/* This line adds the weight of the token (based on influence of the speaker)  to the hearer's exemplar cloud for the vowel */
			sPop[iHearer].sVowel[iNewVowelCat].iF[iNewVowelFreq] = sPop[iHearer].sVowel[iNewVowelCat].iF[iNewVowelFreq] + sPop[iSpeaker].dInfluence;
		else
			printf("Failed to categorize\n");
		
	}
	return 0;
}
/*
 * This function determine the frequency the speaker produces for a particular vowel.
 * There are two proposed algorithms for doing this based on whether the -E swithci is used on the command line
 * The first picks an exemplar randomly, based on weighting, from all the exemplars of the vowel. This is the default.
 * The second calulcates a weighted average of all the exemplars within a particular window (PROD_WINDOW) of the target.
 * 
 */
int fProduceVowel(struct sBot sSpeaker, int iVowel) {
	int iTarget, weight = 0;
	int LOWERBOUND, UPPERBOUND;
	int iSumProduct=0, iWeight=0;
	int i=0, k=0, y=0, oldk=0;
	double x=0.0;

/*	printf("Bot %i producing Vowel %s\n", sSpeaker.iNum, sSpeaker.sVowel[iVowel].sIPA);*/

	/* First determine the overall weight of all exemplars for the vowel */
	for (k=0; k<SEGMENTS; k++) {
		weight = weight + sSpeaker.sVowel[iVowel].iF[k];
	}

	/* Generate a random numbers */
	x = rand()/((double)RAND_MAX + 1);

	/* This random number determines how far through the weights to go. This acheives a weighted random selection of an exemplar */
	iTarget = x * weight;	
/*	printf("Target %i of total %i\n", iTarget, weight);*/
	k=0;

	/* Cycle through the exemlpars until we're iTarget way through reflecting the random proportion of weight that we're looking for */
	while (y < iTarget) {
		/*
		printf("   %i of %i at %i\n", y, iTarget, k);
		printf("     ...adding %i\n", sSpeaker.sVowel[iVowel].iF[k]);*/
		y=y+sSpeaker.sVowel[iVowel].iF[k];
		k++;
	}

	/* Go back to previous one before we went past iTarget */
	k--;

	/* Alternate method takes the random target then uses an entrenchment method */
	if (TRENCH > 0) {

		/*Make sure the window doesn't spill over the edge of our array */
		if (k-PROD_WINDOW > 0) LOWERBOUND = k-PROD_WINDOW; else LOWERBOUND=0;
		if (k+PROD_WINDOW < ((MAXFREQ-MINFREQ)/JND)) UPPERBOUND = k+PROD_WINDOW; else UPPERBOUND=(MAXFREQ-MINFREQ)/JND;

/*		printf("   Cycling through %i to %i\n", LOWERBOUND, UPPERBOUND);*/
		/* Go through the exemplars within WINDOW of the target and take their weighted average */
		for (i=LOWERBOUND+1; i<=UPPERBOUND; i++) {
/*			printf("     adding %i-%i\n", i, sSpeaker.sVowel[iVowel].iF[i]);*/
			iSumProduct = iSumProduct + sSpeaker.sVowel[iVowel].iF[i] * i;
			iWeight = iWeight + sSpeaker.sVowel[iVowel].iF[i];
		}
		if (iWeight > 0) {
			oldk = k;
			k = iSumProduct / iWeight; }
		else {
			oldk = k;
			k = 0; }
		printf("  Target: %i(%i). Entrenched: %i(%i)\n", oldk*JND+MINFREQ, oldk, k*JND+MINFREQ, k);
	}

/*	printf("Freq of %ith vowel <%i> with was %f\n", k, 200+k*10, x);*/
	return k;
}
int fCategorizeVowel(struct sBot sHearer, int iVowel) {
	int i, k, LOWERBOUND, UPPERBOUND;
	int max_weight =0;
	int weight=0;
	int iVowelCat = UNCATEGORIZED;

	printf("  Bot %i hearing freq of %i [%i]\n", sHearer.iNum, iVowel*JND + MINFREQ, iVowel);

	for (i=0; i<VOWELS; i++) {
		if ((iVowel - PERCEPT_WINDOW) < 0) LOWERBOUND = 0; else LOWERBOUND = iVowel - PERCEPT_WINDOW;
		if ((iVowel + PERCEPT_WINDOW) > (MAXFREQ-MINFREQ)/JND) UPPERBOUND = (MAXFREQ-MINFREQ)/JND; else UPPERBOUND = iVowel + PERCEPT_WINDOW;
		for (k = LOWERBOUND; k<=UPPERBOUND; k++) {
			weight = weight + sHearer.sVowel[i].iF[k];
/*			printf("      Adding weight %i to vowel %i segment %i {%i} = %i\n", sHearer.sVowel[i].iF[k], i, k, k*JND+MINFREQ, weight);*/
		}
		printf("    Comparing %s [%i] weight %i \t", sHearer.sVowel[i].sIPA, i, weight);
		if ((weight == max_weight) && (weight > 0)) {
			iVowelCat = UNCATEGORIZED;
		}
		if (weight > max_weight) {
			iVowelCat = i;
			max_weight = weight;
		}
		weight = 0;
	}
	printf("\n");
	if (iVowelCat < UNCATEGORIZED)
		printf("    %i categorized as %s [%i]\n", iVowel*JND + MINFREQ, sHearer.sVowel[iVowelCat].sIPA, iVowelCat);

	return iVowelCat;
}

double random_uniform_0_1(void) {
	double x;
	x = (double) rand() / (double)(RAND_MAX);
	return (x);
}
double random_normal(void) {
	double U1, U2, V1, V2;
	double S=2;
	while (S>=1) {
		U1 = random_uniform_0_1();
		U2 = random_uniform_0_1();
		V1 = 2.0*U1-1.0;
		V2 = 2.0*U2-1.0;
		S = V1*V1 + V2*V2;
	};
	double X1 = V1*sqrt((-2.0*log(S))/S);
	return X1;
}



