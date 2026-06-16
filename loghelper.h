#ifndef LOGHELPER_H
#define LOGHELPER_H

#include <QString>

class LogHelper {
public:
    static void record(int userId, const QString &type, const QString &detail);
};

#endif // LOGHELPER_H