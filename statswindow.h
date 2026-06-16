#ifndef STATSWINDOW_H
#define STATSWINDOW_H

#include <QWidget>

class QLabel;
class QPushButton;

class StatsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit StatsWindow(QWidget *parent = nullptr);

private slots:
    void onRefresh();

private:
    void setupUI();
    int querySingleInt(const QString &sql);
    QLabel *createCard(const QString &title, QWidget *parent);

    QLabel *m_totalBooksValue;
    QLabel *m_borrowedValue;
    QLabel *m_availableValue;
    QLabel *m_overdueValue;
    QLabel *m_activeReadersValue;
    QLabel *m_todayBorrowValue;
    QLabel *m_todayReturnValue;
    QPushButton *m_refreshBtn;
};

#endif // STATSWINDOW_H
