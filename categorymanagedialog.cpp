#include "categorymanagedialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

CategoryManageDialog::CategoryManageDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    loadCategories();
}

void CategoryManageDialog::setupUI()
{
    setWindowTitle("图书分类管理");
    resize(450, 400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 输入行
    QHBoxLayout *inputLayout = new QHBoxLayout();
    m_nameEdit = new QLineEdit();
    m_nameEdit->setPlaceholderText("输入分类名称");
    m_addBtn = new QPushButton("添加");
    inputLayout->addWidget(m_nameEdit);
    inputLayout->addWidget(m_addBtn);

    // 表格
    m_table = new QTableWidget();
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({"分类ID", "分类名称"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    // 操作行
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_editBtn   = new QPushButton("编辑所选");
    m_deleteBtn = new QPushButton("删除所选");
    btnLayout->addWidget(m_editBtn);
    btnLayout->addWidget(m_deleteBtn);
    btnLayout->addStretch();

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_table);
    mainLayout->addLayout(btnLayout);

    connect(m_addBtn,    &QPushButton::clicked, this, &CategoryManageDialog::onAdd);
    connect(m_editBtn,   &QPushButton::clicked, this, &CategoryManageDialog::onEdit);
    connect(m_deleteBtn, &QPushButton::clicked, this, &CategoryManageDialog::onDelete);
}

void CategoryManageDialog::loadCategories()
{
    QSqlQuery query;
    if (!query.exec("SELECT category_id, category_name FROM categories ORDER BY category_id")) {
        QMessageBox::critical(this, "错误", "加载分类失败：" + query.lastError().text());
        return;
    }

    m_table->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(query.value("category_id").toString()));
        m_table->setItem(row, 1, new QTableWidgetItem(query.value("category_name").toString()));
        row++;
    }
}

void CategoryManageDialog::onAdd()
{
    QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入分类名称");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO categories (category_name) VALUES (?)");
    query.addBindValue(name);
    if (query.exec()) {
        m_nameEdit->clear();
        loadCategories();
    } else {
        QMessageBox::critical(this, "错误", "添加失败：" + query.lastError().text());
    }
}

void CategoryManageDialog::onEdit()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择一个分类");
        return;
    }

    int catId = m_table->item(row, 0)->text().toInt();
    QString oldName = m_table->item(row, 1)->text();

    // 将当前名称填到输入框让用户修改
    m_nameEdit->setText(oldName);

    // 简单做法：用输入框新值更新
    QString newName = m_nameEdit->text().trimmed();
    if (newName.isEmpty() || newName == oldName) {
        QMessageBox::warning(this, "提示", "请修改分类名称后再点击编辑按钮");
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE categories SET category_name = ? WHERE category_id = ?");
    query.addBindValue(newName);
    query.addBindValue(catId);
    if (query.exec()) {
        m_nameEdit->clear();
        loadCategories();
    } else {
        QMessageBox::critical(this, "错误", "修改失败：" + query.lastError().text());
    }
}

void CategoryManageDialog::onDelete()
{
    int row = m_table->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "提示", "请先选择一个分类");
        return;
    }

    int catId = m_table->item(row, 0)->text().toInt();
    QString catName = m_table->item(row, 1)->text();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认删除",
        QString("确定要删除分类「%1」吗？").arg(catName),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    QSqlQuery query;
    query.prepare("DELETE FROM categories WHERE category_id = ?");
    query.addBindValue(catId);
    if (query.exec()) {
        loadCategories();
    } else {
        QMessageBox::critical(this, "错误", "删除失败：" + query.lastError().text());
    }
}
