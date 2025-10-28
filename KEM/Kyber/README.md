### kyber算法

- 代码来源于NIST三轮提交原代码
- - 添加了cpucycles.h cpucycles.c test_main.c文件
- - 修改了最初的makefile文件
- 变体代码未测试
- 运行平台：Linux

```shell
# 分别进入每个算法目录
如：kyber512
# 运行命令
make test_main
./test_main
```
- cpu周期计算方法有待提升，但是主要目的是为了统一的基准测试