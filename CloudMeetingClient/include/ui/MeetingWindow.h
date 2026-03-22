#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QSplitter>

class ParticipantListWidget;
class ChatPanel;
class VideoTileWidget;
class ToolBarPanel;

class MeetingWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MeetingWindow(QWidget *parent = nullptr);
    ~MeetingWindow();

    void bindSignals();
    void refreshLayout();
    void showRoomClosedDialog();

signals:
    void leaveRequested();

private:
    void setupUi();
    QWidget* makeTopBar();
    QWidget* makeVideoArea();

    // Top bar
    QLabel      *m_titleLabel   = nullptr;
    QLabel      *m_roomIdLabel  = nullptr;
    QPushButton *m_copyBtn      = nullptr;

    // Video area
    QScrollArea         *m_videoScrollArea = nullptr;
    QVBoxLayout         *m_videoLayout     = nullptr;

    // Right panel
    ParticipantListWidget *m_participantList = nullptr;
    ChatPanel             *m_chatPanel       = nullptr;
    ToolBarPanel          *m_toolBar         = nullptr;
};
