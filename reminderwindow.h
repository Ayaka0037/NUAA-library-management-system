#ifndef REMINDERWINDOW_H
#define REMINDERWINDOW_H

#include <QWidget>

class QTableWidget;
class QTabWidget;
class QPushButton;

class ReminderWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ReminderWindow(QWidget *parent = nullptr);

private slots:
    void onRefresh();
    void onExportOverdue();
    void onExportDueSoon();

private:
    void setupUI();
    void loadOverdue();
    void loadDueSoon();
    void exportTable(QTableWidget *table, const QString &defaultName);

    QTabWidget   *m_tabWidget;
    QTableWidget *m_overdueTable;
    QTableWidget *m_dueSoonTable;
    QPushButton  *m_refreshBtn;
    QPushButton  *m_exportOverdueBtn;
    QPushButton  *m_exportDueSoonBtn;
};

#endif // REMINDERWINDOW_H
