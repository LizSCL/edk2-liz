# EDK II

## 核心架构相关包（Platform / Processor / Architecture）

| 文件夹名               | 作用说明                                           |
| ------------------ | ---------------------------------------------- |
| **ArmPkg**         | 提供用于 ARM 架构平台的基本模块（如CPU、内存管理等）。                |
| **ArmPlatformPkg** | 包含针对特定 ARM 平台的支持包，通常是特定开发板的驱动和配置。              |
| **ArmVirtPkg**     | 提供 ARM 虚拟化平台（如 QEMU）的支持，适用于在虚拟机中运行 ARM 固件。     |
| **EmulatorPkg**    | 用于创建通用固件模拟器（例如运行在 Windows/Linux 上的 UEFI 模拟环境）。 |
| **UefiCpuPkg**     | 提供与 UEFI CPU 初始化和上下文管理相关的模块，支持多种架构（x86, ARM等）。 |

---

## 基础模块及库

| 文件夹名             | 作用说明                                             |
| ---------------- | ------------------------------------------------ |
| **MdePkg**       | 最核心的模块包，包含 UEFI 所需的基础库、头文件和标准接口。                 |
| **MdeModulePkg** | 提供一组可复用的 UEFI 驱动和应用程序模块（例如 Shell、Boot Manager）。  |
| **BaseTools**    | 包含构建系统需要的工具，如 `GenFw`, `GenFds`, `build.exe` 等。  |
| **Conf**         | 配置构建环境的目录，含 `target.txt`, `tools_def.txt` 等关键文件。 |

---

## 测试、实验性、贡献类包

| 文件夹名                     | 作用说明                                  |
| ------------------------ | ------------------------------------- |
| **UnitTestFrameworkPkg** | 用于运行单元测试的基础框架。                        |
| **contrib**              | 社区贡献的实验性或非主线功能代码。                     |
| **StandaloneMmPkg**      | MM（Management Mode）相关模块支持。            |

---

## 安全、加密相关包

| 文件夹名                 | 作用说明                               |
| -------------------- | ---------------------------------- |
| **SecurityPkg**      | 提供各种安全机制模块，如 Secure Boot 支持、验证等。   |
| **CryptoPkg**        | 提供加密算法（如SHA, RSA, AES）的实现，供其他模块调用。 |
| **SignedCapsulePkg** | 提供对已签名固件升级（Capsule）的支持。            |

---

## 网络和远程管理相关包

| 文件夹名           | 作用说明                             |
| -------------- | -------------------------------- |
| **NetworkPkg** | 各类网络协议（TCP/IP、HTTP、DNS）和驱动支持。    |
| **RedfishPkg** | 支持 Redfish 协议（基于 REST 的服务器管理标准）。 |

---

## 存储和文件系统支持

| 文件夹名             | 作用说明                                       |
| ---------------- | ------------------------------------------ |
| **FatPkg**       | 提供 FAT 文件系统的实现（用于UEFI中访问FAT格式的分区）。         |
| **FmpDevicePkg** | 支持 Firmware Management Protocol（用于固件更新管理）。 |

---

## 平台与硬件包

| 文件夹名                    | 作用说明                                             |
| ----------------------- | ------------------------------------------------ |
| **PcAtChipsetPkg**      | 传统 PC AT 架构芯片组相关的模块支持。                           |
| **IntelFsp2Pkg**        | 支持 Intel FSP 2.x 固件封装（Firmware Support Package）。 |
| **IntelFsp2WrapperPkg** | Intel FSP2 的包装器，方便集成到 EDK2 平台中。                  |
| **DynamicTablesPkg**    | 提供 ACPI 动态表生成的工具和支持模块。                           |
| **SourceLevelDebugPkg** | 提供源码级调试支持（Source Level Debugging）。               |
| **PrmPkg**              | Platform Runtime Mechanism 的支持包，用于运行时扩展机制。       |
| **UefiPayloadPkg**      | 构建 UEFI Payload 的模块，适合嵌入式或Coreboot集成环境。          |

---

## 实用工具包

| 文件夹名            | 作用说明                                                      |
| --------------- | --------------------------------------------------------- |
| **ShellPkg**    | 包含 UEFI Shell 的实现和相关命令集。                                  |
| **EmbeddedPkg** | 支持嵌入式平台的模块（与 ARM 等平台交叉）。                                  |
| **OvmfPkg**     | OVMF（Open Virtual Machine Firmware）支持包，用于QEMU中的UEFI虚拟机固件。 |

---

## 工具与元数据目录（非构建包）

| 文件夹名                                                                                                       | 作用说明                                        |
| ---------------------------------------------------------------------------------------------------------- | ------------------------------------------- |
| **.azurepipelines**, **.github**, **.mergify**, **.devcontainer**, **.pytool**                             | 用于项目 CI/CD 流水线、GitHub Actions、Python 工具配置等。 |
| `.editorconfig`, `.gitignore`, `.gitmodules`, `.mailmap`, `edksetup.bat/sh`, `ReadMe.rst`, `License.txt` 等 | 项目级配置文件，无需更改，构建环境或文档用。                      |