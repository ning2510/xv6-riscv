>docs branch:
>
>Record notes from lab3 to lab11.

### 1. 前言

#### 1.1 安装环境

> [6.S081 / Fall 2020 (mit.edu)](https://pdos.csail.mit.edu/6.828/2020/tools.html)

安装 `RISC-V GNU Complier Toolchain`

```shell
$ git clone --recursive https://github.com/riscv/riscv-gnu-toolchain
```

安装依赖包

```shell
$ sudo apt-get install autoconf automake autotools-dev curl libmpc-dev libmpfr-dev libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc zlib1g-dev libexpat-dev
```

构建

```shell
$ cd riscv-gnu-toolchain
$ ./configure --prefix=/usr/local
$ sudo make
$ cd ..
```

安装 `QEMU 5.1.0`

```shell
$ wget https://download.qemu.org/qemu-5.1.0.tar.xz
$ tar xf qemu-5.1.0.tar.xz
```

编译

```shell
$ cd qemu-5.1.0
$ ./configure --disable-kvm --disable-werror --prefix=/usr/local --target-list="riscv64-softmmu"
$ make
$ sudo make install
$ cd ..
```



#### 1.2 拉取源码

> [Lab: Xv6 and Unix utilities (mit.edu)](https://pdos.csail.mit.edu/6.828/2020/labs/util.html)

```shell
$ git clone git://g.csail.mit.edu/xv6-labs-2020
$ cd xv6-labs-2020
$ git checkout util
```



#### 1.3 测试 xv6

```shell
$ cd xv6-labs-2020
$ make qemu
```

此时会报错

```shell
user/sh.c:58:1: error: infinite recursion detected [-Werror=infinite-recursion]
   58 | runcmd(struct cmd *cmd)
```

修改 `user/sh.c` 文件

```diff
# 57行
+ __attribute__((noreturn))
void
runcmd(struct cmd *cmd)
```

再次编译即可

```shell
$ make qemu
qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 3 -nographic -drive file=fs.img,if=none,format=raw,id=x0 -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

xv6 kernel is booting

hart 2 starting
hart 1 starting
init: starting sh
$ 
```



#### 1.4 GDB 调试 xv6

> [xv6 调试不完全指北](https://www.cnblogs.com/KatyuMarisaBlog/p/13727565.html)

配置 `.gdbinit`

```shell
$ cp .gdbinit.tmpl-riscv .gdbinit
```

`.gdbinit` 内容如下：

> `make qemu-gdb` 命令打开的 `TCP` 端口是 26000

```shell
set confirm off
set architecture riscv:rv64
target remote 127.0.0.1:26000
symbol-file kernel/kernel
set disassemble-next-line auto

```



**终端1：**

```shell
$ make qemu-gdb
```

**终端2：**

```shell
# 第一次连接
$ riscv64-unknown-elf-gdb kernel/kernel
...
# 输入 target remote localhost:26000
(gdb) target remote localhost:26000

# 之后连接
$ riscv64-unknown-elf-gdb kernel/kernel
GNU gdb (GDB) 12.1
Copyright (C) 2022 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-pc-linux-gnu --target=riscv64-unknown-elf".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from kernel/kernel...
The target architecture is set to "riscv:rv64".
warning: Can't read data for section '.debug_frame' in file '/home/hadoop/xv6-riscv/kernel/kernel'
--Type <RET> for more, q to quit, c to continue without paging--
0x0000000000001000 in ?? ()
(gdb) file user/_sleep 
Reading symbols from user/_sleep...
(gdb) b main
Breakpoint 1 at 0x0: file user/sleep.c, line 7.
(gdb) c
```



#### 1.5 相关命令

**运行：**`make qemu`

**退出 `QEMU`：**`ctrl a + x`

**清除编译产物：**`make clean`

**GDB 调试：**

- `make qemu-gdb`
- `riscv64-unknown-elf-gdb kernel/kernel`



