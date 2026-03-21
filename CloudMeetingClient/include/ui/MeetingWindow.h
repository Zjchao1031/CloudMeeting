#pragma once
#include <QWidget>

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

private:
    ParticipantListWidget *m_participantList = nullptr;
    ChatPanel             *m_chatPanel       = nullptr;
    ToolBarPanel          *m_toolBar         = nullptr;
};
