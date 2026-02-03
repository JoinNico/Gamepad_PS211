#define LOG_TAG    "icm20602.c"
#include "icm20602.h"
#include "spi.h"

#include "../../3rdParty/elog/elog.h"

// int16_t icm20602_gyro_x = 0, icm20602_gyro_y = 0, icm20602_gyro_z = 0;            // 三轴陀螺仪数据      gyro (陀螺仪)
// int16_t icm20602_acc_x = 0, icm20602_acc_y = 0, icm20602_acc_z = 0;               // 三轴加速度计数据    acc (accelerometer 加速度计)
// float icm20602_transition_factor[2] = {4096, 16.4};
//
// //-------------------------------------------------------------------------------------------------------------------
// // 函数简介     ICM20602 写寄存器
// // 参数说明     reg             寄存器地址
// // 参数说明     data            数据
// // 返回参数     void
// // 使用示例     icm20602_write_register(ICM20602_PWR_MGMT_1, 0x80);
// // 备注信息     内部调用
// //-------------------------------------------------------------------------------------------------------------------
// static void icm20602_write_register (uint8_t reg, uint8_t data)
// {
//     uint8_t tx_data[2];
//     ICM20602_CS_Clr();
//
//     tx_data[0] = reg | ICM20602_SPI_W;
//     tx_data[1] = data;
//     HAL_SPI_Transmit(&hspi3, tx_data,2,100);
//
//     ICM20602_CS_Set();
// }
//
// //-------------------------------------------------------------------------------------------------------------------
// // 函数简介     ICM20602 读寄存器
// // 参数说明     reg             寄存器地址
// // 返回参数     uint8           读取到的数据
// // 使用示例     icm20602_read_register(ICM20602_WHO_AM_I);
// // 备注信息     内部调用
// //-------------------------------------------------------------------------------------------------------------------
// static uint8_t icm20602_read_register(uint8_t reg)
// {
//     uint8_t tx_buffer[2];
//     uint8_t rx_buffer[2];
//
//     // 设置读命令：寄存器地址 | 读标志位
//     tx_buffer[0] = reg | ICM20602_SPI_R;  // 假设ICM20602_SPI_R = 0x80
//     tx_buffer[1] = 0x00;  // 虚拟字节，用于接收数据
//
//     ICM20602_CS_Clr();  // 拉低片选
//
//     // 同时发送和接收数据
//     HAL_SPI_TransmitReceive(&hspi3, tx_buffer, rx_buffer, 2, HAL_MAX_DELAY);
//
//     ICM20602_CS_Set();  // 拉高片选
//
//     return rx_buffer[1];  // 接收到的数据在第二个字节
// }
//
// //-------------------------------------------------------------------------------------------------------------------
// // 函数简介     ICM20602 连续读寄存器
// // 参数说明     reg             起始寄存器地址
// // 参数说明     data            数据缓冲区
// // 参数说明     len             要读取的数据长度（字节数）
// // 返回参数     void
// // 使用示例     icm20602_read_registers(ICM20602_ACCEL_XOUT_H, dat, 6);
// // 备注信息     内部调用
// //-------------------------------------------------------------------------------------------------------------------
// static void icm20602_read_registers(uint8_t reg, uint8_t *data, uint32_t len)
// {
//     uint8_t tx_buffer[len + 1];
//     uint8_t rx_buffer[len + 1];
//
//     // 准备发送数据：起始地址 + 读标志位
//     tx_buffer[0] = reg | 0x80;  // 设置读位（最高位为1）
//
//     // 剩余字节填充虚拟数据，用于接收
//     for (uint32_t i = 1; i <= len; i++)
//     {
//         tx_buffer[i] = 0x00;  // 虚拟字节
//     }
//
//     // 拉低片选，开始SPI通信
//     ICM20602_CS_Clr();  // 或 HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
//
//     // 执行SPI传输
//     HAL_SPI_TransmitReceive(&hspi3, tx_buffer, rx_buffer, len + 1, 1000);
//
//     // 拉高片选，结束SPI通信
//     ICM20602_CS_Set();  // 或 HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
//
//     // 复制接收数据到输出缓冲区（跳过第一个字节）
//     for (uint32_t i = 0; i < len; i++)
//     {
//         data[i] = rx_buffer[i + 1];
//     }
// }
//
//
// //-------------------------------------------------------------------------------------------------------------------
// // 函数简介     ICM20602 自检
// // 参数说明     void
// // 返回参数     uint8           1-自检失败 0-自检成功
// // 使用示例     icm20602_self_check();
// // 备注信息     内部调用
// //-------------------------------------------------------------------------------------------------------------------
// static uint8_t icm20602_self_check (void)
// {
//     uint8_t dat = 0, return_state = 0;
//     uint16_t timeout_count = 0;
//
//     while(0x12 != dat)                                                          // 判断 ID 是否正确
//     {
//         if(ICM20602_TIMEOUT_COUNT < timeout_count ++)
//         {
//             return_state =  1;
//             break;
//         }
//         dat = icm20602_read_register(ICM20602_WHO_AM_I);
//         HAL_Delay(10);
//     }
//     return return_state;
// }
//
// //-------------------------------------------------------------------------------------------------------------------
// // 函数简介     获取 ICM20602 加速度计数据
// // 参数说明     void
// // 返回参数     void
// // 使用示例     icm20602_get_acc();                                             // 执行该函数后，直接查看对应的变量即可
// // 备注信息
// //-------------------------------------------------------------------------------------------------------------------
// void icm20602_get_acc (void)
// {
//     uint8_t dat[6];
//
//     icm20602_read_registers(ICM20602_ACCEL_XOUT_H, dat, 6);
//     icm20602_acc_x = (int16_t)(((uint16_t)dat[0] << 8 | dat[1]));
//     icm20602_acc_y = (int16_t)(((uint16_t)dat[2] << 8 | dat[3]));
//     icm20602_acc_z = (int16_t)(((uint16_t)dat[4] << 8 | dat[5]));
// }
//
// //-------------------------------------------------------------------------------------------------------------------
// // 函数简介     获取ICM20602陀螺仪数据
// // 参数说明     void
// // 返回参数     void
// // 使用示例     icm20602_get_gyro();                                            // 执行该函数后，直接查看对应的变量即可
// // 备注信息
// //-------------------------------------------------------------------------------------------------------------------
// void icm20602_get_gyro (void)
// {
//     uint8_t dat[6];
//
//     icm20602_read_registers(ICM20602_GYRO_XOUT_H, dat, 6);
//     icm20602_gyro_x = (int16_t)(((uint16_t)dat[0] << 8 | dat[1]));
//     icm20602_gyro_y = (int16_t)(((uint16_t)dat[2] << 8 | dat[3]));
//     icm20602_gyro_z = (int16_t)(((uint16_t)dat[4] << 8 | dat[5]));
// }
//
// //-------------------------------------------------------------------------------------------------------------------
// // 函数简介     初始化 ICM20602
// // 参数说明     void
// // 返回参数     uint8_t           1-初始化失败 0-初始化成功
// // 使用示例     icm20602_init();
// // 备注信息
// //-------------------------------------------------------------------------------------------------------------------
// uint8_t icm20602_init (void)
// {
//     uint8_t val = 0x0, return_state = 0;
//     uint16_t timeout_count = 0;
//
//     HAL_Delay(10);                                                        // 上电延时
//
//     do
//     {
//         if(icm20602_self_check())
//         {
//             // 如果程序在输出了断言信息 并且提示出错位置在这里
//             // 那么就是 ICM20602 自检出错并超时退出了
//             // 检查一下接线有没有问题 如果没问题可能就是坏了
//             log_e("icm20602 self check error.");
//             return_state = 1;
//             break;
//         }
//
//         icm20602_write_register(ICM20602_PWR_MGMT_1, 0x80);                     // 复位设备
//         HAL_Delay(2);
//
//         do
//         {                                                                       // 等待复位成功
//             val = icm20602_read_register(ICM20602_PWR_MGMT_1);
//             if(ICM20602_TIMEOUT_COUNT < timeout_count ++)
//             {
//                 // 如果程序在输出了断言信息 并且提示出错位置在这里
//                 // 那么就是 ICM20602 自检出错并超时退出了
//                 // 检查一下接线有没有问题 如果没问题可能就是坏了
//                 log_e("icm20602 reset error.");
//                 return_state = 1;
//                 break;
//             }
//         }while(0x41 != val);
//         if(1 == return_state)
//         {
//             break;
//         }
//
//         icm20602_write_register(ICM20602_PWR_MGMT_1,     0x01);                 // 时钟设置
//         icm20602_write_register(ICM20602_PWR_MGMT_2,     0x00);                 // 开启陀螺仪和加速度计
//         icm20602_write_register(ICM20602_CONFIG,         0x01);                 // 176HZ 1KHZ
//         icm20602_write_register(ICM20602_SMPLRT_DIV,     0x07);                 // 采样速率 SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
//
//         // ICM20602_ACCEL_CONFIG 寄存器
//         // 设置为 0x00 加速度计量程为 ±2  g   获取到的加速度计数据除以 16384  可以转化为带物理单位的数据 单位 g(m/s^2)
//         // 设置为 0x08 加速度计量程为 ±4  g   获取到的加速度计数据除以 8192   可以转化为带物理单位的数据 单位 g(m/s^2)
//         // 设置为 0x10 加速度计量程为 ±8  g   获取到的加速度计数据除以 4096   可以转化为带物理单位的数据 单位 g(m/s^2)
//         // 设置为 0x18 加速度计量程为 ±16 g   获取到的加速度计数据除以 2048   可以转化为带物理单位的数据 单位 g(m/s^2)
//         switch(ICM20602_ACC_SAMPLE_DEFAULT)
//         {
//             default:
//             {
//                 log_e("ICM20602_ACC_SAMPLE_DEFAULT set error.");
//                 return_state = 1;
//             }break;
//             case ICM20602_ACC_SAMPLE_SGN_2G:
//             {
//                 icm20602_write_register(ICM20602_ACCEL_CONFIG, 0x00);
//                 icm20602_transition_factor[0] = 16384;
//             }break;
//             case ICM20602_ACC_SAMPLE_SGN_4G:
//             {
//                 icm20602_write_register(ICM20602_ACCEL_CONFIG, 0x08);
//                 icm20602_transition_factor[0] = 8192;
//             }break;
//             case ICM20602_ACC_SAMPLE_SGN_8G:
//             {
//                 icm20602_write_register(ICM20602_ACCEL_CONFIG, 0x10);
//                 icm20602_transition_factor[0] = 4096;
//             }break;
//             case ICM20602_ACC_SAMPLE_SGN_16G:
//             {
//                 icm20602_write_register(ICM20602_ACCEL_CONFIG, 0x18);
//                 icm20602_transition_factor[0] = 2048;
//             }break;
//         }
//         if(1 == return_state)
//         {
//             break;
//         }
//
//         // ICM20602_GYRO_CONFIG 寄存器
//         // 设置为 0x00 陀螺仪量程为 ±250  dps    获取到的陀螺仪数据除以 131     可以转化为带物理单位的数据 单位为 °/s
//         // 设置为 0x08 陀螺仪量程为 ±500  dps    获取到的陀螺仪数据除以 65.5    可以转化为带物理单位的数据 单位为 °/s
//         // 设置为 0x10 陀螺仪量程为 ±1000 dps    获取到的陀螺仪数据除以 32.8    可以转化为带物理单位的数据 单位为 °/s
//         // 设置为 0x18 陀螺仪量程为 ±2000 dps    获取到的陀螺仪数据除以 16.4    可以转化为带物理单位的数据 单位为 °/s
//         switch(ICM20602_GYRO_SAMPLE_DEFAULT)
//         {
//             default:
//             {
//                 log_e("ICM20602_GYRO_SAMPLE_DEFAULT set error.");
//                 return_state = 1;
//             }break;
//             case ICM20602_GYRO_SAMPLE_SGN_250DPS:
//             {
//                 icm20602_write_register(ICM20602_GYRO_CONFIG, 0x00);
//                 icm20602_transition_factor[1] = 131.0;
//             }break;
//             case ICM20602_GYRO_SAMPLE_SGN_500DPS:
//             {
//                 icm20602_write_register(ICM20602_GYRO_CONFIG, 0x08);
//                 icm20602_transition_factor[1] = 65.5;
//             }break;
//             case ICM20602_GYRO_SAMPLE_SGN_1000DPS:
//             {
//                 icm20602_write_register(ICM20602_GYRO_CONFIG, 0x10);
//                 icm20602_transition_factor[1] = 32.8;
//             }break;
//             case ICM20602_GYRO_SAMPLE_SGN_2000DPS:
//             {
//                 icm20602_write_register(ICM20602_GYRO_CONFIG, 0x18);
//                 icm20602_transition_factor[1] = 16.4;
//             }break;
//         }
//         if(1 == return_state)
//         {
//             break;
//         }
//
//         icm20602_write_register(ICM20602_ACCEL_CONFIG_2, 0x03);// Average 4 samples   44.8HZ   //0x23 Average 16 samples
//     }while(0);
//     return return_state;
// }

/*********************************************************************************************************************
 * COPYRIGHT NOTICE
 * Copyright (c) 2018,逐飞科技
 * All rights reserved.
 * 技术讨论QQ群：一群：179029047(已满)  二群：244861897
 *
 * 以下所有内容版权均属逐飞科技所有，未经允许不得用于商业用途，
 * 欢迎各位使用并传播本程序，修改内容时必须保留逐飞科技的版权声明。
 *
 * @file       		ICM20602
 * @company	   		成都逐飞科技有限公司
 * @author     		逐飞科技(QQ3184284598)
 * @version    		查看doc内version文件 版本说明
 * @Software 		IAR 8.3 or MDK 5.24
 * @Taobao   		https://seekfree.taobao.com/
 * @date       		2019-04-30
 * @note
					接线定义：
					------------------------------------
					SCL                 查看STM32F103ZETx HAL ICM20602 Hardware SPI.ioc文件
					SDA                 查看STM32F103ZETx HAL ICM20602 Hardware SPI.ioc文件
					------------------------------------
 ********************************************************************************************************************/


int16_t icm_gyro_x,icm_gyro_y,icm_gyro_z;
int16_t icm_acc_x,icm_acc_y,icm_acc_z;

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ICM20602 SPI写寄存器
//  @param      cmd     寄存器地址
//  @param      val     需要写入的数据
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void icm_spi_w_reg_byte(uint8_t cmd, uint8_t val)
{
    uint8_t dat[2];
    ICM20602_CS_Clr();

    dat[0] = cmd | ICM20602_SPI_W;
    dat[1] = val;
    HAL_SPI_Transmit(&hspi3,dat,2,500);

    ICM20602_CS_Set();
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ICM20602 SPI读寄存器
//  @param      cmd     寄存器地址
//  @param      *val    接收数据的地址
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------

void icm_spi_r_reg_byte(uint8_t cmd, uint8_t *val)
{
    uint8_t dat[2];

    ICM20602_CS_Clr();
    dat[0] = cmd | ICM20602_SPI_R;
    dat[1] = *val;
	HAL_SPI_TransmitReceive(&hspi3,dat,dat,2,500);

    ICM20602_CS_Set();

    *val = dat[1];
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      ICM20602 SPI多字节读寄存器
//  @param      *val    接收数据的地址
//  @param      num     读取数量
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void icm_spi_r_reg_bytes(uint8_t * val, uint8_t num)
{
    ICM20602_CS_Clr();
	HAL_SPI_TransmitReceive(&hspi1,val,val,num,500);
    ICM20602_CS_Set();
}


//-------------------------------------------------------------------------------------------------------------------
//  @brief      ICM20602自检函数
//  @param      NULL
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void icm20602_self3_check(void)
{
    uint8_t dat=0;
    while(0x12 != dat)   //读取ICM20602 ID
    {
        icm_spi_r_reg_byte(ICM20602_WHO_AM_I,&dat);
        HAL_Delay(10);
    }

}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      初始化ICM20602
//  @param      NULL
//  @return     void
//  @since      v1.0
//  Sample usage:
//-------------------------------------------------------------------------------------------------------------------
void icm20602_init_spi(void)
{
    uint8_t val = 0x0;

    HAL_Delay(10);  //上电延时


    icm20602_self3_check();//检测

    icm_spi_w_reg_byte(ICM20602_PWR_MGMT_1,0x80);//复位设备
    HAL_Delay(2);
    do
    {//等待复位成功
        icm_spi_r_reg_byte(ICM20602_PWR_MGMT_1,&val);
    }while(0x41 != val);

    icm_spi_w_reg_byte(ICM20602_PWR_MGMT_1,     0x01);            //时钟设置
    icm_spi_w_reg_byte(ICM20602_PWR_MGMT_2,     0x00);            //开启陀螺仪和加速度计
    icm_spi_w_reg_byte(ICM20602_CONFIG,         0x01);            //176HZ 1KHZ
    icm_spi_w_reg_byte(ICM20602_SMPLRT_DIV,     0x07);            //采样速率 SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
    icm_spi_w_reg_byte(ICM20602_GYRO_CONFIG,    0x18);            //±2000 dps
    icm_spi_w_reg_byte(ICM20602_ACCEL_CONFIG,   0x10);            //±8g
    icm_spi_w_reg_byte(ICM20602_ACCEL_CONFIG_2, 0x03);            //Average 4 samples   44.8HZ   //0x23 Average 16 samples
	//ICM20602_GYRO_CONFIG寄存器
    //设置为:0x00 陀螺仪量程为:±250 dps     获取到的陀螺仪数据除以131           可以转化为带物理单位的数据， 单位为：°/s
    //设置为:0x08 陀螺仪量程为:±500 dps     获取到的陀螺仪数据除以65.5          可以转化为带物理单位的数据，单位为：°/s
    //设置为:0x10 陀螺仪量程为:±1000dps     获取到的陀螺仪数据除以32.8          可以转化为带物理单位的数据，单位为：°/s
    //设置为:0x18 陀螺仪量程为:±2000dps     获取到的陀螺仪数据除以16.4          可以转化为带物理单位的数据，单位为：°/s

    //ICM20602_ACCEL_CONFIG寄存器
    //设置为:0x00 加速度计量程为:±2g          获取到的加速度计数据 除以16384      可以转化为带物理单位的数据，单位：g(m/s^2)
    //设置为:0x08 加速度计量程为:±4g          获取到的加速度计数据 除以8192       可以转化为带物理单位的数据，单位：g(m/s^2)
    //设置为:0x10 加速度计量程为:±8g          获取到的加速度计数据 除以4096       可以转化为带物理单位的数据，单位：g(m/s^2)
    //设置为:0x18 加速度计量程为:±16g         获取到的加速度计数据 除以2048       可以转化为带物理单位的数据，单位：g(m/s^2)
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      获取ICM20602加速度计数据
//  @param      NULL
//  @return     void
//  @since      v1.0
//  Sample usage:				执行该函数后，直接查看对应的变量即可
//-------------------------------------------------------------------------------------------------------------------
void get_icm20602_accdata_spi(void)
{
    struct
    {
        uint8_t reg;
        uint8_t dat[6];
    }buf;

    buf.reg = ICM20602_ACCEL_XOUT_H | ICM20602_SPI_R;

    icm_spi_r_reg_bytes(&buf.reg, 7);
    icm_acc_x = (int16_t)(((uint16_t)buf.dat[0]<<8 | buf.dat[1]));
    icm_acc_y = (int16_t)(((uint16_t)buf.dat[2]<<8 | buf.dat[3]));
    icm_acc_z = (int16_t)(((uint16_t)buf.dat[4]<<8 | buf.dat[5]));
}

//-------------------------------------------------------------------------------------------------------------------
//  @brief      获取ICM20602陀螺仪数据
//  @param      NULL
//  @return     void
//  @since      v1.0
//  Sample usage:				执行该函数后，直接查看对应的变量即可
//-------------------------------------------------------------------------------------------------------------------
void get_icm20602_gyro_spi(void)
{
    struct
    {
        uint8_t reg;
        uint8_t dat[6];
    }buf;

    buf.reg = ICM20602_GYRO_XOUT_H | ICM20602_SPI_R;

    icm_spi_r_reg_bytes(&buf.reg, 7);
    icm_gyro_x = (int16_t)(((uint16_t)buf.dat[0]<<8 | buf.dat[1]));
    icm_gyro_y = (int16_t)(((uint16_t)buf.dat[2]<<8 | buf.dat[3]));
    icm_gyro_z = (int16_t)(((uint16_t)buf.dat[4]<<8 | buf.dat[5]));
}







//-------------------------------------------------------------------------------------------------------------------
//  @brief      将ICM20602数据转化为带有物理单位的数据
//  @param      NULL
//  @return     void
//  @since      v1.0
//  Sample usage:               执行该函数后，直接查看对应的变量即可
//-------------------------------------------------------------------------------------------------------------------
float unit_icm_gyro_x;
float unit_icm_gyro_y;
float unit_icm_gyro_z;

float unit_icm_acc_x;
float unit_icm_acc_y;
float unit_icm_acc_z;
void icm20602_data_change(void)
{
    //ICM20602_GYRO_CONFIG寄存器
    //设置为:0x00 陀螺仪量程为:±250 dps     获取到的陀螺仪数据除以131           可以转化为带物理单位的数据， 单位为：°/s
    //设置为:0x08 陀螺仪量程为:±500 dps     获取到的陀螺仪数据除以65.5          可以转化为带物理单位的数据，单位为：°/s
    //设置为:0x10 陀螺仪量程为:±1000dps     获取到的陀螺仪数据除以32.8          可以转化为带物理单位的数据，单位为：°/s
    //设置为:0x18 陀螺仪量程为:±2000dps     获取到的陀螺仪数据除以16.4          可以转化为带物理单位的数据，单位为：°/s

    //ICM20602_ACCEL_CONFIG寄存器
    //设置为:0x00 加速度计量程为:±2g          获取到的加速度计数据 除以16384      可以转化为带物理单位的数据，单位：g(m/s^2)
    //设置为:0x08 加速度计量程为:±4g          获取到的加速度计数据 除以8192       可以转化为带物理单位的数据，单位：g(m/s^2)
    //设置为:0x10 加速度计量程为:±8g          获取到的加速度计数据 除以4096       可以转化为带物理单位的数据，单位：g(m/s^2)
    //设置为:0x18 加速度计量程为:±16g         获取到的加速度计数据 除以2048       可以转化为带物理单位的数据，单位：g(m/s^2)
    unit_icm_gyro_x = (float)icm_gyro_x/131;
    unit_icm_gyro_y = (float)icm_gyro_y/131;
    unit_icm_gyro_z = (float)icm_gyro_z/131;

    unit_icm_acc_x = (float)icm_acc_x/4096;
    unit_icm_acc_y = (float)icm_acc_y/4096;
    unit_icm_acc_z = (float)icm_acc_z/4096;

    log_i("Gyro: X=%.3f, Y=%.3f, Z=%.3f rad/s | Acc: X=%.3f, Y=%.3f, Z=%.3f g",
           unit_icm_gyro_x, unit_icm_gyro_y, unit_icm_gyro_z,
           unit_icm_acc_x, unit_icm_acc_y, unit_icm_acc_z);

}
