#ifndef _CA_SEC_UTIL_H_
#define _CA_SEC_UTIL_H_

#include <ca_sec.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/*                                                                            */
/*                              TYPES DEFINITIONS                             */
/*                                                                            */
/******************************************************************************/

#define SEC_128BIT_KEY_SIZE 16
#define SEC_64BIT_KEY_SIZE 8

enum SEC_CRYPTO_TYPE {
	SEC_DECRYPT,
	SEC_ENCRYPT,
};

/**
 *  @brief
 *    The key buffer management for the SEC API.
*/
typedef struct SEC_KEY_BUF
{
	struct {
		unsigned char xPL2[SEC_128BIT_KEY_SIZE]; 
		/**< Identifier for the protecting key that encrypted by root key */
		unsigned char xPL1[SEC_128BIT_KEY_SIZE]; 
		/**< Identifier for the protecting key that encrypted by PL2 */
		unsigned char xOddKey[SEC_128BIT_KEY_SIZE]; 
		/**< Identifier for the ODD key that encrypted by PL1 */
		unsigned char xEvenKey[SEC_128BIT_KEY_SIZE]; 
		/**< Identifier for the EVEN key that encrypted by PL1 */
	} xProtectingKey;

	struct {
		unsigned char xOddKey[SEC_128BIT_KEY_SIZE];
		/**< Identifier for the ODD clear key text */
		unsigned char xEvenKey[SEC_128BIT_KEY_SIZE];
		/**< Identifier for the EVEN clear key text */
	} xKey;

	struct {
		unsigned char xOddIV[SEC_128BIT_KEY_SIZE];
		/**< Identifier for the Initialization Vector of ODD Key*/
		unsigned char xEvenIV[SEC_128BIT_KEY_SIZE];
		/**< Identifier for the Initialization Vector of EVEN Key*/
	} xIV; 
} SEC_KEY_BUF;

/**
 *  @brief
 *    The usage of the transport session ID(TSID), such as play AV stream or PVR record.
*/
typedef enum
{
	TSID_FOR_LIVE = 0,
	/**< Indicates the TSID is for the play AV stream usage.
	 *	 By default, the SEC API would consider the TSID to be this usage.
	 *   BTW. If in the PVR Playback scene, we also need to use in this usage. 
	 */
	TSID_FOR_PVR,
	/**< Indicates the TSID if for the PVR Record usage.
	 */
} TSecTSIDUsage;

/******************************************************************************/
/*                                                                            */
/*                               SEC FUNCTION UTILS                           */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    Get the Crypto FD from the SEC by the TSID and crypto type. 
 *
 *  @param[in]  xTSID
 *    The Transport Session ID
 *
 *  @param[in]  crypto_type
 *    The crypto type specifies the encryption or decryption.
 *
 *  @retval  The Crypto FD
 *    The operation was completed successfully.
 *
 *  @retval  SEC_INVALID_VALUE
 *    Errors occurred and the operation could not be completed successfully.
*/
int secGetCryptoFdFromSessID(TTransportSessionId xTSID, 
	enum SEC_CRYPTO_TYPE crypto_type);

/**
 *  @brief
 *    Get the SEC Session from the SEC by the TSID and crypto type. 
 *
 *  @param[in]  xTSID
 *    The Transport Session ID
 *
 *  @param[in]  crypto_type
 *    The crypto type specifies the encryption or decryption.
 *
 *  @retval  The SEC session structure.
 *    The operation was completed successfully.
 *
 *  @retval  NULL
 *    Errors occurred and the operation could not be completed successfully.
*/
TSecCipherSession secSessionGetByID(TTransportSessionId xTSID, 
	enum SEC_CRYPTO_TYPE crypto_type);

/**
 *  @brief
 *    Get the EMI information from the SEC by session. 
 *
 *  @param[in]  xSession
 *    The SEC Session.
 *
 *  @retval  EMI.
 *    The operation was completed successfully.
 *
 *  @retval  1
 *    Errors occurred and the operation could not be completed successfully.
*/
int secSessionGetEmi(TSecCipherSession xSession);

/**
 *  @brief
 *    Set the PID list into the SEC Session. 
 *
 *  @param[in] xSessionID
 *    The SEC Session.
 *
 *  @param[in] crypto_type
 *    The crypto type specifies the encryption or decryption.
 *
 *  @param[in] xPidList
 *    The pointer of the input PID List.
 *
 *  @param[in] xPidCount
 *    The number of the input PID List.
 *
*/
void secSetPid2Sess(TTransportSessionId xSessionID, 
	enum SEC_CRYPTO_TYPE crypto_type,
	TUnsignedInt16 *xPidList,TUnsignedInt32 xPidCount);

/**
 *  @brief
 *    Get the PID list information from the SEC session. 
 *
 *  @param[in] crypto_type
 *    The crypto type specifies the encryption or decryption.
 *
 *  @param[out]  xPidList
 *    The pointer of the input PID List.
 *
 *  @param[out] xPidCount
 *    The number of the output PID List.
 *
*/
void secGetPidFromSessID(TTransportSessionId xSessionID, 
	enum SEC_CRYPTO_TYPE crypto_type,
	TUnsignedInt16 *xPidList,TUnsignedInt32 *xPidCount);

/**
 *  @brief
 *    Update the IV information into the SEC session. 
 *
 *  @param[in] xSessionID
 *    The SEC Session.
 *
 *  @param[in]  pxInitVector
 *    The pointer of the IV.
 *
 *  @param[in]  pxKeyId
 *    Identifier of the key provided
 *
 *  @param[in]  xKeyIdSize
 *    Size in bytes of the key ID (pxKeyId).
 *
 *  @param[in] xInitVectorSize
 *    Identifier of the size in bytes of IV.
 *
 *  @retval  SEC_NO_ERROR.
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    Errors occurred and the operation could not be completed successfully.
*/
TSecStatus secSessionUpdateIV (
	TSecCipherSession xSession, size_t xKeyIdSize, 
	TUnsignedInt8 *pxKeyId, size_t xInitVectorSize, 
	const TUnsignedInt8 *pxInitVector);

/**
 *  @brief
 *    Free the PID list of the SEC Session. 
 *
 *  @param[in] xSessionID
 *    The SEC Session.
 *
 *  @param[in] crypto_type
 *    The crypto type specifies the encryption or decryption.
 *
*/
void secFreeSessPID(TTransportSessionId xSessionID,
	enum SEC_CRYPTO_TYPE crypto_type);

/**
 *  @brief
 *    Get THE KEY information from the SEC session.
 *
 *  @param[in] xSession 
 *    Input parameter for indexing the internal SESSION.
 *
 *  @param[in] xKeyHandleArray[]
 *    Output the session's key handles.
 *
 *  @param[out] xKeyNum 
 *    Output the session's key number.
 *
 *  @param[out] ppKey
 *    Output the key/iv buffer address to this pointer.
 *
 *  @param[out] xParity 
 *    Output the key/iv parity to this pointer.
 *
 *  @param[out] xKeyType
 *    Clear key is 1, KL key is 2.
 *
 *  @retval  SEC_NO_ERROR.
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER
 *    Errors occurred and the operation could not be completed successfully.
 */
TSecStatus secGetSessionKeyInfo(
	TSecCipherSession xSession, int xKeyHandleArray[],
	int *xKeyNum, void *ppKey[], int *xParity, int *xKeyType);

/*
 *  @brief
 *    Start or Stop the playback info for this session.
 *
 *  @param[in] xSession 
 *    Input parameter for indexing the internal SESSION.
 *
 *  @param[in]xEnDis
 *    0: stop, others: start.
 *
 *  @retval  SEC_NO_ERROR.
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER/SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 */
TSecStatus secSetPlaybackInfo(
	TSecCipherSession xSession, int xEnDis);

/**
 *  @brief
 *	  Set the Transport Session ID(TSID) Usage
 *    This function shall be call before the open session such as secOpenStreamDecryptSession.
 *
 *  @param[in]  xTSID
 *    The Transport Session ID.
 *
 *  @param[in] usage
 *    The usage for Transport Session ID.
 *
 *  @retval  SEC_NO_ERROR.
 *    The operation was completed successfully.
 *
 *  @retval  SEC_ERROR_BAD_PARAMETER/SEC_ERROR
 *    Errors occurred and the operation could not be completed successfully.
 */
TSecStatus secSetTSIDUsage(TTransportSessionId xTSID, TSecTSIDUsage usage);

#ifdef __cplusplus
}
#endif

#endif // _CA_SEC_UTIL_H_