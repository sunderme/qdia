#include "config.h"
#include <QSettings>

Config::Config()
{
    QSettings settings("QDia","QDia");
    showGrid=settings.value("view/showGrid", true).toBool();
}

Config::~Config()
{
    QSettings settings("QDia","QDia");
    settings.setValue("view/showGrid", showGrid);
}
