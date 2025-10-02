#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QDir>

#ifdef CPP_CRASH_HANDLER
#include <signal.h>
#include <stacktrace>
#include <iostream>
#include <fstream>

#include <signal.h>
#include <unistd.h>

void handler(int sig) {
    (void)sig;
    /* De-register this signal in the hope of avoiding infinite loops
     * if asyns signal unsafe things fail later on. But can likely still deadlock. */
    signal(sig, SIG_DFL);
    // std::stacktrace::current
    auto trace=std::stacktrace::current();
    QString backtraceFilename = QDir::tempPath() + QString("/qdia_backtrace.txt");
    std::ofstream f(backtraceFilename.toLatin1());
    // Write to the file
    f << std::stacktrace::current();

    // Close the file
    f.close();

    std::cout<<std::stacktrace::current();

    _Exit(1);
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "qdia_" + QLocale(locale).name();
#ifdef Q_OS_MACOS
        QString fn=":/i18n/translation/" + baseName+".qm";
#else
        QString fn=qApp->applicationDirPath()+"/translations/" + baseName+".qm";
#endif
        if (translator.load(fn)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w(argc, argv);
    w.show();

#ifdef CPP_CRASH_HANDLER
    // cpp23 crash handler
    signal(SIGSEGV, handler);
#endif

    return a.exec();
}
