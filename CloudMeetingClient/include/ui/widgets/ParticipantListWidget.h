#pragma once
#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

class VideoTileWidget;

class ParticipantInfo {
public:
    QString userId;
    QString nickname;
    bool    isHost      = false;
    bool    cameraOn    = false;
    bool    micOn       = false;
};

class ParticipantListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ParticipantListWidget(QWidget *parent = nullptr);

    void refresh();
    void loadMockData();

private:
    void buildList();
    QWidget* makeItem(const ParticipantInfo &info);

    QVBoxLayout       *m_listLayout = nullptr;
    QList<ParticipantInfo> m_participants;
};
