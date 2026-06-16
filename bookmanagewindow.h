#ifndef BOOKMANAGEWINDOW_H
#define BOOKMANAGEWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTableWidget;
class QComboBox;

class BookManageWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BookManageWindow(int userId, QWidget *parent = nullptr);

private slots:
    void onSearch();        // 查询图书
    void onAddBook();       // 添加图书
    void onEditBook();      // 编辑所选图书
    void onDeleteBook();    // 删除所选图书
    void onFilterCategory();// 按分类筛选
    void onManageCategories();// 管理分类
    void onExport();        // 导出CSV

private:
    void setupUI();
    bool isBookBorrowed(int bookId);
    void loadCategoryFilter();

    QLineEdit    *m_searchEdit;
    QPushButton  *m_searchBtn;
    QPushButton  *m_addBtn;
    QPushButton  *m_editBtn;
    QPushButton  *m_deleteBtn;
    QPushButton  *m_categoryBtn;
    QPushButton  *m_exportBtn;
    QComboBox    *m_categoryFilter;
    QTableWidget *m_bookTable;
    int m_userId;
};

#endif // BOOKMANAGEWINDOW_H
