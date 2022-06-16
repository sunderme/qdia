#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qdia_" + QLocale(locale).name();
        QString fn=qApp->applicationDirPath()+"/translations/" + baseName;
        if (translator.load(fn)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w(argc, argv);
    w.show();
    return a.exec();
}
