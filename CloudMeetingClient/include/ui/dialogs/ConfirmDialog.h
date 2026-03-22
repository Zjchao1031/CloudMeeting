#pragma once
#include <QDialog>
#include <QLabel>
#include <QPushButton>

// 通用确认/错误/提示弹窗
class ConfirmDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Type {
        Confirm,      // 普通确认（主操作为强调色）
        Danger,       // 危险确认（主操作为危险色）
        Info,         // 纯提示（仅确定按钮）
        Error         // 错误提示
    };

    explicit ConfirmDialog(const QString &title,
                           const QString &message,
                           Type type = Type::Confirm,
                           QWidget *parent = nullptr);

    // 工厂方法
    static void showInfo(const QString &title, const QString &message, QWidget *parent = nullptr);
    static void showError(const QString &title, const QString &message, QWidget *parent = nullptr);
    static bool showConfirm(const QString &title, const QString &message, QWidget *parent = nullptr);
    static bool showDanger(const QString &title, const QString &message,
                           const QString &confirmText, QWidget *parent = nullptr);

private:
    void setupUi(const QString &title, const QString &message, Type type,
                 const QString &confirmText = QString());
};
