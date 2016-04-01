/***************************************************************************
 * This code is based on public domain Szymon Stefanek AES implementation: *
 * http://www.pragmaware.net/software/rijndael/index.php                   *
 *                                                                         *
 * Dynamic tables generation is based on the Brian Gladman work:           *
 * http://fp.gladman.plus.com/cryptography_technology/rijndael             *
 ***************************************************************************/
#include "rar.hpp"

#ifdef USE_SSE
#include <wmmintrin.h>
#endif

static byte S[256],S5[256],rcon[30];
static byte T1[256][4],T2[256][4],T3[256][4],T4[256][4];
static byte T5[256][4],T6[256][4],T7[256][4],T8[256][4];
static byte U1[256][4],U2[256][4],U3[256][4],U4[256][4];


inline void Xor128(void *dest,const void *arg1,const void *arg2)
{
#if defined(PRESENT_INT32) && defined(ALLOW_MISALIGNED)
  ((uint32*)dest)[0]=((uint32*)arg1)[0]^((uint32*)arg2)[0];
  ((uint32*)dest)[1]=((uint32*)arg1)[1]^((uint32*)arg2)[1];
  ((uint32*)dest)[2]=((uint32*)arg1)[2]^((uint32*)arg2)[2];
  ((uint32*)dest)[3]=((uint32*)arg1)[3]^((uint32*)arg2)[3];
#else
  for (int I=0;I<16;I++)
    ((byte*)dest)[I]=((byte*)arg1)[I]^((byte*)arg2)[I];
#endif
}


inline void Xor128(byte *dest,const byte *arg1,const byte *arg2,
                   const byte *arg3,const byte *arg4)
{
#if defined(PRESENT_INT32) && defined(ALLOW_MISALIGNED)
  (*(uint32*)dest)=(*(uint32*)arg1)^(*(uint32*)arg2)^(*(uint32*)arg3)^(*(uint32*)arg4);
#else
  for (int I=0;I<4;I++)
    dest[I]=arg1[I]^arg2[I]^arg3[I]^arg4[I];
#endif
}


inline void Copy128(byte *dest,const byte *src)
{
#if defined(PRESENT_INT32) && defined(ALLOW_MISALIGNED)
  ((uint32*)dest)[0]=((uint32*)src)[0];
  ((uint32*)dest)[1]=((uint32*)src)[1];
  ((uint32*)dest)[2]=((uint32*)src)[2];
  ((uint32*)dest)[3]=((uint32*)src)[3];
#else
  for (int I=0;I<16;I++)
    dest[I]=src[I];
#endif
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// API
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Rijndael::Rijndael()
{
  if (S[0]==0)
    GenerateTables();
  CBCMode = true; // Always true for RAR.
}


void Rijndael::Init(bool Encrypt,const byte *key,uint keyLen,const byte * initVector)
{
#ifdef USE_SSE
  // Check SSE here instead of constructor, so if object is a part of some
  // structure memset'ed before use, this variable is not lost.
  int CPUInfo[4];
  __cpuid(CPUInfo, 1);
  AES_NI=(CPUInfo[2] & 0x2000000)!=0;
#endif

  uint uKeyLenInBytes;
  switch(keyLen)
  {
    case 128:
      uKeyLenInBytes = 16;
      m_uRounds = 10;
      break;
    case 192:
      uKeyLenInBytes = 24;
      m_uRounds = 12;
      break;
    case 256:
      uKeyLenInBytes = 32;
      m_uRounds = 14;
      break;
  }

  byte keyMatrix[_MAX_KEY_COLUMNS][4];

  for(uint i = 0; i < uKeyLenInBytes; i++)
    keyMatrix[i >> 2][i & 3] = key[i];

  if (initVector==NULL)
    memset(m_initVector, 0, sizeof(m_initVector));
  else
    for(int i = 0; i < MAX_IV_SIZE; i++)
      m_initVector[i] = initVector[i];

  keySched(keyMatrix);

  if(!Encrypt)
    keyEncToDec();
}



void Rijndael::blockDecrypt(const byte *input, size_t inputLen, byte *outBuffer)
{
  if (inputLen <= 0)
    return;

  size_t numBlocks=inputLen/16;
#ifdef USE_SSE
  if (AES_NI)
  {
    blockDecryptSSE(input,numBlocks,outBuffer);
    return;
  }
#endif

  byte block[16], iv[4][4];
  memcpy(iv,m_initVector,16);

  for (size_t i = numBlocks; i > 0; i--)
  {
    byte temp[4][4];

    Xor128(temp,input,m_expandedKey[m_uRounds]);

    Xor128(block,   T5[temp[0][0]],T6[temp[3][1]],T7[temp[2][2]],T8[temp[1][3]]);
    Xor128(block+4, T5[temp[1][0]],T6[temp[0][1]],T7[temp[3][2]],T8[temp[2][3]]);
    Xor128(block+8, T5[temp[2][0]],T6[temp[1][1]],T7[temp[0][2]],T8[temp[3][3]]);
    Xor128(block+12,T5[temp[3][0]],T6[temp[2][1]],T7[temp[1][2]],T8[temp[0][3]]);

    for(int r = m_uRounds-1; r > 1; r--)
    {
      Xor128(temp,block,m_expandedKey[r]);
      Xor128(block,   T5[temp[0][0]],T6[temp[3][1]],T7[temp[2][2]],T8[temp[1][3]]);
      Xor128(block+4, T5[temp[1][0]],T6[temp[0][1]],T7[temp[3][2]],T8[temp[2][3]]);
      Xor128(block+8, T5[temp[2][0]],T6[temp[1][1]],T7[temp[0][2]],T8[temp[3][3]]);
      Xor128(block+12,T5[temp[3][0]],T6[temp[2][1]],T7[temp[1][2]],T8[temp[0][3]]);
    }

    Xor128(temp,block,m_expandedKey[1]);
    block[ 0] = S5[temp[0][0]];
    block[ 1] = S5[temp[3][1]];
    block[ 2] = S5[temp[2][2]];
    block[ 3] = S5[temp[1][3]];
    block[ 4] = S5[temp[1][0]];
    block[ 5] = S5[temp[0][1]];
    block[ 6] = S5[temp[3][2]];
    block[ 7] = S5[temp[2][3]];
    block[ 8] = S5[temp[2][0]];
    block[ 9] = S5[temp[1][1]];
    block[10] = S5[temp[0][2]];
    block[11] = S5[temp[3][3]];
    block[12] = S5[temp[3][0]];
    block[13] = S5[temp[2][1]];
    block[14] = S5[temp[1][2]];
    block[15] = S5[temp[0][3]];
    Xor128(block,block,m_expandedKey[0]);

    if (CBCMode)
      Xor128(block,block,iv);

    Copy128((byte*)iv,input);
    Copy128(outBuffer,block);

    input += 16;
    outBuffer += 16;
  }

  memcpy(m_initVector,iv,16);
}


#ifdef USE_SSE
void Rijndael::blockDecryptSSE(const byte *input, size_t numBlocks, byte *outBuffer)
{
  __m128i initVector = _mm_loadu_si128((__m128i*)m_initVector);
  __m128i *src=(__m128i*)input;
  __m128i *dest=(__m128i*)outBuffer;
  __m128i *rkey=(__m128i*)m_expandedKey;
  while (numBlocks > 0)
  {
    __m128i rl = _mm_loadu_si128(rkey + m_uRounds);
    __m128i d = _mm_loadu_si128(src++);
    __m128i v = _mm_xor_si128(rl, d);

    for (int i=m_uRounds-1; i>0; i--)
    {
      __m128i ri = _mm_loadu_si128(rkey + i);
      v = _mm_aesdec_si128(v, ri);
    }

    __m128i r0 = _mm_loadu_si128(rkey);
    v = _mm_aesdeclast_si128(v, r0);

    if (CBCMode)
      v = _mm_xor_si128(v, initVector);
    initVector = d;
    _mm_storeu_si128(dest++,v);
    numBlocks--;
  }
  _mm_storeu_si128((__m128i*)m_initVector,initVector);
}
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ALGORITHM
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Rijndael::keySched(byte key[_MAX_KEY_COLUMNS][4])
{
  int j,rconpointer = 0;

  // Calculate the necessary round keys
  // The number of calculations depends on keyBits and blockBits
  int uKeyColumns = m_uRounds - 6;

  byte tempKey[_MAX_KEY_COLUMNS][4];

  // Copy the input key to the temporary key matrix

  memcpy(tempKey,key,sizeof(tempKey));

  int r = 0;
  int t = 0;

  // copy values into round key array
  for(j = 0;(j < uKeyColumns) && (r <= m_uRounds); )
  {
    for(;(j < uKeyColumns) && (t < 4); j++, t++)
      for (int k=0;k<4;k++)
        m_expandedKey[r][t][k]=tempKey[j][k];

    if(t == 4)
    {
      r++;
      t = 0;
    }
  }

  while(r <= m_uRounds)
  {
    tempKey[0][0] ^= S[tempKey[uKeyColumns-1][1]];
    tempKey[0][1] ^= S[tempKey[uKeyColumns-1][2]];
    tempKey[0][2] ^= S[tempKey[uKeyColumns-1][3]];
    tempKey[0][3] ^= S[tempKey[uKeyColumns-1][0]];
    tempKey[0][0] ^= rcon[rconpointer++];

    if (uKeyColumns != 8)
      for(j = 1; j < uKeyColumns; j++)
        for (int k=0;k<4;k++)
          tempKey[j][k] ^= tempKey[j-1][k];
    else
    {
      for(j = 1; j < uKeyColumns/2; j++)
        for (int k=0;k<4;k++)
          tempKey[j][k] ^= tempKey[j-1][k];

      tempKey[uKeyColumns/2][0] ^= S[tempKey[uKeyColumns/2 - 1][0]];
      tempKey[uKeyColumns/2][1] ^= S[tempKey[uKeyColumns/2 - 1][1]];
      tempKey[uKeyColumns/2][2] ^= S[tempKey[uKeyColumns/2 - 1][2]];
      tempKey[uKeyColumns/2][3] ^= S[tempKey[uKeyColumns/2 - 1][3]];
      for(j = uKeyColumns/2 + 1; j < uKeyColumns; j++)
        for (int k=0;k<4;k++)
          tempKey[j][k] ^= tempKey[j-1][k];
    }
    for(j = 0; (j < uKeyColumns) && (r <= m_uRounds); )
    {
      for(; (j < uKeyColumns) && (t < 4); j++, t++)
        for (int k=0;k<4;k++)
          m_expandedKey[r][t][k] = tempKey[j][k];
      if(t == 4)
      {
        r++;
        t = 0;
      }
    }
  }
}

void Rijndael::keyEncToDec()
{
  for(int r = 1; r < m_uRounds; r++)
  {
    byte n_expandedKey[4][4];
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++)
      {
        byte *w=m_expandedKey[r][j];
        n_expandedKey[j][i]=U1[w[0]][i]^U2[w[1]][i]^U3[w[2]][i]^U4[w[3]][i];
      }
    memcpy(m_expandedKey[r],n_expandedKey,sizeof(m_expandedKey[0]));
  }
}


#define ff_poly 0x011b
#define ff_hi   0x80

#define FFinv(x)    ((x) ? pow[255 - log[x]]: 0)

#define FFmul02(x) (x ? pow[log[x] + 0x19] : 0)
#define FFmul03(x) (x ? pow[log[x] + 0x01] : 0)
#define FFmul09(x) (x ? pow[log[x] + 0xc7] : 0)
#define FFmul0b(x) (x ? pow[log[x] + 0x68] : 0)
#define FFmul0d(x) (x ? pow[log[x] + 0xee] : 0)
#define FFmul0e(x) (x ? pow[log[x] + 0xdf] : 0)
#define fwd_affine(x) \
    (w = (uint)x, w ^= (w<<1)^(w<<2)^(w<<3)^(w<<4), (byte)(0x63^(w^(w>>8))))

#define inv_affine(x) \
    (w = (uint)x, w = (w<<1)^(w<<3)^(w<<6), (byte)(0x05^(w^(w>>8))))

void Rijndael::GenerateTables()
{
  unsigned char pow[512],log[256];
  int i = 0, w = 1;
  do
  {
    pow[i] = (byte)w;
    pow[i + 255] = (byte)w;
    log[w] = (byte)i++;
    w ^=  (w << 1) ^ (w & ff_hi ? ff_poly : 0);
  } while (w != 1);

  for (int i = 0,w = 1; i < sizeof(rcon)/sizeof(rcon[0]); i++)
  {
    rcon[i] = w;
    w = (w << 1) ^ (w & ff_hi ? ff_poly : 0);
  }
  for(int i = 0; i < 256; ++i)
  {
    unsigned char b=S[i]=fwd_affine(FFinv((byte)i));
    T1[i][1]=T1[i][2]=T2[i][2]=T2[i][3]=T3[i][0]=T3[i][3]=T4[i][0]=T4[i][1]=b;
    T1[i][0]=T2[i][1]=T3[i][2]=T4[i][3]=FFmul02(b);
    T1[i][3]=T2[i][0]=T3[i][1]=T4[i][2]=FFmul03(b);
    S5[i] = b = FFinv(inv_affine((byte)i));
    U1[b][3]=U2[b][0]=U3[b][1]=U4[b][2]=T5[i][3]=T6[i][0]=T7[i][1]=T8[i][2]=FFmul0b(b);
    U1[b][1]=U2[b][2]=U3[b][3]=U4[b][0]=T5[i][1]=T6[i][2]=T7[i][3]=T8[i][0]=FFmul09(b);
    U1[b][2]=U2[b][3]=U3[b][0]=U4[b][1]=T5[i][2]=T6[i][3]=T7[i][0]=T8[i][1]=FFmul0d(b);
    U1[b][0]=U2[b][1]=U3[b][2]=U4[b][3]=T5[i][0]=T6[i][1]=T7[i][2]=T8[i][3]=FFmul0e(b);
  }
}


#if 0
// Test Removed
#endif
