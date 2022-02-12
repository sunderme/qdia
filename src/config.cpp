#include "config.h"

Config::Config()
{
    QSettings settings("QDia","QDia");
    showGrid=settings.value("view/showGrid", true).toBool();
}

Config::~Config()
{
    QSettings settings("JS","QDia");
    settings.setValue("view/showGrid", showGrid);
}
