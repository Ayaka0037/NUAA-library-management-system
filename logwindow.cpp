#include "logwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

LogWindow::LogWindow(QWidget *parent) : QWidget(parent)
{
    setupUI();
    loadLogs();
}

void LogWindow::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- 页面标题栏 ---
    QWidget *header = new QWidget();
    header->setStyleSheet("background: white; border-left: 4px solid #3498db; border-bottom: 1px solid #e8eaed;");
    QHBoxLayout *hLayout = new QHBoxLayout(header);
    hLayout->setContentsMargins(14, 10, 14, 10);
    QLabel *hIcon = new QLabel("📋");
    QLabel *hTitle = new QLabel("操作日志");
    hIcon->setStyleSheet("font-size: 20px; background: transparent; border: none;");
    hTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #2c3e50; background: transparent; border: none;");
    hLayout->addWidget(hIcon);
    hLayout->addWidget(hTitle);
    hLayout->addStretch();
    mainLayout->addWidget(header);

    QWidget *body = new QWidget();
    body->setStyleSheet("background: #f5f6fa;");
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(16, 12, 16, 12);

    m_logTable = new QTableWidget();
    m_logTable->setColumnCount(5);
    m_logTable->setHorizontalHeaderLabels({"日志ID", "操作员", "操作类型", "详情", "时间"});
    m_logTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    bodyLayout->addWidget(m_logTable);
    mainLayout->addWidget(body, 1);
}

void LogWindow::loadLogs()
{
    QSqlQuery query;
    if (!query.exec("SELECT l.log_id, u.real_name, l.operation_type, l.operation_detail, l.created_at "
                    "FROM operation_logs l LEFT JOIN users u ON l.user_id = u.user_id "
                    "ORDER BY l.log_id DESC")) {
        QMessageBox::critical(this, "错误", "加载日志失败：" + query.lastError().text());
        return;
    }

    m_logTable->setRowCount(0);
    int row = 0;
    while (query.next()) {
        m_logTable->insertRow(row);
        m_logTable->setItem(row, 0, new QTableWidgetItem(query.value("log_id").toString()));
        m_logTable->setItem(row, 1, new QTableWidgetItem(query.value("real_name").toString()));
        m_logTable->setItem(row, 2, new QTableWidgetItem(query.value("operation_type").toString()));
        m_logTable->setItem(row, 3, new QTableWidgetItem(query.value("operation_detail").toString()));
        m_logTable->setItem(row, 4, new QTableWidgetItem(query.value("created_at").toString()));
        row++;
    }
}