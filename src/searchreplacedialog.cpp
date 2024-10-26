#include "searchreplacedialog.h"

SearchReplaceDialog::SearchReplaceDialog(QWidget *parent) : QDialog(parent){
    // create a dialog with two text fields, find next and replace buttons
    // and a checkbox for case sensitivity
    QLabel *findLabel = new QLabel(tr("Find:"));
    findLineEdit = new QLineEdit;
    QLabel *replaceLabel = new QLabel(tr("Replace:"));
    replaceLineEdit = new QLineEdit;
    QPushButton *findButton = new QPushButton(tr("Find &Next"));
    connect(findButton,&QPushButton::clicked,this,&SearchReplaceDialog::m_findNext);
    QPushButton *replaceButton = new QPushButton(tr("&Replace"));
    connect(replaceButton,&QPushButton::clicked,this,&SearchReplaceDialog::m_replace);
    QPushButton *replaceAllButton = new QPushButton(tr("Replace &All"));
    connect(replaceAllButton,&QPushButton::clicked,this,&SearchReplaceDialog::m_replaceAll);
    QPushButton *closeButton = new QPushButton(tr("Close"));
    connect(closeButton,&QPushButton::clicked,this,&SearchReplaceDialog::close);
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(findLabel,0,0);
    layout->addWidget(findLineEdit,0,1);
    layout->addWidget(findButton,0,2);
    layout->addWidget(replaceLabel,1,0);
    layout->addWidget(replaceLineEdit,1,1);
    layout->addWidget(replaceButton,1,2);
    layout->addWidget(replaceAllButton,1,3);
    layout->addWidget(closeButton,2,2);
    setLayout(layout);
}

QString SearchReplaceDialog::findText() const
{
    return findLineEdit->text();
}

QString SearchReplaceDialog::replaceText() const
{
    return replaceLineEdit->text();
}

void SearchReplaceDialog::m_findNext()
{
    emit findNext();
}

void SearchReplaceDialog::m_replace()
{
    emit replace();
}

void SearchReplaceDialog::m_replaceAll()
{
    emit replaceAll();
}
