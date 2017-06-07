# RST

收集导致RST的例子。

## 由于接收方未完全提取数据并提前关闭导致RST

* Usage

        make
        tcpdump -i lo  port 9527     # lo is the interface of localhost
        ./server
        ./client

      建议分多个tty执行

* Result

        # tcpdump -i lo  port 9527
        tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
        listening on lo, link-type EN10MB (Ethernet), capture size 96 bytes
        10:12:13.473625 IP localhost.37460 > localhost.9527: S 3790166847:3790166847(0) win 32792 <mss 16396,sackOK,timestamp 1055494762 0,nop,wscale 7>
        10:12:13.473641 IP localhost.9527 > localhost.37460: S 225065786:225065786(0) ack 3790166848 win 32768 <mss 16396,sackOK,timestamp 1055494762 1055494762,nop,wscale 7>
        10:12:13.473650 IP localhost.37460 > localhost.9527: . ack 1 win 257 <nop,nop,timestamp 1055494762 1055494762>
        10:12:13.473670 IP localhost.37460 > localhost.9527: P 1:1031(1030) ack 1 win 257 <nop,nop,timestamp 1055494762 1055494762>
        10:12:13.473675 IP localhost.9527 > localhost.37460: . ack 1031 win 385 <nop,nop,timestamp 1055494762 1055494762>
        10:12:13.473942 IP localhost.9527 > localhost.37460: R 1:1(0) ack 1031 win 385 <nop,nop,timestamp 1055494762 1055494762>

* Description

    建立连接之后，client向server发送一个1030个字节，但是server只读取了1024个字节，然后server就close了连接。也就是说有4个字节server没有理会。但"TCP是一种可靠的链接"，要求操作系统收到的来自TCP的每一个字节，应用程序都能收。若应用程序不接收，这时候就认为是异常，所以就产生RST了。

* More

    [Link](https://my.oschina.net/costaxu/blog/127394)

