# SeaOS – Proof of Concept

> **Status:** Bootsector + freestanding C kernel that prints a message under QEMU/Bochs.

## Prerequisites

* **Cross-compiler** `i686-elf-gcc`, `i686-elf-ld`, `i686-elf-objcopy`
  * On Ubuntu you can install via:
    ```bash
    sudo apt install gcc binutils build-essential nasm qemu-system-x86
    # or build a cross-compiler:
    # https://wiki.osdev.org/GCC_Cross-Compiler
    ```

## Build & Run

```bash
make            # 生成 build/SeaOS.img
make run        # 使用 QEMU 启动
```

启动后在 QEMU 控制台 (按 `Ctrl + Alt + 3`) 或 Bochs 调试窗口可看到：

```
SeaOS booting...
SeaOS kernel loaded successfully.
```

## 下一步

* 转入保护模式 / 长模式并设置分页
* 替换自制内核为 Linux 内核或自研微内核
* 构建用户空间、兼容层与 SeaSuite 应用
* 集成自动化更新、AI 子系统等（详见蓝图文档）