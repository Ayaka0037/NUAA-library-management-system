#ifndef CATEGORYMANAGEDIALOG_H
#define CATEGORYMANAGEDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QTableWidget;

class CategoryManageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CategoryManageDialog(QWidget *parent = nullptr);

private slots:
    void onAdd();
    void onEdit();
    void onDelete();

private:
    void setupUI();
    void loadCategories();

    QTableWidget *m_table;
    QLineEdit    *m_nameEdit;
    QPushButton  *m_addBtn;
    QPushButton  *m_editBtn;
    QPushButton  *m_deleteBtn;
};

#endif // CATEGORYMANAGEDIALOG_H
