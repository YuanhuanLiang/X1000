#ifndef FPC_FINGERPRINT_H
#define FPC_FINGERPRINT_H

typedef void (*notify_callback)(int, int, int);
typedef struct customer_config
{
    int max_enroll_finger_num;                 /*  1 ~ 100    0 : defalut  */
    int min_enroll_count_for_one_finger;       /*  3 ~ 8     0 : defalut  */
    int enroll_timeout;                        /*  0 ~ ...    0 : defalut  */
    int authenticate_timeout;                  /*  0 ~ ...    0 : defalut  */
    char*uart_devname;                         /*  /dev/tty*               */
    char file_path[128];                       /*  "/xxx/xxx"    0 : defalut  */
} customer_config_t;



typedef enum fpc_msg_type {
    FPC_ENROLL_SUCCESS          = 1,
    FPC_ENROLL_FAILED           = 2,
    FPC_ENROLL_ING              = 3,
    FPC_ENROLL_DUPLICATE        = 4,
    FPC_ENROLL_TIMEOUT          = 5,
    FPC_AUTHENTICATE_SUCCESS    = 6,
    FPC_AUTHENTICATED_FAILED    = 7,
    FPC_AUTHENTICATED_TIMEOUT   = 8,
    FPC_REMOVE_SUCCESS          = 9,
    FPC_REMOVE_FAILED           = 10,
    FPC_LOW_QUALITY             = 11,
    FPC_CANCLED                 = 12
} fpc_msg_type_t;


/******************************************************************************
 *  Description: Init fingerprint sensor
 *
 *  Param: notify_callback function, Customer Config Param
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int fpc_fingerprint_init(notify_callback notify, void *param_config);

/******************************************************************************
 *  Description: Init fingerprint algorithm encry chip
 *
 *               used to encry chip down power and then re-power, call it to
 *               reinit and continue to user the algorithm.
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int fpc_fingerprint_encry_chip_init(void);

/*****************************************************************************
 *  Description: Close fingerprint sensor
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int fpc_fingerprint_destroy(void);

/******************************************************************************
 *  Description: Reset fingerprint sensor
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int fpc_fingerprint_reset(void);

/*****************************************************************************
 *  Description: Enroll fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 ****************************************************************************/
int fpc_fingerprint_enroll(void);

/****************************************************************************
 *  Description: Authenticate fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 ***************************************************************************/
int fpc_fingerprint_authenticate(void);

/***************************************************************************
 *  Description: Delete fingerprint
 *
 *  Param: fingerprint_id(int), the id of fingerprint to delete (1 ~ MAX(20))
 *             type, 0 delete by index (default), 1 delete all
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int fpc_fingerprint_delete(int fingerprint_id, int type);

/**************************************************************************
 *  Description: Cancel regist/verify fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int fpc_fingerprint_cancel(void);

/**************************************************************************
 *  Description: Get template info, including the current template sum
 *                    and the template serial index in array
 *
 *  Param:  output template_info(int []), the serial index of template (array)
 *                       template_info[MAX]
 *
 *  Return:  -1 ~ MAX(20), the template sum
 *              -1 failure
 *               >=0 success
 *
 *************************************************************************/
int fpc_fingerprint_get_template_info(uint32_t template_info[]);

#endif /* FPC_FINGERPRINT_H */
