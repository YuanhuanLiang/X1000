#ifndef _cat9555_h_
#define _cat9555_h_

#ifdef __cplusplus
extern "C" {
#endif

enum{
CAT9555_CMD_INPUT_P0,
CAT9555_CMD_INPUT_P1,
CAT9555_CMD_OUTPUT_P0,
CAT9555_CMD_OUTPUT_P1,
CAT9555_CMD_POLARITY_INTVERT_P0,
CAT9555_CMD_POLARITY_INTVERT_P1,
CAT9555_CMD_CONF_P0,
CAT9555_CMD_CONF_P1,
};

struct cat9555_p_data{
    unsigned char p0_default_out;
    unsigned char p1_default_out;
    
    unsigned char p0_polarity_invert;
    unsigned char p1_polarity_invert;
    
    unsigned char p0_conf;
    unsigned char p1_conf;

    unsigned char* remap_offset;
    unsigned char remap_count;
};

#ifdef __cplusplus
}
#endif

#endif

/*******************************************************************************
    END OF FILE
*******************************************************************************/
