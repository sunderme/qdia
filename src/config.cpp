/**
 * @file config.cpp
 * @brief Configuration management implementation for QDia application
 */

#include "config.h"
#include <QSettings>

/**
 * @brief Config constructor
 *
 * Loads configuration settings from persistent storage using QSettings.
 * Initializes member variables with stored values or defaults if not present.
 *
 * Settings are stored in:
 * - Organization: QDia
 * - Application: QDia
 */
Config::Config()
{
    QSettings settings("QDia","QDia");
    showGrid=settings.value("view/showGrid", true).toBool();
    style=settings.value("view/style", "<default>").toString();
}

/**
 * @brief Config destructor
 *
 * Saves current configuration settings to persistent storage using QSettings.
 * Automatically called when the Config object is destroyed.
 */
Config::~Config()
{
    QSettings settings("QDia","QDia");
    settings.setValue("view/showGrid", showGrid);
    settings.setValue("view/style", style);
}
