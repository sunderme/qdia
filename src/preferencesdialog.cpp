#include "preferencesdialog.h"

#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyleFactory>

PreferencesDialog::PreferencesDialog(QWidget *parent)
{
    // generate a dialog
    // with selection dropdown to select the windows style

    QLabel *styleLabel = new QLabel(tr("Style:"));
    QString defaultStyleName = QApplication::style()->objectName();
    styleComboBox = new QComboBox;
    // add all qt styles to the dropdown
    const QStringList availableStyles=QStyleFactory::keys();
    styleComboBox->addItems(availableStyles);
    // select the current style
    int n=availableStyles.indexOf(defaultStyleName,0,Qt::CaseInsensitive);
    if(n>=0){
        styleComboBox->setCurrentIndex(n);
    }

    // and buttons for ok/cancel the dialog
    QPushButton *okButton = new QPushButton(tr("OK"));
    connect(okButton,&QPushButton::clicked,this,&PreferencesDialog::accept);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton,&QPushButton::clicked,this,&PreferencesDialog::reject);

    // layout the dialog
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(styleLabel,0,0);
    layout->addWidget(styleComboBox,0,1);
    layout->addWidget(okButton,1,0);
    layout->addWidget(cancelButton,1,1);
    setLayout(layout);
}

QString PreferencesDialog::getStyle()
{
    return styleComboBox->currentText();
}
