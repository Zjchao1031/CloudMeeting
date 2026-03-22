#pragma once
#include <QDialog>
#include <QLabel>
#include <QPushButton>

/**
 * @file ConfirmDialog.h
 * @brief 声明通用确认、提示与错误弹窗。
 */

/**
 * @class ConfirmDialog
 * @brief 提供统一风格的确认、危险操作和提示类对话框。
 */
class ConfirmDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * @enum Type
     * @brief 定义弹窗的展示与交互类型。
     */
    enum class Type {
        Confirm, ///< 普通确认弹窗。
        Danger,  ///< 危险操作确认弹窗。
        Info,    ///< 信息提示弹窗。
        Error    ///< 错误提示弹窗。
    };

    /**
     * @brief 构造通用确认对话框。
     * @param[in] title 对话框标题。
     * @param[in] message 对话框正文内容。
     * @param[in] type 对话框类型。
     * @param[in] parent 父窗口指针。
     */
    explicit ConfirmDialog(const QString &title,
                           const QString &message,
                           Type type = Type::Confirm,
                           QWidget *parent = nullptr);

    /**
     * @brief 显示信息提示对话框。
     * @param[in] title 对话框标题。
     * @param[in] message 提示内容。
     * @param[in] parent 父窗口指针。
     */
    static void showInfo(const QString &title, const QString &message, QWidget *parent = nullptr);

    /**
     * @brief 显示错误提示对话框。
     * @param[in] title 对话框标题。
     * @param[in] message 错误内容。
     * @param[in] parent 父窗口指针。
     */
    static void showError(const QString &title, const QString &message, QWidget *parent = nullptr);

    /**
     * @brief 显示普通确认对话框。
     * @param[in] title 对话框标题。
     * @param[in] message 提示内容。
     * @param[in] parent 父窗口指针。
     * @return 若用户确认则返回 `true`，否则返回 `false`。
     */
    static bool showConfirm(const QString &title, const QString &message, QWidget *parent = nullptr);

    /**
     * @brief 显示危险操作确认对话框。
     * @param[in] title 对话框标题。
     * @param[in] message 提示内容。
     * @param[in] confirmText 确认按钮显示文本。
     * @param[in] parent 父窗口指针。
     * @return 若用户确认则返回 `true`，否则返回 `false`。
     */
    static bool showDanger(const QString &title, const QString &message,
                           const QString &confirmText, QWidget *parent = nullptr);

private:
    /**
     * @brief 初始化对话框界面。
     * @param[in] title 对话框标题。
     * @param[in] message 对话框正文内容。
     * @param[in] type 对话框类型。
     * @param[in] confirmText 确认按钮显示文本。
     */
    void setupUi(const QString &title, const QString &message, Type type,
                 const QString &confirmText = QString());
};
