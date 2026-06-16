#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>

class QTableWidget;

class LogWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LogWindow(QWidget *parent = nullptr);

private:
    void setupUI();
    void loadLogs();

    QTableWidget *m_logTable;
};

#endif // LOGWINDOW_H