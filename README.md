# LoRa个人学习笔记
重拾LoRa，将离线笔记搬运到GitHub上来。

## 总体介绍



## 相关概念





## 关键参数



## 工程改动



## 相关外设



### 华为ME909s-821

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

  *99#：中国联通的WCDMA的拨号网络号码

  Ctrl + O：保存修改的文件

* 拨号上网！

  ```shell
  sudo wvdial
  ```


## 上位机

