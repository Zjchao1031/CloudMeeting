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
     * @return 当前缓存的会议密码文本。
     */
    QString password() const;

    /**
     * @brief 弹出密码输入子对话框。
     * @return 若用户完成密码输入并确认则返回 `true`，否则返回 `false`。
     */
    bool promptPassword();

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

    QLineEdit   *m_roomIdEdit = nullptr; ///< 房间号输入框。
    QPushButton *m_joinBtn    = nullptr; ///< 确认加入按钮。
    QPushButton *m_cancelBtn  = nullptr; ///< 取消按钮。
    QString      m_password;             ///< 用户输入的会议密码缓存。
};
