依赖库
Apache Portable Runtime 1.6.3
地址 http://mirrors.shuosc.org/apache//apr/apr-1.6.3.tar.gz
编译步骤
$ tar xvzf apr-1.6.3.tar.gz // 解压apr-1.6.3.tar.gz
$ cd apr-1.6.3 // 进入apr-1.6.3.tar.gz目录
$ ./configure --prefix=/usr/local/apr // 指定安装到/usr/local/apr目录
$ make
$ sudo make install

libbson
地址 https://github.com/mongodb/libbson/
依赖工具
RedHat / Fedora:
$ sudo yum install git gcc automake autoconf libtool
Debian / Ubuntu:
$ sudo apt-get install git gcc automake autoconf libtool
FreeBSD:
$ su -c 'pkg install git gcc automake autoconf libtool'

编译步骤
$ ./autogen.sh
$ make
$ sudo make install

zlog 1.2.12
地址 https://github.com/HardySimpson/zlog/releases/tag/1.2.12
$ make
$ sudo make install

libmysqlclient15-dev
$sudo apt-get install libmysqlclient15-dev

uuid
$sudo apt-get install uuid-dev
