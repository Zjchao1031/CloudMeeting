#pragma once
#include <QWidget>

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
};
