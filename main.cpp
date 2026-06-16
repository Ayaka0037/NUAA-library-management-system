#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include "logindialog.h"
#include "mainmenu.h"

static void ensureSchema()
{
    QSqlQuery q;

    // ======================== 用户表 ========================
    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "user_id   INT AUTO_INCREMENT PRIMARY KEY, "
           "username  VARCHAR(50)  NOT NULL UNIQUE, "
           "password  VARCHAR(100) NOT NULL, "
           "role      VARCHAR(20)  NOT NULL DEFAULT 'operator', "
           "real_name VARCHAR(50), "
           "status    VARCHAR(20)  NOT NULL DEFAULT 'active'"
           ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    // ======================== 读者表 ========================
    q.exec("CREATE TABLE IF NOT EXISTS readers ("
           "reader_id      INT AUTO_INCREMENT PRIMARY KEY, "
           "reader_code    VARCHAR(20)  NOT NULL UNIQUE, "
           "name           VARCHAR(50)  NOT NULL, "
           "status         VARCHAR(20)  NOT NULL DEFAULT 'active', "
           "borrowed_count INT NOT NULL DEFAULT 0, "
           "total_borrowed INT NOT NULL DEFAULT 0"
           ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    // ======================== 分类表 ========================
    q.exec("CREATE TABLE IF NOT EXISTS categories ("
           "category_id   INT AUTO_INCREMENT PRIMARY KEY, "
           "category_name VARCHAR(50) NOT NULL UNIQUE"
           ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    // ======================== 图书表 ========================
    q.exec("CREATE TABLE IF NOT EXISTS books ("
           "book_id        INT AUTO_INCREMENT PRIMARY KEY, "
           "isbn           VARCHAR(30), "
           "title          VARCHAR(200) NOT NULL, "
           "author         VARCHAR(100), "
           "publisher      VARCHAR(100), "
           "category_id    INT DEFAULT NULL, "
           "total_count    INT NOT NULL DEFAULT 1, "
           "available_count INT NOT NULL DEFAULT 1, "
           "status         VARCHAR(20) NOT NULL DEFAULT 'active'"
           ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    // ======================== 借阅记录表 ========================
    q.exec("CREATE TABLE IF NOT EXISTS borrow_records ("
           "record_id   INT AUTO_INCREMENT PRIMARY KEY, "
           "book_id     INT NOT NULL, "
           "reader_id   INT NOT NULL, "
           "borrow_date DATE NOT NULL, "
           "due_date    DATE NOT NULL, "
           "return_date DATE DEFAULT NULL, "
           "status      VARCHAR(20) NOT NULL DEFAULT 'borrowed', "
           "fine_amount DECIMAL(10,2) DEFAULT 0, "
           "operator_id INT DEFAULT NULL"
           ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");

    // ======================== 操作日志表 ========================
    q.exec("CREATE TABLE IF NOT EXISTS operation_logs ("
           "log_id           INT AUTO_INCREMENT PRIMARY KEY, "
           "user_id          INT NOT NULL DEFAULT 0, "
           "operation_type   VARCHAR(50) NOT NULL, "
           "operation_detail TEXT, "
           "ip_address       VARCHAR(45) DEFAULT '', "
           "created_at       DATETIME DEFAULT CURRENT_TIMESTAMP"
           ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4");
}

static void seedIfEmpty()
{
    QSqlQuery q;
    q.exec("SELECT COUNT(*) FROM books");
    if (q.next() && q.value(0).toInt() > 0)
        return;  // 已有数据，跳过

    // 尝试读取 seed_data.sql，支持多个路径
    // Qt Creator 运行时工作目录通常是项目根目录
    QStringList paths = {
        QDir::currentPath() + "/seed_data.sql",               // 项目根 / 当前目录
        QCoreApplication::applicationDirPath() + "/seed_data.sql",    // exe 同目录
        QCoreApplication::applicationDirPath() + "/../seed_data.sql", // exe 上一级
        QCoreApplication::applicationDirPath() + "/../../seed_data.sql", // shadow build
    };

    QString filePath;
    for (const QString &p : paths) {
        if (QFile::exists(p)) {
            filePath = p;
            break;
        }
    }

    if (filePath.isEmpty()) {
        qDebug() << "[ensureSchema] seed_data.sql 未找到，跳过种子数据";
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "[ensureSchema] 无法打开 seed_data.sql:" << file.errorString();
        return;
    }

    QTextStream in(&file);
    QString sql = in.readAll();
    file.close();

    // 按分号拆分执行（跳过注释行和空语句）
    QStringList statements = sql.split(';', Qt::SkipEmptyParts);
    int ok = 0, fail = 0;
    for (const QString &stmt : statements) {
        QString trimmed = stmt.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith("--"))
            continue;
        if (q.exec(trimmed))
            ok++;
        else {
            qDebug() << "[ensureSchema] SQL失败:" << q.lastError().text()
                     << "\n  语句:" << trimmed.left(80);
            fail++;
        }
    }
    qDebug() << "[ensureSchema] 种子数据执行完毕:" << ok << "成功," << fail << "失败";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 全局按钮兜底样式 — 确保未单独设 stylesheet 的按钮（如 QMessageBox 按钮）可见
    app.setStyleSheet(
        "QPushButton {"
        "  background-color: #e8ecf1;"
        "  color: #2c3e50;"
        "  border: 1px solid #c0c6cf;"
        "  border-radius: 4px;"
        "  padding: 5px 14px;"
        "  font-size: 13px;"
        "}"
        "QPushButton:hover {"
        "  background-color: #d5dbe3;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #c0c8d2;"
        "}"
        "QPushButton:disabled {"
        "  background-color: #e8ecf1;"
        "  color: #a0a8b0;"
        "}");

    // 连接数据库
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("dbcd");
    db.setUserName("root");
    db.setPassword("123456");

    if (!db.open()) {
        QMessageBox::critical(nullptr, "错误", "数据库连接失败：" + db.lastError().text());
        return -1;
    }
    qDebug() << "数据库连接成功";

    // 自动建表 + 种子数据
    ensureSchema();
    seedIfEmpty();

    // 显示登录对话框
    LoginDialog login;
    if (login.exec() != QDialog::Accepted) {
        return 0;  // 用户取消登录
    }

    int userId = login.getUserId();
    QString userRole = login.getUserRole();
    QString userName = login.getUserName();

    // 创建主菜单，传入userId和role
    MainMenu menu(userId, userRole);
    menu.show();

    return app.exec();
}
