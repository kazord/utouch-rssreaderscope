#include "db.h"

#include <QMutex>
#include <QDebug>

#include <QStringList>
#include <QSqlQuery>
#include <qsqlerror.h>
#include <QStandardPaths>
#include <QQueue>
#include <QLatin1String>

#define TABLE_NAME "feeds_1"

client::DB* client::DB::_instance = nullptr;

client::DB::DB()
{
    qDebug() << "Starting database";
    _db = QSqlDatabase::addDatabase("QSQLITE");
    #if 1
    _db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+QString::fromStdString("/.local/share/unity-scopes/leaf-net/%1/sqlite.db").arg(PACKAGE_NAME));
    #else
        _db.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::HomeLocation)+QString::fromStdString("/.local/%1_sqlite.db").arg(PACKAGE_NAME));
    #endif
    if(!_db.open())
        qDebug() << "Database starting fail";
    else {
        QStringList tables = _db.tables();
        if (!tables.contains(TABLE_NAME, Qt::CaseInsensitive)) {
            QSqlQuery q;
            if (!q.exec(QLatin1String(
    "create table " TABLE_NAME "(id integer primary key, name varchar, url varchar, favicon varchar, main varchar, item varchar, link varchar, author varchar, category varchar, title varchar, desc varchar, date varchar, multimedia varchar, color varchar, date_format int, html_decode int)")))
                qDebug() << "Can't create feeds table : "<< q.lastError().text();
            else {
                qDebug() << "Feeds table created";
                //last version
                 if (tables.contains("feeds", Qt::CaseInsensitive)) {
                     //migration
                     if(q.exec(QLatin1String(
                         "INSERT INTO " TABLE_NAME "(name, url, main, item, link, author, category, title, desc, date, multimedia, color, date_format, html_decode, favicon) "
                                                 "SELECT feeds.name, feeds.url, feeds.main, feeds.item, feeds.link, feeds.author, feeds.category, feeds.title, feeds.desc, feeds.date, feeds.multimedia, feeds.color, feeds.date_format, feeds.html_decode, feeds.favicon "
                                                 "FROM feeds")))
                        q.exec(QLatin1String("DROP TABLE feeds"));
                     else
                          qDebug() << "Can't upgrade table : "<< q.lastError().text();
                 }
            }
        }
    }
}

client::DB::~DB() {
    _db.close();
}

client::DB* client::DB::instance() {
    QMutex mutex;
    mutex.lock();
    if(nullptr == _instance)
        _instance = new DB();
    mutex.unlock();
    return _instance;
}

bool client::DB::removeFeed(int id) {
    QSqlQuery q;
    if (!q.prepare(QLatin1String(
                       "delete from " TABLE_NAME " where id = ?"
                       ))) {
        qDebug() << "remove feed " << id << " failed : "<< q.lastError().text();
        return false;
    }
    q.addBindValue(id);
    return q.exec();
}

int client::DB::idFromUrl(const QString& url) {
    //prequery
    QSqlQuery pq;
    if (!pq.prepare(QLatin1String(
                       "select id from " TABLE_NAME " where url = ?"
                       ))) {
        qDebug() << "url check fail "<< pq.lastError().text();
        return -1;
    }
    pq.addBindValue(url);
    pq.exec();
    if(pq.next()) {
        return pq.value(0).toInt();
    }
    else
        return -1;
}

bool client::DB::addFeed(const feed::Infos &info) {
    if(idFromUrl(info.url) != -1) {
        qDebug() << "url already in DB "<< info.url;
        return false;
    }
    QSqlQuery q;
    if (!q.prepare(QLatin1String(
                       "insert into " TABLE_NAME "(name, url, main, item, link, author, category, title, desc, date, multimedia, color, date_format, html_decode, favicon) values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
                       ))) {
        qDebug() << "New feed saving failed : "<< q.lastError().text();
        return false;
    }
    q.addBindValue(info.name);
    q.addBindValue(info.url);
    q.addBindValue(info.main);
    q.addBindValue(info.item);
    q.addBindValue(info.link);
    q.addBindValue(info.author);
    q.addBindValue(info.category);
    q.addBindValue(info.title);
    q.addBindValue(info.desc);
    q.addBindValue(info.date);
    q.addBindValue(info.multimedia);
    q.addBindValue(info.color);
    q.addBindValue(info.dateFormat);
    q.addBindValue((info.html));
    q.addBindValue(info.favicon);

    return q.exec();
}

bool client::DB::updateFeed(int id, const feed::Infos &newinfo) {
    QSqlQuery q;
    if (!q.prepare(QLatin1String(
                       "update " TABLE_NAME " SET name = ?, url = ?, main = ?, item = ?, link = ?, author = ?, category = ?, title = ?, desc = ?, date = ?, multimedia = ?, color = ?, date_format = ?, html_decode = ?, favicon = ? WHERE id = ?"
                       ))) {
        qDebug() << "Update feed failed : "<< q.lastError().text();
        return false;
    }
    q.addBindValue(newinfo.name);
    q.addBindValue(newinfo.url);
    q.addBindValue(newinfo.main);
    q.addBindValue(newinfo.item);
    q.addBindValue(newinfo.link);
    q.addBindValue(newinfo.author);
    q.addBindValue(newinfo.category);
    q.addBindValue(newinfo.title);
    q.addBindValue(newinfo.desc);
    q.addBindValue(newinfo.date);
    q.addBindValue(newinfo.multimedia);
    q.addBindValue(newinfo.color);
    q.addBindValue(newinfo.dateFormat);
    q.addBindValue(newinfo.html);
    q.addBindValue(newinfo.favicon);

    q.addBindValue(id);

    if (q.exec())
        return true;
    else {
        qDebug() << "Update feed failed : "<< q.lastError().text();
            return false;
    }
}
bool client::DB::updateFeed(int id, QString& key, QString& value) {
    QSqlQuery q;
    QString req = QString("update ").append(TABLE_NAME).append(" SET ").append(key).append(" = ? WHERE id = ?");
    if (!q.prepare(req.toLatin1())) {
        qDebug() << "Update feed failed : "<< q.lastError().text();
        return false;
    }
    q.addBindValue(value);
    q.addBindValue(id);

    if (q.exec())
        return true;
    else {
        qDebug() << "Update feed failed : "<< q.lastError().text();
            return false;
    }
}
bool client::DB::updateFeed(int id, QString& key, int value) {
    QSqlQuery q;
    QString req = QString("update ").append(TABLE_NAME).append(" SET ").append(key).append(" = ? WHERE id = ?");
    if (!q.prepare(req.toLatin1())) {
        qDebug() << "Update feed failed : "<< q.lastError().text();
        return false;
    }
    q.addBindValue(value);
    q.addBindValue(id);

    if (q.exec())
        return true;
    else {
        qDebug() << "Update feed failed : "<< q.lastError().text();
            return false;
    }
}

QQueue<feed::Infos>  client::DB::feeds() {

    QQueue<feed::Infos> ret;
    QSqlQuery q;
    if(!q.exec("SELECT id, url, main, item, link, author, category, title, desc, date, multimedia, color, date_format, html_decode, name, favicon FROM " TABLE_NAME))
    {
        qDebug() << "Feeds fetch failed : "<< q.lastError().text();
        return ret;
    }
    while (q.next()) {
        feed::Infos f;
        f.id = q.value(0).toInt();
        f.url = q.value(1).toString();
        f.main = q.value(2).toString();
        f.item = q.value(3).toString();
        f.link = q.value(4).toString();
        f.author = q.value(5).toString();
        f.category = q.value(6).toString();
        f.title = q.value(7).toString();
        f.desc = q.value(8).toString();
        f.date = q.value(9).toString();
        f.multimedia = q.value(10).toString();
        f.color = q.value(11).toString();
        f.dateFormat = (feed::DateFormat) q.value(12).toInt();
        f.html = (feed::HTML) q.value(13).toInt();
        f.name = q.value(14).toString();
        f.favicon = q.value(15).toString();
        ret.append(f);
    }
    return ret;
}
feed::Infos client::DB::feed(const int id) {
    feed::Infos f;
    QSqlQuery q;
    if(!q.prepare("SELECT id, url, main, item, link, author, category, title, desc, date, multimedia, color, date_format, html_decode, name, favicon FROM " TABLE_NAME " WHERE id = ?"))
    {
        qDebug() << "Feeds fetch failed : "<< q.lastError().text();
        return f;
    }
    q.addBindValue(id);
    if(!q.exec()) {
        qDebug() << "Feeds fetch failed : "<< q.lastError().text();
        return f;
    }
    if(q.next()) {

        f.id = q.value(0).toInt();
        f.url = q.value(1).toString();
        f.main = q.value(2).toString();
        f.item = q.value(3).toString();
        f.link = q.value(4).toString();
        f.author = q.value(5).toString();
        f.category = q.value(6).toString();
        f.title = q.value(7).toString();
        f.desc = q.value(8).toString();
        f.date = q.value(9).toString();
        f.multimedia = q.value(10).toString();
        f.color = q.value(11).toString();
        f.dateFormat = (feed::DateFormat) q.value(12).toInt();
        f.html = (feed::HTML) q.value(13).toInt();
        f.name = q.value(14).toString();
        f.favicon = q.value(15).toString();
    }
    return f;
}
