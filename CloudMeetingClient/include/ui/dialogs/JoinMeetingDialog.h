#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>

/**
 * @file JoinMeetingDialog.h
 * @brief 声明加入会议对话框。
 */

/**
 * @class JoinMeetingDialog
 * @brief 提供房间号与密码输入的加入会议界面。
 */
class JoinMeetingDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @brief 构造加入会议对话框。
     * @param[in] parent 父窗口指针。
     */
    explicit JoinMeetingDialog(QWidget *parent = nullptr);

    /**
     * @brief 获取用户输入的房间号。
     * @return 去除首尾空白后的房间号文本。
     */
    QString roomId() const;

    /**
     * @brief 获取用户输入的会议密码。
     * @return 密码输入框的文本，未填写时为空字符串。
     */
    QString password() const;

private slots:
    /**
     * @brief 响应加入按钮点击事件并校验房间号。
     */
    void onJoinClicked();

private:
    /**
     * @brief 初始化加入会议对话框界面。
     */
    void setupUi();

    QLineEdit   *m_roomIdEdit    = nullptr; ///< 房间号输入框。
    QLineEdit   *m_passwordEdit  = nullptr; ///< 密码输入框（选填）。
    QPushButton *m_joinBtn       = nullptr; ///< 确认加入按钮。
    QPushButton *m_cancelBtn     = nullptr; ///< 取消按钮。
};
