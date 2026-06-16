#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>

class QPushButton;
class QStackedWidget;
class QLabel;
class BorrowWindow;
class ReturnWindow;
class BookManageWindow;
class ReaderManageWindow;
class ReminderWindow;
class StatsWindow;
class LogWindow;

class MainMenu : public QWidget
{
    Q_OBJECT
public:
    explicit MainMenu(int userId, const QString &userRole, QWidget *parent = nullptr);

private slots:
    void switchPage(int index);
    void onExit();

private:
    void setupUI();
    void setupSidebar(QWidget *sidebar);
    void setupPages();
    void setupWelcomePage();
    QPushButton *createNavButton(const QString &text);

    // 左侧导航按钮
    QPushButton *m_navButtons[8];
    QLabel      *m_logoLabel;

    // 右侧页面容器
    QStackedWidget *m_stack;

    // 各功能页面
    QWidget        *m_welcomePage;
    BorrowWindow       *m_borrowPage;
    ReturnWindow       *m_returnPage;
    BookManageWindow   *m_bookPage;
    ReaderManageWindow *m_readerPage;
    ReminderWindow     *m_reminderPage;
    StatsWindow        *m_statsPage;
    LogWindow          *m_logPage;

    int m_userId;
    QString m_userRole;
};

#endif // MAINMENU_H
