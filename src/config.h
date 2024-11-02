#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config
{
public:
    Config();
    ~Config();

    // global configuration settings
    bool showGrid;
    QString style;

};

#endif // CONFIG_H
