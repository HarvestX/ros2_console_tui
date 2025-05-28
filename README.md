# ros2_console_tui

A terminal-based interactive log viewer for ROS 2 applications.

---

## ✨ Features

- 🔍 Real-time viewing and filtering of `/rosout` logs.
- 🧩 Supports multiple log levels: `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`.
- 🎛 Text-based UI (TUI) powered by `ncurses`.
- 🎨 Configurable filters via YAML file (`~/.ros2_console_tui/config.yaml`).
- This is inspired by rqt_console https://github.com/ros-visualization/rqt_console

---

## 📦 Installation

### 1. Clone the repository into a ROS 2 workspace

```bash
mkdir -p ~/ws_tui/src
cd ~/ws_tui/src
git clone https://github.com/HarvestX/ros2_console_tui.git
```

### 2. Build the workspace

```bash
cd ~/ws_tui
colcon build
source install/setup.bash
```

---

## 🚀 Usage

Run the TUI log viewer with:

```bash
ros2 run ros2_console_tui_node log_viewer
```

> **Note:** Make sure your ROS 2 system is running and publishing logs to `/rosout`.

---

## 🛠 Keyboard Shortcuts

| Key     | Action                      |
| ------- | --------------------------- |
| `a`     | Show all log levels         |
| `d`     | Show only `DEBUG` logs      |
| `i`     | Show only `INFO` logs       |
| `w`     | Show only `WARN` logs       |
| `e`     | Show only `ERROR` logs      |
| `f`     | Show only `FATAL` logs      |
| `p`     | Pause updating logs      |
| `c`     | Clear current log buffer    |
| `↑ / ↓` | Scroll up/down through logs |
| `q`     | Quit the viewer             |

---

## ⚙️ Configuration

A config file is automatically created at:

`~/.ros2_console_tui/config.yaml`

You can manually exclude logs from specific node names:

```yaml
excluded_names:
  - /some_node_to_ignore
  - /another_node_to_ignore
```

---

## 🧪 Development Notes

- Requires ROS 2 Humble or later.
- Listens to `/rosout` via `rclcpp::Node`.

### Future Plans

- Rearrange and separate UI and ROS node
- Search and highlight
- Regex filtering
- Export logs to file

A terminal-based interactive log viewer for ROS 2 applications.
