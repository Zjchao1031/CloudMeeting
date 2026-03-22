#include "ui/widgets/ToolBarPanel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QFrame>

ToolBarPanel::ToolBarPanel(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void ToolBarPanel::setupUi()
{
    setFixedHeight(64);
    setStyleSheet("background: #16162A; border-top: 1px solid #2E2E44;");

    auto *row = new QHBoxLayout(this);
    row->setContentsMargins(20, 0, 20, 0);
    row->setSpacing(12);

    m_cameraBtn = new QPushButton("Cam", this);
    m_cameraBtn->setToolTip("\u6444\u50cf\u5934");
    row->addWidget(m_cameraBtn);

    m_micBtn = new QPushButton("Mic", this);
    m_micBtn->setToolTip("\u9ea6\u514b\u98ce");
    row->addWidget(m_micBtn);

    m_screenShareBtn = new QPushButton("Share", this);
    m_screenShareBtn->setToolTip("\u5c4f\u5e55\u5171\u4eab");
    row->addWidget(m_screenShareBtn);

    auto *div1 = new QFrame(this);
    div1->setFrameShape(QFrame::VLine);
    div1->setFixedHeight(32);
    div1->setStyleSheet("color: #2E2E44;");
    row->addWidget(div1);

    auto *capLabel = new QLabel("\u91c7\u96c6", this);
    capLabel->setObjectName("hintLabel");
    capLabel->setFixedWidth(28);
    m_captureSlider = new QSlider(Qt::Horizontal, this);
    m_captureSlider->setRange(0, 100);
    m_captureSlider->setValue(75);
    m_captureSlider->setFixedWidth(90);
    m_captureSlider->setToolTip("\u91c7\u96c6\u97f3\u91cf");
    row->addWidget(capLabel);
    row->addWidget(m_captureSlider);

    auto *div2 = new QFrame(this);
    div2->setFrameShape(QFrame::VLine);
    div2->setFixedHeight(32);
    div2->setStyleSheet("color: #2E2E44;");
    row->addWidget(div2);

    auto *playLabel = new QLabel("\u64ad\u653e", this);
    playLabel->setObjectName("hintLabel");
    playLabel->setFixedWidth(28);
    m_playbackSlider = new QSlider(Qt::Horizontal, this);
    m_playbackSlider->setRange(0, 100);
    m_playbackSlider->setValue(75);
    m_playbackSlider->setFixedWidth(90);
    m_playbackSlider->setToolTip("\u64ad\u653e\u97f3\u91cf");
    row->addWidget(playLabel);
    row->addWidget(m_playbackSlider);

    row->addStretch();

    m_leaveBtn = new QPushButton("\u9000\u51fa\u4f1a\u8bae", this);
    m_leaveBtn->setObjectName("leaveBtn");
    row->addWidget(m_leaveBtn);

    // Apply initial styles
    updateButtonStyle(m_cameraBtn,      m_cameraOn);
    updateButtonStyle(m_micBtn,         m_micOn);
    updateButtonStyle(m_screenShareBtn, m_screenShareOn);

    connect(m_cameraBtn,      &QPushButton::clicked, this, &ToolBarPanel::onCameraClicked);
    connect(m_micBtn,         &QPushButton::clicked, this, &ToolBarPanel::onMicClicked);
    connect(m_screenShareBtn, &QPushButton::clicked, this, &ToolBarPanel::onScreenShareClicked);
    connect(m_leaveBtn,       &QPushButton::clicked, this, &ToolBarPanel::leaveRequested);
    connect(m_captureSlider,  &QSlider::valueChanged, this, &ToolBarPanel::captureVolumeChanged);
    connect(m_playbackSlider, &QSlider::valueChanged, this, &ToolBarPanel::playbackVolumeChanged);
}

void ToolBarPanel::updateButtonStyle(QPushButton *btn, bool active)
{
    if (active) {
        btn->setStyleSheet(
            "QPushButton { background: rgba(79,142,247,0.18); color: #4F8EF7;"
            "  border: none; border-radius: 8px; padding: 6px 14px;"
            "  min-width: 52px; min-height: 36px; font-size: 13px; font-weight: 600; }"
            "QPushButton:hover { background: rgba(79,142,247,0.28); }"
        );
    } else {
        btn->setStyleSheet(
            "QPushButton { background: #2A2A3E; color: #8888A8;"
            "  border: none; border-radius: 8px; padding: 6px 14px;"
            "  min-width: 52px; min-height: 36px; font-size: 13px; }"
            "QPushButton:hover { background: #363650; color: #E8E8F0; }"
        );
    }
}

void ToolBarPanel::onCameraClicked()
{
    m_cameraOn = !m_cameraOn;
    updateButtonStyle(m_cameraBtn, m_cameraOn);
    emit cameraToggled(m_cameraOn);
}

void ToolBarPanel::onMicClicked()
{
    m_micOn = !m_micOn;
    updateButtonStyle(m_micBtn, m_micOn);
    emit micToggled(m_micOn);
}

void ToolBarPanel::onScreenShareClicked()
{
    m_screenShareOn = !m_screenShareOn;
    updateButtonStyle(m_screenShareBtn, m_screenShareOn);
    emit screenShareToggled(m_screenShareOn);
}
