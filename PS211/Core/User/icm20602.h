
#ifndef _icm20602_h_
#define _icm20602_h_

#include "main.h"

#define ICM20602_CS_Clr()  	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_RESET)
#define ICM20602_CS_Set()  	HAL_GPIO_WritePin(IMU_CS_GPIO_Port, IMU_CS_Pin, GPIO_PIN_SET)

typedef enum
{
    ICM20602_ACC_SAMPLE_SGN_2G ,                                                // 加速度计量程 ±2G  (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
    ICM20602_ACC_SAMPLE_SGN_4G ,                                                // 加速度计量程 ±4G  (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
    ICM20602_ACC_SAMPLE_SGN_8G ,                                                // 加速度计量程 ±8G  (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
    ICM20602_ACC_SAMPLE_SGN_16G,                                                // 加速度计量程 ±16G (ACC = Accelerometer 加速度计) (SGN = signum 带符号数 表示正负范围) (G = g 重力加速度 g≈9.80 m/s^2)
}icm20602_acc_sample_config;

typedef enum
{
    ICM20602_GYRO_SAMPLE_SGN_250DPS ,                                           // 陀螺仪量程 ±250DPS  (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
    ICM20602_GYRO_SAMPLE_SGN_500DPS ,                                           // 陀螺仪量程 ±500DPS  (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
    ICM20602_GYRO_SAMPLE_SGN_1000DPS,                                           // 陀螺仪量程 ±1000DPS (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
    ICM20602_GYRO_SAMPLE_SGN_2000DPS,                                           // 陀螺仪量程 ±2000DPS (GYRO = Gyroscope 陀螺仪) (SGN = signum 带符号数 表示正负范围) (DPS = Degree Per Second 角速度单位 °/S)
}icm20602_gyro_sample_config;

#define ICM20602_ACC_SAMPLE_DEFAULT     ( ICM20602_ACC_SAMPLE_SGN_4G )          // 在这设置默认的 加速度计 初始化量程
#define ICM20602_GYRO_SAMPLE_DEFAULT    ( ICM20602_GYRO_SAMPLE_SGN_2000DPS )    // 在这设置默认的 陀螺仪   初始化量程

#define ICM20602_TIMEOUT_COUNT      ( 0x00FF )                                  // ICM20602 超时计数

//================================================定义 ICM20602 内部地址================================================
#define ICM20602_DEV_ADDR           ( 0x69 )                                    // SA0接地：0x68 SA0上拉：0x69 模块默认上拉
#define ICM20602_SPI_W              ( 0x00 )
#define ICM20602_SPI_R              ( 0x80 )

#define ICM20602_XG_OFFS_TC_H       ( 0x04 )
#define ICM20602_XG_OFFS_TC_L       ( 0x05 )
#define ICM20602_YG_OFFS_TC_H       ( 0x07 )
#define ICM20602_YG_OFFS_TC_L       ( 0x08 )
#define ICM20602_ZG_OFFS_TC_H       ( 0x0A )
#define ICM20602_ZG_OFFS_TC_L       ( 0x0B )
#define ICM20602_SELF_TEST_X_ACCEL  ( 0x0D )
#define ICM20602_SELF_TEST_Y_ACCEL  ( 0x0E )
#define ICM20602_SELF_TEST_Z_ACCEL  ( 0x0F )
#define ICM20602_XG_OFFS_USRH       ( 0x13 )
#define ICM20602_XG_OFFS_USRL       ( 0x14 )
#define ICM20602_YG_OFFS_USRH       ( 0x15 )
#define ICM20602_YG_OFFS_USRL       ( 0x16 )
#define ICM20602_ZG_OFFS_USRH       ( 0x17 )
#define ICM20602_ZG_OFFS_USRL       ( 0x18 )
#define ICM20602_SMPLRT_DIV         ( 0x19 )
#define ICM20602_CONFIG             ( 0x1A )
#define ICM20602_GYRO_CONFIG        ( 0x1B )
#define ICM20602_ACCEL_CONFIG       ( 0x1C )
#define ICM20602_ACCEL_CONFIG_2     ( 0x1D )
#define ICM20602_LP_MODE_CFG        ( 0x1E )
#define ICM20602_ACCEL_WOM_X_THR    ( 0x20 )
#define ICM20602_ACCEL_WOM_Y_THR    ( 0x21 )
#define ICM20602_ACCEL_WOM_Z_THR    ( 0x22 )
#define ICM20602_FIFO_EN            ( 0x23 )
#define ICM20602_FSYNC_INT          ( 0x36 )
#define ICM20602_INT_PIN_CFG        ( 0x37 )
#define ICM20602_INT_ENABLE         ( 0x38 )
#define ICM20602_FIFO_WM_INT_STATUS ( 0x39 )
#define ICM20602_INT_STATUS         ( 0x3A )
#define ICM20602_ACCEL_XOUT_H       ( 0x3B )
#define ICM20602_ACCEL_XOUT_L       ( 0x3C )
#define ICM20602_ACCEL_YOUT_H       ( 0x3D )
#define ICM20602_ACCEL_YOUT_L       ( 0x3E )
#define ICM20602_ACCEL_ZOUT_H       ( 0x3F )
#define ICM20602_ACCEL_ZOUT_L       ( 0x40 )
#define ICM20602_TEMP_OUT_H         ( 0x41 )
#define ICM20602_TEMP_OUT_L         ( 0x42 )
#define ICM20602_GYRO_XOUT_H        ( 0x43 )
#define ICM20602_GYRO_XOUT_L        ( 0x44 )
#define ICM20602_GYRO_YOUT_H        ( 0x45 )
#define ICM20602_GYRO_YOUT_L        ( 0x46 )
#define ICM20602_GYRO_ZOUT_H        ( 0x47 )
#define ICM20602_GYRO_ZOUT_L        ( 0x48 )
#define ICM20602_SELF_TEST_X_GYRO   ( 0x50 )
#define ICM20602_SELF_TEST_Y_GYRO   ( 0x51 )
#define ICM20602_SELF_TEST_Z_GYRO   ( 0x52 )
#define ICM20602_FIFO_WM_TH1        ( 0x60 )
#define ICM20602_FIFO_WM_TH2        ( 0x61 )
#define ICM20602_SIGNAL_PATH_RESET  ( 0x68 )
#define ICM20602_ACCEL_INTEL_CTRL   ( 0x69 )
#define ICM20602_USER_CTRL          ( 0x6A )
#define ICM20602_PWR_MGMT_1         ( 0x6B )
#define ICM20602_PWR_MGMT_2         ( 0x6C )
#define ICM20602_I2C_IF             ( 0x70 )
#define ICM20602_FIFO_COUNTH        ( 0x72 )
#define ICM20602_FIFO_COUNTL        ( 0x73 )
#define ICM20602_FIFO_R_W           ( 0x74 )
#define ICM20602_WHO_AM_I           ( 0x75 )
#define ICM20602_XA_OFFSET_H        ( 0x77 )
#define ICM20602_XA_OFFSET_L        ( 0x78 )
#define ICM20602_YA_OFFSET_H        ( 0x7A )
#define ICM20602_YA_OFFSET_L        ( 0x7B )
#define ICM20602_ZA_OFFSET_H        ( 0x7D )
#define ICM20602_ZA_OFFSET_L        ( 0x7E )
//================================================定义 ICM20602 内部地址================================================

extern int16_t icm20602_gyro_x, icm20602_gyro_y, icm20602_gyro_z;                 // 三轴陀螺仪数据
extern int16_t icm20602_acc_x, icm20602_acc_y, icm20602_acc_z;                    // 三轴加速度计数据
extern float icm20602_transition_factor[2];

void    icm20602_get_acc            (void);
void    icm20602_get_gyro           (void);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     将 ICM20602 加速度计数据转换为实际物理数据
// 参数说明     acc_value       任意轴的加速度计数据
// 返回参数     void
// 使用示例     float data = icm20602_acc_transition(icm20602_acc_x);           // 单位为 g(m/s^2)
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
#define icm20602_acc_transition(acc_value)      ((float)acc_value / icm20602_transition_factor[0])

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     将 ICM20602 陀螺仪数据转换为实际物理数据
// 参数说明     gyro_value      任意轴的陀螺仪数据
// 返回参数     void
// 使用示例     float data = icm20602_gyro_transition(icm20602_gyro_x);         // 单位为 °/s
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
#define icm20602_gyro_transition(gyro_value)    ((float)gyro_value / icm20602_transition_factor[1])

uint8_t icm20602_init(void);

void icm20602_init_spi(void);
void get_icm20602_accdata_spi(void);
void get_icm20602_gyro_spi(void);
void icm20602_data_change(void);

#endif

