/*
 * Handle bitcoin mining pool comms without a json library as it is not needed for fixed order & format messages
 * todo: submit result
 */
/*
 * This is a snippet from my miner program under development, no guarantees on correctness!
 * nb: hardly any error/bounds checking (todo?)
 * expects global variables & defines:
 * #define miningaddr "yourminingaddress"
 * // buffer for json from pool
 * char decjsnbuf[3000];
 * // for mining.subscribe
 * char p_Jobid[20];     // Job id string
 * char p_xnonce1[10];   // Extra nonce 1
 * char p_xnonce2sz[2];  // Extra nonce 2 size
 * // for mining.set_difficulty
 * char p_ShareDif[8];   // Share difficulty ("10000", so left justified?)
 * char p_sessionid[4];    //
 * // for mining.notify
 * char p_prevblockhash[66]; // hash of previous block header
 * char p_coinb1[128];
 * char p_coinb2[400];
 * char p_partialmerkle[64*14+2]; // partial merkle tree, enough room for ~2^14 transactions/block
 *                                // no seperator between hashes, new hash starts every 64 characters
 * char p_version[10];
 * char p_nbits[10];
 * char p_ntime[10];
 * char p_clean[8];
*/
#define mining_subscribe "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": []}\n"
#define parprnt(a) { CDCSER.print(#a); CDCSER.print(":"); CDCSER.println(a); }
char *jp,*bp;

void saveitem()
{
  char *p = bp;
  
  while(*jp != '"' && *jp != ',' && *jp != ']' && *jp != ':' && *jp != '}')
    *bp++ = *jp++;
  *bp++ = '\0';
  if(*jp == '"') jp++; // skip "
  if(*jp == ',') jp++; // skip ,
  Serial.println(p);
}

bool readitem()
{
  bool endd = false;
  char c = *jp;

  switch(c)
  {
    case ':':
    case '{':
      jp++;
      break;
    case '"':
      jp++;
      saveitem();
      break;
    case '[':
      *bp++ = c;
      *bp++ = '\0';
      CDCSER.println("[");
      jp++;
      break;
    case ']':
      *bp++ = c;
      *bp++ = '\0';
      CDCSER.println("]");
      jp++;
      if(*jp == ',') jp++;
      break;
    case '}':
      *bp++ = '\0';
      endd = true;
      break;
    default:
      saveitem();
  }
  return endd;
}

// converts string in poolbf[] to array of lines in jp[][], every key/value/[/] gets its own string. Does not include some characters from source (:{}")
void jsonstringify(char *poolbf)
{
  jp = poolbf;
  bp = decjsnbuf;
  while(!readitem()) ;
  int i = (int)bp-(int)decjsnbuf;
  CDCSER.print("Stringify length ");
  CDCSER.println(i);
}

// skip i lines, p points to character in first line to be skipped. Returns pointer to first char in line after skipping
char *jsnskiplines(char *p,int i)
{
  while(i && *p) p++;
  p++;
  i--;
  if (i) p = jsnskiplines(p,i);
  return p;
}
char *pppppppppppppppjsnskiplines(char *p,int i)
{
  while(i)
  {
    while(*p) p++;
    p++;
    i--;
  }
  return p;
}

#define timeout 80000

int poolread(uint8_t *b) // read line from pool, return 1 for timeout, 2 for invalid data
{
  long t;
  int i = 0;
  char tmout = 0;

  t = millis()+timeout;
  while(!poolclient.available()&& !tmout)
  {
     if(millis()>t) tmout = 1;
  }
  if (!tmout)
  {
    do {
      b[i++] = poolclient.read();
      if(millis()>t) tmout = 1;
    } while (i<3000 && b[i-1] != '\n' && !tmout);
    if(tmout || i == 2000) tmout = 2;
  }
  else
  {
    CDCSER.println("No response from pool");
  }
  if(!tmout) b[i-1] = 0;

  return tmout;
}
/*
 *  Handle incomming pool message
 *  Called whenever poolclient.available() == true in loop()
 */

int pool_message()
{
  uint8_t pool_data_rcv[4000];
  char method[20];
  char *p,*pm;
  int i,rv;
  
  CDCSER.println("Handling incomming pool message");
  rv = poolread(pool_data_rcv);
  delay(0);
  CDCSER.print("Received: ");
  CDCSER.println((char *)pool_data_rcv);
  jsonstringify((char *)pool_data_rcv);
  CDCSER.println("Looking for method");
  p = decjsnbuf;
  delay(0);
  while(strcmp(p,"method") && *p) { CDCSER.print("Skip: "); CDCSER.println(p);  p = jsnskiplines(p,1); }
  if(!*p) { CDCSER.println("Invalid message from pool"); return 1;} // no valid message, discard
  p = jsnskiplines(p,1);
  strcpy(method,p);
  CDCSER.print("Method found: ");
  CDCSER.println(method);
  delay(0);

  if(!strcmp(method,"mining.set_difficulty"))
  {
    CDCSER.println("Handling mining.set_difficulty");
    p = decjsnbuf;
    p = jsnskiplines(p,2);
    strcpy(p_ShareDif,p);
    parprnt(p_ShareDif);
  }
  if(!strcmp(method,"mining.notify"))
  {
    // received mining parameters, decode & copy
    CDCSER.println("Handling mining.notify");
    p = decjsnbuf;
    //delay(100);
    
    p = jsnskiplines(p,2);
    strcpy(p_Jobid,p);
    parprnt(p_Jobid);
    p = jsnskiplines(p,1);
    strcpy(p_prevblockhash,p);
    parprnt(p_prevblockhash);
    p = jsnskiplines(p,1);
    strcpy(p_coinb1,p);
    parprnt(p_coinb1);
    p = jsnskiplines(p,1);
    strcpy(p_coinb2,p);
    parprnt(p_coinb2);
    p = jsnskiplines(p,2);
    pm = p_partialmerkle;
    delay(0);
    while(strcmp(p,"]"))
    {
      strcpy(pm,p);
      pm += 64;
      p = jsnskiplines(p,1);
    }
    *pm = 0;
    parprnt(p_partialmerkle);
    p = jsnskiplines(p,1);
    strcpy(p_version,p);
    parprnt(p_version);
    p = jsnskiplines(p,1);
    strcpy(p_nbits,p);
    parprnt(p_nbits);
    p = jsnskiplines(p,1);
    strcpy(p_ntime,p);
    parprnt(p_ntime);
    p = jsnskiplines(p,1);
    strcpy(p_clean,p);
    parprnt(p_clean);
  }
  return 0;
}

/*  Connect to pool
 *  assumes open connection to pool in poolclient, eg
 *  WiFiClient poolclient;
 *  poolclient.connect(POOL_URL, POOL_PORT);
 *  Called in setup()
 */
int poolConnect()
{
  uint8_t pool_data_rcv[3000];
  char *p;
  int i,rv;
  
  i = 0;
  // send mining.subscribe
  CDCSER.println("Sending subscribe:");
  CDCSER.println(mining_subscribe);
  poolclient.print(mining_subscribe);
  delay(10);
  // line 1
  rv = poolread(pool_data_rcv);
  if(rv) return rv;
  CDCSER.print("Received(1): ");
  CDCSER.println((char *)pool_data_rcv);
  jsonstringify((char *)pool_data_rcv);
  p = decjsnbuf;
  p = jsnskiplines(p,5);
  strcpy(p_Jobid,p);
  p = jsnskiplines(p,3);
  strcpy(p_xnonce1,p);
  p = jsnskiplines(p,1);
  strcpy(p_xnonce2sz,p);
  p = jsnskiplines(p,3);
  strcpy(p_sessionid,p);
  CDCSER.println("DATA:");
  CDCSER.println(p_Jobid);
  CDCSER.println(p_xnonce1);
  CDCSER.println(p_xnonce2sz);
  CDCSER.println(p_sessionid);
  // line 2
  rv = poolread(pool_data_rcv);
  if(rv) return rv;
  CDCSER.print("Received(2): ");
  CDCSER.println((char *)pool_data_rcv);
  jsonstringify((char *)pool_data_rcv);
  p = decjsnbuf;
  p = jsnskiplines(p,2);
  strcpy(p_ShareDif,p);
  CDCSER.println(p_ShareDif);

  // send mining.authorize
  char sendstr[100];

  sendstr[0] = 0;
  strcat(sendstr,"{\"params\": [\"");
  strcat(sendstr,miningaddr);
  strcat(sendstr,"\", \"password\"], \"id\": ");
  strcat(sendstr,p_sessionid);
  strcat(sendstr,", \"method\": \"mining.authorize\"}\n");
  CDCSER.println(sendstr);
  poolclient.print(sendstr);

  return 0;
}
