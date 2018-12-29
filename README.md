# LoRa个人学习笔记
重拾LoRa，将离线笔记搬运到GitHub上来，也算是总结一下这个项目。

## 项目介绍

这是大三开始的一个“蜻蜓点水”项目，主要涉及LoRa、STM32、树莓派、Node.js

[TOC]



## 一、相关概念

总的来说：LoRa与LoRaWAN、LoRaWAN的Class A,B,C、LoRaWAN的两种入网方式



### LoRa与LoRaWAN

起初我并没有分清LoRa与LoRaWAN，后来发现是非常有必要区分两者：

> LoRa（Long Range）是由法国格勒诺布尔的Cycleo开发并于2012年被Semtech收购的专利数字无线数据通信技术。LoRa使用免许可的亚千兆赫兹无线电频段，如169 MHz，433 MHz，868 MHz（欧洲）和915 MHz（北美）。 LoRa能够以低功耗实现超远距离传输（农村地区超过10公里）。该技术分为两部分：物理层LoRa，和上层LoRaWAN。

> LoRaWAN是LoRa运营的网络，可以被物联网用于远程和非连接行业。 LoRaWAN是媒体访问控制（MAC）层协议，但主要是用于管理LPWAN网关和端节点设备之间的通信的网络层协议，作为由LoRa联盟维护的路由协议。 LoRaWAN规范1.0版于2015年6月发布。从基本的角度来说，人们可以认为LoRaWAN是一个新的WiFi，可以连接各个行业的新物联网设备。

| 区别 | LoRa                                                        | LoRaWAN                                                      |
| ---- | ----------------------------------------------------------- | ------------------------------------------------------------ |
| 本质 | LoRa是Semtech的 LoRa 芯片使用的chirp扩频（CSS）的调制技术。 | LoRaWAN是一种媒体访问控制（MAC）层协议，是一种拥有广泛覆盖能力的无线网络。 |
| 应用 | 鲁棒调制，实现不同数据速率。                                | LPWAN，具有广泛覆盖能力的低功耗无线网络。                    |
| 位置 | LoRa系统的物理层（PHY）                                     | LoRaWAN四层结构：RF、PHY、MAC、Application                   |

#### LoRaWAN的Class A、Class B、Class C

- Class A（可双向通讯的终端装置）即时性最好

  用于需要以最低功耗操作的终端装置。

  这种装置常常在它送出 uplink 之后，只需要与 server 端进行很短暂的 downlink 通讯 (例如只收个ACK 而已)

  在任何其他时间，从 server downlink 必须等到下一次的 scheduled uplink (所以通讯没办法很即时，例如下一次的 scheduled uplink 可能是在128秒之后)

- Class B（可双向通讯的终端装置，但有 scheduled receive slots）

  相较于 A 类的随机 receive windows，Class B 的装置会在排程的时间打开一个额外的接收窗。为了让终端装置在排程时间打开它的 receive window，它需要从 gateway 接收一个用于时间同步的 Beacon (如此一来，server 就能知道终端装置何时在 listening)，相较于 A 类会更即时。

- Class C（可双向通讯的终端装置，尽可能安排最多的 receive slots）

  C 类的终端装置是几乎连续地开着 receive windows，只有在发送时才会关闭接收视窗

  C 类对 server 与终端装置通讯带来最低的延迟 (latency)，所以即时性最好，但消耗功率最高

**项目中使用最低功耗的Class A**

#### LoRaWAN的两种入网方式

ABP（activation by personalization）

- DevAddr、NwkSKey 与 AppSKey 是直接储存在装置上的；
- 每个装置都有自己的 NwkSKey 与 AppSKey，这样泄漏一个装置的密钥，不会牵连到其他装置。

OTAA（over-the-air activation）

- DevEUI、AppEUI、AppKey 金钥这三个东西，AppKey 的目的是为了产生 NwkSKey 与 AppSKey。
- 接着由 End Device 发起请求，进行入网程序。
- 过程中如果失去 session context，必须重新跑 Join Procedure。

**项目中使用OTAA**









## 二、关键参数

总的来说：扩频因子(SF)，编码率(CR)，带宽(BW)，信噪比(SNR)，接收的信号强度指示(RSSI)，等效全向辐射功率(EIRP)，速率自适应(ADR)，OVSF: Orthogonal Variable Spreading Factor，正交可变扩频因子

### Lora技术用语解析

设置各种参数的目的

> 通过调整扩频因子和纠错率：最终在带宽占用、数据速率、链路预算改善以及抗干扰性之间达到最佳平衡

#### SF(SpreadingFactor) 扩频因子

当扩频因子为1时，数据1就用“1”来表示，扩频因子为4时，可能用“1011”来表示1；这样传输的目的是降低误码率也就是信噪比，但是却减少了可以传输的实际数据，所以，扩频因子越大，传输的数据数率就越小。简而言之：**扩大带宽、减少干扰**

当扩频因子为4时，有4个正交的扩频码，正交的扩频码可以让同时传输的无线信号互不干扰，也就是说，扩频因子为4时，可以同时传输4个人的信息。简而言之：**根据对速率的不同要求分配不同数量的码道，提高利用率**

> 扩频因子越大，传播时间越长，传播距离越广



#### CR(code rate) 编码率

RF(radio frequency) 发射频率

#### 发射功率

> 提高通信距离常用的办法是提高发射功率，同时也带来更多的能耗。



#### Band Width(BW) 调制带宽

带宽就是单位时间内的最大数据流量，也可以说是单位时间内最大可能提供多少个二进制位传输。

> 1M带宽指的是1Mbps=1 megabits per second
>
> 增加信号带宽，可以提高有效数据速率缩短传输时间，但会牺牲灵敏度

#### SNR(Signal-to-noise ratio) 信噪比

SNR 是一个正的dBm，表示信号比噪声的强度。SNR 越大，说明混在信号里的噪声越小，否则相反。

> 典型实例：SNR至少比RSSI高20~25dB。

#### RSSI (Received Signal Strength Indication) 接收的信号强度指示

RSSI 是一个负的dBm，表示RF信号的数值。信号越强，连线品质越好。因此RSSI越接近于0越好。

> -60dBm的信号比-80dBm的品质好。



EIRP(equivalent isotropically radiated power)等效全向辐射功率 或叫有效全向辐射功率

#### 速率自适应(ADR)

LoRaWAN网络服务器通过一种速率自适应（ADR）方案来控制数据传输速率和每一终端设备的射频输出。

#### 传输速率与通讯距离

LoRa 的传输率可以自由调整，传输率越低，传输的距离可以越远。

![](D:\Documents\GitHub\LoRa\img\Data_Rate.png)

## 三、系统搭建

### 网关(Raspberry PI)

LoRaWAN网络架构是一个典型的星形拓扑结构，在这个网络架构中，LoRa网关是一个透明的中继，连接前端终端设备和后端中央服务器。网关与服务器通过标准IP连接，而终端设备采用单跳与一个或多个网关通信，所有的节点均是双向通信。

#### 安装LoRa网关

##### 安装树莓派系统

- 磁盘写入工具：etcher

- SSH服务安装与开机自动启动

  打开  /etc/rc.local文件；

  在语句exit 0之前加入：

  ```shell
  /etc/init.d/ssh start
  ```

- 给只读文件加上写权限

  ```
  sudo chmod a+w filename
  ```

  同理，给只读文件加上可执行权限

  ```shell
  sudo chmod a+x filename
  ```

- 树莓派3-配置-通过VNC访问系统

  ```shell
  sudo apt-get install tightvncserver
  ```

  设置密码，等等，省略好多可以忽略的步骤。。。

  使用如下命令手工启动VNC服务器程序

  ```shell
  tightvncserver -geometry 800x600 :1
  ```

  然后就可以去同局域网的PC端访问了，密码就是刚才设置的。

- 预设值wifi密码

  打开  /etc/wpa_supplicant/wpa_supplicant.conf

  ```shell
  ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
  update_config=1
  country=GB
  
  network={
  	ssid="family"
  	psk="416liuliuliu"
  	key_mgmt=WPA-PSK
  }
  
  network={
  	ssid="nkbh_micromouse"
  	psk="115115115115"
  	key_mgmt=WPA-PSK
  }
  ```

- 更改键盘布局

  树莓派(raspberry pi)是英国产品，默认键盘布局是英国(GB)，我们用的键盘布局一般是美国(US)

  ```shell
  sudo dpkg-reconfigure keyboard-configuration
  ```

  进入后，选通用的101键PC键盘；

  按tab键，到OK，按Enter确定

  在键盘layout选择中，选Other；

  按tab键，到OK，按Enter确定

  然后在选项中，选English(US)；

  按tab键，到OK，按Enter确定

  再选English(US, alternative international)；

  ![](/img/树莓派键盘.png "树莓派键盘")

  按tab键，到OK，按Enter确定

  然后一路按OK；

  最后，重启系统

  ```shell
  sudo reboot
  ```

#### 华为ME909s-821

​	由于项目需求，LoRaWAN网关必须安装到郊外，意味着Raspberry PI 无法使用WIFI联网，目前的代替方案是：Huawei ME909s-821 4G模块。话不多说，Raspberry PI 开始：

* 因为无法访问外网，所以首先修改树莓派的更新源

    ```shell
    sudo nano /etc/apt/sources.list
    ```

    ```sh
    # deb http://raspbian.raspberrypi.org/raspbian/ stretch main contrib non-free rpi
    # Uncomment line below then 'apt-get update' to enable 'apt-get source'
    # deb-src http://raspbian.raspberrypi.org/raspbian/ stretch main contrib non-free rpi
    # use ustc mirror:
    # deb http://mirrors.aliyun.com/raspbian/raspbian/ stretch main non-free contrib rpi
    # deb-src http://mirrors.aliyun.com/raspbian/raspbian/ stretch main non-free contrib rpi
    deb http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian/ stretch main contrib non-free rpi
    deb-src http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian/ stretch main contrib non-free rpi
    ```

    ```shell
    sudo apt-get update
    sudo nano /etc/apt/sources.list.d/raspi.list
    ```

    ```shell
    # deb http://archive.raspberrypi.org/debian/ stretch main ui
    # Uncomment line below then 'apt-get update' to enable 'apt-get source'
    # deb-src http://archive.raspberrypi.org/debian/ stretch main ui
    # use ustc mirror:
    deb http://mirrors.aliyun.com/debian/ stretch main ui
    deb-src http://mirrors.aliyun.com/debian/ stretch main ui
    
    ```

* 安装 minicom

  ```shell
  sudo apt-get install minicom
  ```

* 注意天线

  根据 Mini PCIe 转接板上的标签标识，将主集天线的连接器插入到Mini PCIe 转接板的主集天线接口（M）。使用同样方法，将分集天线的连接器插入到Mini PCIe 转接板的分集天线接口（A）。

  > 如果你只有一根天线，请连接主集天线（M）

* 查看是否识别Huawei ME909s-821

  ```shell
  lsusb
  ```

* 安装模式切换工具

  ```shell
  sudo apt-get install usb-modeswitch usb-modeswitch-data
  ```

* 安装拨号软件

  ```shell
  sudo apt-get install wvdial
  ```

  ```shell
  正在读取软件包列表... 完成
  正在分析软件包的依赖关系树       
  正在读取状态信息... 完成       
  将会同时安装下列软件：
    libpcap0.8 libuniconf4.6 libwvstreams4.6-base libwvstreams4.6-extras ppp
  下列【新】软件包将被安装：
    libpcap0.8 libuniconf4.6 libwvstreams4.6-base libwvstreams4.6-extras ppp wvdial
  升级了 0 个软件包，新安装了 6 个软件包，要卸载 0 个软件包，有 14 个软件包未被升级。
  ...省略...
  正在设置 libwvstreams4.6-extras (4.6.1-11) ...
  正在设置 libuniconf4.6 (4.6.1-11) ...
  正在设置 wvdial (1.61-4.1) ...
  Success!  You can run "wvdial" to connect to the internet.
    (You can also change your configuration by editing /etc/wvdial.conf)
  正在处理用于 libc-bin (2.24-11+deb9u3) 的触发器 ...
  正在处理用于 systemd (232-25+deb9u2) 的触发器 ...
  ```

* 扫描可用的Modem

  ```shell
  sudo wvdialconf  /etc/wvdial.conf
  ```

  成功找到Modem会打出如下message

  ```shell
  Editing `/etc/wvdial.conf'.
  
  Scanning your serial ports for a modem.
  
  ttyUSB0<*1>: ATQ0 V1 E1 -- OK
  ttyUSB0<*1>: ATQ0 V1 E1 Z -- OK
  ttyUSB0<*1>: ATQ0 V1 E1 S0=0 -- OK
  ttyUSB0<*1>: ATQ0 V1 E1 S0=0 &C1 -- +CME ERROR: 50
  ttyUSB0<*1>: ATQ0 V1 E1 S0=0 &D2 -- +CME ERROR: 50
  ttyUSB0<*1>: ATQ0 V1 E1 S0=0 +FCLASS=0 -- +CME ERROR: 50
  ttyUSB0<*1>: Modem Identifier: ATI -- Manufacturer: Huawei Technologies Co., Ltd.
  ttyUSB0<*1>: Speed 9600: AT -- OK
  ttyUSB0<*1>: Max speed is 9600; that should be safe.
  ttyUSB0<*1>: ATQ0 V1 E1 S0=0 -- OK
  ttyUSB1<*1>: ATQ0 V1 E1 -- failed with 2400 baud, next try: 9600 baud
  ttyUSB1<*1>: ATQ0 V1 E1 -- failed with 9600 baud, next try: 9600 baud
  ttyUSB1<*1>: ATQ0 V1 E1 -- and failed too at 115200, giving up.
  ttyUSB2<*1>: ATQ0 V1 E1 -- OK
  ttyUSB2<*1>: ATQ0 V1 E1 Z -- OK
  ttyUSB2<*1>: ATQ0 V1 E1 S0=0 -- OK
  ttyUSB2<*1>: ATQ0 V1 E1 S0=0 &C1 -- +CME ERROR: 50
  ttyUSB2<*1>: ATQ0 V1 E1 S0=0 &D2 -- +CME ERROR: 50
  ttyUSB2<*1>: ATQ0 V1 E1 S0=0 +FCLASS=0 -- +CME ERROR: 50
  ttyUSB2<*1>: Modem Identifier: ATI -- Manufacturer: Huawei Technologies Co., Ltd.
  ttyUSB2<*1>: Speed 9600: AT -- OK
  ttyUSB2<*1>: Max speed is 9600; that should be safe.
  ttyUSB2<*1>: ATQ0 V1 E1 S0=0 -- OK
  ttyUSB3<*1>: ATQ0 V1 E1 -- ERROR
  ttyUSB3<*1>: failed with 2400 baud, next try: 9600 baud
  ttyUSB3<*1>: ATQ0 V1 E1 -- ERROR
  ttyUSB3<*1>: failed with 9600 baud, next try: 9600 baud
  ttyUSB3<*1>: ATQ0 V1 E1 -- ERROR
  ttyUSB3<*1>: and failed too at 115200, giving up.
  ttyUSB4<*1>: ATQ0 V1 E1 -- failed with 2400 baud, next try: 9600 baud
  ttyUSB4<*1>: ATQ0 V1 E1 -- failed with 9600 baud, next try: 9600 baud
  ttyUSB4<*1>: ATQ0 V1 E1 -- and failed too at 115200, giving up.
  
  Found a modem on /dev/ttyUSB0.
  Modem configuration written to /etc/wvdial.conf.
  ttyUSB0<Info>: Speed 9600; init "ATQ0 V1 E1 S0=0"
  ttyUSB2<Info>: Speed 9600; init "ATQ0 V1 E1 S0=0"
  ```

  接下来可以打开wvdial.conf配置文件，进行相应的修改

* 配置文件wvdial.conf

  ```shell
  sudo nano /etc/wvdial.conf
  ```

  修改三个引号行，我的修改如下：

  ```shell
  [Dialer Defaults]
  Init1 = ATZ
  Init2 = ATQ0 V1 E1 S0=0
  Modem Type = Analog Modem
  Phone = *99#
  ISDN = 0
  Password = guest
  New PPPD = yes
  Username = guest
  Modem = /dev/ttyUSB0
  Baud = 9600
  ```

  > *99#：中国联通的WCDMA的拨号网络号码
  >
  > Ctrl + O：保存修改的文件

* 拨号上网！

  ```shell
  sudo wvdial
  ```

#### 玻璃钢天线





### 节点(STM32L151)

#### 主机板

##### 普通版 (WisNode)

无论普通还是升级，网关配置完成后的第一步：填appkey

对应TTN的设置，修改：LORAWAN_DEVICE_EUI 与 LORAWAN_APPLICATION_KEY

```c
#define LORAWAN_DEVICE_EUI                          { IEEE_OUI, 0xFF, 0xFE, 0xFD, 0xFC, 0x02 }
																										//Modifications in the case of mass production
/*!	
 * Application IEEE EUI (big endian)
 */
#define LORAWAN_APPLICATION_EUI                     { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0x88, 0xA8 }
                                                     //70b3d57ef00046a4   70B3D57E D0007DFA
/*!
 * AES encryption/decryption cipher application key
 */
#define LORAWAN_APPLICATION_KEY                     { 0x73, 0xB6, 0x49, 0xCD, 0xA4, 0x90, 0x1E, 0x9F, 0xBC, 0xE5, 0xCD, 0x68, 0x68, 0xB6, 0x14, 0xC8 }
                                                     //Modifications in the case of mass production
```

##### 升级版 (Tracker Node)

#### 外围

##### [浊度传感器](https://item.taobao.com/item.htm?spm=a1z09.2.0.0.593e2e8dtuesUp&id=530303920152&_u=eo8d3095bd5)

##### PH计传感器

##### 温度传感器

## 四、上位机

### [服务器 (The Things Network)](https://console.thethingsnetwork.org/applications/suvan/data)

#### [Payload结构](https://mydevices.com/cayenne/docs/lora/#lora-cayenne-low-power-payload)

#### 数据类型

| 通道 | 类型 | Object               | 量纲     |
| ---- | :--: | -------------------- | -------- |
| 01   |  88  | GPS 定位             | 定位 N/A |
| 02   |  67  | Temperature 温度     | °C       |
| 03   |  03  | Analog Output 电量   | mv       |
| 04   |  71  | Accelerometer 加速度 | g        |
| 05   |  7E  | Acidity 酸碱PH值     | N/A      |
| 06   |  7F  | Conductivity 电导率  | uS/cm    |
| 07   |  96  | Turbidity 水质浊度   | NTU      |
| 08   |  97  | COD 化学需氧量       | mg/L     |

### 阿里云VPS

##### [CentOS 7 安装 MATE 桌面环境](http://blog.csdn.net/m0_37876745/article/details/78188848)

- 安装 X Window System

  ```shell
  yum groups install "X Window System"
  ```

- 安装 MATE Desktop

  ```powershell
  yum groups install "MATE Desktop"
  ```

- 设置默认通过桌面环境启动服务器

  ```shell
  systemctl set-default graphical.target
  ```

- 重启服务器

  ```shell
  reboot
  ```

  ```shell
  systemctl set-default multi-user.target  //设置成命令模式
  systemctl set-default graphical.target  //设置成图形模式
  ```




### 服务平台 (Node.js)

#### 局域网 Ubuntu 配置Node.js环境

- 安装 Node.js

  ```shell
  curl -sL https://deb.nodesource.com/setup_8.x | sudo -E bash -
  sudo apt-get install -y nodejs
  ```

  如果失败，可能是没有安装 curl ：curl 是利用URL语法在命令行方式下工作的开源文件传输工具。它被广泛应用在Unix、多种Linux发行版中，并且有DOS和Win32、Win64下的移植版本

  ```shell
  sudo apt-get install curl
  ```

- 安装 git

  ```shell
  sudo apt-get install git
  ```

- 创建工程目录

  ```
  mkdir Filename
  ```

- [安装 The Things Network Node.js Application SDK](https://www.npmjs.com/package/ttn)

  ```shell
  cd Filename
  npm install --save ttn
  ```

- [创建脚本文件 For example](https://www.npmjs.com/package/ttn)

  ```shell
  第一行注掉：
  //import { data, application } from "ttn"
  第二行添加：
  let ttn = require("ttn");
  let data = ttn.data;
  let application = ttn.application;
  ```

  多谢 3-304 马斌老师！

#### 阿里云 CentOS 配置Node.js环境





### 简易平台(Node-RED)

#### 本地 Node-RED(Raspberry PI)

树莓派本身预安装Node-RED，所以只需要[update](https://www.ibm.com/developerworks/cn/cloud/library/cl-cn-bluemix-nodered/index.html)

```shell
update-nodejs-and-nodered
```



```shell
All done.
  You can now start Node-RED with the command  node-red-start
  or using the icon under   Menu / Programming / Node-RED
  Then point your browser to localhost:1880 or http://{your_pi_ip-address}:1880
```



```shell
cd ~/.node-red
sudo npm install -g node-red-admin
```



```shell
node-red-admin hash-pw
```

把密码记好

```shell
Password: 
$2a$08$ysZq7ivCwbQBNCCsvSJ89u1IUzskYfchCnoLVIm7KySLIqI5vqvDu
```

```
nano ~/.node-red/settings.js
```

![](img\nide-red.png)

```shell
sudo systemctl enable nodered.service
sudo reboot
```

#### 云端 Node-RED (阿里云)

安装Node-red



注意，阿里云需要打开1880端口

> 管理控制台 -->  产品与服务  -->  安全组规则 -->  入方向  -->  添加自定义 TCP

![](/img/Node_red_aliyun.png)

#### Node-red 点亮 led

开启：node-red，打开http://192.168.0.108:1880/

> 192.168.0.108是树莓派在局域网的IP地址

Manage palette，安装 node-red-dashboard & node-red-node-pi-gpiod

实例

```json
[{"id":"7c1e7b4b.da34e4","type":"rpi-gpio out","z":"1fe5d95c.88e647","name":"led","pin":"7","set":true,"level":"0","freq":"100","out":"out","x":810,"y":340,"wires":[]},{"id":"8b812881.120738","type":"ui_switch","z":"1fe5d95c.88e647","name":"","label":"LED","group":"f94ed544.fa12c8","order":0,"width":0,"height":0,"passthru":false,"decouple":"false","topic":"","style":"","onvalue":"true","onvalueType":"bool","onicon":"","oncolor":"","offvalue":"false","offvalueType":"bool","officon":"","offcolor":"","x":430,"y":340,"wires":[["dd1b0223.69643"]]},{"id":"dd1b0223.69643","type":"change","z":"1fe5d95c.88e647","name":"","rules":[{"t":"change","p":"payload","pt":"msg","from":"true","fromt":"bool","to":"1","tot":"num"},{"t":"change","p":"payload","pt":"msg","from":"false","fromt":"bool","to":"0","tot":"num"}],"action":"","property":"","from":"","to":"","reg":false,"x":620,"y":340,"wires":[["7c1e7b4b.da34e4"]]},{"id":"f94ed544.fa12c8","type":"ui_group","z":"","name":"Switch","tab":"d010dc9f.a4fc1","order":1,"disp":true,"width":"6"},{"id":"d010dc9f.a4fc1","type":"ui_tab","z":"","name":"My PI3b","icon":"dashboard","order":1}]
```

打开 dashboard，完成





## 五、其它

### Q&A

#### [LoRa通信模块为什么封装都有一个2mm的通孔，LDO为什么距离模块那么近？](http://bbs.21ic.com/icview-2542492-1-1.html)

​	LoRa通信对供电的纯净性有比较高的要求，虽然芯片能够对电源噪声进行抑制，但LoRa灵敏度实在太高(-135 dBm~-148 dBm，Zig-Bee也只能低至-96 dBm，白噪声主要分布在-120 dBm)，让我们不得不极为重视GND的纯净性。因此有两个对策：

- 一般使用纹波/噪声比较小的LDO或者线性稳压器件，以降低噪声，减少发热量（当然控制好压差）；
- LDO输出的GND与SX1276/78的GND PAD非常接近。

#### [在Ubuntu中，有时候运用sudo  apt-get install 安装软件时，会出现一下的情况](http://blog.csdn.net/u011596455/article/details/60322568)

E: Could not get lock /var/lib/dpkg/lock - open (11: Resource temporarily unavailable)

E: Unable to lock the administration directory (/var/lib/dpkg/), is another process using it?

#### LoRa为什么Radio层传输速率越快，通信距离越短?

从以下表格可以看出SF=9相对7的时候，SNR灵敏度提高了5dB，SF=12时候更是提高12.5dB。从无线电传输公式可以得知：

> 传输损耗(dB)= 20lgF（MHz）+20lgD(km)+经验常数

SNR越大，意味着通信链路预算值越高，也意味着传输距离更远。用经验公式推断：增加6dB的链路预算，可以提高大约2km的传输距离（实际情况会发现，增加12dB并没有获得更多的通信距离，这是还有其他的参量发生作用，在此不详细讨论）  

### Other

#### 第一代网关展示

![](/img/网关1.png)

#### 第二代网关展示