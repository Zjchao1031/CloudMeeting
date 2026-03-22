#pragma once
#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>

class ToolBarPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ToolBarPanel(QWidget *parent = nullptr);

signals:
    void cameraToggled(bool on);
    void micToggled(bool on);
    void screenShareToggled(bool on);
    void captureVolumeChanged(int value);
    void playbackVolumeChanged(int value);
    void leaveRequested();

private slots:
    void onCameraClicked();
    void onMicClicked();
    void onScreenShareClicked();

private:
    void setupUi();
    void updateButtonStyle(QPushButton *btn, bool active);

    QPushButton *m_cameraBtn      = nullptr;
    QPushButton *m_micBtn         = nullptr;
    QPushButton *m_screenShareBtn = nullptr;
    QPushButton *m_leaveBtn       = nullptr;
    QSlider     *m_captureSlider  = nullptr;
    QSlider     *m_playbackSlider = nullptr;

    bool m_cameraOn      = false;
    bool m_micOn         = false;
    bool m_screenShareOn = false;
};
