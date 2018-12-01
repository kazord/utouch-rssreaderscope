#ifndef CLIENTXML_H
#define CLIENTXML_H

#include <QStringList>
#include <QXmlStreamReader>

#include "types.h"

namespace client {

namespace XML {
    QStringList fromOPML(QXmlStreamReader &reader);
    QStringList foundChannel(QXmlStreamReader &reader, const QString &parent, const QString &localname);
    void extractInnerLinks(articles::Article& article, QString desc);
    QString fastHTMLDecode(QString desc);
    QStringList parseTitle(QXmlStreamReader &xml);
    bool autodetect(QXmlStreamReader &reader, autodetect::Infos &info);
    void extractArticles(const feed::Infos& param, articles::ArticleList& result, QXmlStreamReader& XMLroot);
    QString cleanHTML(QString desc);
}

}



#endif // CLIENTXML_H
