#ifndef RETURNWINDOW_H
#define RETURNWINDOW_H

#include <QWidget>

class QTableWidget;
class QPushButton;
class QLineEdit;

class ReturnWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ReturnWindow(int operatorId, QWidget *parent = nullptr);

private slots:
    void onSearchReader();
    void onReturnBook();

private:
    void setupUI();
    void loadBorrowList(int readerId);

    QLineEdit *m_readerCodeEdit;
    QPushButton *m_searchBtn;
    QTableWidget *m_borrowTable;
    QPushButton *m_returnBtn;

    int m_operatorId;
    int m_currentReaderId;
};

#endif // RETURNWINDOW_H