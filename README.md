# X-Trader

这是一个基于CTP API的交易系统框架，支持多种交易策略和市场数据处理功能。系统设计模块化，便于扩展和维护。

交流请加vx: X_Trader_Lab

## 特性

- 支持多种交易策略：包括做市商策略、统计套利策略、订单流策略等
- 支持市场数据订阅和处理
- 提供订单管理和执行功能
- 支持策略回测和实时交易
- 跨平台支持（Windows和Linux）

## 目录结构

- `api/` - CTP API相关头文件和库文件
- `bin/` - 配置文件目录
- `src/` - 源代码目录
  - `framework/` - 核心框架代码
  - `strategy/` - 交易策略实现
  - `trade-core/` - 交易核心模块
  - `utils/` - 工具类代码

## 使用方法

1. 配置CTP API环境
2. 修改配置文件以适应您的交易账户信息
3. 选择或实现所需的交易策略
4. 编译并运行系统

## 策略示例

系统包含多种预实现策略：
- `decline_scalping` - 下降趋势套利策略
- `high_low` - 高低价策略
- `market_correction` - 市场回调策略
- `statistical_arbitrage` - 统计套利策略

## 开发者指南

如需添加新策略，请继承`strategy`基类并实现相应的事件处理方法，如`on_tick()`, `on_bar()`, `on_order()`等。

## 许可证

本项目采用Apache-2.0 license。

对应的配套视频：

https://www.bilibili.com/cheese/play/ss16157

https://www.bilibili.com/cheese/play/ss19871

<h3>捐助</h3>
<p>如果您觉得我们的开源软件对你有所帮助，请扫下方二维码打赏我们一杯咖啡。</p>
<table><tr>
<td><img src="https://foruda.gitee.com/images/1767354466267474794/cff0945f_16543158.png" width="300" /></td>
<td><img src="https://foruda.gitee.com/images/1767354490948751827/1f90ad17_16543158.jpeg" width="300" /></td>
</tr></table>

### 联系
微信： X_Trader_Lab
