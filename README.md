# LoRa个人学习笔记
重拾LoRa，将离线笔记搬运到GitHub上来，也算是总结一下这个项目。

## 项目介绍

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

> LoRaWAN网络架构是一个典型的星形拓扑结构，在这个网络架构中，LoRa网关是一个透明的中继，连接前端终端设备和后端中央服务器。网关与服务器通过标准IP连接，而终端设备采用单跳与一个或多个网关通信，所有的节点均是双向通信。

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

**项目中使用最低功耗的OTAA**









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



## 三、系统搭建

### 网关(树莓派)

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
  需要下载 893 kB/1,216 kB 的归档。
  解压缩后会消耗 3,297 kB 的额外空间。
  您希望继续执行吗？ [Y/n] y
  获取:1 http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian stretch/main armhf libpcap0.8 armhf 1.8.1-3 [123 kB]
  获取:2 http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian stretch/main armhf libwvstreams4.6-base armhf 4.6.1-11 [193 kB]
  获取:3 http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian stretch/main armhf libwvstreams4.6-extras armhf 4.6.1-11 [330 kB]
  获取:4 http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian stretch/main armhf libuniconf4.6 armhf 4.6.1-11 [139 kB]
  获取:5 http://mirrors.tuna.tsinghua.edu.cn/raspbian/raspbian stretch/main armhf wvdial armhf 1.61-4.1 [107 kB]
  已下载 893 kB，耗时 2秒 (353 kB/s)
  正在预设定软件包 ...
  正在选中未选择的软件包 libpcap0.8:armhf。
  (正在读取数据库 ... 系统当前共安装有 124818 个文件和目录。)
  正准备解包 .../0-libpcap0.8_1.8.1-3_armhf.deb  ...
  正在解包 libpcap0.8:armhf (1.8.1-3) ...
  正在选中未选择的软件包 libwvstreams4.6-base。
  正准备解包 .../1-libwvstreams4.6-base_4.6.1-11_armhf.deb  ...
  正在解包 libwvstreams4.6-base (4.6.1-11) ...
  正在选中未选择的软件包 libwvstreams4.6-extras。
  正准备解包 .../2-libwvstreams4.6-extras_4.6.1-11_armhf.deb  ...
  正在解包 libwvstreams4.6-extras (4.6.1-11) ...
  正在选中未选择的软件包 libuniconf4.6。
  正准备解包 .../3-libuniconf4.6_4.6.1-11_armhf.deb  ...
  正在解包 libuniconf4.6 (4.6.1-11) ...
  正在选中未选择的软件包 ppp。
  正准备解包 .../4-ppp_2.4.7-1+4_armhf.deb  ...
  正在解包 ppp (2.4.7-1+4) ...
  正在选中未选择的软件包 wvdial。
  正准备解包 .../5-wvdial_1.61-4.1_armhf.deb  ...
  正在解包 wvdial (1.61-4.1) ...
  正在处理用于 libc-bin (2.24-11+deb9u3) 的触发器 ...
  正在设置 libwvstreams4.6-base (4.6.1-11) ...
  正在处理用于 systemd (232-25+deb9u2) 的触发器 ...
  正在处理用于 man-db (2.7.6.1-2) 的触发器 ...
  正在设置 libpcap0.8:armhf (1.8.1-3) ...
  正在设置 ppp (2.4.7-1+4) ...
  Created symlink /etc/systemd/system/multi-user.target.wants/pppd-dns.service → /lib/systemd/system/pppd-dns.service.
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

  修改三个引号行

  ```shell
  [Dialer Defaults]
  Init1 = ATZ
  Init2 = ATQ0 V1 E1 S0=0
  Modem Type = Analog Modem
  ; Phone = <Target Phone Number>
  ISDN = 0
  ; Password = <Your Password>
  New PPPD = yes
  ; Username = <Your Login Name>
  Modem = /dev/ttyUSB0
  Baud = 9600
  ```

  我的修改如下：

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

### 节点(STM32)

#### 浊度

#### PH计

#### 温度

## 四、上位机

### [The Things Network(服务器)](https://console.thethingsnetwork.org/applications/suvan/data)

### VPS

## 五、其它