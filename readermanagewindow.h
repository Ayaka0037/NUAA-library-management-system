#ifndef READERMANAGEWINDOW_H
#define READERMANAGEWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTableWidget;

class ReaderManageWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ReaderManageWindow(int userId, QWidget *parent = nullptr);

private slots:
    void onSearch();
    void onAddReader();
    void onEditReader();
    void onToggleFreeze();
    void onDeleteReader();
    void onExport();

private:
    void setupUI();
    bool hasActiveBorrows(int readerId);

    QLineEdit    *m_searchEdit;
    QPushButton  *m_searchBtn;
    QPushButton  *m_addBtn;
    QPushButton  *m_editBtn;
    QPushButton  *m_freezeBtn;
    QPushButton  *m_deleteBtn;
    QPushButton  *m_exportBtn;
    QTableWidget *m_table;
    int m_userId;
};

#endif // READERMANAGEWINDOW_H
