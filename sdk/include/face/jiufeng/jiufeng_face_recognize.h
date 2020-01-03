#ifndef _JIUFENG_FACE_RECOGNIZE_H
#define _JIUFENG_FACE_RECOGNIZE_H

#include <types.h>
#include <camera_v4l2/camera_v4l2_manager.h>


typedef int jiufeng_face_id_t;
typedef void (*jiufeng_face_notify_callback)(int, jiufeng_face_id_t, void*);
typedef struct jiufeng_face_customer_config
{
    uint8_t rotate_degree;                  /* DEGREE_0 or DEGREE_90 */
    uint8_t duplicate_threshold;            /* duplicate check threshold -- oxFF: don't check */
    uint8_t enroll_step;                    /* enroll times */
    uint8_t enroll_threshold;               /* enroll JIUFENG_FACE_ENROLL_STEP_NUM times,  */
                                            /* similarity score of between them bigger than it  */
    int max_enroll_face_num;                /*  1 ~ ...    0 : defalut  */
    int enroll_timeout;                     /*  0 ~ ...    0 : defalut  */
    int authenticate_timeout;               /*  0 ~ ...    0 : defalut  */
    char file_path[128];                    /*  "/xxx/xxx"    0 : defalut  */
} jiufeng_face_customer_config_t;


typedef enum jiufeng_face_msg_type {
    JIUFENG_FACE_RECOGNIZE_ENROLL_SUCCESS          = 1,
    JIUFENG_FACE_RECOGNIZE_ENROLL_ING              = 2,
    JIUFENG_FACE_RECOGNIZE_ENROLL_FAILED           = 3,
    JIUFENG_FACE_RECOGNIZE_ENROLL_DUPLICATE        = 4,
    JIUFENG_FACE_RECOGNIZE_ENROLL_TIMEOUT          = 5,
    JIUFENG_FACE_RECOGNIZE_AUTHENTICATE_SUCCESS    = 6,
    JIUFENG_FACE_RECOGNIZE_AUTHENTICATED_NO_MATCH  = 7,
    JIUFENG_FACE_RECOGNIZE_AUTHENTICATED_FAILED    = 8,
    JIUFENG_FACE_RECOGNIZE_AUTHENTICATED_TIMEOUT   = 9,
    JIUFENG_FACE_RECOGNIZE_REMOVE_SUCCESS          = 10,
    JIUFENG_FACE_RECOGNIZE_REMOVE_FAILED           = 11,
    JIUFENG_FACE_RECOGNIZE_DETECT                  = 12
} jiufeng_face_msg_type_t;

typedef enum {
    JIUFENG_DELETE_BY_ID,
    JIUFENG_DELETE_ALL,
} jiufeng_delete_type_t;


typedef struct {
    long  left;
    long  top;
    long  right;
    long  bottom;
}jiufeng_face_coordinate;

/**
 * @fn int jiufeng_face_recognize_init(jiufeng_face_notify_callback notify, void *param_config);
 *
 * @brief  初始化模块
 *
 * @param notify       消息回调函数
 *        param_config 用户参数
 *
 *
 * @retval 0 成功
 * @retval <0 失败
 *
 * @note 该接口必须首先调用
 */
int jiufeng_face_recognize_init(jiufeng_face_notify_callback notify, void *param_config);


/**
 * @fn int jiufeng_face_recognize_destroy(ei_face_notify_callback notify, void *param_config);
 *
 * @brief  反初始化模块
 *         不再使用算法时关闭模块
 *
 *
 * @retval 0 成功
 * @retval <0 失败
 *
 */
int jiufeng_face_recognize_destroy(void);


/**
 * @fn int jiufeng_face_recognize_enroll(void);
 *
 * @brief  开始注册
 *
 *
 * @retval 0 成功
 * @retval <0 失败
 *
 * @note 操作非阻塞
 */
int jiufeng_face_recognize_enroll(void);

/**
 * @fn int jiufeng_face_recognize_authenticate(void);
 *
 * @brief  开始验证
 *
 *
 * @retval 0 成功
 * @retval <0 失败
 *
 * @note 操作非阻塞
 */
int jiufeng_face_recognize_authenticate(void);

/**
 * @fn int jiufeng_face_recognize_delete(int id, jiufeng_delete_type_t type);
 *
 * @brief  删除id
 *
 * @param id   指定id
 *        type 操作类型,单个id或所有id
 *
 * @retval 0 成功
 * @retval <0 失败
 *
 * @note 操作非阻塞
 */
int jiufeng_face_recognize_delete(int id, jiufeng_delete_type_t type);

/**
 * @fn int jiufeng_face_recognize_cancel(uint32_t template_ids[]);
 *
 * @brief  获取当前已注册的id列表
 *
 * @param template_ids id列表
 *
 * @retval 0 成功
 * @retval <0 失败
 *
 */
int jiufeng_face_recognize_cancel(void);

/**
 * @fn int jiufeng_face_recognize_get_template_info(uint32_t template_ids[]);
 *
 * @brief  获取当前已注册的id列表
 *
 * @param template_ids id列表
 *
 * @retval 0 成功
 * @retval <0 失败
 *
 */
int jiufeng_face_recognize_get_template_info(uint32_t template_ids[]);


/**
 * @fn jiufeng_face_recognize_handle(uint8_t* ybuf, uint32_t width, uint32_t height);
 *
 * @brief  进行人脸特征处理
 *
 * @param ybuf   图像y值数据
 *        width  图像宽度
 *        height 图像高度
 *
 */
void jiufeng_face_recognize_handle(uint8_t* ybuf, uint32_t width, uint32_t height);

#endif /* JIUFENG_FACE_RECOGNIZE_FINGERPRINT_H */
