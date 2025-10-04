/**
 * @file config.h
 * @brief Configuration management header for QDia application
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

/**
 * @class Config
 * @brief Manages application configuration settings
 *
 * This class handles persistent storage and retrieval of application settings
 * using QSettings. It manages preferences like grid visibility and UI style.
 */
class Config
{
public:
    Config();
    ~Config();

    // global configuration settings
    /**
     * @var bool showGrid
     * @brief Controls visibility of the grid in the application
     *
     * Default value: true
     */
    bool showGrid;

    /**
     * @var QString style
     * @brief Stores the current UI style name
     *
     * Default value: "<default>" (system default style)
     */
    QString style;

};

#endif // CONFIG_H
