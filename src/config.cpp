#include "config.h"
#include <QSettings>

Config::Config()
{
    QSettings settings("QDia","QDia");
    showGrid=settings.value("view/showGrid", true).toBool();
    style=settings.value("view/style", "<default>").toString();
}

Config::~Config()
{
    QSettings settings("QDia","QDia");
    settings.setValue("view/showGrid", showGrid);
    settings.setValue("view/style", style);
}
