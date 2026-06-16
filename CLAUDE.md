# 图书管理系统 (Library Management System)

基于 Qt 6 + C++17 的图书管理系统，ODBC 连接 MySQL。单窗口 App 风格：左侧深色导航栏 + 右侧 QStackedWidget 内容区。

## 首次在新电脑上打开

### 1. 找到 Qt 安装路径
```powershell
# 常见位置
Get-ChildItem C:\Qt\*\mingw_64 -Directory
Get-ChildItem D:\Qt\*\mingw_64 -Directory
Get-ChildItem D:\QT\*\mingw_64 -Directory

# 找到 MinGW
Get-ChildItem C:\Qt\Tools\mingw* -Directory
Get-ChildItem D:\Qt\Tools\mingw* -Directory
Get-ChildItem D:\QT\Tools\mingw* -Directory
```

### 2. 构建命令（根据实际路径调整）
```powershell
# 把下面两个路径改成实际找到的
$env:Path = "<Qt路径>\mingw_64\bin;<Qt路径>\mingw_64\libexec;<Qt路径>\Tools\mingwXXXX_64\bin;" + $env:Path

cd <项目目录>

# 首次构建或 .pro 改动后需要 qmake
qmake library-management-system.pro

# 增量构建
mingw32-make -j4
```

### 3. 数据库配置
项目使用 ODBC 连接 MySQL：
- DSN 名称：`dbcd`
- 用户：`root`
- 密码：`123456`

`main.cpp` 的 `ensureSchema()` 在首次启动时自动建表和种子数据。修改 `main.cpp` 第 15-18 行的连接参数以匹配新环境。

### 4. 如果 qmake 报错 "Cannot run compiler g++"
说明 MinGW 路径没配对，检查 `Tools\mingw*` 目录名是否和 PATH 里一致。

### 5. 构建成功后
```powershell
.\release\library-management-system.exe
```

## 项目结构

```
├── main.cpp                    # 入口：DB 连接 + ensureSchema() + 登录
├── mainmenu.h/.cpp             # 主窗口 Shell（侧边栏 + QStackedWidget）
├── logindialog.h/.cpp          # 登录对话框（卡片式 UI）
├── borrowwindow.h/.cpp         # 借书
├── returnwindow.h/.cpp         # 还书
├── bookmanagewindow.h/.cpp     # 图书 CRUD + 导出
├── addbookwindow.h/.cpp        # 添加图书（弹出窗口）
├── editbookdialog.h/.cpp       # 编辑图书（模态对话框）
├── readermanagewindow.h/.cpp   # 读者 CRUD + 导出
├── readereditdialog.h/.cpp     # 添加/编辑读者（模态）
├── categorymanagedialog.h/.cpp # 分类管理（模态）
├── reminderwindow.h/.cpp       # 到期提醒（双标签页）
├── statswindow.h/.cpp          # 统计看板（7 指标卡片）
├── logwindow.h/.cpp            # 操作日志查看
├── loghelper.h/.cpp            # 日志写入工具（static）
├── library-management-system.pro  # qmake 项目文件
├── README.md                   # GitHub 项目首页
├── LICENSE                     # MIT
└── .gitignore
```

## 架构

### 启动流程
```
main.cpp → 连接数据库 → ensureSchema() → LoginDialog (模态) → MainMenu (show)
```

### 主窗口结构
```
┌──────────────┬─────────────────────────┐
│ 左侧导航栏    │ QStackedWidget (右侧)    │
│ (200px #2c3e) │  0: 欢迎首页             │
│              │  1: BorrowWindow         │
│ 📚 Logo      │  2: ReturnWindow         │
│ 🏠 首页      │  3: BookManageWindow     │
│ 📥 借书      │  4: ReaderManageWindow   │
│ 📤 还书      │  5: ReminderWindow       │
│ 📖 图书管理  │  6: StatsWindow          │
│ 👤 读者管理  │  7: LogWindow            │
│ ⏰ 到期提醒  │                         │
│ 📊 统计看板  │                         │
│ 📋 操作日志  │                         │
│ 🚪 退出      │                         │
└──────────────┴─────────────────────────┘
```

### 类层级
```
QDialog
  ├── LoginDialog           (模态登录)
  ├── EditBookDialog        (编辑图书)
  ├── ReaderEditDialog      (添加/编辑读者)
  └── CategoryManageDialog  (管理分类)

QWidget
  ├── MainMenu              (主窗口 Shell)
  ├── BorrowWindow          (借书)
  ├── ReturnWindow          (还书)
  ├── BookManageWindow      (图书 CRUD)
  ├── AddBookWindow         (添加图书，弹出)
  ├── ReaderManageWindow    (读者 CRUD)
  ├── ReminderWindow        (到期提醒)
  ├── StatsWindow           (统计看板)
  └── LogWindow             (操作日志)

工具类
  └── LogHelper             (static 日志写入)
```

## 数据库

### 连接（main.cpp）
```cpp
QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
db.setDatabaseName("dbcd");    // ODBC DSN
db.setUserName("root");
db.setPassword("123456");
```

### 6 张表
| 表 | 用途 | 关键字段 |
|----|------|----------|
| `users` | 系统操作员 | user_id, username, password, role, status |
| `readers` | 读者 | reader_id, reader_code, name, status, borrowed_count |
| `books` | 图书 | book_id, isbn, title, author, publisher, category_id, total_count, available_count |
| `categories` | 分类 | category_id, category_name |
| `borrow_records` | 借阅记录 | record_id, book_id, reader_id, borrow_date, due_date, return_date, fine_amount |
| `operation_logs` | 操作日志 | log_id, user_id, operation_type, operation_detail, ip_address |

### ensureSchema() 自动执行
- `CREATE TABLE IF NOT EXISTS` 全部 6 张表（users, readers, categories, books, borrow_records, operation_logs）
- 随后调用 `seedIfEmpty()`：检查 books 表是否为空，为空则读取 `seed_data.sql` 自动灌入种子数据
- 种子数据包含 3 用户、12 分类、200 图书、80 读者、~150 借阅记录、~40 操作日志

## 代码规范

### 模式
- **头文件**：Qt 类前向声明 (`class QLineEdit;`)，`.cpp` 中 `#include` 完整头
- **信号槽**：新式语法 `connect(btn, &QPushButton::clicked, this, &Class::onXxx)`
- **数据库**：参数化查询 `prepare()` + `addBindValue()`，多步操作用事务
- **窗口**：嵌入页面 parent = `m_stack`，弹出窗口 `WA_DeleteOnClose`
- **布局**：root = QVBoxLayout(0 margin)，标题栏 header + body(QLabel + QHBoxLayout padding 16,12)

### 错误处理
| 方法 | 场景 |
|------|------|
| `QMessageBox::critical()` | 数据库失败 |
| `QMessageBox::warning()` | 验证失败 |
| `QMessageBox::information()` | 操作成功 |
| `QMessageBox::question()` | 确认操作 |

### 配色
- 侧边栏：`#2c3e50` / `#1a252f`（Logo 区）
- 主色调：`#3498db`（选中/强调）
- 内容背景：`#f5f6fa`
- 标题栏：白底 + 4px 蓝色左边框

## 修改代码后的构建

### 只改了 .cpp 文件
```powershell
mingw32-make -j4
```

### 新增/删除了文件
```powershell
qmake library-management-system.pro
mingw32-make -j4
```

新增文件后还需编辑 `library-management-system.pro`，在 `SOURCES` / `HEADERS` 中添加对应条目。

## 已知问题
1. `logindialog.cpp` — 登录日志在认证前记录，此时 m_userId = -1
2. `borrowwindow.cpp` — 图书搜索 SELECT 只返回 3 列但代码访问了 author/isbn
3. `users.password` 明文存储
4. 还书日志在事务外记录，回滚时日志不一致
5. RBAC 按钮隐藏代码被注释（`mainmenu.cpp`）
