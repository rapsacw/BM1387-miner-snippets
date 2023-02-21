// BM1387 Routine to set any frequency in x-MHz steps in the range of 50..799MHz
// Not all frequencies are possible, so impossible frequencies are replaced with the nearest possible frequency,
// with a preference for a lower frequency.
// Also register solutions using a lower multiplier are preferred over solutions with higher multiplier
// (The pll max. frequency is 3200MHz, things can get toasty at that frequency i guess)
//
// Copyright: anyone is free to do anything with this
//
// Usage
// Change the define for FREQ_STEP to suit your desired resolution, note that 750/FREQ_STEP needs to be integer.
// The uint16_t ftable[750/FREQ_STEP] contains the register settings, you'll need this in your program
// Maketable(): call this (once during initialisation) before using the next function;
// BM1387Frequency(int freq): set the frequency for all asics. You will have to include a command to calculate
// the checksum and serial write to the asics. Returns true if succesfull.
//
// all lines containing 'Serial' or 'sprintf' are for debugging, remove if not needed.
//
// Theory of operation
// A BM1387 frequency command looks like
// {0x58, 0x09, 0x00, 0x0C, 0x00, 0xMM, 0x02, 0xdD, 0x00};
// where MM is a 7 bit multiplier (multiplies the crystal frequency of 25MHz), MM needs to be >16.
// D is a  3 bit divider
// d is also a 3 bit divider
// The dividers divide by d and D(, a value of 0 is interpreted as 7?).
// This results in a frequency of M/(d*D), finally this resulting frequency get divided by 2 (MM/(d*D*2)).
// Maketable() tries all combinations of MM/D/d, and saves all register combinations or 
// nearest matches for all frequencies in FREQ_STEP MHz steps in ftable[]

#define FREQ_STEP 5       // Choose step size so that 750/STEP_SIZE is an integer
#define MIN_FREQ 50       // do not change!
#define MAX_FREQ 799      // don't go over 799

uint16_t ftable[(MAX_FREQ-50)/FREQ_STEP]; // Parameters 5 & 7 for BM1387 set frequency command

int BM1387Frequency(int freq)
{
  uint8_t frcommand[] = {0x58, 0x09, 0x00, 0x0C, 0x00, 0x50, 0x02, 0x41, 0x00};
  uint16_t regs;

  if(freq < MIN_FREQ || freq>MAX_FREQ) { Serial.println("Invalid frequency, not set."); return 0; }
  Serial.print("Requested frequency: ");
  Serial.print(freq);
  regs = ftable[(freq-50)/FREQ_STEP];
  frcommand[5] = regs>>8;
  frcommand[7] = regs&0xff;
  CDCSER.print("MHz, set frequency: ");
  float rf = frcommand[5] * 25.0;
  rf /= (frcommand[7]>>4);
  rf /= (frcommand[7]&0x7);
  rf /= 2;
  Serial.print((int)rf);
  Serial.println("MHz");

  /*
    // Insert routine here that creates the crc5 checksum for the command and sends it to the asics, like
    sendcrc5(frcommand,9);
    delay(10);
  */
  return 1;
}

// recursive routine (too lazy for loops) to find nearest/lower frequency if exact match isn't possible
int findnearest(uint16_t *m,int i,int offset)
{
  int rv;

  rv = 0;
  if((i+offset)<=MAX_FREQ)
  {
    if(m[i+offset]) rv = i+offset;
  }
  if((i-offset)>=MIN_FREQ)
  {
    if(m[i-offset]) rv = i-offset; // overwrites higher solution if present
  }
  if(rv == 0) rv = findnearest(m,i,offset+1); // nothing found yet, search wider
  return rv;
}

void maketable()
{
  uint16_t mult[MAX_FREQ-MIN_FREQ];
  int i,m,d,e;

  delay(1000);
  Serial.println("Starting..");
  for(i=0;i<(MAX_FREQ-MIN_FREQ);i++) mult[i] = 0;
  // find register entries for all possible (integer)frequencies from MIN_FREQ .. MAX_FREQ MHz
  Serial.println("All found frequency settings:");
  for(m=17;m<128;m+=1)
  {
    for(d=1;d<7;d++)
    {
      for(e=1;e<7;e++)
      {
        float f;

        f = (m*25.0); // multiplier
        f /= (float)d; // divider 1
        f /= (float)e; // divider 2
        f /= 2;
        if((f >= MIN_FREQ) && (f < MAX_FREQ))
        {
          int freq = (int)(f+0.5);
          freq -= MIN_FREQ;
          if(!mult[freq]) // keep entry with lowest multiplier for energy efficiency
          {
            mult[freq] = m << 8;
            mult[freq] |= d<< 4;
            mult[freq] |= e;
          }
        }
      }
    }
  }
  // show all found frequency settings, entire 'for()'block can be deleted
  for(i=0;i<MAX_FREQ;i++)
  {
    if(mult[i]!=0)
    {
      char str[10];
      sprintf(str,"%04x",mult[i]);
      Serial.print(i+MIN_FREQ); Serial.print("MHz: Parameter 5_7= ");
      Serial.println(str);
    }
  }
  // now compress results to a table with FREQ_STEP MHz resolution
  Serial.println();
  Serial.println("Frequency table in steps of ");
  Serial.print()FREQ_STEP);
  Serial.println(" MHz:");
  for(i=0;i<((MAX_FREQ-MIN_FREQ)/FREQ_STEP);i++)
  {
    uint16_t value;
    char str[5];

    Serial.print(i*FREQ_STEP+MIN_FREQ); 
    Serial.print("MHz: ");
    if(mult[i*FREQ_STEP] == 0)
    {
      int j;
      Serial.print("(No direct solution for ");
      Serial.print(MIN_FREQ+i*FREQ_STEP);
      Serial.print("MHz, using settings for ");
      j = findnearest(mult,i*FREQ_STEP,1);
      Serial.print(MIN_FREQ+j);
      Serial.print("MHz) ");
      value = mult[j];
    }
    else value = mult[i*FREQ_STEP];
    ftable[i] = value;
    Serial.print("Parameters 5 & 7: ");
    sprintf(str,"%04x",ftable[i]);
    Serial.println(str);
  }
}

void setup() {
  Serial.begin(115200);

  maketable();
}

void loop() {
  // put your main code here, to run repeatedly:

}
