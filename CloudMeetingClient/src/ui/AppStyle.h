#pragma once
#include <QString>

/**
 * @file AppStyle.h
 * @brief 定义应用全局界面样式表。
 */

/**
 * @namespace AppStyle
 * @brief 提供客户端统一使用的全局样式资源。
 */
namespace AppStyle {

inline const char *GLOBAL_QSS = R"QSS(
/* ===== 全局基础 ===== */
QWidget {
    background-color: #1E1E2E;
    color: #E8E8F0;
    font-family: "Segoe UI", "Microsoft YaHei", sans-serif;
    font-size: 14px;
}

QMainWindow {
    background-color: #1E1E2E;
}

/* ===== 主操作按钮（强调色填充）===== */
QPushButton#primaryBtn {
    background-color: #4F8EF7;
    color: #FFFFFF;
    border: none;
    border-radius: 8px;
    padding: 10px 20px;
    font-size: 14px;
    font-weight: 600;
    min-width: 200px;
    min-height: 42px;
}
QPushButton#primaryBtn:hover {
    background-color: #6BA3FA;
}
QPushButton#primaryBtn:pressed {
    background-color: #3A74E0;
}
QPushButton#primaryBtn:disabled {
    background-color: #3A3A50;
    color: #5A5A72;
}

/* ===== 次级按钮（描边样式）===== */
QPushButton#outlineBtn {
    background-color: transparent;
    color: #4F8EF7;
    border: 2px solid #4F8EF7;
    border-radius: 8px;
    padding: 10px 20px;
    font-size: 14px;
    font-weight: 600;
    min-width: 200px;
    min-height: 42px;
}
QPushButton#outlineBtn:hover {
    background-color: rgba(79, 142, 247, 0.12);
}
QPushButton#outlineBtn:pressed {
    background-color: rgba(79, 142, 247, 0.22);
}

/* ===== 危险按钮（红色）===== */
QPushButton#dangerBtn {
    background-color: #E05C5C;
    color: #FFFFFF;
    border: none;
    border-radius: 8px;
    padding: 10px 20px;
    font-size: 14px;
    font-weight: 600;
    min-height: 42px;
}
QPushButton#dangerBtn:hover {
    background-color: #F07070;
}
QPushButton#dangerBtn:pressed {
    background-color: #C04848;
}

/* ===== 小型按钮 ===== */
QPushButton#smallBtn {
    background-color: #4F8EF7;
    color: #FFFFFF;
    border: none;
    border-radius: 6px;
    padding: 6px 16px;
    font-size: 13px;
    font-weight: 600;
    min-height: 32px;
}
QPushButton#smallBtn:hover {
    background-color: #6BA3FA;
}

QPushButton#smallOutlineBtn {
    background-color: transparent;
    color: #4F8EF7;
    border: 1px solid #4F8EF7;
    border-radius: 6px;
    padding: 6px 16px;
    font-size: 13px;
    min-height: 32px;
}
QPushButton#smallOutlineBtn:hover {
    background-color: rgba(79,142,247,0.12);
}

/* ===== 图标切换按钮（工具栏）===== */
QPushButton#toolIconBtn {
    background-color: #2A2A3E;
    color: #8888A8;
    border: none;
    border-radius: 8px;
    padding: 8px;
    min-width: 48px;
    min-height: 48px;
    font-size: 20px;
}
QPushButton#toolIconBtn:hover {
    background-color: #363650;
    color: #E8E8F0;
}
QPushButton#toolIconBtn[active="true"] {
    background-color: rgba(79, 142, 247, 0.18);
    color: #4F8EF7;
}

/* 退出按钮 */
QPushButton#leaveBtn {
    background-color: #E05C5C;
    color: #FFFFFF;
    border: none;
    border-radius: 8px;
    padding: 8px 20px;
    font-size: 14px;
    font-weight: 600;
    min-height: 42px;
}
QPushButton#leaveBtn:hover {
    background-color: #F07070;
}
QPushButton#leaveBtn:pressed {
    background-color: #C04848;
}

/* ===== 文本输入框 ===== */
QLineEdit {
    background-color: #2A2A3E;
    color: #E8E8F0;
    border: 1.5px solid #3A3A52;
    border-radius: 8px;
    padding: 8px 12px;
    font-size: 14px;
    selection-background-color: #4F8EF7;
}
QLineEdit:focus {
    border-color: #4F8EF7;
}
QLineEdit:disabled {
    color: #5A5A72;
}
QLineEdit[echoMode="2"] {
    lineedit-password-character: 9679;
}

/* 多行输入框 */
QTextEdit {
    background-color: #2A2A3E;
    color: #E8E8F0;
    border: 1.5px solid #3A3A52;
    border-radius: 8px;
    padding: 8px 12px;
    font-size: 14px;
    selection-background-color: #4F8EF7;
}
QTextEdit:focus {
    border-color: #4F8EF7;
}

/* ===== SpinBox ===== */
QSpinBox {
    background-color: #2A2A3E;
    color: #E8E8F0;
    border: 1.5px solid #3A3A52;
    border-radius: 8px;
    padding: 6px 10px;
    font-size: 14px;
    min-height: 36px;
}
QSpinBox:focus {
    border-color: #4F8EF7;
}
QSpinBox::up-button, QSpinBox::down-button {
    background-color: #363650;
    border: none;
    width: 24px;
    subcontrol-origin: border;
}
QSpinBox::up-button {
    subcontrol-position: top right;
    border-top-right-radius: 8px;
}
QSpinBox::down-button {
    subcontrol-position: bottom right;
    border-bottom-right-radius: 8px;
}
QSpinBox::up-button:hover, QSpinBox::down-button:hover {
    background-color: #4F8EF7;
}
QSpinBox::up-arrow {
    image: none;
    width: 0;
    height: 0;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-bottom: 6px solid #E8E8F0;
}
QSpinBox::down-arrow {
    image: none;
    width: 0;
    height: 0;
    border-left: 5px solid transparent;
    border-right: 5px solid transparent;
    border-top: 6px solid #E8E8F0;
}

/* ===== 滑块 ===== */
QSlider::groove:horizontal {
    background: #3A3A52;
    height: 4px;
    border-radius: 2px;
}
QSlider::handle:horizontal {
    background: #4F8EF7;
    width: 16px;
    height: 16px;
    margin: -6px 0;
    border-radius: 8px;
}
QSlider::sub-page:horizontal {
    background: #4F8EF7;
    border-radius: 2px;
}
QSlider::handle:horizontal:hover {
    background: #6BA3FA;
}

/* ===== 滚动条 ===== */
QScrollBar:vertical {
    background: #1E1E2E;
    width: 8px;
    margin: 0;
}
QScrollBar::handle:vertical {
    background: #3A3A52;
    border-radius: 4px;
    min-height: 30px;
}
QScrollBar::handle:vertical:hover {
    background: #4F8EF7;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    height: 0;
}
QScrollBar:horizontal {
    background: #1E1E2E;
    height: 8px;
}
QScrollBar::handle:horizontal {
    background: #3A3A52;
    border-radius: 4px;
    min-width: 30px;
}

/* ===== 对话框 ===== */
QDialog {
    background-color: #2A2A3E;
    border-radius: 16px;
}

/* ===== Label ===== */
QLabel {
    background-color: transparent;
    color: #E8E8F0;
}
QLabel#hintLabel {
    color: #8888A8;
    font-size: 12px;
}
QLabel#titleLabel {
    color: #E8E8F0;
    font-size: 18px;
    font-weight: 700;
}
QLabel#accentLabel {
    color: #4F8EF7;
    font-size: 13px;
    font-weight: 600;
}
QLabel#statusLabel {
    color: #8888A8;
    font-size: 12px;
}

/* ===== 分隔线 ===== */
QFrame[frameShape="4"],
QFrame[frameShape="5"] {
    color: #3A3A52;
    background-color: #3A3A52;
}

/* ===== 菜单 ===== */
QMenu {
    background-color: #2A2A3E;
    color: #E8E8F0;
    border: 1px solid #3A3A52;
    border-radius: 8px;
    padding: 4px;
}
QMenu::item {
    padding: 8px 20px;
    border-radius: 6px;
}
QMenu::item:selected {
    background-color: rgba(79, 142, 247, 0.18);
    color: #4F8EF7;
}

/* ===== 菜单栏 ===== */
QMenuBar {
    background-color: #1A1A2A;
    color: #8888A8;
    font-size: 13px;
}
QMenuBar::item:selected {
    background-color: rgba(79, 142, 247, 0.18);
    color: #4F8EF7;
    border-radius: 4px;
}

/* ===== 分割器手柄 ===== */
QSplitter::handle {
    background-color: #3A3A52;
    width: 1px;
    height: 1px;
}
QSplitter::handle:hover {
    background-color: #4F8EF7;
}

/* ===== 滚动区域 ===== */
QScrollArea {
    background-color: transparent;
    border: none;
}
QScrollArea > QWidget > QWidget {
    background-color: transparent;
}

/* ===== 工具提示 ===== */
QToolTip {
    background-color: #2A2A3E;
    color: #E8E8F0;
    border: 1px solid #4F8EF7;
    border-radius: 6px;
    padding: 4px 8px;
    font-size: 12px;
}
)QSS";

} // namespace AppStyle
