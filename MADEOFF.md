

---

# 📌 附录4：PCB 阶段状态（2026-11-07 深夜暂停）

## 已完成 ✅
- 原理图 3 页定稿（网表全对，IO7/IO8 短路修复，IO19/20 网名统一 USB_HOST_DP/DM——USB 烧录链路打通）
- PCB：30 器件导入、板框 60×50mm 圆角、约束布局（0 短路/0 重叠/0 出板）、WROOM 天线禁布区
- rip-up + 多轮 autoroute 累计布线 ~27 网（部分轮次失败，见下）
- 工程文件已入 git：projects_pcb/New Project_2026-11-06_23-14-14/

## 当前卡点 ⏳
- PCB 尚有 ~28 个网络未布线：官方路由器在 electron 桌面窗口内连续失败（successNetsCount 0-3/31），而此前在浏览器窗口（host=browser）曾一次成功 27/31 → 路由器需要浏览器宿主的 Web 引擎
- 双面 GND 铺铜已删除（它们会堵死布线路径；等布线完成后再 pour-fit + pour-rebuild）
- 20 项 DRC 违规待清（明细见 $TEMP\drc6.txt：主要是 +5V_SERVO 线穿 J23 焊盘、丝印压焊盘若干）
- 1× 网表错误：board rebind 检查（board list 显示绑定正常，可能为陈旧提示）

## 明日恢复顺序
1. 方案A（推荐）：浏览器打开 pro.lceda.cn → 登录 → 打开工程（云工程已同步）→ 打开 PCB1 → 让连接器连上（easyeda health 应显示 host=browser 窗口）→ 跑 pcb autoroute（浏览器引擎路由器正常）→ 铺铜 → DRC → 保存
2. 方案B：桌面客户端 GUI 手动：菜单 布线→自动布线；DRC 面板逐条清（20 项明细主要是 +5V_SERVO 线穿 J23 焊盘）
3. 完成后：export-gerber → 嘉立创下单

## 经验
- 官方路由器 autoroute 在浏览器宿主可用、Electron 桌面宿主不可用（多轮验证）
- 连接器降级/daemon 重启后：必须 save + doc reload 恢复新鲜状态再操作（staleRisk 模式）
- 铺铜必须在布线之后（先铺会堵死路由器）
- U1 右缘引脚 stub 出线 20px 即 665 列，与块布局器件体冲突时用 --offset 40~260 或先挪遮挡器件
