#ifndef DB_H
#define DB_H

#include <QSql>
#include <QSqlDatabase>
#include <QQueue>

#include "types.h"

namespace client {
    class DB
    {
    public:
        static DB *instance();
        ~DB();
        QQueue<feed::Infos> feeds();
        feed::Infos feed(const int id);
        bool addFeed(const feed::Infos &info);
        bool updateFeed(int id, const feed::Infos &info);
        bool updateFeed(int id, QString& key, QString& value);
        bool updateFeed(int id, QString& key, int value);
        bool removeFeed(int id);
        int idFromUrl(const QString& url);

    private:
        DB();
        static DB* _instance;
        QSqlDatabase _db;
    };
}
#endif // DB_H
