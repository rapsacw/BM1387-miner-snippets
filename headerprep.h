/*
 * Prepare block header from pool data
 * increments ntime & xnonce2 for new jobs
 * expects global uint8_t m_header[80];
 * and
//              pool data, all ascii
// from mining.subscribe
char p_Jobid[20];     // Job id string
char p_xnonce1[10];   // Extra nonce 1
char p_xnonce2sz[4];  // Extra nonce 2 size
// from mining.set_difficulty
char p_ShareDif[8];   // Share difficulty ("10000", so left justified?)
char p_sessionid[4];    //
// from mining.notify
char p_prevblockhash[66]; // hash of previous block header
char p_coinb1[128];
char p_coinb2[400];
char p_partialmerkle[64*14+2]; // partial merkle tree
char p_version[10];
char p_nbits[10];
char p_ntime[10];
char p_clean[8];
char p_coinbase[600];   // assembled coinbase transaction

//              Mining parameters (all binary)
int m_TicketDif;
float m_TargetHashrate;     // Set hashrate in GH/s
float m_tpm = 4.0;          // Target # tickets per minute (not yet implemented)
int m_ShareDif;

uint8_t m_version[4];
uint8_t m_prevblockhash[32];
uint8_t m_merkle[14][32];
int     m_nmerkle;
char  *m_xnonce2;           // pointer to xnonce2 position in p_coinbase
uint8_t m_nbits[4];
uint8_t m_ntime[4];
long    m_Tntime;
uint8_t m_header[80];
uint8_t m_havework = 0;   // flag to signal work from pool available
uint8_t m_mining = 0;     // flag to signal mining from pool
// Asic parameters
typedef struct
{
  uint16_t TargetFreq;
  uint16_t RealFreq;
  float    TargetHashrate;
  long     ActiveDuration;
  int      ValidNonces;
} ASIC;
int     a_nAsics;
ASIC    a_asic[MAX_ASICS];
// timers
long rt,mi,st;
// system
#define LittleEndian 0
#define BigEndian 1
uint8_t s_endian;
*/

int zeroes_4 = 0;
#define zeroes_8 "00000000"


// ascii<>bin conversions, NOTE: all ascii has to be LOWER CASE!! or you'll never get a valid share!!
char hexily(unsigned char d) // return hex representation of 1 nibble
{
  if(d<=9) return '0'+d;
  return 'a'+d-10;
}

uint8_t dehexily(char c) // return bin nibble
{
  if(c<'a') return c-'a'+10;
  else return c - '0';
}

void strtobin(uint8_t *bin, char *str, int binlen)
{
  int i;

  for(i=0;i<binlen;i++)
  {
    bin[i] = dehexily(str[i*2]<<4) | dehexily(str[i*2+1]);
  }
}

void bintostr(uint8_t *bin, char *str, int binlen)
{
  int i;
  
  for(i=0;i<binlen;i++)
  {
    str[i*2] = hexily(bin[i]>>4);
    str[i*2+1] = hexily(bin[i]&&0xf);
  }
}

// increment an 8 char hex string, overflows to zeroes
void inc_8chr(char *p)
{
  int i;

  i = 7;
  do
  {
    if(p[i]<'a')
    {
      if(p[i] < '9')
      {
        p[i]++;
        break;
      }
      else
      {
        p[i] = 'a';
        break;
      }
    }
    else
    {
      if(p[i] < 'f')
      {
        p[i]++;
        break;
      }
      else
      {
        p[i] = '0';
        i--;
      }
    }
  } while (i>=0);
}

void inc_4bin(uint8_t *p) // increment 4 byte integer the hard way (for endian independence), overflows to zeroes
{
  int i = 3;
  while(i >= 0)
  {
    if(p[i]<=254)
    {
      p[i]++;
      break;
    }
    // else ..
    p[i] = 0;
    i--;
  }
}

uint8_t nibble2bin(char nib)
{
  if((nib>='0') && (nib <='9')) return nib-'0';
  if((nib>='a') && (nib <= 'f')) return nib-'a'+10;
  if((nib>='A') && (nib <= 'F')) return nib-'A'+10; //!! should never happen !!
}

int hexstr2bin(char *str, uint8_t *buf)
{
  int i;

  i = 0;
  while(str[i])
  {
    buf[i/2] = (nibble2bin(str[i]) << 4) | nibble2bin(str[i+1]);
    i += 2;
  }
  return i/2;
}

void memcpy_reverse(void *dst, void *src,int len)
{
  int i;
  for(i=0;i<len;i++)
  {
    ((uint8_t *)dst)[i] = ((uint8_t *)src)[len-i-1];
  }
}

/*
 * Construct 80-byte header from (ascii-to-binned) pool data and assembled (ascii) coinbase transaction
 */
void Header_construct()
{
  uint8_t sha_coinmerkle[32]; // will hold coinbase hash & merkle hash
  sha256_ctx ctx;
  int i;
  
  // double hash coinbase
  sha256_init(&ctx);
  sha256_update(&ctx, (const unsigned char *)p_coinbase, strlen(p_coinbase));
  sha256_final(&ctx, sha_coinmerkle);
  sha256_init(&ctx);
  sha256_update(&ctx, sha_coinmerkle, 32);
  sha256_final(&ctx, sha_coinmerkle);
  // double hash merkle
  for(i=0;i<m_nmerkle;i++)
  {
    sha256_init(&ctx);
    sha256_update(&ctx, sha_coinmerkle, 32);
    sha256_update(&ctx, m_merkle[i], 32);
    sha256_final(&ctx, sha_coinmerkle);
    sha256_init(&ctx);
    sha256_update(&ctx, sha_coinmerkle, 32);
    sha256_final(&ctx, sha_coinmerkle);
  }
  // Construct binary header
  memcpy_reverse(m_header,&m_version,4);
  memcpy(&m_header[4],m_prevblockhash,32);
  memcpy(&m_header[36],sha_coinmerkle,32);
  memcpy_reverse(&m_header[68],m_ntime,4);
  memcpy_reverse(&m_header[72],m_nbits,4);
  memcpy(&m_header[76],&zeroes_4,4);
  //CDCSER.println("header:");
  //showresponse(m_header,80);
  //CDCSER.println();
}
/*
 * Create new job by incrementing ntime (binary) or xnonce2 (in situ ascii)
 * calls Header_construct to create a new binary block header
 */
void Header_nextjob()
{
  uint8_t sha_coinmerkle[32]; // will hold coinbase hash & merkle hash
  sha256_ctx ctx;
  int i;

  // increment ntime(binary) or xnonce2(ascii)
  // incrementing ntime is not needed really with 'low' hashrates.
  if(labs(millis()-m_Tntime)>1000) // roll nTime, should only happen upto 60x according to 'the internet',
  {                                // but most pools send new work in less than 60 seconds, so no need to limit it here ..
    inc_4bin(m_ntime); // increment nTime
    strcpy(m_xnonce2,zeroes_8); // and reset xnonce2
    m_Tntime = millis(); // time of last change of ntime
  }
  else
  {
    inc_8chr(m_xnonce2); // inc. xnonce2
  }
  Header_construct();
}

/*
 * Convert pool data to binary
 * assembles coinbase transaction
 * calls Header_construct to create a new binary block header
 */
void header_makebin()
{
  int i,len;
  char coinb[65];
  uint8_t sha_coinmerkle[32]; // will hold coinbase hash & merkle hash
  sha256_ctx ctx;

  // convert version to bin
  hexstr2bin(p_version, m_version);
  // convert prevblockhash to bin
  hexstr2bin(p_prevblockhash, m_prevblockhash);
  // convert nBits to bin
  hexstr2bin(p_nbits, m_nbits);
  // convert ntime to bin
  hexstr2bin(p_ntime, m_ntime);
  // convert all merkle entries to bin
  i = 0;
  do
  {
    strtobin(m_merkle[i], &p_partialmerkle[i*64], 32);
    i++;
  } while(p_partialmerkle[i*64]);
  m_nmerkle = i; // keep #merkle entries
  // assemble coinbase transaction
  strcpy(p_coinbase,p_coinb1);
  strcat(p_coinbase,p_xnonce1);
  m_xnonce2 = &p_coinbase[strlen(p_coinbase)];
  strcat(p_coinbase,zeroes_8);
  strcat(p_coinbase,p_coinb2);

  m_Tntime = millis();
  Header_construct();
}
