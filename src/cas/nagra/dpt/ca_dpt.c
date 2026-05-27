/**
**  @file ca_dpt.c
**
**  @brief
**   Device Personalization Toolbox (DPT)
**
**  $Id: //CAK/components/dpt/TAGS/DPT_3_8_3/src/com/ca_dpt.c#1 $
**
**  CLASSIFICATION: CONFIDENTIAL
**
*/

#if defined NASC32_TIER0_NO_SCS
#define NASC32
#define NOCS31A
#endif

#if defined NASC32
#define NASC31
#define NOCS32
#endif

#if defined NASC31_TIER0   /* This is just an alias to ensure backward compatibility. When possible, NASC31_TIER0_NO_SCS should be used instead. */
#define NASC31_TIER0_NO_SCS
#endif

#if defined NASC31_TIER0_NO_SCS
#define NASC31
#define NOCS31A
#endif

#if defined NASC31
#define NASC30
#define NOCS30
#define USE_SEC
#endif

#if defined NASC15 || defined NASC30
#define NOCS30
#endif

/******************************************************************************/
/*                                                                            */
/*                           GENERAL INCLUDE FILES                            */
/*                                                                            */
/******************************************************************************/
#include "ca_defs.h"
#include "ca_dpt.h"

#ifdef NOCS30
#include "ca_cert.h"
#endif

#ifdef USE_SEC
#include "ca_sec.h"
#include "nocs_csd.h"
#else
#include "nocs_bsd.h"
#endif

#include <stdio.h>


/******************************************************************************/
/*                                                                            */
/*                              TYPES & CONSTANTS                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    Global variable used to send commands to the CERT IP block.
*/
#ifdef NOCS30
TCertCommand gCertCmd[7];
#endif

#ifdef USE_SEC
#define NAGRA_IPR_NOTICE  \
  "\xA9\x32\x30\x31\x31\x4E\x61\x67\x72\x61\x76\x69\x73\x69\x6F\x6E"
#define NAGRA_IPR_NOTICE_SIZE  16
/**< Size in bytes of NAGRA_IPR_NOTICE */
#endif

/**
 *  @brief
 *    Global variable used as a temporary buffer for Pairing Key encryption in NASC3.0.
*/
#ifdef NASC30
TUnsignedInt8 gBuffer[2048];
#endif

/**
 *  @brief
 *    Global variable used as a log buffer.
*/
TChar gLogBuffer[256];

#define dptGetHexChar(xThis) \
 ((TChar)(xThis) > 9 ? (TChar)('A' + (xThis) - 10) : (TChar)('0' + (xThis)))

#define LOG_NUM_BYTES_PER_LINE 16

#if defined USE_SEC || BSDAPI_VERSION_INT < 0x030000
#define dptPrint(a) printf(a);
#else
#define dptPrint(a) bsdPrint(a);
#endif


/******************************************************************************/
/*                                                                            */
/*                                  PROTOTYPES                                */
/*                                                                            */
/******************************************************************************/

static void dptGetCscDataDescriptor
(
  const TUnsignedInt8* pxCscData,
        TUnsignedInt8 xTag,
        TUnsignedInt8** ppxDescData,
        TUnsignedInt8* pxDescDataLength
);

static void dptGetCscDataDescriptorByKpp
(
  const TUnsignedInt8* pxCscData,
        TUnsignedInt8 xTag,
        TUnsignedInt8 xKpp,
        TUnsignedInt8** ppxDescData,
        TUnsignedInt8* pxDescDataLength
);


#ifdef NOCS30
static int dptGetCuid
(
  TCertFunctionTable*  pxCertFt,
  TCertResourceHandle  pxCertHandle,
  TUnsignedInt8*       pxCuid
);

static TUnsignedInt8 dptGetKeyUsage
(
  TUnsignedInt16 xEmi
);
#endif

#ifdef NASC30
static int dptProcessCertRootLevelKey
(
        TCertFunctionTable*  pxCertFt,
        TCertResourceHandle  pxCertHandle,
  const TUnsignedInt8*       pxCscData,
        TUnsignedInt16        xEmi
);

static int dptCertEncryptData
(
  const TUnsignedInt8*       pxCscData,
        TUnsignedInt16        xEmi,
  const TUnsignedInt8*       pxDataIn,
        TUnsignedInt8*       pxDataOut,
        size_t                xDataSize
);

static int dptCertDecryptData
(
  const TUnsignedInt8*       pxCscData,
        TUnsignedInt16        xEmi,
  const TUnsignedInt8*       pxDataIn,
        TUnsignedInt8*       pxDataOut,
        size_t                xDataSize
);
#endif

#ifndef NOCS31A
static int dptComputeCscDataCheckNumber
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxCheckNumber
);

static int dptVerifyCscDataCn
(
    const TUnsignedInt8* pxCscData
);
#endif

int dptVerifyCscDataCrc
(
  const TUnsignedInt8*  pxCscData
);

#ifdef NOCS30
static int dptVerifyCscDataCertTestCn
(
  const TUnsignedInt8* pxCscData
);
#endif

static void dptInitCrcCcitt
(
  void
);

static void dptInitCrc32
(
  void
);

#if SECAPI_VERSION_INT >= 0x050001 || BSDAPI_VERSION_INT >= 0x030400
static TUnsignedInt8* dptAllocateBuffer
(
  size_t xSize
);

static int dptFreeBuffer
(
  TUnsignedInt8* pxBuffer
);
#endif

void dptPrintArray
(
  const TUnsignedInt8*   pxBuffer,
        TSize             xSize
);

static void dptPrintCertCmd
(
  size_t   xNumCmd
);

#ifdef NOCS30
static int dptCertGetStbCaSn
(
    TCertFunctionTable*  pxCertFt,
    TCertResourceHandle  pxCertHandle,
    TUnsignedInt8*       pxStbCaSn
);
#endif

#ifdef NOCS30
static int dptCertCheckSocConfig
(
    TCertFunctionTable*  pxCertFt,
    TCertResourceHandle  pxCertHandle
);
#endif


/******************************************************************************/
/*                                                                            */
/*                              PRIVATE FUNCTIONS                             */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    Print a byte array buffer in hexadecimal format to the standard output
 *
 *  @param[in] pxBuffer
 *    Buffer to print
 *
 *  @param[in] xSize
 *    Number of bytes to print
 *
*/
void dptPrintArray
(
  const TUnsignedInt8*   pxThis,
        TSize             xSize
)
{
  size_t   numBytes = LOG_NUM_BYTES_PER_LINE;
  size_t   index;
  size_t   jndex;
  size_t   bndex;

  for (index = 0;
       index < xSize;
       index += LOG_NUM_BYTES_PER_LINE)
  {
    bndex=0;
    gLogBuffer[bndex++]='[';
    bndex += sprintf(gLogBuffer + bndex, "%04X", index);
    gLogBuffer[bndex++]=']';
    numBytes = min(LOG_NUM_BYTES_PER_LINE, xSize - index);
    for (jndex = index; jndex < index + numBytes; jndex ++)
    {
      gLogBuffer[bndex++]=' ';
      gLogBuffer[bndex++]=(TChar)dptGetHexChar(pxThis[jndex] / 16);
      gLogBuffer[bndex++]=(TChar)dptGetHexChar(pxThis[jndex] % 16);
    }
    gLogBuffer[bndex++]='\n';
    gLogBuffer[bndex++]=0x00;
    dptPrint(gLogBuffer);
  }
}

/**
 *  @brief
 *    Print a byte array buffer in hexadecimal format without index to the standard output
 *
 *  @param[in] pxBuffer
 *    Buffer to print
 *
 *  @param[in] xSize
 *    Number of bytes to print
 *
*/
void dptPrintArrayRaw
(
  const TUnsignedInt8*   pxThis,
        TSize             xSize
)
{
  size_t   numBytes = LOG_NUM_BYTES_PER_LINE;
  size_t   index;
  size_t   jndex;
  size_t   bndex;

  for (index = 0;
       index < xSize;
       index += LOG_NUM_BYTES_PER_LINE)
  {
    bndex=0;
    numBytes = min(LOG_NUM_BYTES_PER_LINE, xSize - index);
    for (jndex = index; jndex < index + numBytes; jndex ++)
    {
      gLogBuffer[bndex++]=' ';
      gLogBuffer[bndex++]=(TChar)dptGetHexChar(pxThis[jndex] / 16);
      gLogBuffer[bndex++]=(TChar)dptGetHexChar(pxThis[jndex] % 16);
    }
    gLogBuffer[bndex++]='\n';
    gLogBuffer[bndex++]=0x00;
    dptPrint(gLogBuffer);
  }
}

#ifdef NOCS30
static void dptPrintCertCmd
(
  size_t   xNumCmd
)
{
  size_t i;

  for(i=0;i<xNumCmd;i++)
  {
    sprintf(gLogBuffer, "\nCMD %d: %02X %02X %02X %02X\n", i,
      gCertCmd[i].opcodes[0], gCertCmd[i].opcodes[1], gCertCmd[i].opcodes[2], gCertCmd[i].opcodes[3]);
    dptPrint(gLogBuffer);
    sprintf(gLogBuffer, "INPUT %d:\n", i);
    dptPrint(gLogBuffer);
    dptPrintArray(gCertCmd[i].inputData, 32);
    sprintf(gLogBuffer, "STATUS %d: %02X %02X %02X %02X\n", i,
      gCertCmd[i].status[0], gCertCmd[i].status[1], gCertCmd[i].status[2], gCertCmd[i].status[3]);
    dptPrint(gLogBuffer);
    sprintf(gLogBuffer, "OUTPUT %d:\n", i);
    dptPrint(gLogBuffer);
    dptPrintArray(gCertCmd[i].outputData, 32);

  }
}
#endif

/**
 *  @brief
 *    This function retrieves a descriptor, identified by the tag xTag, from
 *    the CSC data area.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[in] xTag
 *    Tag of the descriptor to be found.
 *
 *  @param[out] ppxDescData
 *    Pointer to the payload of the descriptor found within the CSC data buffer.
 *
 *  @param[out] pxDescDataLength
 *    Length in bytes of the payload of the descriptor found within the CSC data
 *    buffer.
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
*/
static void dptGetCscDataDescriptor
(
  const TUnsignedInt8* pxCscData,
        TUnsignedInt8 xTag,
        TUnsignedInt8** ppxDescData,
        TUnsignedInt8* pxDescDataLength
)
{
  TUnsignedInt8* pCurrent;
  TUnsignedInt32 cscDataLength=0;
  TUnsignedInt8 headerLength;
  TUnsignedInt8 tag;
  TUnsignedInt8 dataLength;
  *ppxDescData = NULL;
  *pxDescDataLength = 0;

  /* The descriptor loop starts at 14th byte of the CSC data area */
  pCurrent=(TUnsignedInt8*)(pxCscData + 14);

  /* Compute CSC data length */
  cscDataLength=dptArrayToUnsignedIntN(pxCscData,4)-6;

  while((TUnsignedInt32)(pCurrent - pxCscData) < cscDataLength)
  {
    tag = *pCurrent;
    dataLength = *(pCurrent+1);
    headerLength = 2;
    if(tag == xTag)
    {
      /* Yes, this is the descriptor that I'm looking for. */
      *pxDescDataLength = dataLength;
      *ppxDescData = pCurrent+headerLength;
      break;
    }
    else
    {
      /* This is not the right descriptor, go to the next one. */
      pCurrent = pCurrent + headerLength + dataLength;
    }
  }
}

/**
 *  @brief
 *    This function retrieves a descriptor, identified by the tuple (tag, KPP)
 *    from the CSC data area.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[in] xTag
 *    Tag of the descriptor to be found.
 *
 *  @param[in] xKpp
 *    KPP of the descriptor to be found.
 *
 *  @param[out] ppxDescData
 *    Pointer to the payload of the descriptor found within the CSC data buffer.
 *
 *  @param[out] pxDescDataLength
 *    Length in bytes of the payload of the descriptor found within the CSC data
 *    buffer.
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
*/
static void dptGetCscDataDescriptorByKpp
(
  const TUnsignedInt8* pxCscData,
        TUnsignedInt8 xTag,
        TUnsignedInt8 xKpp,
        TUnsignedInt8** ppxDescData,
        TUnsignedInt8* pxDescDataLength
)
{
  TUnsignedInt8* pCurrent;
  TUnsignedInt32 cscDataLength=0;
  TUnsignedInt8 headerLength;
  TUnsignedInt8 tag;
  TUnsignedInt8 kpp;
  TUnsignedInt8 dataLength;
  *ppxDescData = NULL;
  *pxDescDataLength = 0;

  /* The descriptor loop starts at 14th byte of the CSC data area */
  pCurrent=(TUnsignedInt8*)(pxCscData + 14);

  /* Compute CSC data length */
  cscDataLength=dptArrayToUnsignedIntN(pxCscData,4)-6;

  while((TUnsignedInt32)(pCurrent - pxCscData) < cscDataLength)
  {
    /* KPP only supported by descriptors 0x05, 0x06 and 0x07 */
    if(xTag != 0x05){ break; }

    tag = *pCurrent;
    dataLength = *(pCurrent+1);
    kpp = *(pCurrent+2);
    headerLength = 2;
    if(tag == xTag && kpp == xKpp)
    {
      /* Yes, this is the descriptor that I'm looking for. */
      *pxDescDataLength = dataLength;
      *ppxDescData = pCurrent+headerLength;
      break;
    }
    else
    {
      /* This is not the right descriptor, go to the next one. */
      pCurrent = pCurrent + headerLength + dataLength;
    }
  }
}

/**
 *  @brief
 *    This function retrieves the CERT CUID (aka NUID) programmed during the
 *    personalization of the SoC.
 *
 *  @param[in] pxCertFt
 *    Pointer to the CERTAPI function table
 *
 *  @param[in] pxCertHandle
 *    CERT handle
 *
 *  @param[out] pxCuid
 *    Buffer allocated by the caller where to copy the CERT CUID
 *
 *  @return
 *    0 if operations are successfull, -1 otherwise.
 *
*/
#ifdef NOCS30
static int dptGetCuid
(
  TCertFunctionTable*  pxCertFt,
  TCertResourceHandle  pxCertHandle,
  TUnsignedInt8*       pxCuid
)
{
  TSize            numProcCmd = 0;
  int             retVal=-1;

  /* Initialize */
  memset(gCertCmd, 0x00, sizeof(gCertCmd));

  /* Build transaction */
  gCertCmd[0].opcodes[0] = 0x04;
  gCertCmd[0].opcodes[1] = 0x01;
  gCertCmd[0].opcodes[2] = 0x00;
  gCertCmd[0].opcodes[3] = 0x01;
  gCertCmd[0].timeout = CERT_TIMEOUT_DEFAULT;
  gCertCmd[1].opcodes[0] = 0x04;
  gCertCmd[1].opcodes[1] = 0x04;
  gCertCmd[1].opcodes[2] = 0x00;
  gCertCmd[1].opcodes[3] = 0x01;
  gCertCmd[1].timeout = CERT_TIMEOUT_DEFAULT;

  do
  {
    /* Send transaction  */
    if(CERT_NO_ERROR != pxCertFt->certExchange(pxCertHandle, 2, gCertCmd, &numProcCmd)){break;}

    /* Check status registers are OK */
    if(gCertCmd[0].status[3] & 0x01 || gCertCmd[1].status[3] & 0x01)
    {
      dptPrint("[ERROR] dptGetCuid\n");
      dptPrintCertCmd(2);
      break;
    }

    /* Read 48-bit CUID */
    memcpy(pxCuid, &(gCertCmd[0].outputData[0]), 6);
    retVal=0;
  }
  while(0);

  return (retVal);
}
#endif


/**
 *  @brief
 *    This function retrieves the key usage related to an EMI
 *
 *  @param[in] xEmi
 *    EMI
 *
 *  @return
 *    Key usage related to supported EMI. 0 otherwise.
 *
*/
#ifdef NOCS30
static TUnsignedInt8 dptGetKeyUsage
(
  TUnsignedInt16 xEmi
)
{
  TUnsignedInt8 usage;

  switch(xEmi)
  {
    /* AES-128 */
    case 0x4020:
    case 0x4021:
    case 0x4022:
    case 0x4023:
    case 0x4024:
    case 0x4026:
    case 0x4027:
      usage = 0xD0;
      break;

   /* TDES-128 */
    case 0x4040:
    case 0x4041:
    case 0x4043:
      usage = 0xD1;
      break;

    default:
      usage=0x00;
      break;
  }
  return(usage);
}
#endif

/**
 *  @brief
 *    This function retrieves the KPP related to an EMI for the manufacturer
 *    keyladder (MKL)
 *
 *  @param[in] xEmi
 *    EMI
 *
 *  @return
 *    KPP related to supported EMI. 0 otherwise.
 *
*/
static TUnsignedInt8 dptGetMklKpp
(
  TUnsignedInt16 xEmi
)
{
  TUnsignedInt8 kpp;

  switch(xEmi)
  {
    /* AES-128 */
    case 0x0020:
    case 0x0021:
    case 0x0022:
      kpp = 0x20;
      break;

    /* DVB-CSA2 */
    case 0x0000:
      kpp = 0x2A;
      break;

    /* DVB-CSA3 */
    case 0x0001:
      kpp = 0x29;
      break;


    default:
      kpp=0x00;
      break;

  }
  return(kpp);
}


/**
 *  @brief
 *    This function processes a CERT root level key
 *
 *  @param[in] pxCertFt
 *    Pointer to the CERTAPI function table
 *
 *  @param[in] pxCertHandle
 *    CERT handle
 *
 *  @param[in] pxCscData
 *    Buffer containing CSC data
 *
 *  @param[in] xEmi
 *    Encryption algorithm
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
 *
*/
#ifdef NASC30
static int dptProcessCertRootLevelKey
(
        TCertFunctionTable*  pxCertFt,
        TCertResourceHandle  pxCertHandle,
  const TUnsignedInt8*       pxCscData,
        TUnsignedInt16        xEmi
)
{
  TUnsignedInt8 *pDesc;
  TUnsignedInt8   descLen;
  TUnsignedInt8   alist[17];
  TUnsignedInt8  pCuid[6];
  TUnsignedInt8   keyUsage;
  size_t          numProcCmd = 0;
  size_t          numCmd = 0;
  TBoolean        failed = FALSE;
  int             i;
  int             retVal=-1;

  /* Initialize */
  memset(gCertCmd, 0x00, sizeof(gCertCmd));
  memset(alist, 0x00, sizeof(alist));
  alist[15]=0x04;
  alist[16]=0x09;

  do
  {
    /* Check SoC config bits: SdbgDisable and PersoDone must be set */
    if(0 != dptCertCheckSocConfig(pxCertFt, pxCertHandle)){break;}

    if(NULL==pxCscData){break;}

#ifdef NASC32
    /* Get keycomp descriptor */
    dptGetCscDataDescriptor(pxCscData, 0x31, &pDesc, &descLen);
    if(NULL == pDesc){break;}
#else
    /* Get activation descriptor */
    dptGetCscDataDescriptor(pxCscData, 0xA0, &pDesc, &descLen);
    if(NULL == pDesc){break;}

    /* Read CERT CUID */
    if(dptGetCuid(pxCertFt, pxCertHandle, pCuid)){break;}
#endif
    /* Get key usage */
    if(0 == (keyUsage=dptGetKeyUsage(xEmi))){break;}

    /* Build transaction */
#ifndef NASC32
    memcpy(&(gCertCmd[numCmd].inputData[0]), &pDesc[0], 2);
    memcpy(&(gCertCmd[numCmd].inputData[3]), &pCuid[0], 2);
    memcpy(&(gCertCmd[numCmd].inputData[5]), &pDesc[3], 9);
    memcpy(&(gCertCmd[numCmd].inputData[14]), &pDesc[2], 1);
    memcpy(&(gCertCmd[numCmd].inputData[15]), alist, 17);
    gCertCmd[numCmd].opcodes[0] = 0x0F;
    gCertCmd[numCmd].opcodes[1] = 0x01;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    memcpy(&(gCertCmd[numCmd].inputData[0]), &pCuid[2], 4);
    memcpy(&(gCertCmd[numCmd].inputData[4]), &pCuid[2], 4);
    memcpy(&(gCertCmd[numCmd].inputData[8]), &pCuid[2], 4);
    memcpy(&(gCertCmd[numCmd].inputData[12]), &pCuid[2], 4);
    gCertCmd[numCmd].opcodes[0] = 0x0F;
    gCertCmd[numCmd].opcodes[1] = 0x02;
    gCertCmd[numCmd].opcodes[2] = 0x20;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    memcpy(&(gCertCmd[numCmd].inputData[0]), &pDesc[12], 8);
    gCertCmd[numCmd].opcodes[0] = 0x0F;
    gCertCmd[numCmd].opcodes[1] = 0x04;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
#endif
    memcpy(&(gCertCmd[numCmd].inputData[0]), &pDesc[0], 2);
    memcpy(&(gCertCmd[numCmd].inputData[2]), &keyUsage, 1);
    gCertCmd[numCmd].opcodes[0] = 0x05;
    gCertCmd[numCmd].opcodes[1] = 0x01;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    memcpy(&(gCertCmd[numCmd].inputData[0]), NAGRA_IPR_NOTICE, 16);
    memcpy(&(gCertCmd[numCmd].inputData[16]), NAGRA_IPR_NOTICE, 16);
    gCertCmd[numCmd].opcodes[0] = 0x05;
    gCertCmd[numCmd].opcodes[1] = 0x02;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    memcpy(&(gCertCmd[numCmd].inputData[16]), NAGRA_IPR_NOTICE, 16);
    gCertCmd[numCmd].opcodes[0] = 0x05;
    gCertCmd[numCmd].opcodes[1] = 0x04;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    gCertCmd[numCmd].opcodes[0] = 0x06;
    gCertCmd[numCmd].opcodes[1] = 0x01;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;

    /* Send transaction  */
    if(CERT_NO_ERROR != pxCertFt->certExchange(pxCertHandle, numCmd, gCertCmd, &numProcCmd)){break;}

    /* Check status registers are OK */
    for(i=0;i<(int)numCmd;i++)
    {
      if(gCertCmd[i].status[3] & 0x01)
      {
        if(i==0 || i==1 || i==2)
        {
          dptPrint("[ERROR] dptProcessCertRootLevelKey\n");
          dptPrint("Preliminary processing failed. Check that CSC data are correct and match the chipset\n");
        }
        else if(i==3 || i==4 || i==5)
        {
          dptPrint("[ERROR] dptProcessCertRootLevelKey\n");
          dptPrint("dptProcessCertRootLevelKey: Key processing failed\n");
        }
        else if(i==6)
        {
          dptPrint("[ERROR] dptProcessCertRootLevelKey\n");
          dptPrint("dptProcessCertRootLevelKey: Key output failed\n");
        }
        if(gCertCmd[i].status[3] & 0x02)
        {
          dptPrint("[ERROR] dptProcessCertRootLevelKey\n");
          dptPrint("dptProcessCertRootLevelKey: Key is pending on CERT key interface! bsdUseCertKey or secUseCertKey may have failed\n");
        }
        dptPrintCertCmd(i+1);

        failed=TRUE;
        break;
      }
    }
    if(failed){break;}

    /* Everything is OK */
    retVal=0;
  }
  while(0);
  return(retVal);
}
#endif


/**
 *  @brief
 *    This function makes a RAM2RAM data encryption with a key computed by the
 *    CERT block.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[in] xEmi
 *    Cryptographic algorithm to use for the encryption (AES or TDES)
 *
 *  @param[in] pxDataIn
 *    Buffer containing the data to encrypt. To be allocated by the caller.
 *
 *  @param[out] pxDataOut
 *    Buffer wherer to write encrypted data. To be allocated by the caller.
 *
 *  @param[in] xDataSize
 *    Size in bytes of data (pxDataIn, pxDataOut) to operate on. Must be a
 *    multiple of 16 (AES or TDES).
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
*/
#ifdef NASC30
static int dptCertEncryptData
(
  const TUnsignedInt8*       pxCscData,
        TUnsignedInt16        xEmi,
  const TUnsignedInt8*       pxDataIn,
        TUnsignedInt8*       pxDataOut,
        size_t                xDataSize
)
{
  TCertFunctionTable*  pCertFt;
  TCertResourceHandle  pCertHandle = NULL;
  int                   retVal = -1;
#ifdef USE_SEC
  TSecFunctionTable*   pSecFt;
  TSecCipherSession    pSecSession = NULL;
#else
  TBsdCipherSession    pBsdSession = NULL;
#endif

  do
  {
    if(NULL==pxCscData || NULL==pxDataIn || NULL==pxDataOut){break;}

    /* Get function tables */
#ifdef USE_SEC
    if(NULL == (pSecFt = secGetFunctionTable())){break;}
#endif
    if(NULL == (pCertFt = certGetFunctionTable())){break;}

    /* Encrypt data with CERT key */
#ifdef USE_SEC
    if(SEC_NO_ERROR != pSecFt->secOpenRam2RamEncryptSession(&pSecSession)){break;}
    if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}
    if(0 != dptProcessCertRootLevelKey(pCertFt, pCertHandle, pxCscData, xEmi)){pCertFt->certUnlock(pCertHandle);break;}
  #if SECAPI_VERSION_INT >= 0x050001
    if(SEC_NO_ERROR != pSecFt->secUseCertKey(pSecSession, xEmi, 0, NULL)){pCertFt->certUnlock(pCertHandle);break;}
  #else
    if(SEC_NO_ERROR != pSecFt->secUseCertKey(pSecSession, xEmi, SEC_KEY_PARITY_UNDEFINED)){pCertFt->certUnlock(pCertHandle);break;}
  #endif /* SECAPI_VERSION_INT >= 0x050001 */
    if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}
    if(SEC_NO_ERROR != pSecFt->secSessionEncrypt(
         pSecSession, pxDataIn, pxDataOut, xDataSize, (TUnsignedInt8*)NAGRA_IPR_NOTICE, 16)){break;}
    if(SEC_NO_ERROR != pSecFt->secCloseSession(pSecSession)){break;}
#else
    if(BSD_NO_ERROR != bsdOpenRam2RamEncryptSession(&pBsdSession)){break;}
    if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}
    if(0 != dptProcessCertRootLevelKey(pCertFt, pCertHandle, pxCscData, xEmi)){pCertFt->certUnlock(pCertHandle);break;}
    if(BSD_NO_ERROR != bsdUseCertKey(pBsdSession, xEmi)){pCertFt->certUnlock(pCertHandle);break;}
    if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}
    if(BSD_NO_ERROR != bsdSessionEncrypt(
         pBsdSession, pxDataIn, pxDataOut, xDataSize, (TUnsignedInt8*)NAGRA_IPR_NOTICE, 16)){break;}
    if(BSD_NO_ERROR != bsdCloseSession(pBsdSession)){break;}
#endif /* USE_SEC */
    /* Everything is OK */
    retVal=0;
  }
  while(0);
  return(retVal);
}

/**
 *  @brief
 *    This function makes a RAM2RAM data decryption with a key computed by the
 *    CERT block.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[in] xEmi
 *    Cryptographic algorithm to use for the decryption (AES or TDES)
 *
 *  @param[in] pxDataIn
 *    Buffer containing the data to decrypt. To be allocated by the caller.
 *
 *  @param[out] pxDataOut
 *    Buffer wherer to write decrypted data. To be allocated by the caller.
 *
 *  @param[in] xDataSize
 *    Size in bytes of data (pxDataIn, pxDataOut) to operate on. Must be a
 *    multiple of 16 (AES or TDES).
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
*/
static int dptCertDecryptData
(
  const TUnsignedInt8*       pxCscData,
        TUnsignedInt16        xEmi,
  const TUnsignedInt8*       pxDataIn,
        TUnsignedInt8*       pxDataOut,
        size_t                xDataSize
)
{
  TCertFunctionTable*  pCertFt;
  TCertResourceHandle  pCertHandle = NULL;
  int                   retVal = -1;
#ifdef USE_SEC
  TSecFunctionTable*   pSecFt;
  TSecCipherSession    pSecSession = NULL;
#else
  TBsdCipherSession    pBsdSession = NULL;
#endif

  do
  {
    if(NULL==pxCscData || NULL==pxDataIn || NULL==pxDataOut){break;}

    /* Get function tables */
#ifdef USE_SEC
    if(NULL == (pSecFt = secGetFunctionTable())){break;}
#endif
    if(NULL == (pCertFt = certGetFunctionTable())){break;}

    /* Decrypt data with CERT key */
#ifdef USE_SEC
    if(SEC_NO_ERROR != pSecFt->secOpenRam2RamDecryptSession(&pSecSession)){break;}
    if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}
    if(0 != dptProcessCertRootLevelKey(pCertFt, pCertHandle, pxCscData, xEmi)){pCertFt->certUnlock(pCertHandle);break;}
  #if SECAPI_VERSION_INT >= 0x050001
    if(SEC_NO_ERROR != pSecFt->secUseCertKey(pSecSession, xEmi, 0, NULL)){pCertFt->certUnlock(pCertHandle);break;}
  #else
    if(SEC_NO_ERROR != pSecFt->secUseCertKey(pSecSession, xEmi, SEC_KEY_PARITY_UNDEFINED)){pCertFt->certUnlock(pCertHandle);break;}
  #endif /* SECAPI_VERSION_INT >= 0x050001 */
    if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}
    if(SEC_NO_ERROR != pSecFt->secSessionDecrypt(
         pSecSession, pxDataIn, pxDataOut, xDataSize, (TUnsignedInt8*)NAGRA_IPR_NOTICE, 16)){break;}
    if(SEC_NO_ERROR != pSecFt->secCloseSession(pSecSession)){break;}
#else
    if(BSD_NO_ERROR != bsdOpenRam2RamDecryptSession(&pBsdSession)){break;}
    if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}
    if(0 != dptProcessCertRootLevelKey(pCertFt, pCertHandle, pxCscData, xEmi)){pCertFt->certUnlock(pCertHandle);break;}
    if(BSD_NO_ERROR != bsdUseCertKey(pBsdSession, xEmi)){pCertFt->certUnlock(pCertHandle);break;}
    if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}
    if(BSD_NO_ERROR != bsdSessionDecrypt(
         pBsdSession, pxDataIn, pxDataOut, xDataSize, (TUnsignedInt8*)NAGRA_IPR_NOTICE, 16)){break;}
    if(BSD_NO_ERROR != bsdCloseSession(pBsdSession)){break;}
#endif /* USE_SEC */
    /* Everything is OK */
    retVal=0;
  }
  while(0);
  return(retVal);
}
#endif


#ifndef NOCS31A
/**
 *  @brief
 *    This function computes the CSC data check number through an MKL TDES-ECB
 *    RAM2RAM operation computed over the first 16 bytes of the CSC data.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @param[out] pxCheckNumber
 *    Buffer allocated by the caller containing the computed check number.
 *
 *  @return
 *    0 in case of a successful operation, -1 otherwise.
*/
static int dptComputeCscDataCheckNumber
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxCheckNumber
)
{
  int retVal=-1;

  do
  {
    if(NULL==pxCscData || NULL==pxCheckNumber){break;}


#ifdef USE_SEC
	if (CSD_NO_ERROR != csdGetDataIntegrityCheckNumber(pxCscData, 16, pxCheckNumber)){ break; }
#else
	if (BSD_NO_ERROR != bsdGetDataIntegrityCheckNumber(pxCscData, 16, pxCheckNumber)){ break; }
#endif

    retVal=0;
  }
  while(0);
  return retVal;
}
#endif


/**
 *  @brief
 *    This function verifies the CRC of the CSC data.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @return
 *    0 if both check numbers are equal, -1 otherwise.
 *
*/
int dptVerifyCscDataCrc
(
  const TUnsignedInt8*  pxCscData
)
{
  TUnsignedInt32 len;
  TUnsignedInt8 aCrcComputed[2];
  int retVal=-1;

  do
  {
    if(NULL==pxCscData){break;}

    /* Get the length from the first four bytes */
    len=dptArrayToUnsignedIntN(pxCscData, 4);

    /* Compute CRC16 over CSC data, length included. Substract 2 bytes because
       of the 2-byte crc field located at the end of the record. */
    if(dptCrc16Ccitt(pxCscData, len-2, aCrcComputed)){break;}

    /* Compare computed CRC with CRC stored at the end of the CSC data record */
    if(memcmp(pxCscData+len-2, aCrcComputed, 2)){break;}

    retVal=0;
  }
  while(0);

  if(retVal!=0)
  {
    dptPrint("[ERROR] dptVerifyCscDataCrc\n");
  }
  return(retVal);
}


#ifndef NOCS31A
/**
 *  @brief
 *    This function computes the CSC data check number and compare it to the one
 *    stored in the CSC data.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @return
 *    0 if both check numbers are equal, -1 otherwise.
 *
*/
static int dptVerifyCscDataCn
(
  const TUnsignedInt8* pxCscData
)
{
  TUnsignedInt8  aComputeCheckNumber[4];
  TUnsignedInt8* pCheckNumber;
  TUnsignedInt32 len;
  int retVal=-1;

  do
  {
    if(NULL==pxCscData){break;}

    /* Compute CSC data check number  */
    if(dptComputeCscDataCheckNumber(pxCscData, aComputeCheckNumber)){break;}
    /* Get reference check number from CSC data */
    len=dptArrayToUnsignedIntN(pxCscData, 4);
    pCheckNumber = (TUnsignedInt8*)(pxCscData + len - 6);
    /* Compare them */
    if(memcmp(pCheckNumber, aComputeCheckNumber, 4)){break;}

    /* Success */
    retVal=0;
  }
  while(0);

  if(retVal!=0)
  {
    dptPrint("[ERROR] dptVerifyCscDataCn\n");
  }
  return retVal;
}
#endif

/**
 *  @brief
 *    This function compares the CERT test check number stored in the CSC data
 *    with the test check number computed by the CERT block.
 *
 *  @param[in] pxCscData
 *    Buffer containing the CSC data to be parsed.
 *
 *  @return
 *    0 if both check numbers are equal, -1 otherwise.
 *
*/
#ifdef NOCS30
static int dptVerifyCscDataCertTestCn
(
  const TUnsignedInt8* pxCscData
)
{
  TUnsignedInt8 *pCertCnDesc;
  TUnsignedInt8   certDescLen;
  TUnsignedInt8 aComputedCertTestCn[8];
  int retVal = -1;

  do
  {
    if(NULL==pxCscData){break;}

    dptGetCscDataDescriptor(pxCscData, 0xC0, &pCertCnDesc, &certDescLen);
    if(NULL == pCertCnDesc){break;}

    if(0 != dptGetCertCheckNumber(DPT_CERT_TEST_CHECK_NUMBER, aComputedCertTestCn)){break;}

    /* Compare the 8-byte long test check number computed by the CERT block
    *  with the CSC data CERT test check number that starts at the second byte
    *  of the descriptor playload.
    */
    if(0 != memcmp(aComputedCertTestCn, pCertCnDesc + 1, 8)){break;}

    retVal=0;
  }
  while(0);

  if(retVal!=0)
  {
    dptPrint("[ERROR] dptVerifyCscDataCertTestCn\n");
  }
  return retVal;
}
#endif


static int            gCrcCcittInit = 0;
static unsigned short gCrcCcittTable[256];

/**
 *  @brief
 *    This function is used to fill the table to compute the CRC16-CCITT.
 *
*/
static void dptInitCrcCcitt
(
  void
)
{
  int i, j;
  unsigned short crc, c;

  for (i=0; i<256; i++)
  {
    crc = 0;
    c   = ((unsigned short) i) << 8;

    for (j=0; j<8; j++)
    {
      if ( (crc ^ c) & 0x8000 ) crc = ( crc << 1 ) ^ 0x1021;
      else                      crc =   crc << 1;

      c = c << 1;
    }
    gCrcCcittTable[i] = crc;
  }
  gCrcCcittInit = 1;
}


static int              gCrc32Init = 0;
static unsigned long    gCrc32Table[256];

/**
 *  @brief
 *    This function is used to fill the table to compute the CRC32.
 *
*/
static void dptInitCrc32
(
  void
)
{
  int i, j;
  unsigned long crc;

  for (i=0; i<256; i++)
  {
    crc = (unsigned long) i;

    for (j=0; j<8; j++)
    {
      if ( crc & 0x00000001L ) crc = ( crc >> 1 ) ^ 0xEDB88320L;
      else                     crc =   crc >> 1;
    }
    gCrc32Table[i] = crc;
  }
  gCrc32Init = 1;
}

#if SECAPI_VERSION_INT >= 0x050001 || BSDAPI_VERSION_INT >= 0x030400
static TUnsignedInt8* dptAllocateBuffer
(
  size_t xSize
)
{
  TUnsignedInt8* pBuffer;

  pBuffer=NULL;
  do
  {
    #ifdef USE_SEC
    TSecFunctionTable*   pSecFt;
    if(NULL == (pSecFt=secGetFunctionTable())){break;}
    if(NULL == pSecFt->secAllocateBuffer){break;}
    if(NULL == (pBuffer=pSecFt->secAllocateBuffer(xSize))){break;}
    #else
    if(NULL == (pBuffer=bsdAllocateBuffer(xSize))){break;}
    #endif
  }
  while(0);
  return(pBuffer);
}

static int dptFreeBuffer
(
  TUnsignedInt8* pxBuffer
)
{
  int retVal=-1;
  do
  {
    #ifdef USE_SEC
    TSecFunctionTable*   pSecFt;
    if(NULL == (pSecFt=secGetFunctionTable())){break;}
    if(NULL == pSecFt->secFreeBuffer){break;}
    if(SEC_NO_ERROR != pSecFt->secFreeBuffer(pxBuffer)){break;}
    #else
    if(BSD_NO_ERROR != bsdFreeBuffer(pxBuffer)){break;}
    #endif
    retVal=0;
  }
  while(0);
  return(retVal);
}
#endif

#ifdef NOCS30
static int dptCertGetStbCaSn
(
    TCertFunctionTable*  pxCertFt,
    TCertResourceHandle  pxCertHandle,
    TUnsignedInt8*       pxStbCaSn
)
{
    TSize numProcCmd = 0;
    int   retVal=-1;

    /* Initialize */
    memset(gCertCmd, 0x00, sizeof(gCertCmd));

    /* Build cert GPD transaction */
    gCertCmd[0].opcodes[0] = 0x04;
    gCertCmd[0].opcodes[1] = 0x01;
    gCertCmd[0].opcodes[2] = 0x00;
    gCertCmd[0].opcodes[3] = 0x01;
    gCertCmd[0].timeout = CERT_TIMEOUT_DEFAULT;
    gCertCmd[1].opcodes[0] = 0x04;
    gCertCmd[1].opcodes[1] = 0x04;
    gCertCmd[1].opcodes[2] = 0x00;
    gCertCmd[1].opcodes[3] = 0x01;
    gCertCmd[1].timeout = CERT_TIMEOUT_DEFAULT;

    do
    {
        /* Send transaction  */
        if(CERT_NO_ERROR != pxCertFt->certExchange(pxCertHandle, 2, gCertCmd, &numProcCmd)){break;}

        /* Check status registers are OK */
        if(gCertCmd[0].status[3] & 0x01 || gCertCmd[1].status[3] & 0x01)
        {
            dptPrint("[ERROR] dptCertGetStbCaSn::certExchange failed\n");
            dptPrintCertCmd(2);
            break;
        }

        /* Read 32-bit Stb CA SN */
        memcpy(pxStbCaSn, &(gCertCmd[1].outputData[0]), 4);
        retVal=0;
    }
    while(0);

    return (retVal);
}
#endif

#ifdef NOCS30
static int dptCertCheckSocConfig
(
    TCertFunctionTable*  pxCertFt,
    TCertResourceHandle  pxCertHandle
)
{
    TUnsignedInt8 socConfig;
    TSize         numProcCmd = 0;
    int           retVal=-1;

    /* Initialize */
    memset(gCertCmd, 0x00, sizeof(gCertCmd));

    /* Build cert GPD transaction */
    gCertCmd[0].opcodes[0] = 0x04;
    gCertCmd[0].opcodes[1] = 0x01;
    gCertCmd[0].opcodes[2] = 0x00;
    gCertCmd[0].opcodes[3] = 0x01;
    gCertCmd[0].timeout = CERT_TIMEOUT_DEFAULT;
    gCertCmd[1].opcodes[0] = 0x04;
    gCertCmd[1].opcodes[1] = 0x04;
    gCertCmd[1].opcodes[2] = 0x00;
    gCertCmd[1].opcodes[3] = 0x01;
    gCertCmd[1].timeout = CERT_TIMEOUT_DEFAULT;

    do
    {
        /* Send transaction  */
        if(CERT_NO_ERROR != pxCertFt->certExchange(pxCertHandle, 2, gCertCmd, &numProcCmd)){break;}

        /* Check status registers are OK */
        if(gCertCmd[0].status[3] & 0x01 || gCertCmd[1].status[3] & 0x01)
        {
            dptPrint("[ERROR] dptCertCheckSocConfig::certExchange failed\n");
            dptPrintCertCmd(2);
            break;
        }

        /* Read 8-bit SoC Config */
        socConfig = gCertCmd[1].outputData[4];
        /* SdbgDisable and PersoDone must be set */
        if((socConfig & 0x09) != 0x09)
        {
          break;
        }
        retVal=0;
    }
    while(0);

    return (retVal);
}
#endif


/******************************************************************************/
/*                                                                            */
/*                              PUBLIC FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    Tool to convert a byte array to a value
 *
 *  @param[in] pxArray
 *    Array to convert
 *
 *  @param[in] xSize
 *    Size in bytes of array
 *
 *  @return
 *    Converted value
 *
*/
TUnsignedInt32 dptArrayToUnsignedIntN
(
  const TUnsignedInt8*  pxArray,
        TUnsignedInt8    xSize
)
{
  TUnsignedInt32  result = 0;
  TUnsignedInt8   index;

  for (index = 0; index < xSize; index ++)
  {
    result = 256 * result + pxArray[index];
  }
  return result;
}


/**
 *  @brief
 *    Tool to convert a 32-bit integer  to a byte array
 *
 *  @param[in] xInteger
 *    32-bit integer to convert
 *
 *  @param[out] pxArray
 *    Buffer containing the converted array
 *
 *  @return
 *    0 if success; -1 otherwise
 *
*/
int dptUnsignedInt32ToArray
(
  TUnsignedInt32   xInteger,
  TUnsignedInt8*  pxArray
)
{
  TUnsignedInt8  index;
  TUnsignedInt8  size=4;
  int            status = 0;


  if (pxArray == NULL)
  {
    status = -1;
  }
  else
  {
    for (index = 0; index < size; index ++)
    {
      pxArray[index] = (TUnsignedInt8)(xInteger >> (8* (size -1 -index)));
    }
  }
  return status;
}


/**
 *  @brief
 *    Tool to convert a 16-bit integer  to a byte array
 *
 *  @param[in] xInteger
 *    16-bit integer to convert
 *
 *  @param[out] pxArray
 *    Buffer containing the converted array
 *
 *  @return
 *    0 if success; -1 otherwise
 *
*/
int dptUnsignedInt16ToArray
(
  TUnsignedInt16   xInteger,
  TUnsignedInt8*  pxArray
)
{
  TUnsignedInt8  index;
  TUnsignedInt8  size=2;
  int            status = 0;


  if (pxArray == NULL)
  {
    status = -1;
  }
  else
  {
    for (index = 0; index < size; index ++)
    {
      pxArray[index] = (TUnsignedInt8)(xInteger >> (8* (size -1 -index)));
    }
  }
  return status;
}


int dptCrc16Ccitt
(
  const TUnsignedInt8* pxData,
        size_t          xSize,
        TUnsignedInt8* pxCrc
)
{
  TUnsignedInt16 tmp, short_c;
  TUnsignedInt16 crc = 0;
  int i;
  int retVal=0;

  if ( ! gCrcCcittInit ) dptInitCrcCcitt();

  for (i=0; i<(int)xSize; i++)
  {
    short_c  = 0x00ff & (TUnsignedInt16) pxData[i];
    tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ gCrcCcittTable[tmp];
  }
  retVal=dptUnsignedInt16ToArray(crc, pxCrc);
  return retVal;
}


int dptCrc32
(
  const TUnsignedInt8* pxData,
        size_t          xSize,
        TUnsignedInt8* pxCrc
)
{
  TUnsignedInt32 crc = 0xFFFFFFFFL;
  int i;
  int retVal=0;

  if ( ! gCrc32Init ) dptInitCrc32();

  for (i=0; i<(int)xSize; i++)
  {
    crc = gCrc32Table[ (crc ^ pxData[i]) & 0xff ] ^ (crc >> 8);
  }
  crc = crc ^ 0xFFFFFFFFL;
  retVal=dptUnsignedInt32ToArray(crc, pxCrc);
  return retVal;
}

#ifdef NOCS30
int dptGetCertCheckNumber
(
  TUnsignedInt8   xCheckNumType,
  TUnsignedInt8* pxCertCheckNumber
)
{
  TCertFunctionTable*  pCertFt;
  TCertResourceHandle  pCertHandle = NULL;
  size_t                numCmd = 0;
  TSize                 numProcCmd = 0;
  int                   retVal = -1;

  pCertFt = certGetFunctionTable();
  /* Initialize */
  memset(gCertCmd, 0x00, sizeof(gCertCmd));

#ifdef NOCS32
  if(DPT_CERT_TEST_CHECK_NUMBER == xCheckNumType)
  {
    gCertCmd[numCmd].inputData[3] = 0x00;
    gCertCmd[numCmd].inputData[11] = 0x01;
  }
  else
  {
    gCertCmd[numCmd].inputData[3] = 0x01;
    gCertCmd[numCmd].inputData[11] = 0x01;
  }
  gCertCmd[numCmd].opcodes[0] = 0x0C;
  gCertCmd[numCmd].opcodes[1] = 0x01;
  gCertCmd[numCmd].opcodes[2] = 0x00;
  gCertCmd[numCmd].opcodes[3] = 0x01;
  gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
  gCertCmd[numCmd].opcodes[0] = 0x0C;
  gCertCmd[numCmd].opcodes[1] = 0x04;
  gCertCmd[numCmd].opcodes[2] = 0x00;
  gCertCmd[numCmd].opcodes[3] = 0x01;
  gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
#else
  /* Define the CERT test check number transaction */
  gCertCmd[numCmd].opcodes[0] = 0x08;
  gCertCmd[numCmd].opcodes[1] = 0x01;
  if(DPT_CERT_TEST_CHECK_NUMBER == xCheckNumType)
  {
    gCertCmd[numCmd].opcodes[2] = 0x00;
  }
  else
  {
    gCertCmd[numCmd].opcodes[2] = 0x80;
  }
  gCertCmd[numCmd].opcodes[3] = 0x01;
  gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
#endif

  do
  {
    if(pxCertCheckNumber==NULL){break;}

    /* Lock the CERT resource */
    if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}

    /* Send transaction to retrieve test check number */
    if(CERT_NO_ERROR != pCertFt->certExchange(pCertHandle, numCmd, gCertCmd, &numProcCmd)){pCertFt->certUnlock(pCertHandle);break;}

    /* Unlock the CERT resource */
    if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}

    /* Check the status register is OK */
    if(gCertCmd[0].status[3] & 0x01)
    {
      dptPrint("[ERROR] dptGetCertCheckNumber\n");
      dptPrintCertCmd(1);
      break;
    }

#ifdef NOCS32
    /* The CERT check number is located in the last 8 bytes of the output data. Copy it. */
    memcpy(pxCertCheckNumber, &(gCertCmd[1].outputData[24]), 8);
#else
    /* The CERT check number is located in the first 8 bytes of the output data. Copy it. */
    memcpy(pxCertCheckNumber, &(gCertCmd[0].outputData[0]), 8);
#endif
    retVal = 0;
  }
  while(0);
  return retVal;
}
#endif


#ifdef NASC30
int dptCertEncryptPairingData
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxPairingData
)
{
  int retVal=-1;
  TUnsignedInt32 len;
  TUnsignedInt8 *pBufIn, *pBufOut;

  do
  {
    if(NULL==pxCscData || NULL==pxPairingData){break;}

    len=dptArrayToUnsignedIntN(pxPairingData, 4);

#if SECAPI_VERSION_INT >= 0x050001 || BSDAPI_VERSION_INT >= 0x030400
    if(NULL==(pBufIn=dptAllocateBuffer(len))){break;}
    if(NULL==(pBufOut=dptAllocateBuffer(len))){break;}
    memcpy(pBufIn, pxPairingData+4, len);
    /* Encrypt pairing data with EMI 0x4026 (AES-CBC) */
    if(dptCertEncryptData(pxCscData, 0x4026, pBufIn, pBufOut, len)){break;}
    memcpy(pxPairingData+4, pBufOut, len);
    if(dptFreeBuffer(pBufIn)){break;}
    if(dptFreeBuffer(pBufOut)){break;}
#else
    /* Encrypt pairing data with EMI 0x4026 (AES-CBC) */
    if(dptCertEncryptData(pxCscData, 0x4026, pxPairingData+4, gBuffer, len)){break;}
    memcpy(pxPairingData+4, gBuffer, len);
#endif
    retVal=0;
  }
  while(0);

  if(retVal!=0)
  {
    dptPrint("[ERROR] dptCertEncryptPairingData\n");
  }
  return retVal;
}

int dptCertVerifyEncryptedPairingData
(
  const TUnsignedInt8*  pxCscData,
  const TUnsignedInt8*  pxPairingData,
        TUnsignedInt8*  pxEncryptedPairingData
)
{
  int retVal=-1;
  TUnsignedInt32 lenPK, lenCPK;
  TUnsignedInt8 *pBufIn, *pBufOut;

  do
  {
    if(NULL==pxCscData || NULL==pxPairingData || NULL==pxEncryptedPairingData){break;}

    lenPK=dptArrayToUnsignedIntN(pxPairingData, 4);
    lenCPK=dptArrayToUnsignedIntN(pxEncryptedPairingData, 4);
    if(lenPK!=lenCPK) {break;}

#if SECAPI_VERSION_INT >= 0x050001 || BSDAPI_VERSION_INT >= 0x030400
    if(NULL==(pBufIn=dptAllocateBuffer(lenPK))){break;}
    if(NULL==(pBufOut=dptAllocateBuffer(lenPK))){break;}
    memcpy(pBufIn, pxEncryptedPairingData+4, lenCPK);
    /* Decrypt pairing data with EMI 0x4026 (AES-CBC) */
    if(dptCertDecryptData(pxCscData, 0x4026, pBufIn, pBufOut, lenCPK)){break;}
    memcpy(pxEncryptedPairingData+4, pBufOut, lenCPK);
    if(dptFreeBuffer(pBufIn)){break;}
    if(dptFreeBuffer(pBufOut)){break;}
#else
    /* Decrypt pairing data with EMI 0x4026 (AES-CBC) */
    if(dptCertDecryptData(pxCscData, 0x4026, pxEncryptedPairingData+4, gBuffer, lenCPK)){break;}
    memcpy(pxEncryptedPairingData+4, gBuffer, lenCPK);
#endif

    if(memcmp(pxPairingData, pxEncryptedPairingData, lenPK+4)) {break;}
    retVal=0;
  }
  while(0);

  if(retVal!=0)
  {
    dptPrint("[ERROR] dptCertVerifyEncryptedPairingData\n");
  }
  return retVal;
}
#endif


int dptVerifyCscData
(
  const TUnsignedInt8*  pxCscData
)
{
  int retVal=-1;

  do
  {
    if(NULL==pxCscData){break;}

    /* Verify CRC */
    if(dptVerifyCscDataCrc(pxCscData)){break;}
#ifndef NOCS31A
    /* Verify CSC data check number */
    if(dptVerifyCscDataCn(pxCscData)){break;}
#endif
    /* Verify CERT check number */
#ifdef NOCS30
    if(dptVerifyCscDataCertTestCn(pxCscData)){break;}
#endif
    /* Success */
    retVal=0;
  }
  while(0);

  return(retVal);
}

#ifdef NASC31
int dptNuidCompare
(
    TBoolean *pxResult
)
{
    TUnsignedInt8 nuidBuffer[4] = {0};
    TUnsignedInt8  pCuid[6] = {0};
    TCertFunctionTable*  pCertFt;
    TCertResourceHandle  pCertHandle = NULL;
    int ret = -1;
    *pxResult = FALSE;

    do
    {
        /* NUID */
        if(SEC_NO_ERROR!=secGetNuid(&nuidBuffer))
        {
            dptPrint("[ERROR] dptNuidCompare::secGetNuid\n");
            break;
        }
        dptPrint("NUID from SEC is: ");
        dptPrintArrayRaw(nuidBuffer, 4);

        if(NULL == (pCertFt = certGetFunctionTable()))
        {
            dptPrint("[ERROR] CERT function table is NULL\n");
            break;
        }

        if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle))
        {
            dptPrint("[ERROR] certLock failed.\n");
            break;
        }

        if(0 != dptGetCuid(pCertFt, pCertHandle, pCuid))
        {
            dptPrint("[ERROR] dptGetCuid failed.\n");
        }
        else
        {
            dptPrint("NUID from CERT is: ");
            dptPrintArrayRaw(pCuid, sizeof(pCuid));
            ret = 0;
            if(0 == memcmp(nuidBuffer, &pCuid[2], sizeof(nuidBuffer)))
            {
                dptPrint("NUID(CSD & CERT) is identical.\n");
                *pxResult = TRUE;
            }
            else
            {
                dptPrint("NUID(CSD & CERT) is different.\n");
            }
        }

        if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle))
        {
            dptPrint("[ERROR] certUnlock failed.\n");
            ret = -1;
            break;
        }
    }while(0);
    return ret;
}
#endif

int dptGetStbCaSnString
(
  TChar* pxStbCaSnString
)
{
  TUnsignedInt8 stbCaSnArray[4];
  TUnsignedInt32 stbCaSnInt;
  int retVal=-1;

  do
  {
    if(NULL==pxStbCaSnString){break;}

#ifdef USE_SEC
    if(CSD_NO_ERROR!=csdGetStbCaSn(&(stbCaSnArray[0]))){break;}
#else
    if(BSD_NO_ERROR!=bsdGetStbCaSn(&(stbCaSnArray[0]))){break;}
#endif
    stbCaSnInt=dptArrayToUnsignedIntN(stbCaSnArray, 4);

	  if(sprintf(pxStbCaSnString, "%02d %04d %04d %02d",
      (stbCaSnInt/100000000)%100,
      (stbCaSnInt/10000)%10000,
      stbCaSnInt%10000,
      (((stbCaSnInt/100)%23)+(stbCaSnInt%100))%100) == -1){break;}

    /* Set null terminator */
    pxStbCaSnString[15]=0x00;

    /* Successful operations */
    retVal=0;
  }
  while(0);
return retVal;
}

int dptStbCaSnToString
(
	TUnsignedInt8 * pxStbCaSnArray,
	TChar* pxStbCaSnString
)
{
	TUnsignedInt32 stbCaSnInt;
	int retVal = -1;

	do
	{
		if (NULL == pxStbCaSnString || NULL == pxStbCaSnArray){ break; }

		stbCaSnInt = dptArrayToUnsignedIntN(pxStbCaSnArray, 4);

		if (sprintf(pxStbCaSnString, "%02d %04d %04d %02d",
			(stbCaSnInt / 100000000) % 100,
			(stbCaSnInt / 10000) % 10000,
			stbCaSnInt % 10000,
			(((stbCaSnInt / 100) % 23) + (stbCaSnInt % 100)) % 100) == -1){
			break;
		}

		/* Set null terminator */
		pxStbCaSnString[15] = 0x00;

		/* Successful operations */
		retVal = 0;
	} while (0);
	return retVal;
}

int dptGetPreDescramblingL1ProtectingKey
(
  const TUnsignedInt8*  pxCscData,
        size_t*         pxL1CipheredProtectingKeySize,
        TUnsignedInt8*  pxL1CipheredProtectingKey
)
{
  TUnsignedInt8 *pDesc;
  TUnsignedInt8   descLen;
  int             retVal=-1;

  do
  {
	  if (NULL == pxCscData || NULL == pxL1CipheredProtectingKeySize){ break; }

	  dptGetCscDataDescriptor(pxCscData, 0x01, &pDesc, &descLen);
	  if (NULL != pDesc)
	  {
		  *pxL1CipheredProtectingKeySize = pDesc[0];
		  if (NULL != pxL1CipheredProtectingKey)
		  {
			  memcpy(pxL1CipheredProtectingKey, &pDesc[1], *pxL1CipheredProtectingKeySize);
		  }
		  /* Operation successful. Don't move this statement right after memcpy
		   * because pxL1CipheredProtectingKey is optional */
		  retVal = 0;
	  }
  } while (0);

  return(retVal);
}

int dptGetPreDescramblingL1ProtectingKeyByEmi
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt16   xEmi,
        size_t*         pxL1CipheredProtectingKeySize,
        TUnsignedInt8*  pxL1CipheredProtectingKey
)
{
  TUnsignedInt8 *pDesc;
  TUnsignedInt8   descLen;
  TUnsignedInt8   kpp;
  int             retVal=-1;

  do
  {
	  if (NULL == pxCscData || NULL == pxL1CipheredProtectingKeySize){ break; }
    if(xEmi == 0x0000)
    {
      /* Search DVB-CSA2 key with descriptor tag 0x01 */
      dptGetCscDataDescriptor(pxCscData, 0x01, &pDesc, &descLen);
      if (NULL != pDesc)
      {
        *pxL1CipheredProtectingKeySize = pDesc[0];
        if (NULL != pxL1CipheredProtectingKey)
        {
          memcpy(pxL1CipheredProtectingKey, &pDesc[1], *pxL1CipheredProtectingKeySize);
        }
        /* Operation successful. Don't move this statement right after memcpy
         * because pxL1CipheredProtectingKey may be NULL to get the size */
        retVal = 0;
      }
    }
    if(retVal!=0)
    {
      /* Search key with descriptor tag 0x05 */
      kpp=dptGetMklKpp(xEmi);
      if(kpp == 0){ break; }
      dptGetCscDataDescriptorByKpp(pxCscData, 0x05, kpp, &pDesc, &descLen);
      if (NULL != pDesc)
      {
        *pxL1CipheredProtectingKeySize = pDesc[2];
        if (NULL != pxL1CipheredProtectingKey)
        {
          memcpy(pxL1CipheredProtectingKey, &pDesc[3], *pxL1CipheredProtectingKeySize);
        }
        /* Operation successful. Don't move this statement right after memcpy
         * because pxL1CipheredProtectingKey may be NULL to get the size */
        retVal = 0;
      }
    }
  } while (0);

  return(retVal);
}


int dptGetCscDataConfiguration
(
    const TUnsignedInt8*  pxCscData,
    TUnsignedInt8*        pxCscDataConfiguration
)
{
    TUnsignedInt8  *pDesc;
    TUnsignedInt8   descLen;
    int             retVal=-1;

    do
    {
        if(NULL==pxCscData)
        {
            printf("null csc data pointer.\n");
            break;
        }

        /* Get configuration descriptor */
        dptGetCscDataDescriptor(pxCscData, 0x09, &pDesc, &descLen);
        if(NULL == pDesc)
        {
            printf("descriptor with tag 0x09 not found in csc.\n");
            break;
        }

        if(4 != descLen)
        {
            printf("invalid length %u of descriptor with tag 0x09. should be 4-bytes long\n", descLen);
            break;
        }

        memcpy(pxCscDataConfiguration, pDesc, descLen);

        /* Everything is OK */
        retVal=0;
    }
    while(0);
    return(retVal);
}

#ifdef NASC30
int dptVerifyStbCaSnPairing(const TUnsignedInt8*  pxPairingData)
{
    int retVal=-1;
    TUnsignedInt8 stbCaSnFromCsd[4];

    do
    {
        if(NULL==pxPairingData){break;}

        /* get stb ca sn via csd/bsd  */
#ifdef USE_SEC
        if(CSD_NO_ERROR!=csdGetStbCaSn(stbCaSnFromCsd)){break;}
#else
        if(BSD_NO_ERROR!=bsdGetStbCaSn(stbCaSnFromCsd)){break;}
#endif
        if(memcmp(&pxPairingData[8], stbCaSnFromCsd, 4))
        {
            dptPrint("[ERROR] dptVerifyStbCaSnPairing:: stb ca sn from csd is different.\n");
            break;
        }

        /* Success */
        retVal=0;
    }
    while(0);

    return(retVal);
}

int dptVerifyStbCaSnPairingCert(const TUnsignedInt8*  pxPairingData)
{
    int retVal=-1;
    TUnsignedInt8 stbCaSnFromCert[4];
    TCertFunctionTable*  pCertFt;
    TCertResourceHandle  pCertHandle = NULL;

    do
    {
        if(NULL==pxPairingData){break;}

        /*get stb ca sn via cert*/
        if(NULL == (pCertFt = certGetFunctionTable()))
        {
            dptPrint("[ERROR] dptVerifyStbCaSnPairingCert::CERT function table is NULL\n");
            break;
        }

        if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle))
        {
            dptPrint("[ERROR] dptVerifyStbCaSnPairingCert::certLock failed.\n");
            break;
        }

        if(0 != dptCertGetStbCaSn(pCertFt, pCertHandle, stbCaSnFromCert))
        {
            dptPrint("[ERROR] dptVerifyStbCaSnPairingCert::dptCertGetStbCaSn failed.\n");
            pCertFt->certUnlock(pCertHandle);
            break;
        }
        if(memcmp(&pxPairingData[8], stbCaSnFromCert, 4))
        {
            dptPrint("[ERROR] dptVerifyStbCaSnPairingCert:: stb ca sn from cert is different.\n");
            pCertFt->certUnlock(pCertHandle);
            break;
        }

        if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle))
        {
            dptPrint("[ERROR] dptVerifyStbCaSnPairingCert::certUnlock failed.\n");
            break;
        }

        /* Success */
        retVal=0;
    }
    while(0);

    return(retVal);
}
#endif


#ifdef NASC32
int dptGetDeviceManufacturerId
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxDeviceManufacturerId
)
{
  TUnsignedInt8 *pDesc;
  TUnsignedInt8   descLen;
  int retVal = -1;

  do
  {
    if(NULL==pxCscData){break;}

    dptGetCscDataDescriptor(pxCscData, 0x30, &pDesc, &descLen);
    if(NULL == pDesc){break;}

    memcpy(pxDeviceManufacturerId, pDesc + 24 + 1, 3);
    pxDeviceManufacturerId[3]=0;
    retVal=0;
  }
  while(0);

  if(retVal!=0)
  {
    dptPrint("[ERROR] dptGetDeviceManufacturerId\n");
  }
  return retVal;
}
#endif


#ifdef NASC32
int dptGetDeviceModelId
(
  const TUnsignedInt8*  pxCscData,
        TUnsignedInt8*  pxDeviceModelId
)
{
  TUnsignedInt8 *pDesc;
  TUnsignedInt8   descLen;
  int retVal = -1;

  do
  {
    if(NULL==pxCscData){break;}

    dptGetCscDataDescriptor(pxCscData, 0x30, &pDesc, &descLen);
    if(NULL == pDesc){break;}

    memcpy(pxDeviceModelId, pDesc + 28 + 5, 3);
    pxDeviceModelId[3]=0;
    retVal=0;
  }
  while(0);

  if(retVal!=0)
  {
    dptPrint("[ERROR] dptGetDeviceModelId\n");
  }
  return retVal;
}
#endif


#ifdef NASC32
#if CERTAPI_VERSION_INT >= 0x010400
typedef struct
{
  TBoolean        isCert1Segmented;
  TUnsignedInt8   cert1SegUid[2];
  TBoolean        isCert3Segmented;
  TUnsignedInt8   cert3SegUid[2];
} TDptCertSegInfo;

static int dptGetCertSegInfo
(
  TDptCertSegInfo*  pxSegInfo
)
{
  TCertFunctionTable*  pCertFt;
  TCertResourceHandle  pCertHandle = NULL;
  size_t                numCmd;
  TSize                 numProcCmd = 0;
  int retVal = -1;

  while(1)
  {
    pxSegInfo->isCert1Segmented = FALSE;
    memset(pxSegInfo->cert1SegUid,0x00, 2);
    pxSegInfo->isCert3Segmented = FALSE;
    memset(pxSegInfo->cert3SegUid,0x00, 2);

    if(NULL == (pCertFt = certGetFunctionTable())){break;}

    numCmd = 0;
    gCertCmd[numCmd].opcodes[0] = 0x04;
    gCertCmd[numCmd].opcodes[1] = 0x01;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    gCertCmd[numCmd].opcodes[0] = 0x04;
    gCertCmd[numCmd].opcodes[1] = 0x04;
    gCertCmd[numCmd].opcodes[2] = 0x00;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    gCertCmd[numCmd].opcodes[0] = 0x04;
    gCertCmd[numCmd].opcodes[1] = 0x01;
    gCertCmd[numCmd].opcodes[2] = 0x02;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    gCertCmd[numCmd].opcodes[0] = 0x04;
    gCertCmd[numCmd].opcodes[1] = 0x04;
    gCertCmd[numCmd].opcodes[2] = 0x02;
    gCertCmd[numCmd].opcodes[3] = 0x01;
    gCertCmd[numCmd++].timeout = CERT_TIMEOUT_DEFAULT;
    if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}
    if(CERT_NO_ERROR != pCertFt->certExchange(pCertHandle, numCmd, gCertCmd, &numProcCmd)){pCertFt->certUnlock(pCertHandle);break;}
    if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}
    if(gCertCmd[0].status[3] & 0x01 ||
       gCertCmd[1].status[3] & 0x01 ||
       gCertCmd[2].status[3] & 0x01 ||
       gCertCmd[3].status[3] & 0x01)
    {
      dptPrint("[ERROR] dptGetCertSegInfo: GPD command failed!\n");
      dptPrintCertCmd(4);
      break;
    }

    pxSegInfo->isCert1Segmented = ((gCertCmd[0].status[2] & 0x01) == 0x01);
    memcpy(pxSegInfo->cert1SegUid, &gCertCmd[0].outputData[12], 2);

    pxSegInfo->isCert3Segmented = ((gCertCmd[2].outputData[7] & 0x04) == 0x04);
    memcpy(pxSegInfo->cert3SegUid, &gCertCmd[2].outputData[0], 2);

    retVal = 0;
    break;
  }
  return(retVal);
}

int dptProgramCertSegment
(
  const TUnsignedInt8*  pxCscData
)
{
  TUnsignedInt8 *pDesc1;
  TUnsignedInt8 *pDesc3;
  TUnsignedInt8   descLen;
  TCertFunctionTable*  pCertFt;
  TCertResourceHandle  pCertHandle = NULL;
  size_t                numCmd;
  TSize                 numProcCmd = 0;
  TDptCertSegInfo       certSegInfo;
  int retVal = -1;

  do
  {
    if(NULL==pxCscData){break;}
    pCertFt = certGetFunctionTable();
    if(NULL == pCertFt){break;}
    if(NULL == pCertFt->certAklReset)
    {
      dptPrint("[ERROR] certResetAkl() not implemented!\n");
      break;
    }

    /* Get CERT segmentation descriptors */
    dptGetCscDataDescriptor(pxCscData, 0xB1, &pDesc1, &descLen);
    if(NULL == pDesc1)
    {
      dptPrint("[ERROR] cert3_seg_legacy_descriptor (0xB1) not found in CSCD!\n");
      break;
    }
    dptGetCscDataDescriptor(pxCscData, 0xB3, &pDesc3, &descLen);
    if(NULL == pDesc3)
    {
      dptPrint("[ERROR] cert3_seg_descriptor (0xB3) not found in CSCD!\n");
      break;
    }

    /* Get CERT segmentation info before programming */
    if(dptGetCertSegInfo(&certSegInfo)){break;}

    /* CERT1 prog seg command */
    if(!certSegInfo.isCert1Segmented)
    {
      numCmd = 0;
      memset(gCertCmd, 0x00, sizeof(gCertCmd));
      memcpy(gCertCmd[numCmd].inputData, &pDesc1[4], 24);
      gCertCmd[numCmd].opcodes[0] = 0x03;
      gCertCmd[numCmd].opcodes[1] = 0x01;
      gCertCmd[numCmd].opcodes[2] = 0x00;
      gCertCmd[numCmd].opcodes[3] = 0x01;
      gCertCmd[numCmd++].timeout = CERT_TIMEOUT_OTP;
      if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}
      if(CERT_NO_ERROR != pCertFt->certExchange(pCertHandle, numCmd, gCertCmd, &numProcCmd)){pCertFt->certUnlock(pCertHandle);break;}
      if(CERT_NO_ERROR != pCertFt->certAklReset()){pCertFt->certUnlock(pCertHandle);break;}
      if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}
      if(gCertCmd[0].status[3] & 0x01)
      {
        dptPrint("[ERROR] dptProgramCertSegment: CERT1 PROGSEG command failed!\n");
        dptPrintCertCmd(1);
        break;
      }
    }

    /* CERT3 prog seg command */
    if(!certSegInfo.isCert3Segmented)
    {
      numCmd = 0;
      memset(gCertCmd, 0x00, sizeof(gCertCmd));
      memcpy(gCertCmd[numCmd].inputData, &pDesc3[4], 24);
      gCertCmd[numCmd].opcodes[0] = 0x1A;
      gCertCmd[numCmd].opcodes[1] = 0x01;
      gCertCmd[numCmd].opcodes[2] = 0x00;
      gCertCmd[numCmd].opcodes[3] = 0x01;
      gCertCmd[numCmd++].timeout = CERT_TIMEOUT_OTP;
      if(CERT_NO_ERROR != pCertFt->certLock(&pCertHandle)){break;}
      if(CERT_NO_ERROR != pCertFt->certExchange(pCertHandle, numCmd, gCertCmd, &numProcCmd)){pCertFt->certUnlock(pCertHandle);break;}
      if(CERT_NO_ERROR != pCertFt->certAklReset()){pCertFt->certUnlock(pCertHandle);break;}
      if(CERT_NO_ERROR != pCertFt->certUnlock(pCertHandle)){break;}
      if(gCertCmd[0].status[3] & 0x01)
      {
        dptPrint("[ERROR] dptProgramCertSegment: CERT3 PROGSEG command failed!\n");
        dptPrintCertCmd(1);
        break;
      }
    }

    /* Get CERT segmentation info after programming */
    if(dptGetCertSegInfo(&certSegInfo)){break;}

    /* Check CERT1 SEG */
    if(!certSegInfo.isCert1Segmented)
    {
      dptPrint("[ERROR] dptProgramCertSegment: CERT1 SEG is not programmed (not locked)!\n");
      break;
    }
    if(memcmp(certSegInfo.cert1SegUid, &pDesc1[2], 2) != 0)
    {
      dptPrint("[ERROR] dptProgramCertSegment: CERT1 SEGUID does not match!\n");
      break;
    }

    /* Check CERT3 SEG */
    if(!certSegInfo.isCert3Segmented)
    {
      dptPrint("[ERROR] dptProgramCertSegment: CERT3 SEG is not programmed (not locked)!\n");
      break;
    }
    if(memcmp(certSegInfo.cert3SegUid, &pDesc3[2], 2) != 0)
    {
      dptPrint("[ERROR] dptProgramCertSegment: CERT3 SEGUID does not match!\n");
      break;
    }

    retVal=0;
  }
  while(0);

  return retVal;
}
#endif /* CERTAPI_VERSION_INT >= 0x010400 */
#endif /* NASC32 */
