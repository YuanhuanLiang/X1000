#ifndef __GOODIX_INTERFACE_H__
#define __GOODIX_INTERFACE_H__

/**********************************
 *      Header File inclusion     *
 **********************************/



/**********************************
 *        Macro Definition        *
 **********************************/



/**********************************
 *       Typedef Declaration      *
 **********************************/
typedef void (*notify_callback)(int, int, int);

typedef struct Customer_Config
{
    //int max_enroll_finger_num;                 /*  1 ~ 100    0 : defalut  */
    int min_enroll_count_for_one_finger;       /*  3 ~ 8     0 : defalut  */
    int check_overlay_enable;                  /*  0 , 1      0 : defalut  */
    int image_dump_enable;                     /*  0 , 1      0 : defalut  */
    int algo_log_enable;                       /*  0 , 1      0 : defalut  */
    int enroll_timeout;                        /*  0 ~ ...    0 : defalut  */
    int authenticate_timeout;                  /*  0 ~ ...    0 : defalut  */
    int enroll_image_quality_threshold;        /*  50 ~ 100    0 : defalut  */
    int enroll_coverage_threshold;             /*  10 ~ 100    0 : defalut  */
    int authenticate_image_quality_threshold;  /*  50 ~ 100    0 : defalut  */
    int authenticate_coverage_threshold;       /*  10 ~ 100    0 : defalut  */
    int overlay_threshold;                     /*  1 ~ 100     0 : defalut  */
    int pre_overlay_threshold;                 /*  1 ~ 100     0 : defalut  */
    char file_path[100];                       /*  "/xxx/xxx"    0 : defalut  */
} Customer_Config_t;



/**********************************
 *      Function Declaration      *
 **********************************/

/******************************************************************************
 *  Description: Init fingerprint sensor
 *
 *  Param: notify_callback function, Customer Config Param
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int fingerprint_init(notify_callback notify, void *param_config);

/*****************************************************************************
 *  Description: Close fingerprint sensor
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int fingerprint_destroy(void);

/******************************************************************************
 *  Description: Reset fingerprint sensor
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 *****************************************************************************/
int fingerprint_reset(void);

/*****************************************************************************
 *  Description: Enroll fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 ****************************************************************************/
int fingerprint_enroll(void);

/****************************************************************************
 *  Description: Authenticate fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 ***************************************************************************/
int fingerprint_authenticate(void);

/***************************************************************************
 *  Description: Delete fingerprint
 *
 *  Param: fingerprint_id(int), the id of fingerprint to delete (1 ~ MAX(20))
 *             type, 0 delete by index (default), 1 delete all
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int fingerprint_delete(int fingerprint_id, int type);

/**************************************************************************
 *  Description: Cancel regist/verify fingerprint
 *
 *  Param: NULL
 *
 *  Return: 0 success, -1 failure
 *
 **************************************************************************/
int fingerprint_cancel(void);

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
int fingerprint_get_template_info(int template_info[]);

/*************************************************************************
 * Description: Do software prepare after power on.
 *
 * Param: NULL
 *
 * Return: 0 success, otherwise failed.
 *
 ************************************************************************/
 int fingerprint_post_power_on(void);

/************************************************************************
 * Description: Do software prepare after power off.
 *
 * Param: NULL
 *
 * Return: 0 success, otherwise failed.
 *
 ***********************************************************************/
 int fingerprint_post_power_off(void);



#endif  // __GOODIX_INTERFACE_H__
