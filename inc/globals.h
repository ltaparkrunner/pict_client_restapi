#ifndef GLOBALS_H
#define GLOBALS_H
#include <QStringList>

namespace Config {
    inline const QStringList& netPrefixes() {
        static const QStringList list = {"https://minio:9000", "http://minio:9000", "/net"};
        return list;
    }
}
#endif // GLOBALS_H
