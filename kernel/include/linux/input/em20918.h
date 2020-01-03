#ifndef _EM20918_H_
#define _EM20918_H_


#define DEF_ZERO                        0X0
#define DEF_0XFF                        0XFF



/* EM20918 reg define */
#define EM20918_REG_PID                 0X00
#define EM20918_REG_CONFIG              0X01
#define EM20918_REG_INTERRUPT           0x02
#define EM20918_REG_PS_LT               0X03
#define EM20918_REG_PS_HT               0X04
#define EM20918_REG_PS_DATA             0X08
#define EM20918_REG_RESET               0x0E
#define EM20918_REG_OFFSET              0X0F

/* EM20918 reg operate bit offset */
#define PS_EN_BIT_OFFSET                 7
#define PS_SLP_BIT_OFFSET                6
#define PS_DR_BIT_OFFSET                 3
#define PS_FLAG_BIT_OFFSET               7
#define ALS_FLAG_BIT_OFFSET              3 /* em20918 no als */
#define OFFSET_CTRL_BIT_OFFSET           1

/* EM20918 reg operate bit mask */
#define PS_EN_MASK                      (0x1<<PS_EN_BIT_OFFSET)
#define PS_SLP_MASK                     (0x1<<PS_SLP_BIT_OFFSET)
#define PS_DR_MASK                      (0x7<<PS_DR_BIT_OFFSET)
#define PS_FLAG_MASK                    (0x1<<PS_FLAG_BIT_OFFSET)
#define ALS_FLAG_MASK                   (0x1<<ALS_FLAG_BIT_OFFSET)
#define OFFSET_CTRL_MASK                (0xF<<OFFSET_CTRL_BIT_OFFSET)

/* EM20918 reg operate bit value*/
#define EM20918_PID                     0x31

#define PS_EN_DIS                       0x0
#define PS_EN_EN                        0x1
#define PS_SLP_100MS                    0x0
#define PS_SLP_800MS                    0x1
#define PS_DR_15MA                      0x0
#define PS_DR_30MA                      0x1
#define PS_DR_60MA                      0x2
#define PS_DR_120MA                     0x3
#define PS_DR_25MA                      0x4
#define PS_DR_50MA                      0x5
#define PS_DR_100MA                     0x6
#define PS_DR_200MA                     0x7

#define PS_FLAG_INT_CLEAR               0x0
#define PS_FLAG_INT_OCCURRED            0x1
#define ALS_FLAG_INT_CLEAR              0x0
#define ALS_FLAG_INT_OCCURRED           0x1

#define OFFSET_CTRL_ne0                 0x0
#define OFFSET_CTRL_ne32                0x1
#define OFFSET_CTRL_ne64                0x2
#define OFFSET_CTRL_ne96                0x3
#define OFFSET_CTRL_ne128               0x4
#define OFFSET_CTRL_ne160               0x5
#define OFFSET_CTRL_ne192               0x6
#define OFFSET_CTRL_ne224               0x7
#define OFFSET_CTRL_ne256               0x8
#define OFFSET_CTRL_ne288               0x9
#define OFFSET_CTRL_ne320               0xA
#define OFFSET_CTRL_ne352               0xB
#define OFFSET_CTRL_ne384               0xC
#define OFFSET_CTRL_ne416               0xD
#define OFFSET_CTRL_ne448               0xE
#define OFFSET_CTRL_ne480               0xF




struct em20918_platform_data {
    int int_pin;
    int interrupt_key_code;
    int capture_key_code;
};


struct reg_op_t {
    u8 op;                     /* reg operate(read or write) */
    u8 reg_addr;
    u8 val;
};


struct capture_setting_t {
    //u8 detect_rate             /* detect rate(100ms; 800ms) */
    u32 capture_time_ms;       /* capture total time(ms) */
    u32 capture_num;           /* capture number */
};


struct reg_status_t {
    u8 reg_addr;
    u8 default_val;
    u8 current_val;
};


#endif

