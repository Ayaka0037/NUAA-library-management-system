#ifndef BORROWWINDOW_H
#define BORROWWINDOW_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QTableWidget;
class QLabel;
struct BookInfo{
    int id;
    QString title;
    QString author;
    QString isbn;
    int available;
};

class BorrowWindow : public QWidget
{
    Q_OBJECT
public:
    explicit BorrowWindow(int operatorId, QWidget *parent = nullptr);

private slots:
    void onSearchReader();
    void onSearchBook();
    void onConfirmBorrow();
    void onBookSelected(const BookInfo &book);
private:
    void setupUI();
    void clearReaderInfo();
    void clearBookInfo();
    void showBookSelection(const QList<BookInfo> &books);
    QLineEdit   *m_readerCodeEdit;
    QPushButton *m_searchReaderBtn;
    QLabel      *m_readerNameLabel;
    QLabel      *m_readerStatusLabel;
    QLabel      *m_borrowedCountLabel;

    QLineEdit   *m_bookKeywordEdit;
    QPushButton *m_searchBookBtn;
    QLabel      *m_bookTitleLabel;
    QLabel      *m_bookAvailableLabel;

    QPushButton *m_borrowBtn;

    int m_operatorId;      // 操作员ID（登录用户）
    int m_currentReaderId;
    int m_currentBookId;
    QString m_currentBookTitle;
    int m_currentAvailable;
};

#endif // BORROWWINDOW_H