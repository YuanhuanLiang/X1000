#ifndef __MAFP_API_H__
#define __MAFP_API_H__


typedef void (*notify_callback)(int, int, int);


typedef enum {
    MAFP_MSG_FINGER_DOWN          = 0,
    MAFP_MSG_FINGER_UP            = 1,
    MAFP_MSG_NO_LIVE              = 2,
    MAFP_MSG_PROCESS_SUCCESS      = 3,
    MAFP_MSG_PROCESS_FAILED       = 4,
    MAFP_MSG_ENROLL_SUCCESS       = 5,
    MAFP_MSG_ENROLL_FAILED        = 6,
    MAFP_MSG_MATCH_SUCESS         = 7,
    MAFP_MSG_MATCH_FAILED         = 8,
    MAFP_MSG_CANCEL               = 9,
    MAFP_MSG_ENROLL_TIMEOUT       = 10,
    MAFP_MSG_MATCH_TIMEOUT        = 11,
    MAFP_MSG_DELETE_SUCCESS       = 12,
    MAFP_MSG_DELETE_FAILED        = 13,
    MAFP_MSG_DUPLICATE_FINGER     = 14,
    MAFP_MSG_BAD_IMAGE            = 15,
    MAFP_MSG_MAX
} mafp_msg_t;


typedef enum {
    MAFP_DELETE_BY_ID,
    MAFP_DELETE_ALL,
} mafp_del_type_t;


/******************************************************************************
 *  Description: Init fingerprint sensor
 *
 *  Param: cfg_dir, configuration storage path
 *         notify_callback, callback function
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int mafp_api_init(char *cfg_dir, notify_callback notify);


/*****************************************************************************
 *  Description: Close fingerprint sensor
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int mafp_api_destroy(void);


/*****************************************************************************
 *  Description: Enroll fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 ****************************************************************************/
int mafp_api_enroll(void);


/****************************************************************************
 *  Description: Authenticate fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 ***************************************************************************/
int mafp_api_authenticate(void);


/***************************************************************************
 *  Description: Delete fingerprint
 *
 *  Param: fingerprint_id(int), the id of fingerprint to delete (1 ~ MAX(20))
 *             type, 0 delete by index (default), 1 delete all
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int mafp_api_delete(int fingerprint_id, int type);


/**************************************************************************
 *  Description: Cancel regist/verify fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int mafp_api_cancel(void);


/**************************************************************************
 *  Description: Set timeout of enroll(waiting finger down/up)
 *
 *  Param: enroll_timeout(int), the timeout of enroll(waiting finger down/up)
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int mafp_api_set_enroll_timeout(int enroll_timeout);


/**************************************************************************
 *  Description: Set timeout of authenticate(waiting finger down/up)
 *
 *  Param: authenticate_timeout(int), the timeout of authenticate(waiting finger down/up)
 *
 *  Return: 0 success, -1 failure
 *
 *************************************************************************/
int mafp_api_set_authenticate_timeout(int authenticate_timeout);


/**************************************************************************
 *  Description: Get template info, including the current template sum
 *                    and the template serial index in array
 *
 *  Param:  output template_info(int []), the serial index of template (array)
 *                       template_info[MAX]
 *
 *  Return:  -1 ~ MAX(100), the template sum
 *              -1 failure
 *               >=0 success
 *
 *************************************************************************/
int mafp_api_get_template_info(int template_info[]);


/**************************************************************************
 *  Description: Set times of enrolling
 *
 *  Param: times - enroll times
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int mafp_api_set_enroll_times(int times);


#endif


