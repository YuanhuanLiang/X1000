#include <string.h>

#include <types.h>
#include <utils/log.h>
#include <utils/common.h>
#include <utils/file_ops.h>
#include <utils/image_convert.h>
#include <signal/signal_handler.h>
#include <power/power_manager.h>
#include <fb/fb_manager.h>
#include <graphics/gr_drawer.h>
#include <camera_v4l2/camera_v4l2_manager.h>
#include <face/ei/ei_face_recognize.h>

#define LOG_TAG                         "ei_face_recognize"
#define FIRMWARE_BASE_PATH              "/usr/firmware/ei_face"         /* this path must match with sdk top makefile specifiled
                                                                           if algorithm lib want to access it !*/

#define PREVIEW_WIDTH                   240
#define PREVIEW_HEIGHT                  180
#define PREVIEW_START_PORT_X            0
#define PREVIEW_START_PORT_Y            140

#define FACE_BOX_EN                     1
#define FACE_BOX_RGB                    0xFFFF00 //yellow

#define FACE_DUPLICATE_THRESHOLD        55

/*
 * camera : 320x240  screen : 240x320
 * If you want the image to fill the screen, you need to set the rotation 90 degrees
 */
#define PREVIEW_ROTATE_DEGREE           DEGREE_0 // DEGREE_x defined in camera_v4l2_manager.h

#define DEFAULT_WIDTH                   320
#define DEFAULT_HEIGHT                  240
#define DEFAULT_BPP                     16
#define DEFAULT_NBUF                    1

#define AUTH_MAX_TIMES                  1
#define MAX_ENROLL_TIMES                5
#define MAX_ENROLL_FACE_NUM             50


#define AUTH_TIMEOUT                    0   /* 0 said no timeout check*/
#define ENROLL_TIMEOUT                  0


#define MK_PIXEL_FMT(x)                                 \
        do{                                             \
            x.rbit_len = fbm->get_redbit_length();      \
            x.rbit_off = fbm->get_redbit_offset();      \
            x.gbit_len = fbm->get_greenbit_length();    \
            x.gbit_off = fbm->get_greenbit_offset();    \
            x.bbit_len = fbm->get_bluebit_length();     \
            x.bbit_off = fbm->get_bluebit_offset();     \
        }while(0)


enum {
    ENROLL_TEST,
    AUTH_TEST,
    LIST_TEST,
    DELETE_TEST,
    SUSPEND_TEST,
    EXIT_TEST,
    TEST_MAX,
};

enum {
    STATE_IDLE,
    STATE_BUSY
};

typedef enum {
    CHANNEL_SEQUEUE_COLOR       = 0x00,
    CHANNEL_SEQUEUE_BLACK_WHITE = 0x01
} chselect_m;

static struct fb_manager* fbm;
static struct gr_drawer* gr_drawer;
static struct camera_v4l2_manager* cvm;
static struct power_manager* power_manager;
static struct signal_handler* signal_handler;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static int work_state = STATE_IDLE;
static int face_count;
static int auth_count;
static uint32_t faces[32];
static ei_face_customer_config_t ei_face_config;

static struct _capt_op {
    chselect_m view_channel;            // select view channel
    chselect_m recognize_channel;       // select recognize channel
    uint16_t *ppbuf;                    // map lcd piexl buf
    uint32_t fb_h;
    uint32_t fb_w;
}capt_op;

static const char* test2string(int item) {
    switch (item) {
    case ENROLL_TEST:
        return "enroll test";
    case AUTH_TEST:
        return "authenticate test";
    case LIST_TEST:
        return "list faces test";
    case DELETE_TEST:
        return "delete test";
    case SUSPEND_TEST:
        return "suspend test";
    case EXIT_TEST:
        return "exit";
    default:
        return "unknown test";
    }
}

static void print_tips(void) {
    fprintf(stderr, "\n================ ei Face Menu =================\n");
    fprintf(stderr, "  %d.Enroll\n", ENROLL_TEST);
    fprintf(stderr, "  %d.Authenticate\n", AUTH_TEST);
    fprintf(stderr, "  %d.List enrolled faces\n", LIST_TEST);
    fprintf(stderr, "  %d.Delete\n", DELETE_TEST);
    fprintf(stderr, "  %d.Suspend\n", SUSPEND_TEST);
    fprintf(stderr, "  %d.Exit\n", EXIT_TEST);
    fprintf(stderr, "======================================================\n");
}

static void print_delete_tips(void) {
    fprintf(stderr, "==============================================\n");
    fprintf(stderr, "Please select delete type.\n");
    fprintf(stderr, "  %d.Delete by id\n", EI_DELETE_BY_ID);
    fprintf(stderr, "  %d.Delete all\n", EI_DELETE_ALL);
    fprintf(stderr, "==============================================\n");
}

static void print_delete_id_tips(void) {
    fprintf(stderr, "==============================================\n");
    fprintf(stderr, "Please enter face id.\n");
    fprintf(stderr, "==============================================\n");
}

static void dump_faces(void) {
    fprintf(stderr, "\n============== Face List ==============\n");
    for (int i = 0; i < face_count; i++)
        fprintf(stderr, "Face[%d]: %d\n", i, faces[i]);

    fprintf(stderr, "=========================================\n");
}

static void set_state_idle(void) {
    pthread_mutex_lock(&lock);

    work_state = STATE_IDLE;
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);
}

static void set_state_busy(void) {
    pthread_mutex_lock(&lock);

    work_state = STATE_BUSY;
    pthread_cond_signal(&cond);

    pthread_mutex_unlock(&lock);
}

static void wait_state_idle(void) {
    pthread_mutex_lock(&lock);

    while (work_state == STATE_BUSY)
        pthread_cond_wait(&cond, &lock);

    pthread_mutex_unlock(&lock);
}

__attribute__((unused)) static void draw_text(int rgb,int x, int y, char* text)
{
    gr_drawer->set_pen_color((uint8_t)((rgb&0x00ff0000)>>16),
                             (uint8_t)((rgb&0x0000ff00)>>8),
                             (uint8_t)((rgb&0x000000ff)));
    gr_drawer->draw_text(x, y, text, 0);
    gr_drawer->display();
}

static void draw_horizontal_line(int rgb, int x, int y,
                                 int w,   int h, int len)
{
    uint32_t i,j,pixel;
    struct rgb_pixel_fmt pixel_fmt;

    MK_PIXEL_FMT(pixel_fmt);

    pixel = make_pixel((uint8_t)((rgb&0x00ff0000)>>16),
                       (uint8_t)((rgb&0x0000ff00)>>8),
                       (uint8_t)((rgb&0x000000ff)), pixel_fmt);

    // Border protection
    if (y < PREVIEW_START_PORT_Y) {
        y = PREVIEW_START_PORT_Y-1;
    }
    if (y > h+PREVIEW_START_PORT_Y) {
        y = h+PREVIEW_START_PORT_Y-1;
    }
    if (x < PREVIEW_START_PORT_X) {
        x = PREVIEW_START_PORT_X-1;
    }
    if (x > w+PREVIEW_START_PORT_X) {
        x = w+PREVIEW_START_PORT_X-1;
    }
    if (len > w+PREVIEW_START_PORT_X-x) {
        len = w+PREVIEW_START_PORT_X-x-1;
    }

    // fill fb buffer
    for (i = y*w+x,j = 0; j < len; ++i,++j) {
        capt_op.ppbuf[i] = pixel;
    }
    gr_drawer->display();
}

static void draw_vertical_line(int rgb, int x, int y,
                               int w,   int h, int len)
{
    uint32_t i,j,pixel;
    struct rgb_pixel_fmt pixel_fmt;

    MK_PIXEL_FMT(pixel_fmt);

    pixel = make_pixel((uint8_t)((rgb&0x00ff0000)>>16),
                       (uint8_t)((rgb&0x0000ff00)>>8),
                       (uint8_t)((rgb&0x000000ff)), pixel_fmt);

    if (y < PREVIEW_START_PORT_Y) {
        y = PREVIEW_START_PORT_Y-1;
    }
    if (y > h+PREVIEW_START_PORT_Y) {
        y = h+PREVIEW_START_PORT_Y-1;
    }
    if (x < PREVIEW_START_PORT_X) {
        x = PREVIEW_START_PORT_X-1;
    }
    if (x > w+PREVIEW_START_PORT_X) {
        x = w+PREVIEW_START_PORT_X-1;
    }
    if (len > h+PREVIEW_START_PORT_Y-y) {
        len = h+PREVIEW_START_PORT_Y-y-1;
    }

    for (i = y*w+x,j = 0; j < len; i+=w,++j) {
        capt_op.ppbuf[i] = pixel;
    }
    gr_drawer->display();
}


/*
 * The face box is square and does not need tube rotation,
 * only need to determine the position of top, bottom, left and right
 */
static void draw_face_box(long top, long bottom, long left, long right)
{
    if (FACE_BOX_EN) {
        uint32_t start_hx1 = left;
        uint32_t start_hx2 = start_hx1;
        uint32_t start_hy1 = top;
        uint32_t start_hy2 = bottom;
        uint32_t hlen = (right-left);
        uint32_t start_vx1 = left;
        uint32_t start_vy1 = top;
        uint32_t start_vy2 = start_vy1;
        uint32_t start_vx2 = right;
        uint32_t vlen = (bottom-top);

        draw_horizontal_line(FACE_BOX_RGB, start_hx1,start_hy1,
                            DEFAULT_HEIGHT, DEFAULT_WIDTH, hlen);

        draw_horizontal_line(FACE_BOX_RGB, start_hx2,start_hy2,
                            DEFAULT_HEIGHT, DEFAULT_WIDTH, hlen);

        draw_vertical_line(FACE_BOX_RGB, start_vx1,start_vy1,
                            DEFAULT_HEIGHT, DEFAULT_WIDTH,vlen);

        draw_vertical_line(FACE_BOX_RGB, start_vx2,start_vy2,
                            DEFAULT_HEIGHT, DEFAULT_WIDTH,vlen);
    }
}

static void get_coordinate_system_h_w(uint8_t degree, uint32_t* height, uint32_t* width)
{
    switch(degree) {
        case DEGREE_0:
        case DEGREE_180:
            *height = PREVIEW_HEIGHT;
            *width  = PREVIEW_WIDTH;
        break;

        case DEGREE_90:
        case DEGREE_270:
            *height = PREVIEW_WIDTH;
            *width  = PREVIEW_HEIGHT;
        break;

        default:
            *height = PREVIEW_HEIGHT;
            *width  = PREVIEW_WIDTH;
        break;
    }
}


static void get_symmetrical_point(long a, long b, long* x, long* y)
{
    *x = 2*a - *x;
    *y = 2*b - *y;
}

static void face_coordinate_convert(ei_face_coordinate *face_coordinate)
{
    long a,b;               // Symmetrical point
    long lt_x,lt_y;         // Upper left  point
    long rb_x,rb_y;         // Lower right point
    uint32_t coorsys_height;
    uint32_t coorsys_width;
    /*
     * Scaled face box
     *
     * step 1: scaled + offset
     * step 2: upper left and lower right coordinates of box rotate 180 degrees
     */
    face_coordinate->top
                = face_coordinate->top*PREVIEW_HEIGHT/DEFAULT_HEIGHT+PREVIEW_START_PORT_Y;
    face_coordinate->bottom
                = face_coordinate->bottom*PREVIEW_HEIGHT/DEFAULT_HEIGHT+PREVIEW_START_PORT_Y;
    face_coordinate->left
                = face_coordinate->left*PREVIEW_WIDTH/DEFAULT_WIDTH+PREVIEW_START_PORT_X;
    face_coordinate->right
                = face_coordinate->right*PREVIEW_WIDTH/DEFAULT_WIDTH+PREVIEW_START_PORT_X;

    /* point symmetrical process */
    if (PREVIEW_ROTATE_DEGREE == DEGREE_180 ||
        PREVIEW_ROTATE_DEGREE == DEGREE_270) {

        // Determine the rotation point
        get_coordinate_system_h_w(PREVIEW_ROTATE_DEGREE, &coorsys_height, &coorsys_width);
        a = coorsys_width/2  + PREVIEW_START_PORT_X;
        b = coorsys_height/2 + PREVIEW_START_PORT_Y;

        // rotate face box
        lt_x = face_coordinate->left;
        lt_y = face_coordinate->top;
        rb_x = face_coordinate->right;
        rb_y = face_coordinate->bottom;
        get_symmetrical_point(a,b,&lt_x,&lt_y);
        get_symmetrical_point(a,b,&rb_x,&rb_y);
        face_coordinate->top    = rb_y;
        face_coordinate->left   = rb_x;
        face_coordinate->right  = lt_x;
        face_coordinate->bottom = lt_y;
    }
}

static void ei_face_callback(int msg, ei_face_id_t id, void* param)
{
    int score = (int)param;
    ei_face_coordinate *face_coordinate = (ei_face_coordinate*)param;

    switch(msg)
    {
        case EI_FACE_RECOGNIZE_ENROLL_SUCCESS:
            LOGI("========> EI_FACE_RECOGNIZE_ENROLL_SUCCESS , id = %ld\n", id);
            set_state_idle();
            break;

        case EI_FACE_RECOGNIZE_ENROLL_FAILED:
            LOGI("========> EI_FACE_RECOGNIZE_ENROLL_FAILED.\n");
            set_state_idle();
            break;

        case EI_FACE_RECOGNIZE_ENROLL_DUPLICATE:
            LOGI("========> EI_FACE_RECOGNIZE_ENROLL_DUPLICATE\n");
            set_state_idle();
            break;

        case EI_FACE_RECOGNIZE_ENROLL_TIMEOUT:
            LOGI("========> EI_FACE_RECOGNIZE_ENROLL_TIMEOUT\n");
            set_state_idle();
            break;

        case EI_FACE_RECOGNIZE_ENROLL_ING:
            LOGI("========> EI_FACE_RECOGNIZE_ENROLL_ING\n");
            break;

        case EI_FACE_RECOGNIZE_AUTHENTICATE_SUCCESS:
            LOGI("========> EI_FACE_RECOGNIZE_AUTHENTICATE_SUCCESS ,id = %ld, score: %d\n", id, score);
            auth_count = 0;
            ei_face_recognize_authenticate();
            // set_state_idle();
            break;
        case EI_FACE_RECOGNIZE_AUTHENTICATED_NO_MATCH:
            LOGI("========> EI_FACE_RECOGNIZE_AUTHENTICATED_NO_MATCH\n");
            auth_count = 0;
            ei_face_recognize_authenticate();
            // set_state_idle();
            break;
        case EI_FACE_RECOGNIZE_AUTHENTICATED_FAILED:
            LOGI("========> EI_FACE_RECOGNIZE_AUTHENTICATED_FAILED\n");
            if (auth_count >= AUTH_MAX_TIMES - 1) {
                LOGE("Face auth failure! Canceling...\n");
                ei_face_recognize_cancel();
                auth_count = 0;
                // set_state_idle();
            } else {
                ei_face_recognize_authenticate();
                auth_count++;
                LOGE("Face auth failure! %d try left\n", AUTH_MAX_TIMES - auth_count);
            }
            ei_face_recognize_authenticate();
            break;

        case EI_FACE_RECOGNIZE_AUTHENTICATED_TIMEOUT:
            LOGI("========> EI_FACE_RECOGNIZE_AUTHENTICATED_TIMEOUT.\n");
            set_state_idle();
            break;

        case EI_FACE_RECOGNIZE_REMOVE_SUCCESS:
            LOGI("========> EI_FACE_RECOGNIZE_REMOVE_SUCCESS id = %ld\n", id);
            break;

        case EI_FACE_RECOGNIZE_REMOVE_FAILED:
            LOGI("========> EI_FACE_RECOGNIZE_REMOVE_FAILED\n");
            break;
        case EI_FACE_RECOGNIZE_DETECT:
            LOGI("========> EI_FACE_RECOGNIZE_DETECT\n");
            face_coordinate_convert(face_coordinate);
            draw_face_box(face_coordinate->top, face_coordinate->bottom,
                          face_coordinate->left,face_coordinate->right);
        default:
            break;
    }
}

static int do_enroll(void) {
    int error = 0;

    error = ei_face_recognize_enroll();
    if (error < 0) {
        LOGE("Failedl to enroll faceprint\n");
        return -1;
    }

    return 0;
}

static int do_auth(void) {
    int error = 0;

    face_count = ei_face_recognize_get_template_info(faces);
    if (face_count <= 0) {
        LOGW("No any valid face's templete\n");
        return -1;
    }

    error = ei_face_recognize_authenticate();
    if (error < 0) {
        LOGE("Failed to auth faceprint\n");
        return -1;
    }

    return 0;
}

static int do_list(void) {
    face_count = ei_face_recognize_get_template_info(faces);
    if (face_count < 0) {
        LOGE("Failed to get template info\n");
        return -1;
    }

    dump_faces();

    return 0;
}

static int do_delete(void) {
    int error = 0;
    char id_buf[128] = {0};
    char type_buf[32] = {0};

restart:
    print_delete_tips();

    if (fgets(type_buf, sizeof(type_buf), stdin) == NULL)
        goto restart;
    if (strlen(type_buf) != 2)
        goto restart;

    int type = strtol(type_buf, NULL, 0);
    if (type > EI_DELETE_ALL || type < 0)
        goto restart;

    if (type == EI_DELETE_BY_ID) {
restart2:
        print_delete_id_tips();

        if (fgets(id_buf, sizeof(id_buf), stdin) == NULL)
            goto restart2;
        int id = strtol(id_buf, NULL, 0);

        error = ei_face_recognize_delete(id, type);
        if (error < 0)
            LOGE("Failed to delete face by id %d\n", id);

    } else if (type == EI_DELETE_ALL) {
        error = ei_face_recognize_delete(-1, type);
        if (error < 0)
            LOGE("Failed to delete all face\n");

    } else {
        LOGE("Invalid delete type\n");
        goto restart;
    }

    return error;
}

static int do_suspend(void) {

    power_manager->sleep();

    LOGI("=====> Wakeup <=====\n");

    return 0;
}

static int do_exit(void) {
    int error = 0;
    ei_face_recognize_cancel();
    error = ei_face_recognize_destroy();
    if (error)
        LOGE("Failed to close microarray library\n");

    exit(error);
}

static void do_work(void) {
    int error = 0;
    char sel_buf[64] = {0};
    volatile int action = -1;

    setbuf(stdin, NULL);

    do_list();

    for (;;) {
restart:
        if (action == AUTH_TEST) {
            while(getchar() != 'Q')
                LOGI("Enter 'Q' to exit auth\n");
            ei_face_recognize_cancel();
            set_state_idle();
            action = -1;
        }

        wait_state_idle();

        print_tips();

        while (fgets(sel_buf, sizeof(sel_buf), stdin) == NULL);

        if (strlen(sel_buf) != 2)
            goto restart;

        action = strtol(sel_buf, NULL, 0);
        if (action >= TEST_MAX || action < 0)
            goto restart;

        LOGI("Going to %s\n", test2string(action));

        if (action != LIST_TEST && action != EXIT_TEST && action != SUSPEND_TEST
                && action != DELETE_TEST)
            set_state_busy();

        switch(action) {
        case ENROLL_TEST:
            error = do_enroll();
            break;

        case AUTH_TEST:
            error = do_auth();
            break;

        case LIST_TEST:
            error = do_list();
            break;

        case DELETE_TEST:
            error = do_delete();
            break;

        case SUSPEND_TEST:
            error = do_suspend();
            break;

        case EXIT_TEST:
            error = do_exit();
            break;

        default:
            break;
        }

        if (error) {
            LOGI("Failed to %s\n", test2string(action));
            action = -1;
            set_state_idle();
        }
    }
}

static void handle_signal(int signal)
{
    ei_face_recognize_cancel();
    printf("ctrl + c\n");
    ei_face_recognize_destroy();

    exit(1);
}

static int set_system_time()
{
    struct tm _tm;
    struct timeval tv;
    time_t timep;

    _tm.tm_sec  = 0;
    _tm.tm_min  = 0;
    _tm.tm_hour = 0;
    _tm.tm_mday = 18;
    _tm.tm_mon  = 5;
    _tm.tm_year = 2018 - 1900;

    timep       = mktime(&_tm);
    tv.tv_sec   = timep;
    tv.tv_usec  = 0;

    if (settimeofday (&tv, (struct timezone *) 0) < 0) {
        LOGE("Set system datatime error!/n");
        return -1;
    }

    return 0;
}

static void image_preview(uint8_t* buf, uint32_t pixelformat, uint32_t width, uint32_t height)
{
    uint8_t *rgbbuf     = NULL;
    struct rgb_pixel_fmt pixel_fmt;
    enum AVPixelFormat src_format;

    if (V4L2_PIX_FMT_YUYV == pixelformat)
        src_format = AV_PIX_FMT_YUYV422;
    else {
        LOGE("not want pixelformat!!\n");
        return;
    }

    rgbbuf = (uint8_t *)malloc(image_size(AV_PIX_FMT_BGR24, PREVIEW_WIDTH, PREVIEW_HEIGHT));
    if(NULL == rgbbuf) {
        LOGE("malloc rgbbuf failed!!\n");
        goto out;
    }
    image_convert(buf, width, height, src_format,
                  rgbbuf, PREVIEW_WIDTH, PREVIEW_HEIGHT, AV_PIX_FMT_BGR24);

    MK_PIXEL_FMT(pixel_fmt);
    convert_rgb2pixel(rgbbuf, capt_op.ppbuf,
                      PREVIEW_WIDTH, PREVIEW_HEIGHT,
                      capt_op.fb_w, capt_op.fb_h,
                      PREVIEW_START_PORT_X, PREVIEW_START_PORT_Y,
                      pixel_fmt, PREVIEW_ROTATE_DEGREE);
    fbm->display();

out:
    if (rgbbuf != NULL){
        free(rgbbuf);
    }
}
/*
 * extract y value from yuv iamge
 */
static void yuyv_extract_y(uint8_t* yuyvbuf, uint8_t* ybuf,
                           uint32_t width,   uint32_t height,
                           uint32_t*y_width, uint32_t* y_height,
                           uint8_t degree)
{
    int i, j, k;

    /*
     * the algorithm requires that the image must be erect
     * so don't rotate the image and just rotate the face box
     *   you can see that convert face coordinate at EI_FACE_RECOGNIZE_DETECT
     * and to draw face box
     */
    switch(degree) {
        case DEGREE_0:
        case DEGREE_180:
            // y image: 320 x240
            for (i = 0, j = 0; i < width*height*2; i+=2, j++) {
                ybuf[j] = yuyvbuf[i];
            }
            *y_width  = width;
            *y_height = height;
        break;

        case DEGREE_90:
        case DEGREE_270:
            // y image: 240 x320
            for (i = width*2-1, k = 0; i >= 0; --i) {
                if (!(i%2)) {
                    for (j = 0; j < height; ++j) {
                        ybuf[k++] = yuyvbuf[j*width*2+i];
                    }
                }
            }
            *y_width  = height;
            *y_height = width;
        break;

        default:
        break;
    }
}

/*
 * default pixel format is YUYV422, pixelformat no use
 */
static void frame_receive_cb(uint8_t* buf, uint32_t pixelformat, uint32_t width, uint32_t height, uint32_t seq)
{
    uint8_t* ybuf = NULL;
    uint32_t y_width,y_height;

    if (seq == capt_op.view_channel) {
        image_preview(buf, pixelformat, width, height);
    } else if (seq == capt_op.recognize_channel) {
        ybuf = (uint8_t*)malloc(width*height);
        if (!ybuf) {
            LOGE("Failed to malloc ybuf\n");
            return;
        }
        yuyv_extract_y(buf, ybuf, width, height, &y_width, &y_height, PREVIEW_ROTATE_DEGREE);
        ei_face_recognize_handle(ybuf, y_width, y_height);
        free(ybuf);
    }
}


static int camera_init(void)
{
    int ret;
    struct capt_param_t capt_param;

    cvm = get_camera_v4l2_manager();

    capt_param.width          = DEFAULT_WIDTH;
    capt_param.height         = DEFAULT_HEIGHT;
    capt_param.bpp            = DEFAULT_BPP;
    capt_param.nbuf           = DEFAULT_NBUF;
    capt_param.io             = V4L2_METHOD_MMAP;
    capt_param.fr_cb          = (frame_receive)frame_receive_cb;

    capt_op.view_channel      = CHANNEL_SEQUEUE_COLOR;
    capt_op.recognize_channel = CHANNEL_SEQUEUE_BLACK_WHITE;

    ret = cvm->init(&capt_param);
    if (ret < 0) {
        LOGE("Failed to init camera manager\n");
        goto err_init;
    }
    ret = cvm->start();
    if (ret < 0) {
        LOGE("Failed to start.\n");
        goto err_start;
    }

    return 0;
err_start:
    cvm->deinit();
err_init:
    return -1;
}


static void camera_deinit(void)
{
    cvm->stop();
    cvm->deinit();
}

static int fb_init(void)
{
    int ret;
    fbm = get_fb_manager();
    ret = fbm->init();
    if (ret < 0) {
        LOGE("Failed to init fb\n");
        goto err_fb_init;
    }

    capt_op.ppbuf = (uint16_t *)fbm->get_fbmem();
    if (capt_op.ppbuf == NULL) {
        LOGE("Failed to get fbmem\n");
        goto err_fb_op;
    }

    capt_op.fb_h = fbm->get_screen_height();
    capt_op.fb_w = fbm->get_screen_width();

    return 0;

err_fb_op:
    fbm->deinit();
err_fb_init:
    return -1;
}

static void fb_deinit(void)
{
    fbm->deinit();
    return;
}

static int drawer_init(void)
{
    gr_drawer = get_gr_drawer();
    if (gr_drawer->init() < 0) {
        LOGE("Failed to init fb_manager\n");
        return -1;
    }

    return 0;
}

static void drawer_deinit(void)
{
    gr_drawer->deinit();
}

int main(int argc, char *argv[]) {
    int error = 0;

    power_manager = get_power_manager();
    signal_handler = _new(struct signal_handler, signal_handler);

    signal_handler->set_signal_handler(signal_handler, SIGINT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGQUIT, handle_signal);
    signal_handler->set_signal_handler(signal_handler, SIGTERM, handle_signal);

    ei_face_config.rotate_degree        = PREVIEW_ROTATE_DEGREE;
    ei_face_config.duplicate_threshold  = FACE_DUPLICATE_THRESHOLD;
    ei_face_config.max_enroll_face_num  = MAX_ENROLL_FACE_NUM;
    ei_face_config.enroll_timeout       = ENROLL_TIMEOUT;
    ei_face_config.authenticate_timeout = AUTH_TIMEOUT;

    memcpy(ei_face_config.file_path, get_user_system_dir(getuid()),
                                sizeof(ei_face_config.file_path));
    strcpy(ei_face_config.firmware_path, FIRMWARE_BASE_PATH);
    set_system_time();

    error = camera_init();
    if (error < 0) {
        LOGE("Failed to init camera\n");
        goto err_camera;
    }
    error = fb_init();
    if (error < 0) {
        LOGE("Failed to init fb\n");
        goto err_fb;
    }
    error = drawer_init();
    if (error < 0) {
        LOGE("Failed to init fb\n");
        goto err_drawer;
    }


    error = ei_face_recognize_init(ei_face_callback, &ei_face_config);
    if (error < 0) {
        LOGE("Failed to init\n");
        goto err_face_recognize;
    }

    gr_drawer->set_pen_color(0xFF,0xFF,0xFF);
    gr_drawer->fill_screen();

    do_work();

    while (1)
        sleep(1000);

    camera_deinit();
    fb_deinit();
    drawer_deinit();

    return 0;

err_face_recognize:
    drawer_deinit();
err_drawer:
    fb_deinit();
err_fb:
    camera_deinit();
err_camera:
    return -1;
}
