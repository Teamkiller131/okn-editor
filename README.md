# okn-editor (OmniKillerNexus Editor)

Qt Widgets 基础的全功能编辑器，集成 okn-asset / okn-render / okn-audio / okn-ecs / okn-script / okn-platform / 未来 okn-network。支持 Docking/MDI、多视口、Undo/Redo、资产管线、场景与组件编辑、材质/着色器/动画/粒子/物理/音频/脚本/网络面板，Profiler/Frame Debugger，Git 集成，CLI 工具桥接，自动保存与恢复。

## 功能概览
- 框架：Docking/MDI，多视口/多文档，命令系统 + Undo/Redo，布局/主题/I18N，任务后台执行，日志/通知，自动保存/恢复，插件系统。
- 资产：浏览/搜索/标签，导入规则，检查器与预览，批处理导入/压缩/打包/校验，构建报告，变体/平台配置。
- 场景/ECS：层级树，Inspector（反射驱动），多选/同步，Prefab 占位，多 World 占位。
- 视口/渲染：轨迹球/飞行相机，多模式显示（Lit/Wireframe/GBuffer/Shadow/Light Complexity 等），Gizmo/Overlay/Debug 叠加，管线切换（Forward/Forward+/Clustered/Deferred），后处理开关，着色器热重载与缓存，Capture 占位。
- 材质/着色器：PBR 参数，关键字/变体；节点图占位；跨编译与缓存视图。
- 动画：时间轴/曲线/事件，状态机/Blend 占位，蒙皮/骨骼可视化，Blendshape 占位。
- 粒子/特效：模块/节点式占位，预览与统计。
- 物理/导航：碰撞体/约束，物理材质，NavMesh 烘焙占位。
- 音频：波形/播放，混音总线/侧链/Duck/房间/效果链。
- 脚本：Lua/JS/Python 浏览/热重载，调试占位（断点/单步），绑定信息驱动补全，占位 LSP。
- 网络：配置/调试占位，延迟/丢包/带宽统计占位。
- 工具与构建：CLI 桥接（着色器/材质/纹理/脚本/打包），任务/报告收集。
- 诊断：Profiler 面板（CPU/GPU/内存），Frame Debugger 占位，日志聚合，验证/调试层开关。
- VCS：Git 集成（状态/提交/基本差异占位）。

## 目录概要
- `include/okn/editor/`：核心 API、命令/Undo、布局、设置、项目、日志、任务、插件、主题、I18N、Profiles、Git；各面板与集成桥；资产/场景/视口/材质/动画/粒子/物理/导航/音频/脚本/网络/UI/工具/诊断/Profiler/Utils。
- `src/`：对应实现。
- `samples/`：最小窗口、Docking、资产管线、视口渲染、脚本调试、CLI 示例。
- `tests/`：命令与 Undo/Redo、资产规则、布局、插件、Git、CLI 等占位。
- `CMakeLists.txt`：主 + samples/tests。

## CMake 选项
- `BUILD_SAMPLES` / `BUILD_TESTS`
- `OKN_EDITOR_ENABLE_PLUGINS`
- `OKN_EDITOR_ENABLE_GIT`
- `OKN_EDITOR_ENABLE_I18N`
- `OKN_EDITOR_ENABLE_PROFILING`
- `OKN_EDITOR_ENABLE_FRAME_DEBUG`
- `OKN_EDITOR_ENABLE_NET`
- `OKN_EDITOR_ENABLE_SCRIPT_DEBUG`
- `OKN_EDITOR_ENABLE_AUTOSAVE`
- `OKN_EDITOR_ENABLE_CLI_TOOLS`

## 构建
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . -j
```

## 集成要点
- Qt Widgets 为 UI 基础，Docking/MDI 支持多视口与多文档；命令系统 + Undo/Redo 驱动 Inspector/资产修改。
- 后台任务与 okn-platform 线程/Job 协同；资产/渲染/音频/脚本/网络功能通过各自 bridge 调用对应模块。
- Git 集成用于版本管理；插件系统为未来扩展点（面板/命令/导入器/节点）。