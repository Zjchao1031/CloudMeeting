#pragma once
#include <QWidget>

class ParticipantListWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ParticipantListWidget(QWidget *parent = nullptr);
    void refresh();
};
