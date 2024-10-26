#ifndef SEARCHREPLACEDIALOG_H
#define SEARCHREPLACEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>

class SearchReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    SearchReplaceDialog(QWidget *parent = nullptr);
    QString findText() const;
    QString replaceText() const;
signals:
    void findNext();
    void replace();
    void replaceAll();
private slots:
    void m_findNext();
    void m_replace();
    void m_replaceAll();
private:
    QLineEdit *findLineEdit;
    QLineEdit *replaceLineEdit;
};

#endif // SEARCHREPLACEDIALOG_H
