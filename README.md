# 问墨输入法（Windows）

问墨 Windows 版。平台无关的拼音输入核心复用 CC-CEDICT 离线词库，Windows 外壳使用
Text Services Framework（TSF）实现。

## 当前进度

- C++20 输入核心：拼音组合、退格、清空、候选查询及简繁切换
- CC-CEDICT 拼音词库与第三方许可证
- TSF COM DLL、按键接入和首选候选上屏
- x86-64（x64）与 ARM64 原生构建预设
- CMake 构建和核心单元测试

## 构建核心

需要 CMake 3.20 或更新版本，以及支持 C++20 的编译器。

```powershell
cmake --preset windows-x64
cmake --build --preset windows-x64-release
ctest --preset windows-x64
```

ARM Windows 设备将上述 `x64` 换成 `arm64`。两种架构均为原生 DLL，不能混用；请使用带有
对应 ARM64 工具链的 Visual Studio 2022。构建后，以管理员身份注册：

```powershell
.\scripts\register.ps1 -Architecture x64
# 注销：.\scripts\register.ps1 -Architecture x64 -Unregister
```

## 后续工作

当前是可安装的开发版：输入字母后按空格或回车会上屏第一个精确匹配候选。候选窗口、拼音
分词/模糊匹配、简繁设置界面、MSIX/MSI 打包与代码签名仍待实现。

## 许可

项目源代码依据 [Apache License 2.0](LICENSE) 开放。`resources/cedict_pinyin.tsv` 是
CC-CEDICT 的派生数据，依据 CC BY-SA 4.0 提供，详情见 [DATA-LICENSES.md](DATA-LICENSES.md)
和 [NOTICE](NOTICE)。
