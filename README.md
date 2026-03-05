# 串口调试助手 - AT指令版

## 项目简介

这是一个基于Qt Widgets开发的串口调试助手，专门用于通过AT指令控制WiFi模块（如ESP8266/ESP32等）。支持多连接TCP/UDP通信、WiFi配置、透传模式等功能。

## 软件界面

![主界面](.\img\ui_menu.png)

## 功能特点

### 基础功能

- ✅ 串口参数配置（端口、波特率）
- ✅ 自动扫描可用串口
- ✅ 十六进制/ASCII显示切换
- ✅ 定时发送数据
- ✅ 发送/接收区清空
- ✅ 自动滚屏
- ✅ 文件发送/保存

### AT指令功能

- ✅ 基础AT指令（AT、AT+GMR、ATE0/1等）
- ✅ 模块复位（AT+RST）
- ✅ 查询STA/IP信息
- ✅ 工作模式切换（AP/STA/AP+STA）
- ✅ 普通传输/透传模式切换
- ✅ 多连接/单连接模式切换

### WiFi配置

- ✅ SSID/密码输入
- ✅ WiFi设置保存
- ✅ 一键连接WiFi

### TCP/IP功能

- ✅ 多连接支持（ID 0-4）
- ✅ TCP/UDP/SSL协议选择
- ✅ IP地址/端口配置
- ✅ 连接状态实时显示
- ✅ 发送数据/退出发送

## 项目结构

text

```
serial_at/
├── serial_at.pro              # Qt项目文件
├── main.cpp                    # 程序入口
├── mainwindow.h/cpp            # 主窗口（信号中转）
├── mainwindow.ui                # UI界面设计
├── atcommand/
│   ├── atcommandmanager.h/cpp  # AT指令生成与解析
├── network/
│   ├── networkmanager.h/cpp    # 网络连接管理
├── serial/
│   ├── serialmanager.h/cpp     # 串口通信管理
└── ui/
    ├── uimanager.h/cpp         # UI显示管理
    └── uiactionhandler.h/cpp   # 用户操作处理
```



## 模块说明





| 模块               | 职责                              |
| :----------------- | :-------------------------------- |
| `MainWindow`       | 窗口生命周期管理，信号中转        |
| `SerialManager`    | 串口打开/关闭、数据读写、端口扫描 |
| `ATCommandManager` | AT指令生成、响应解析              |
| `NetworkManager`   | 连接状态跟踪、超时管理            |
| `UIManager`        | UI控件更新、数据显示格式化        |
| `UIActionHandler`  | 处理所有按钮点击事件              |

## 编译环境

- **Qt版本**: Qt 5.9.9 或更高
- **编译器**: MinGW 32-bit 或 MSVC
- **构建系统**: qmake

## 快速开始

### 1. 克隆项目

bash

```
git clone <repository-url>
cd serial_at
```



### 2. 打开项目

- 启动 Qt Creator
- 打开 `serial_at.pro` 文件

### 3. 编译运行

- 点击"构建" -> "构建项目" (Ctrl+B)
- 点击"运行" (Ctrl+R)

## 使用指南

### 串口配置

1. 选择正确的串口号
2. 设置波特率（默认115200）
3. 点击"打开串口"

### WiFi连接

1. 输入WiFi SSID和密码
2. 点击"保存WiFi"保存设置
3. 点击"连接WiFi"发送连接指令

### TCP连接

1. 选择连接ID (0-4)
2. 选择协议类型 (TCP/UDP/SSL)
3. 输入目标IP地址和端口
4. 点击"建立连接"

### 多连接管理

- 启用多连接：点击"启用多连接" (AT+CIPMUX=1)
- 切换连接ID：从下拉框选择
- 查看状态：连接状态区域实时显示

### 数据发送

- **普通模式**: 输入数据，点击"发送"
- **十六进制发送**: 勾选"十六进制发送"
- **定时发送**: 勾选"定时发送"，设置间隔
- **AT指令**: 点击预设按钮或输入自定义指令

## 界面预览

text

```
┌─────────────────────────────────────┐
│ 串口设置     │ 接收区                │
│ 端口: COM3 ▼│ [时间] RX: 数据...    │
│ 波特率: 115200│                      │
│ [打开串口]   │ [十六进制显示] [清空] │
├─────────────┼───────────────────────┤
│ AT指令区    │ 发送区                 │
│ [AT] [复位] │ 输入数据...           │
│ [AP] [STA]  │ [十六进制] [定时发送]  │
├─────────────┤ [发送] [清空]         │
│ 设定信息    │                       │
│ WiFi/TCP设置│                       │
│ 连接状态    │                       │
└─────────────┴───────────────────────┘
```



## 常见AT指令





| 按钮       | 指令         | 说明         |
| :--------- | :----------- | :----------- |
| AT         | AT           | 测试指令     |
| 固件       | AT+GMR       | 查询固件版本 |
| 复位       | AT+RST       | 软复位模块   |
| AP模式     | AT+CWMODE=2  | 设为AP模式   |
| STA模式    | AT+CWMODE=1  | 设为STA模式  |
| 启用多连接 | AT+CIPMUX=1  | 支持多连接   |
| 普通传输   | AT+CIPMODE=0 | 普通模式     |
| 透传传输   | AT+CIPMODE=1 | 透传模式     |

## 二次开发

### 添加新的AT指令

1. 在 `ATCommandManager` 中添加指令生成函数

cpp

```
QString cmdNewFunction() const { return "AT+NEWCMD"; }
```



1. 在 `UIActionHandler` 中添加处理函数

cpp

```
void handleNewFunction() { sendATCommand(m_atCmdMgr->cmdNewFunction()); }
```



1. 在UI中添加按钮，并在 `mainwindow.cpp` 中连接信号

cpp

```
connect(ui->btnNewFunction, &QPushButton::clicked,
        m_actionHandler, &UIActionHandler::handleNewFunction);
```



### 添加新的响应解析

在 `ATCommandManager::parseResponse` 中添加解析逻辑：

cpp

```
if(response.contains("NEW RESPONSE")) {
    // 处理新响应
}
```



## 注意事项

1. **串口权限**: Linux系统可能需要sudo权限访问串口
2. **波特率设置**: 确保与模块配置一致
3. **多连接模式**: 必须先启用多连接才能建立多个连接
4. **透传模式**: 进入透传模式后，输入"+++"退出
5. **IP地址格式**: 支持标准IPv4地址格式

## 许可证

本项目采用 MIT 许可证。

## 作者

- **作者**: hbchen
- **版本**: 1.0
- **日期**: 2026.03

## 更新日志

### v1.0.0 (2026.03)

- 初始版本发布
- 基础串口调试功能
- AT指令控制
- 多连接TCP支持
- WiFi配置功能

------

**如有问题或建议，欢迎提交Issue或Pull Request！**