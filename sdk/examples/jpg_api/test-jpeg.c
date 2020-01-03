#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <jpa_api/jpeg.h>


#define WIDTH                           (640)
#define HEIGHT                          (480)
extern void* jz_jpeg_encode_init(int width,int height);
extern void jz_jpeg_encode_deinit(void *handle);
extern int yuv422_to_jpeg(void *handle,unsigned char *input_image, FILE *fp, int width, int height, int quality);


int main(int argc, char *argv[])
{

    void *jz_jpeg = NULL;

    if(argc < 3) {
        printf("Usage: %s src wxh dstjpg\n", argv[0]);
        return -1;
    }


    int width = WIDTH;
    int height = HEIGHT;

    int quality = 2;

    char *src = argv[1];
    char *dst = argv[2];

    printf("src: %s, dst: %s\n", src, dst);

    FILE *yuv_src = fopen(src, "r+");
    if(yuv_src == NULL) {
        perror("fopen");
    }

    FILE *dst_jpeg = fopen(dst, "w+");
    if(dst_jpeg == NULL) {
        perror("fopen !");
    }

    jz_jpeg = jz_jpeg_encode_init(width, height);

    struct stat tmp;
    lstat(src, &tmp);

    unsigned int len = tmp.st_size;

    unsigned char *frame = (unsigned char *)JZMalloc(128, width*height*2);
    if(frame == NULL) {
        printf("Get src buffer handle failed!\n");
        return -1;
    }

    int ret;
    int read = len;

    unsigned char *p = frame;
    printf("###### src file len :%d \n", len);
    while(read > 0) {

        ret = fread(p, 1, read, yuv_src);
        if(ret < 0) {
            perror("fread");
            break;
        }

        read -= ret;
        p += ret;


        printf("read ... %d, ret: %d\n", read, ret);
    }

    printf("save to jpeg file :\n");

    yuv422_to_jpeg(jz_jpeg, frame, dst_jpeg, width, height, quality);

    printf("encode deinit\n");

    jz_jpeg_encode_deinit(jz_jpeg);

    free(dst_jpeg);
}


