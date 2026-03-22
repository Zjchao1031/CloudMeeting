#pragma once
#include <QDialog>
#include <QIntValidator>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>

class CreateMeetingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CreateMeetingDialog(QWidget *parent = nullptr);

    int     maxMembers()   const;
    bool    hasPassword()  const;
    QString password()     const;

private slots:
    void onPasswordToggleChanged(int state);

private:
    void setupUi();

    QLineEdit   *m_maxMembersEdit  = nullptr;
    QCheckBox   *m_passwordToggle  = nullptr;
    QLineEdit   *m_passwordEdit    = nullptr;
    QLabel      *m_passwordLabel   = nullptr;
    QPushButton *m_confirmBtn      = nullptr;
    QPushButton *m_cancelBtn       = nullptr;
};
