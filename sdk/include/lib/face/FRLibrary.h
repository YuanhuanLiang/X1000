#ifndef _FRLIB_H
#define _FRLIB_H




#ifdef _WIN32
	#if  (WFFR_ENABLE_PCENROLL_ONLY == 1)	// Enable enrolling on PC
		#ifdef WFFR_EXPORTS
			#define WFFR_API __declspec(dllexport)
			#define WFFRREC_API
		#else
			#define WFFR_API __declspec(dllimport)
			#define WFFRREC_API
		#endif
	#else
		#ifdef WFFR_EXPORTS
			#define WFFR_API __declspec(dllexport)
			#define WFFRREC_API __declspec(dllexport)
		#else
			#define WFFR_API __declspec(dllimport)
			#define WFFRREC_API __declspec(dllimport)
		#endif
	#endif
#else
	#if  (WFFR_ENABLE_PCENROLL_ONLY == 1)
		#define WFFR_API __attribute__ ((visibility ("default")))
		#define WFFRREC_API __attribute__ ((visibility ("hidden")))
	#else
		#define WFFR_API __attribute__ ((visibility ("default")))
		#define WFFRREC_API __attribute__ ((visibility ("default")))
	#endif
#endif

#define FR_OK 0
#define FR_ERROR_GENERAL 1
#define FR_ERROR_BAD_ARGS 2
#define FR_ERROR_LICENSE 3
#define FR_ERROR_NOTSUPPORTED 4
#define FR_ERROR_FRMODE 5
#define FR_ERROR_LOADDB 6
#define FR_ERROR_DBUPDATE 7
#define FR_ERROR_LOADUPDATEDDB 8
#define FR_ERROR_INIT 9
#define FR_ERROR_ADDRECORD 10
#define FR_ERROR_CREATEDBPATH 11

/////////// Licese Error Codes //////////
#define FR_LICERROR_HOSTADDR (-1)
#define FR_LICERROR_SOCKET (-2)
#define FR_LICERROR_SERVERERROR (-3)
#define FR_LICERROR_TIMEOUT (-4)
#define FR_LICERROR_SOCKWRITE (-5)
#define FR_LICERROR_SOCKREAD (-6)
#define FR_LICERROR_READDATA (-7)
#define FR_LICERROR_INCORRECTDATA (-13)
/////////// Licese Error Codes //////////


#include "FRLibraryTypes.h"

#ifndef __ANDROID__
#ifdef __cplusplus
extern "C" {
#endif
#endif

//! Verify permanent license.
/*!
  \ingroup INI
  \param pBasePath Base path of the folder which contains all data files
*/
WFFR_API int wfFR_VerifyLic(const char* pBasePath);

//! Create new instance of FR Engine
/*!
  \ingroup INI
  \param pHandle pointer to the handle to be initialized
  \param pBasePath Base path of the folder which contains all data files
  \param iFrameWid video width 
  \param iFrameHig video height 
  \param iFrameStep video width step 
  \param eFrMode Set FRMODE_RECOGNIZE for recognition mode, set FRMODE_ENROLL for enroll mode
  \param enableAntiSpoofing Flag to enable anti-spoofing for face. Set 1 to enable and 0 to disable
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_Init(void** pHandle, const char* pBasePath, int iFrameWid, int iFrameHig, int iFrameStep, FRMode eFrMode, int enableAntiSpoofing);



//! Destroy the instance of Engine and save database
/*!
  \param pHandle pointer to the handle to be freed
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_Release(void** pHandle);

/*! Recognize face from video
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFaces(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pStatus);



/*! Enroll face from video/image
  \param handle the Engine's handle
  \param pImageData query image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iImageWid query image's width
  \param iImageHig query image's height
  \param iImageStep query image's width step
  \param lRecordID record ID to be trained with new images
  \param iForce set to 1 for image mode, 0 for video mode. Only 0 is supported at the moment
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFace(void* handle, const unsigned char* pImageData, int iImageWid, int iImageHig, int iImageStep, unsigned long lRecordID, int iForce, FRIDList* pResult, int* pStatus);


/*! Recognize face saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesFromSavedImage(void* handle, const char* pImageFilename, FRIDList* pResult, int* pStatus);


/*! Recognize face from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesFromJpegBuffer(void* handle, const char* pJpegBuffer, int iJpegBufferSize, FRIDList* pResult, int* spoofStatus);


/*! Detect and Recognize face from video in multi-thread. Detection is syncronous and recognition is async
  \param handle the Engine's handle
  \param pFrameData query image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_DetectRecognizeFacesMultiThread(void* handle, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResult, int* pStatus);



/*! Recognize face from dual camera
  \param handle the Engine's handle
  \param pFrameDataColor query 1st cam image's pixel data (Y channel). Only Gray image / Y channel supported
  \param pFrameDataIR query 2nd cam image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param pResult1 Output search results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pResult2 Output search results for 2nd cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFRREC_API int wfFR_RecognizeFacesDualCam(void* handle, const unsigned char* pFrameDataColor, const unsigned char* pFrameDataIR, int iFrameWid, int  iFrameHig, int iFrameStep, FRIDList* pResultColor, FRIDList* pResultIR, int* pStatus);


/*! Enroll face from dual camera
  \param handle the Engine's handle
  \param pFrameDataColor query 1st cam image's pixel data (Y channel). Only Gray image / Y channel supported
  \param pFrameDataIR query 2nd cam image's pixel data (Y channel). Only Gray image / Y channel supported
  \param iFrameWid query image's width
  \param iFrameHig query image's height
  \param iFrameStep query image's width step
  \param lRecordID record ID to be trained with new images
  \param pResult1 Output results for 1st cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pResult2 Output results for 2nd cam, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceDualCam(void* handle, const unsigned char* pFrameDataColor,  const unsigned char* pFrameDataIR, 
	int iFrameWid, int  iFrameHig, int iFrameStep, unsigned long lRecordID, FRIDList* pResultColor, FRIDList* pResultIR, int* spoofStatus);

/*! Enroll face from saved image like jpeg/bmp/png
  \param handle the Engine's handle
  \param pImageFilename Image file name stored on device like  jpeg/bmp/png
  \param lRecordID record ID to be trained with new images
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceFromSavedImage(void* handle,  const char* pImageFilename, unsigned long lRecordID, FRIDList* pResult, int* pStatus);


/*! Enroll face from Jpeg buffer
  \param handle the Engine's handle
  \param pJpegBuffer Input image Jpeg buffer data
  \param iJpegBufferSize Input image Jpeg buffer data size in bytes
  \param lRecordID record ID to be trained with new images
  \param pResult Output search results, an array of FRIDList structures FRIDList[nFaces] (output)
  \param pStatus Output spoof detection status, 1 if spoof detection is verified for real face 0 otherwise
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_EnrollFaceFromJpegBuffer(void* handle,  const char* pJpegBuffer, int iJpegBufferSize, unsigned long lRecordID, FRIDList* pResult, int* spoofStatus);

//! Creates new record and returns new recordID.
/*!
  \param handle the Engine's handle
  \param pRecordID pointer to record's ID (output)
  \param firstName First name of the person
  \param secondName Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_AddRecord(void* handle, unsigned long* pRecordID, const char* firstName, const char* secondName);



//! Remove single record whose record ID is lRecordID from the database. It will remove only one record for the person.
/*!
  \param handle the Engine's handle
  \param lRecordID record ID to be removed
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RemoveSingleRecord(void* handle, unsigned long  lRecordID);


//! Remove all record of the person whose record ID is lRecordID from the database. It will completely remove the person from the database. 
/*! 
  \param handle the Engine's handle
  \param lRecordID record ID of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RemovePersonByRecordID(void* handle, unsigned long  lRecordID);


//! Remove person from the database by name. It will completely remove the person from the database. 
/*!
  \param handle the Engine's handle
  \param firstName First name of the person
  \param secondName Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RemovePersonByName(void* handle, const char* firstName, const char* secondName);


//! Delete all enrolled people from the database. Database will be empty after this API
/*!
\param handle the Engine's handle
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_RemoveAllRecord(void* handle);


//! Rename record whose record ID is lRecordID from the database.
/*! 
  \param handle the Engine's handle
  \param lRecordID record ID of the person
  \param firstName First name of the person
  \param secondName Second name of the person
  \return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
  */
WFFR_API int wfFR_RenameRecordID(void* handle, unsigned long  lRecordID, const char* firstName, const char* secondName);


#ifdef __cplusplus
//! Get list of names for people enrolled in DB. Also returns the first record ID for each name in database.
/*!
\param handle the Engine's handle
\param outFirstNameList Output list of first names
\param outSecondNameList Output list of second names
\param outRecordIDList Output list of first record ID for each name
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/

WFFR_API int wfFR_GetNamelist(void* handle, std::vector<std::string>& outFirstNameList,  
	std::vector<std::string>& outSecondNameList,  std::vector<int>& outRecordIDList);


//! Get list of all recordID's in sorted order for the person with input name. It also gets number of images enrolled for each recordID.
/*!
\param handle the Engine's handle
\param firstName First name of the person
\param secondName Second name of the person
\param outRecordIDList Output list of unique recordID's enrolled for input name
\param outNumImagesList Output list of number of images enrolled for each recordID
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_GetRecordIDListByName(void* handle, const char* firstName, const char* secondName,  
				std::vector<int>& outRecordIDList, std::vector<int>& outNumImagesList);

#endif

//! Get all images for people enrolled in DB one by one. It will fetch images from all records for the person with record ID lRecordID
/*!
\param handle the Engine's handle
\param lRecordID any one of the Record ID for the person
\param inpImageIndex Image index to to be retrieved from DB
\param totalImages (Ouptut) Total number of images for the person.
\param outImageJpeg (Output) Pointer to the jpeg image byte data.
\param outImageJpegSize (Output) Size in bytes for the ouptut jpeg image.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_GetImageFromDb(void* handle, unsigned long lRecordID, int inpImageIndex,
				int* totalImages, char** outImageJpeg, int* outImageJpegSize);

//! Set Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param sensitivity Sensitivity for face spoof. Range [0,3]. Default is 1 (medium). 
				0 is low (difficult), 
				1 is medium 		
				2 is high sensitivity (easy)
				3 is very high sensitivity (very easy)
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setSpoofingSensitivity(void* handle, int sensitivity);


//! Get Face Spoof detection sensitivity
/*!
\param handle the Engine's handle
\param psensitivity Output sensitivity for face spoof. Range [0,3]. 
				0 is low (difficult), 
				1 is medium 		
				2 is high sensitivity (easy)
				3 is very high sensitivity (very easy)
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSpoofingSensitivity(void* handle, int* psensitivity);


//! Set Face Recognition confidence threshold
/*!
\param confidenceThresold Confidence threshold to be set for for face recognition. Range is [50,80]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRREC_API int wfFR_setRecognitionThreshold(float confidenceThresold);


//! Get Face Recognition confidence threshold
/*!
\param pconfidenceThresold Output confidence threshold for face recognition
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFRREC_API int wfFR_getRecognitionThreshold(float* pconfidenceThresold);

//! Set Face detection minimum size. It should be called before wfFR_Init.
/*!
\param minfacesize Minimum face detection size in percentage of input image dimentions. Range is [10,100]
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setMinFaceDetectionSizePercent(int minfacesize);

//! Get Face detection minimum size
/*!
\param pminfacesize Output minimum face detection size in percentage of input image dimentions.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getMinFaceDetectionSizePercent(int* pminfacesize);


//! Set engine mode to run Face detection only. Turns off recognition 
/*!
\param runDetectionOnly Set to 1 to run detection only, set to 0 to run detection and recognition
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setDetectionOnlyMode(int runDetectionOnly);

//! Get Face detection minimum size
/*!
\param prunDetectionOnly Output run detection only mode
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDetectionOnlyMode(int* prunDetectionOnly);


//! Set engine mode to save detected faces
/*!
\param saveDetectedFaces Set to 1 to save detected faces to folder <basepath>/fd/
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setSaveDetectedFaceFlag(int saveDetectedFaces);

//! Get engine mode to save detected faces
/*!
\param psaveDetectedFaces Output save flag, 0 for saving off and 1 for saving on
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSaveDetectedFaceFlag(int* psaveDetectedFaces);

//! Get name of the detected face image saved during last recognition call
/*!
\param pImageName Output image name. Should be allocated 2048 bytes for ex "char pImageName[2048]"
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSaveDetectedImageName(char pImageName[]);


//! Set Verbose for SDK to enable debugging prints
/*!
\param isEnabled Set 1 to enable prints from from SDK, set to 0 to disable prints
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setVerbose(int isEnabled);


//! Enable or Disable enroll image saving
/*!
\param handle the Engine's handle
\param enableSaving Set 1 to enable enroll image saving and 0 to disable enroll image saving
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_saveEnrollImages(void* handle, int enableSaving);



//! Get status of enroll image saving
/*!
\param handle the Engine's handle
\param saveEnrollImagesStatus Output enroll image saving status, 1 if it is enabled and 0 if it is disabled.
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getSaveEnrollImagesStatus(void* handle, int *saveEnrollImagesStatus);

//! Set base path for Database folder "wffrdb" and "wffrdbpc". It should be called before wfFR_Init() API
/*!
\param pDbBasePath Input base directory folder for database
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_setDatabaseFolderPath(const char* pDbBasePath);

//! Get base path for Database folder "wffrdb" and "wffrdbpc"
/*!
\param pDbBasePath Output base directory folder for database
\return FR_OK in case of success, or error code FR_ERROR_GENERAL otherwise
*/
WFFR_API int wfFR_getDatabaseFolderPath(char* pDbBasePath);

WFFR_API int wfFR_getFaceLandmarks(void* handle, const char* pBasePath, const unsigned char* pFrameData, int iFrameWid, int  iFrameHig, int iFrameStep, FRFace face, int* facepoints);
#ifndef __ANDROID__
#ifdef __cplusplus
}
#endif
#endif


#endif //_FRLIB_H




