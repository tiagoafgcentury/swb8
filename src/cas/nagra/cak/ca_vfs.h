/**
 **  @file ca_vfs.h
 **
 **  @brief
 **   Versatile File System management.
 **
 **  @ingroup VFS
 **
 **  @author M. Ferrini
 **
 **  $Revision: 1.14 $
 **
 ** COPYRIGHT:
 **   2002-2019 NagraVision S.A.
 **
 **
 */
/**
 *  @defgroup VFS Versatile File System
 *  @brief
 *    Versatile File System Driver. This module is mainly used by the DIL
 *    CDE (read only) and CAK ELK.
 */

#ifndef CA_VFS_H
#define CA_VFS_H

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/
#if defined VFS_WRAP || defined VFS_DEBUG_SHELL
#include "vfswrap.h"
#endif /* VFS_WRAP or VFS_DEBUG_SHELL */

#include "ca_defs.h"

/******************************************************************************/
/*                                                                            */
/*                                 CONSTANTS                                  */
/*                                                                            */
/******************************************************************************/
/**
 *  @brief
 *    Invalid value for TVfsFileHandle.
 *
 *  @see TVfsFileHandle
 */
#define VFS_FILE_HANDLE_INVALID   NULL

/**
 *  @brief
 *    Invalid value for TVfsDirectoryHandle.
 *
 *  @see TVfsDirectoryHandle
 */
#define VFS_DIRECTORY_HANDLE_INVALID  NULL

/******************************************************************************/
/*                                                                            */
/*                                  TYPES                                     */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    Each open file is bound to a unique file session handle. This value is
 *    provided at file open time and is used for all other calls referencing
 *    this file.
 */
typedef void*      TVfsFileHandle;


/**
 *  @brief
 *    Each directory opening session is bound to a unique handle. This value is
 *    provided at vfsOpenDirectory call time and is used for all subsequent
 *    calls to vfsReadDirectory and vfsCloseDirectory.
 */
typedef void*      TVfsDirectoryHandle;


/**
 *  @brief
 *    Defines the create mode.
 */
typedef enum
{
  VFS_CREATE_NEW,
  /**< Creates a new file. If the file exists, the creation fails.
   */
  VFS_CREATE_ALWAYS
  /**< Create a new file. If the file exists, it will be overwriten
   *   and existing attributes are cleared.
   */
} TVfsCreateMode;


/**
 *  @brief
 *    This enumeration is the status returned by the VFS functions.
 */
typedef enum
{
  VFS_NO_ERROR,
  /**< The function performed successfully.
   */
  VFS_ERROR_LOW_MEMORY,
  /**< The system has no enough free memory to perform the requested
   *   operation.
   */
  VFS_ERROR_BAD_HANDLE,
  /**< The handle passed to the function is not valid.
   */
  VFS_ERROR_BAD_PARAMETER,
  /**< Some parameters have an unsupported value.
   */
  VFS_ERROR_ACCESS,
  /**< The requested operation is not applicable, as the file is not
   *   open in the right file access.
   */
  VFS_ERROR_FILESYSTEM_FULL,
  /**< The physical file system has no enough space left for the
   *   requested operation.
   */
  VFS_ERROR_END_OF_FILE,
  /**< The end of file has been reached and the operation cannot be
   *   completed (typically a write or a seek operation).
   */
  VFS_ERROR_NO_SUCH_FILE,
  /**< The requested file does not exist
   */
  VFS_ERROR
  /**< Other error. This error status is returned if an unexpected error
   *   occurs. On such an error CAK will try to cancel pending operations
   *   (ex. if an operation fails because of an unexpected condition
   *    CAK will cancel that operation and any previous ones linked.)
   */
} TVfsStatus;


/**
 *  @brief
 *    Flags used to specify how the file will be accessed once open.
 */
typedef enum
{
  VFS_FILE_OPEN_READ_ONLY         = BIT0,
  /**<  Open a file for read access. If the file does not exist,
   *    VFS_ERROR_NO_SUCH_FILE is returned.
   */
  VFS_FILE_OPEN_WRITE_ONLY        = BIT1,
  /**<  Open a file for write access. If the flag VFS_FILE_OPEN_APPEND is set,
   *    the data will always be written at the end of the file. If the flag
   *    VFS_FILE_OPEN_TRUNCATE is set, the current content is discarded at
   *    file opening.
   */
  VFS_FILE_OPEN_READ_WRITE        = BIT2,
  /**<  Open file for read and write access.
   */
  VFS_FILE_OPEN_TRUNCATE          = BIT3,
  /**<  Discard the previous content of the file when opening it
   *    in write or read write access. No effect for reading.
   */
  VFS_FILE_OPEN_APPEND            = BIT4
  /**<  The seek pointer of the file must be moved to the end of
   *    the file before each write. No effect for reading.
   */
} TVfsOpenFlags;


/**
 *  @brief
 *    Defines the seek mode.
 */
typedef enum
{
  VFS_SEEK_ABSOLUTE,
  /**< The offset is from the beginning of the file.
   */
  VFS_SEEK_RELATIVE
  /**< The offset is relative to the seek pointer of the file.
   */
} TVfsSeekMode;


/**
 *  @brief
 *    Information available about a file.
 *
 */
typedef struct
{
  TSize length;
  /**< Size in bytes of the file.
   */
  TSize position;
  /**< Current seek pointer position.
   */
} TVfsFileStatistics;


/**
 *  @brief
 *    Possible type of a directory entry.
 *
 */
typedef enum
{
  VFS_DIRECTORY_ENTRY_FILE,
  /**< The entry is a file.
   */
  VFS_DIRECTORY_ENTRY_SUBDIRECTORY
  /**< The entry is a subdirectory.
   */
} TVfsDirectoryEntryType;


/**
 *  @brief
 *    This section describes the structure of the information available about
 *    each entry (file or subdirectory) of a directory.
 *
 */
typedef struct
{
  TVfsDirectoryEntryType type;
  /**< Either file or directory.
   */
  TChar*      pEntryName;
  /**< Name of the entry including the absolute path. This includes filename
   *   for file entries.
   */
} TVfsDirectoryEntry;

/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPES                            */
/*                                                                            */
/******************************************************************************/


/**
 *  @brief
 *    This function creates an empty file and opens the current file for reading
 *    and writing. The name of the file is defined by \c pxFileName. It is an
 *    absolute path that may contain directories separated by a '/'. The VFS
 *    driver is expected to create all the intermediate directories if they do
 *    not exist.
 *
 *    The creating mode is defined by \c xCreateMode. If the file is created
 *    with the \c VFS_CREATE_NEW mode and the specified file already exists, the
 *    file is not modified and the function returns \c VFS_ERROR_ACCESS. If the
 *    \c VFS_CREATE_ALWAYS mode is set and the specified file already exists,
 *    the function overwrites the file and clears the existing attributes. The
 *    new file handler is returned in \c pxFileHandle. If the call fails, this
 *    value is set to \c VFS_FILE_HANDLE_INVALID.
 *
 *  @param[out] pxFileHandle
 *    File handle returned by this call.
 *  @param[in]  pxFileName
 *    Absolute filename of the file to be open.
 *  @param[in]  xCreateMode
 *    File creation mode.
 *
 *  @retval VFS_NO_ERROR
 *            Success to create the file.
 *  @retval VFS_ERROR_LOW_MEMORY
 *            The system has no enough free memory to perform the requested
 *            operation.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR_FILESYSTEM_FULL
 *            The physical file system has not enough space left for the
 *            requested operation.
 *  @retval VFS_ERROR_ACCESS
 *            The file already exists and the function is called in the
 *            VFS_CREATE_NEW mode.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsCreateFile
(
        TVfsFileHandle* pxFileHandle,
  const TChar*          pxFileName,
        TVfsCreateMode   xCreateMode
);


/**
 *  @brief
 *    This function opens a file for reading or writing. The name of the file
 *    is defined by \c pxFileName.
 *
 *    The opening mode is defined by the flags \c xOpenFlags. One and only one
 *    of the flags \c VFS_FILE_OPEN_READ_ONLY, \c VFS_FILE_OPEN_WRITE_ONLY or
 *    \c VFS_FILE_OPEN_READ_WRITE must be set. If not, the call fails.
 *
 *    Both \c VFS_OPEN_TRUNCATE and \c VFS_FILE_OPEN_APPEND are optional. Both
 *    are relevant in writing operations, ignored otherwise. If
 *    \c VFS_OPEN_TRUNCATE is set, the current content of the file is discarded
 *    before the call returns. If \c VFS_FILE_OPEN_APPEND is set, the seek
 *    pointer is moved at the end of the file before each write operation. On a
 *    successful open, the seek pointer is set at the beginning of the file.
 *    The new file handler is returned in \c pxFileHandle. If the call fails,
 *    this value is set to \c VFS_FILE_HANDLE_INVALID.
 *
 *  @param[out] pxFileHandle
 *    File handle returned by this call.
 *  @param[in]  pxFileName
 *    Absolute filename of the file to be open.
 *  @param[in]  xOpenFlags
 *    File access type.
 *
 *  @retval VFS_NO_ERROR
 *            Success to open file.
 *  @retval VFS_ERROR_LOW_MEMORY
 *            The system has no enough free memory to perform the requested
 *            operation.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR_NO_SUCH_FILE
 *            The requested file does not exist (opening an inexistent file
 *            for reading).
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsOpenFile
(
        TVfsFileHandle* pxFileHandle,
  const TChar*          pxFileName,
        TVfsOpenFlags    xOpenFlags
);



/**
 *  @brief
 *    This function closes a previously opened file.
 *
 *  @param[in]  xFileHandle
 *    Handle of the file to close.
 *
 *  @retval VFS_NO_ERROR
 *            Success to close file.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsCloseFile
(
  TVfsFileHandle xFileHandle
);


/**
 *  @brief
 *    This function removes an existing file from the file system.
 *
 *    Its content is discarded. The \c pxFileName string is an absolute path
 *    that may contain directories. If those directories are empty, they shall
 *    be removed as well (a directory is considered "empty" if it contains
 *    either no file or only "empty" directories).
 *
 *  @param[in]  pxFileName
 *    Absolute filename of the file to be removed from the file system
 *
 *  @retval VFS_NO_ERROR
 *            Success to remove file.
 *  @retval VFS_ERROR_NO_SUCH_FILE
 *            The requested file does not exist.
 *  @retval VFS_ERROR_ACCESS
 *            The requested operation cannot be performed, as the file is
 *            currently accessed.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsRemoveFile
(
  const TChar* pxFileName
);


/**
 *  @brief
 *    This function renames an existing file. This function is only used to
 *    rename files and not directories.
 *
 *    If a file with the new name already exists, the rename operation fails
 *    and \c VFS_ERROR is returned. If the file path is invalid and a file with
 *    the new name already exists, the rename operation fails and
 *    \c VFS_ERROR_NO_SUCH_FILE is returned.
 *
 *  @param[in]  pxOldName
 *    Old name of the file to rename.
 *  @param[in]  pxNewName
 *    New name of the file to rename.
 *
 *  @retval VFS_NO_ERROR
 *            Success to rename the file.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR_NO_SUCH_FILE
 *            The requested file or directory does not exist.
 *  @retval VFS_ERROR_ACCESS
 *            The object cannot be rename because it is used by the system.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsRenameFile
(
  const TChar* pxOldName,
  const TChar* pxNewName
);


/**
 *  @brief
 *    This function gathers miscellaneous information about an opened file.
 *
 *  @param[in]  xFileHandle
 *    Handle of the file to get information from. The file may have
 *    been opened for reading or writing.
 *  @param[out] pxFileStatistics
 *    Buffer provided by the application where to store the retrieved
 *    information.
 *
 *  @retval VFS_NO_ERROR
 *            Success to get file info.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsStatFile
(
  TVfsFileHandle        xFileHandle,
  TVfsFileStatistics*  pxFileStatistics
);


/**
 *  @brief
 *    This function opens the directory specified by its path and returns a
 *    handle useable to list the content of this directory.
 *
 *  @param[out] pxDirectoryHandle
 *    Returned directory handle to pass to subsequent calls to
 *    vfsReadDirectory and vfsCloseDirectory.
 *  @param[in]  pxDirectoryName
 *    Path pointing on the directory to open.
 *
 *  @retval VFS_NO_ERROR
 *            Success to open the directory.
 *  @retval VFS_ERROR_LOW_MEMORY
 *            Not enough memory to open the directory.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR_NO_SUCH_FILE
 *            The directory to open is invalid or inexistent.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsOpenDirectory
(
        TVfsDirectoryHandle* pxDirectoryHandle,
  const TChar*               pxDirectoryName
);


/**
 *  @brief
 *    This function reads one entry of an opened directory.
 *
 *  @param[in]  xDirectoryHandle
 *    Directory handle returned by the \c vfsOpenDirectory function.
 *  @param[out] ppxDirectoryEntry
 *    Structure allocated by the VFS driver and returned to the
 *    caller. This structure contains information about either an
 *    existing file or an existing subdirectory of the opened
 *    directory. It shall be set to \c NULL when the directory has been
 *    completely read. The allocated structure shall remain valid
 *    until \c vfsReadDirectory or \c vfsCloseDirectory is called.
 *
 *  @retval VFS_NO_ERROR
 *            Success to read an entry.
 *  @retval VFS_ERROR_LOW_MEMORY
 *            Not enough memory to allocated the entry structure.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsReadDirectory
(
  TVfsDirectoryHandle    xDirectoryHandle,
  TVfsDirectoryEntry** ppxDirectoryEntry
);


/**
 *  @brief
 *    This function close a directory previously opened with
 *    \c vfsOpenDirectory.
 *
 *  @param[in]  xDirectoryHandle
 *    Directory handle returned by the vfsOpenDirectory function.
 *
 *  @retval VFS_NO_ERROR
 *            Success to close the directory.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsCloseDirectory
(
  TVfsDirectoryHandle xDirectoryHandle
);


/**
 *  @brief
 *    This function writes data into a previously open file.
 *    The data to write are provided in a buffer, \c pxBuffer. The size of the
 *    buffer is specified by \c xSizeOfBuffer.
 *
 *    The data are written at the current seek pointer position of the file.
 *    Data at the current seek position are overwritten. If the file was opened
 *    with the \c VFS_FILE_OPEN_APPEND mode, the seek pointer of the file is set
 *    at the end of the file before each write. The seek pointer of the file is
 *    set after the last byte written in the file.
 *
 *    If xSizeOfBuffer is \c NULL, \c VFS_NO_ERROR is returned. No data is written
 *    If the entire buffer cannot be written due to file system full, the
 *    function returns \c VFS_ERROR_FILESYSTEM_FULL. No data is written.
 *    If the file has not been opened for writing, \c VFS_ERROR_ACCESS is
 *    returned.
 *
 *  @param[in]  xFileHandle
 *    Handle of the file to write into.
 *  @param[in]  pxBuffer
 *    Data buffer to write into the file.
 *  @param[in]  xSizeOfBuffer
 *    Size in bytes of the buffer containing the data to write.
 *
 *  @retval VFS_NO_ERROR
 *            Success to write file.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR_ACCESS
 *            The file was not opened for writing.
 *  @retval VFS_ERROR_FILESYSTEM_FULL
 *            The physical file system has not enough space left for the
 *            requested operation.
 *  @retval VFS_ERROR
 *            Other error.
 *
 *  @remark
 *    -# This function can return asynchronously to increase performances.
 *       From VFS API 1.1.0 Nagravision applications will call the
 *       vfsSynchronizeFile function when flushing is required.
 *
 */
TVfsStatus vfsWriteToFile
(
        TVfsFileHandle  xFileHandle,
  const TUnsignedInt8* pxBuffer,
        TSize           xSizeOfBuffer
);


/**
 *  @brief
 *    This function flushes pending writings on the specified file handle. Its
 *    implementation shall be synchronous. It flushes also pending metadata
 *    treatments like renaming.
 *
 *  @param[in]  xFileHandle
 *    Handle of the file to flush.
 *
 *  @retval VFS_NO_ERROR
 *            Success to write file.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsSynchronizeFile
(
  TVfsFileHandle xFileHandle
);


/**
 *  @brief
 *    This function reads data from a previously open file.
 *
 *    The read data are stored in the provided buffer, \c pxBuffer. The size of
 *    this buffer is specified by \c xSizeOfBuffer and the application expects
 *    that the buffer will be filled with data from the file. The number of
 *    bytes actually read from the file is returned by the \c pxSizeRead
 *    pointed value.
 *
 *    If the function returns any other value than \c VFS_NO_ERROR, the data in
 *    \c pxBuffer and \c pxSizeRead are invalid.Data are read from the current
 *    seek pointer position in the file. The seek pointer of the file is set
 *    after the last byte read in the file.
 *
 *    If the buffer cannot be filled due to end of file, the function returns
 *    \c VFS_NO_ERROR and \c pxSizeRead is set to 0.
 *
 *    If \c xSizeOfBuffer is \c NULL, \c VFS_NO_ERROR is returned. No data is
 *    read. The file must have been open for reading. If not,
 *    \c VFS_ERROR_ACCESS is returned.
 *
 *  @param[in]  xFileHandle
 *    Handle of the file to read from.
 *  @param[out]   pxBuffer
 *    Buffer where to store read bytes. The data in the buffer are not
 *    valid if the function returns any other value than \c VFS_NO_ERROR.
 *  @param[in]  xSizeOfBuffer
 *    Size in bytes of the buffer where to store read data.
 *  @param[out] pxSizeRead
 *    Returned count in bytes of the data read from the file. This
 *    value cannot be bigger than the size of the buffer. This value
 *    is not valid if the function returns any other value than
 *    \c VFS_NO_ERROR.
 *
 *  @retval VFS_NO_ERROR
 *            Success to read file.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR_ACCESS
 *            The file was not open for reading.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsReadFromFile
(
  TVfsFileHandle  xFileHandle,
  TUnsignedInt8* pxBuffer,
  TSize           xSizeOfBuffer,
  TSize*         pxSizeRead
);



/**
 *  @brief
 *    This function changes the seek pointer of the file. The next read or
 *    write operation will occur at the requested position.  If the file was
 *    opened in writing mode \c VFS_FILE_OPEN_APPEND, the next write will still
 *    be done at the end of the file.
 *
 *    Giving an absolute offset of 0 will cause the next read or write
 *    operation to act on the very beginning of the file.
 *
 *    Given an absolute offset out of the file limits, the \c vfsSeekInFile()
 *    function returns \c VFS_ERROR_END_OF_FILE. The seek pointer is left
 *    untouched.
 *
 *  @param[in]  xFileHandle
 *    Handle to the file of which the seek pointer shall
 *    be changed to.
 *  @param[in]  xOffset
 *    New absolute or relative offset of the file.
 *  @param[in]  xSeekMode
 *    Defines is the offset is absolute or relative to the
 *    current position.
 *
 *  @retval VFS_NO_ERROR
 *            Success to seek file pointer.
 *  @retval VFS_ERROR_BAD_HANDLE
 *            The handle passed to the function is not valid.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR_END_OF_FILE
 *            The specified offset sets the file position out of the
 *            file limits.
 *  @retval VFS_ERROR
 *            Other error.
 *
 */
TVfsStatus vfsSeekInFile
(
  TVfsFileHandle xFileHandle,
  TSignedInt32   xOffset,
  TVfsSeekMode   xSeekMode
);


/**
 *  @brief
 *    Return the total size in bytes of a disk partition used by the VFS.
 *
 *  @param[in]  pxDirectoryName
 *    Pointer to a null-terminated string that specifies a directory
 *    on the disk partition of interest.
 *  @param[out] pxTotalSize
 *    Total number of bytes on the given disk partition.
 *
 *  @retval VFS_NO_ERROR
 *            Success.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR
 *            Failure.
 *
 */
TVfsStatus vfsGetTotalSize
(
  const TChar*          pxDirectoryName,
        TUnsignedInt64* pxTotalSize
);


/**
 *  @brief
 *    Return the free size in bytes of a disk partition used by the VFS.
 *
 *  @param[in]  pxDirectoryName
 *    Pointer to a null-terminated string that specifies a directory
 *    on the disk partition of interest.
 *  @param[out] pxFreeSize
 *    Total number of free bytes on the disk partition.
 *
 *  @retval VFS_NO_ERROR
 *            Success.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR
 *            Failure.
 *
 */
TVfsStatus vfsGetFreeSize
(
  const TChar*          pxDirectoryName,
        TUnsignedInt64* pxFreeSize
);


/**
 *  @brief
 *    Return the cluster size in bytes of a disk partition used by the VFS.
 *    A cluster is the smallest amount of space that can be allocated to hold
 *    a file.
 *
 *  @param[in]  pxDirectoryName
 *    Pointer to a null-terminated string that specifies a directory
 *    on the disk partition of interest.
 *  @param[out] pxClusterSize
 *    Cluster size of the disk partition. This value must never be
 *    NULL or equal to 0. If the 'cluster' notion doesn't apply the
 *    cluster size must be set to 1.
 *
 *  @retval VFS_NO_ERROR
 *            Success.
 *  @retval VFS_ERROR_BAD_PARAMETER
 *            Some parameters have an unsupported value.
 *  @retval VFS_ERROR
 *            Failure.
 *
 */
TVfsStatus vfsGetClusterSize
(
  const TChar*          pxDirectoryName,
        TUnsignedInt64* pxClusterSize
);


#endif /* CA_VFS_H */


/* END OF FILE */
