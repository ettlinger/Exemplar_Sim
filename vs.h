#include <math.h>
#define MINFREQ 100
#define MAXFREQ 1100
#define JND 1
#define AGERATE 0.99
#define UNCATEGORIZED 9999
int SEGMENTS = (MAXFREQ-MINFREQ)/JND; /* just noticable difference in Hz */
int IBOTS = 4; /* Number of speaker/hearers */
int CYCLES = 20; /* Number of cycles to iterate */
int VOWELS = 3; /* Number of initial vowels in the system */
int COMBOS = 5; /* Number of toekn exchanges during each cycle */
int VARIANCE = 40; /* Multiplied by normal distribution SD=1 to get variance in segments */
int PERCEPT_WINDOW = 50; /* (in segments) Window within which exemplars are activated to assess categorization */
int PROD_WINDOW = 50; /* (in segments) Window within which exemplars are activated to assess categorization */
int TRENCH = 0; /* If 1, use the Entrenchment model for production (see P 2001) */

struct sVowel {
  double iF[(MAXFREQ-MINFREQ)/JND+1];
  char sIPA[5];
/*  int iF[100][100];*/
};
struct sBot {
  int iNum;
  int iAge;
  struct sVowel sVowel[10];
  double dInfluence;
} sPop[10];
int fLoadBots(struct sBot[], char *);
int fPrintBots(struct sBot[], int);
int fAge(struct sBot[]);
int fTalk(struct sBot[]);
int fProduceVowel(struct sBot, int);
int fCategorizeVowel(struct sBot, int);
double random_normal (void);
double random_uniform_0_1(void);
