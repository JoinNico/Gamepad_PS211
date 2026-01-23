# Gamepad日志

## 2025年1月20日 (周二)

### ✅ 完成的工作
- 焊接了 Gamepad 的下板 PCB

### 📝 遇到的问题
- PCB 上板莫名奇妙少编译了一个元件，只能作废
- IP5306 充电管理电路 KEY 脚输出 500Hz 左右的方波。使用电池供电，Vout 输出卡在 2v；使用 VBUS 供电，Vout正常输出，但是几秒后断电

### 💭 感悟思考
- 第一次在回流焊的时候摔了一下 PCB，元件全“碎”，只能重新焊下板 PCB；我彻底破防 + 裂开
- 调试 IP5306 的时候，这个问题不知如何处理，检查原理图、看datasheet、测 KEY 输出 ……；最后选择购买 IP5306-CK，这款芯片没有 30s 负载电流小于 45mA 自动休眠功能

### 📅 明日计划
- 添加手柄 ADC 驱动

## 2025年1月21日 (周三)

### ✅ 完成的工作
- 完善手柄驱动文件 joystick.c/.h
- 添加串口 UART1 debug.c/.h
- 添加 FreeRTOS

### 📝 遇到的问题
- HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buffer, 6) 这里的adc_buffer[]要和启动的ADC转换通道数匹配
- UART1 同时在两个task之间被调用，需要考虑资源的竞争，添加队列可以使debugprint字符串放入队列缓冲区，再被UART1正常输出；而不是两个task抢同一个UART1资源，导致输出失败或者缺失

### 💭 感悟思考
- 

### 📅 明日计划
- 优化joystick ADC采样

## 2025年1月22日 (周四)

### ✅ 完成的工作
- ADC 从软件触发改为 TIM8 TRGO 的 100 Hz 硬件触发
- 添加了 EasyLogger 文件，但是notwork
-

### 📝 遇到的问题
- CMSIS 的 osDelay 是 OS 实现，需要启动 OS 后才可以使用
- ADC 初始化校准中，不能立刻读取 DMA 的脏数据，需要Delay一会，确保数据整洁
- notwork 似乎是信号量创建的问题

### 💭 感悟思考
- 

### 📅 明日计划
- 

## 2025年1月23日 (周五)

### ✅ 完成的工作
- 修复 EasyLogger notwork
- 调试 EasyLogger PASS

### 📝 遇到的问题
- 看了很久的队列断言
`configASSERT( !( ( pvItemToQueue == NULL ) && ( pxQueue->uxItemSize != ( UBaseType_t ) 0U ) ) );`
调试器说uxItemSize = 3，不是信号量；但问题不在这里，只是StartELogTask任务的Heap空间太小了
- 加入 EasyLogger 前`ram: 7640 B 15.54%; flash: 23360 B 8.91%`

  加入 EasyLogger 后`ram: 29384 B 59.78%; flash: 62672 B 23.91%`

  说明日志系统占用内存太大了，接下来考虑裁剪和优化
### 💭 感悟思考
- 终于 EasyLogger 能正常使用了

### 📅 明日计划
- 