#include "xml.h"

#include <QDateTime>
#include <QRegExp>
#include <QDebug>
#include <stdexcept>
#include <QTextCodec>
#include <QStringList>

#define TRUNC_LENGTH 17

namespace client {

namespace XML {

namespace {
    QString readText(QXmlStreamReader& xml)
    {
        xml.readNext();
        if (xml.tokenType() != QXmlStreamReader::Characters) {
            return QString();
        }

        return xml.text().toString();
    }
    QString readContent(QXmlStreamReader& xml)
    {
        QString content = "";
        QStringRef target = xml.name();
        xml.readNext();
        while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == target))
        {

            if (xml.isStartElement())
                content.append("<"+xml.name().toString()+">");
            else if (xml.isEndElement())
                content.append("</"+xml.name().toString()+">");
            else if(xml.tokenType() == QXmlStreamReader::Characters)
                content.append(xml.text().toString());

            xml.readNext();

        }
        return content;//xml.readElementText(QXmlStreamReader::IncludeChildElements);

    }
    QStringList subFoundChannel(QXmlStreamReader &xml, QString parent, QString ignore) {
        QStringList list;
        while (!xml.atEnd() && !(xml.isEndElement() && xml.name().toString() == parent))
        {
            if (xml.isStartElement() && !xml.name().toString().isEmpty()) {
                if(xml.name().toString() == ignore) {
                    subFoundChannel(xml, ignore, "");//ignore entry zone
                }
                else if(xml.name().toString() == "title") {
                    list << readText(xml);
                }
            }
            xml.readNext();

        }
        return list;
    }

    void parse4Analyse(QXmlStreamReader &xml, autodetect::Infos &info, QString parentName) {
        autodetect::Node node;
        node.name = xml.name().toString();
        node.parent = parentName;
        bool next = true;
        //qDebug() << node.parent << node.name;
        if(xml.name().toString().toStdString() == "channel" || xml.name().toString().toStdString() == "feed") {
        }
        if(xml.name().toString().toStdString() == "item" || xml.name().toString().toStdString() == "entry") {
            if(info.item.name != "item")
                info.item = node;
        }
        if(xml.name().toString().toStdString() == "title") {
            info.title = node;
        }
        if(xml.name().toString().toStdString() == "category") {
            info.category = node;
        }
        if(xml.name().toString().toStdString() == "description" ||
                xml.name().toString().toStdString() == "subtitle" ||
                xml.name().toString().toStdString() == "summary" ||
                xml.name().toString().toStdString() == "content") {
            info.desc = node;
            QString content = readContent(xml);
            next = false;
            if(content.isEmpty()) //this is a trap description, ignore it
                return;
            QRegExp rx("&[a-zA-Z]{2,4};");
            if(content.contains(rx)) {
                info.htmldecode = true;
            }
        }
        //if(xml.name().toString().toStdString() == logo) {
        if(xml.name().toString().toStdString() == "image" ||
                xml.name().toString().toStdString() == "enclosure") {
            info.multimedia = node;
        }
        if(xml.name().toString().toStdString() == "link") {
            info.link = node;
        }
        if(xml.name().toString().toStdString() == "pubDate" ||
                xml.name().toString().toStdString() == "published" ||
                xml.name().toString().toStdString() == "updated" ||
                xml.name().toString().toStdString() == "date") {
            info.date = node;
            QString datetxt = readText(xml);
            if(QDateTime::fromString(datetxt,Qt::ISODate).toMSecsSinceEpoch() > QDateTime::fromString(datetxt,Qt::RFC2822Date).toMSecsSinceEpoch() &&
                    QDateTime::fromString(datetxt,Qt::ISODate).toMSecsSinceEpoch() >= QDateTime::fromString(datetxt).toMSecsSinceEpoch()) {
                info.dateFormat = feed::DateFormat::ISO;
            }
            else if(QDateTime::fromString(datetxt,Qt::RFC2822Date).toMSecsSinceEpoch() > QDateTime::fromString(datetxt,Qt::ISODate).toMSecsSinceEpoch() &&
                    QDateTime::fromString(datetxt,Qt::RFC2822Date).toMSecsSinceEpoch() >= QDateTime::fromString(datetxt).toMSecsSinceEpoch()) {
                info.dateFormat = feed::DateFormat::RFC;
            }
            else {
                info.dateFormat = feed::DateFormat::TXT;
            }
        }
        if(xml.name().toString().toStdString() == "author" ||
                xml.name().toString().toStdString() == "contributor" ||
                xml.name().toString().toStdString() == "creator") {
            info.author  = node;
        }
        //cerr << "- " << node.name.toStdString() << " " << node.parent.toStdString()  << endl;
        if(!info.list.contains(node))
            info.list.append(node);
        if(next)
            xml.readNext();
        while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == node.name))
        {
            //qDebug() << xml.name() << xml.isStartElement() << xml.isEndElement();
            if (xml.isStartElement() && !xml.isEndElement() && !xml.name().toString().isEmpty()) {
                parse4Analyse(xml, info, node.name);
            }
            else
                xml.readNext();

        }
    }
}
QStringList fromOPML(QXmlStreamReader &reader) {
    QStringList urls;
    QString url;
    QString title;
    while (!reader.atEnd() && !reader.hasError())
    {
        if(reader.isStartElement()) {
            //qDebug() << xml.name();
                QXmlStreamAttributes attributes = reader.attributes();
                if (attributes.hasAttribute("type") && attributes.value("type") == "rss") {
			if (attributes.hasAttribute("xmlUrl")) {
			    url = attributes.value("xmlUrl").toString();
			    if(attributes.hasAttribute("title")) {
				    title = attributes.value("title").toString();
				}
			    else title = "";
			    urls << url << title; 
			}
		}
	}
        reader.readNext();
    }//loop
    return urls;
}

QStringList foundChannel(QXmlStreamReader &reader, const QString &parent, const QString &localname) {
    QStringList list;
    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement) {
            if(reader.name().toString() == parent) {
                list  << subFoundChannel(reader, parent, localname);
            }
        }
    }
    return list;
}


bool autodetect(QXmlStreamReader &reader, autodetect::Infos &info) {
    parse4Analyse(reader,info,"");
    //if(info.list.length() > 0)
    if(!info.item.name.isEmpty() && ( info.item.name == info.title.parent || info.item.name == info.desc.parent || info.item.name == info.link.parent || info.item.name == info.category.parent  || info.item.name == info.author.parent ))
        return true;
    else
        return false;

}

QStringList parseTitle(QXmlStreamReader &xml) {
    xml.readNext();
    QStringList titles;
    while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == "channel"))
    {
        if (xml.isStartElement() && xml.name() == "title") {
            titles << readText(xml);
        }
        xml.readNext();
    }
    if(xml.isEndElement() && xml.name() == "channel")
        xml.readNext();
    return titles;
}

/****************** articles linked ************************/
QString cleanHTML(QString desc) {
    QRegExp rx = QRegExp("<[^>]+>");
    rx.setMinimal(true);
    desc.replace(rx, "");
    return desc;
}



void extractInnerLinks(articles::Article& article, QString desc) {
    QRegExp rx = QRegExp("<a (target=\"[^\"]+\" )?href=\"([^\"]+)\"[^>]*>(.+)</a>");
    rx.setMinimal(true);
    int pos = 0;
    while ((pos = rx.indexIn(desc,pos)) != -1) {
        if(rx.cap(3).size() > TRUNC_LENGTH + 3) {
            QString shorter = rx.cap(3);
            shorter.truncate(TRUNC_LENGTH);
            article.innerlinks.emplace_back(make_pair(shorter.append("...").toStdString(),rx.cap(2).toStdString()));
        } else {
            article.innerlinks.emplace_back(make_pair(rx.cap(3).toStdString(),rx.cap(2).toStdString()));
        }
        pos += rx.matchedLength();
    }
}

QString fastHTMLDecode(QString desc) {
    QRegExp rx1("&#x(..);");
    int pos = 0;
    bool ok;
    while ((pos = rx1.indexIn(desc,pos)) != -1) {
       desc.replace(pos, 6, QChar(rx1.cap(1).toInt(&ok,16)));
    }
    QRegExp rx2("&#\d{1,4};");
    pos = 0;
    while ((pos = rx2.indexIn(desc,pos)) != -1) {
       desc.replace(pos, 6, QChar(rx2.cap(1).toInt(&ok,10)));
    }
    desc.replace(QString("&amp;"), QString("&"));
    desc.replace(QString("&apos;"), QString("'"));
    desc.replace(QString("&quot;"), QString("\""));
    desc.replace(QString("&mdash;"), QString("—"));
    desc.replace(QString("&ndash;"), QString("–"));
    desc.replace(QString("&lt;"), QString("<"));
    desc.replace(QString("&gt;"), QString(">"));
    desc.replace(QString("&nbsp;"), QString(" "));
    desc.replace(QString("&rsquo;"), QString("’"));
    desc.replace(QString("&laquo;"), QString("«"));
    desc.replace(QString("&raquo;"), QString("»"));
    //desc.replace(QString("&frac14;"), QString("¼"));
    //desc.replace(QString("&frac12;"), QString("½"));
    //desc.replace(QString("&frac34;"), QString("¾"));
    desc.replace(QString("&Agrave;"), QString("À"));
    desc.replace(QString("&Aacute;"), QString("Á"));
    desc.replace(QString("&Acirc;"), QString("Â"));
    //desc.replace(QString("&Atilde;"), QString("Ã"));
    desc.replace(QString("&Auml;"), QString("Ä"));
    //desc.replace(QString("&Aring;"), QString("Å"));
    //desc.replace(QString("&AElig;"), QString("Æ"));
    desc.replace(QString("&Ccedil;"), QString("Ç"));
    desc.replace(QString("&Egrave;"), QString("È"));
    //desc.replace(QString("&Eacute;"), QString("É"));
    //desc.replace(QString("&Ecirc;"), QString("Ê"));
    desc.replace(QString("&Euml;"), QString("Ë"));
    desc.replace(QString("&Igrave;"), QString("Ì"));
    desc.replace(QString("&Iacute;"), QString("Í"));
    desc.replace(QString("&Icirc;"), QString("Î"));
    desc.replace(QString("&Iuml;"), QString("Ï"));
    //desc.replace(QString("&ETH;"), QString("Ð"));
    //desc.replace(QString("&Ntilde;"), QString("Ñ"));
    desc.replace(QString("&Ograve;"), QString("Ò"));
    desc.replace(QString("&Oacute;"), QString("Ó"));
    desc.replace(QString("&Ocirc;"), QString("Ô"));
    //desc.replace(QString("&Otilde;"), QString("Õ"));
    desc.replace(QString("&Ouml;"), QString("Ö"));
    desc.replace(QString("&times;"), QString("×"));
    desc.replace(QString("&Oslash;"), QString("Ø"));
    desc.replace(QString("&Ugrave;"), QString("Ù"));
    desc.replace(QString("&Uacute;"), QString("Ú"));
    desc.replace(QString("&Ucirc;"), QString("Û"));
    desc.replace(QString("&Uuml;"), QString("Ü"));
    //desc.replace(QString("&Yacute;"), QString("Ý"));
    //desc.replace(QString("&THORN;"), QString("Þ"));
    //desc.replace(QString("&szlig;"), QString("ß"));
    desc.replace(QString("&agrave;"), QString("à"));
    desc.replace(QString("&aacute;"), QString("á"));
    desc.replace(QString("&acirc;"), QString("â"));
    //desc.replace(QString("&atilde;"), QString("ã"));
    desc.replace(QString("&auml;"), QString("ä"));
    //desc.replace(QString("&aring;"), QString("å"));
    //desc.replace(QString("&aelig;"), QString("æ"));
    desc.replace(QString("&ccedil;"), QString("ç"));
    desc.replace(QString("&egrave;"), QString("è"));
    desc.replace(QString("&eacute;"), QString("é"));
    desc.replace(QString("&ecirc;"), QString("ê"));
    desc.replace(QString("&euml;"), QString("ë"));
    desc.replace(QString("&igrave;"), QString("ì"));
    desc.replace(QString("&iacute;"), QString("í"));
    desc.replace(QString("&icirc;"), QString("î"));
    desc.replace(QString("&iuml;"), QString("ï"));
    //desc.replace(QString("&eth;"), QString("ð"));
    //desc.replace(QString("&ntilde;"), QString("ñ"));
    desc.replace(QString("&ograve;"), QString("ò"));
    desc.replace(QString("&oacute;"), QString("ó"));
    desc.replace(QString("&ocirc;"), QString("ô"));
    //desc.replace(QString("&otilde;"), QString("õ"));
    desc.replace(QString("&ouml;"), QString("ö"));
    desc.replace(QString("&divide;"), QString("÷"));
    desc.replace(QString("&oslash;"), QString("ø"));
    desc.replace(QString("&ugrave;"), QString("ù"));
    desc.replace(QString("&uacute;"), QString("ú"));
    desc.replace(QString("&ucirc;"), QString("û"));
    desc.replace(QString("&uuml;"), QString("ü"));
    //desc.replace(QString("&yacute;"), QString("ý"));
    //desc.replace(QString("&thorn;"), QString("þ"));
    //desc.replace(QString("&yuml;"), QString("ÿ"));

    return desc;
}
namespace {
void parseArticle(const feed::Infos& param, articles::Article& article, QXmlStreamReader& xml)
{
    while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == param.item))
    {
        bool readnext = true;
        if(xml.isStartElement()) {
            //qDebug() << xml.name();
            //TODO maybe use switch
            if (xml.name() == param.link) {
                QXmlStreamAttributes attributes = xml.attributes();
                if (attributes.hasAttribute("href")) {
                    article.link = attributes.value("href").toString().toStdString();
                }
                else
                    article.link = readText(xml).toStdString();
            } else if (xml.name() == param.author) {
                if(!article.author.empty()) {
                    article.author.append(", ");
                    article.author.append(cleanHTML(xml.readElementText(QXmlStreamReader::IncludeChildElements)).simplified().toStdString());
                }
                else
                     article.author = cleanHTML(xml.readElementText(QXmlStreamReader::IncludeChildElements)).simplified().toStdString();
                      //article.author = readText(xml).toStdString();
            } else if (xml.name() == param.category) {
                article.category = readText(xml).toStdString();
            } else if (xml.name() == param.title) {
                QString title = readText(xml).replace("&","&amp;");
                article.title = "<font color='"+param.color.toStdString()+"'>"+title.toStdString()+"</font>";
            } else if (xml.name() == param.desc) {
                QString desc = readContent(xml);
                readnext = false;
                if((param.html&feed::HTML::TagsDecode) == feed::HTML::TagsDecode)
                    desc = fastHTMLDecode(desc);
                if((param.html&feed::HTML::RemoveHTML) == feed::HTML::RemoveHTML)
                    desc = cleanHTML(desc);
                if(param.html == feed::HTML::DoNothing)
                    desc.replace("&","&amp;");
                article.description = desc.toStdString();
                extractInnerLinks(article, desc);
            } else if (xml.name() == param.date) {
                if(param.dateFormat == feed::DateFormat::ISO){
                    article.intdate = QDateTime::fromString(readText(xml),Qt::ISODate).toMSecsSinceEpoch();
                } else if(param.dateFormat == feed::DateFormat::RFC) {
                    article.intdate = QDateTime::fromString(readText(xml),Qt::RFC2822Date).toMSecsSinceEpoch();
                } else {//try basic
                    article.intdate = QDateTime::fromString(readText(xml)).toMSecsSinceEpoch();
                }
                article.date = QDateTime::fromMSecsSinceEpoch(article.intdate).toLocalTime().toString(Qt::DefaultLocaleShortDate).toStdString();
            } else if (xml.name() == param.multimedia) {
                QXmlStreamAttributes attributes = xml.attributes();
                if (attributes.hasAttribute("url")) {
                    article.media = attributes.value("url").toString().toStdString();
                }
            }
            else if(xml.name() != param.item){
                //qDebug() << xml.name();
                xml.readElementText(QXmlStreamReader::IncludeChildElements);
            }

        }
        if(readnext)
            xml.readNext();
    }//loop
    article.readability = (param.html == feed::HTML::ReadabilityOnOpen);
    article.icon = param.favicon.toStdString();
    if(article.media == "") {
        QRegExp rx("^\s*<img src=\"([^\"]+)\"( [a-z]+=\"([^\"]+)\")* ?/?>\s*$");
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        int pos = rx.indexIn(QString::fromStdString(article.description));
        //cerr << "#" << article.description <<"#"<< pos << endl;
        if (pos > -1) {
            article.media = rx.cap(1).toStdString();
            article.description = rx.cap(3).toStdString();
        }
        else
            article.media = param.favicon.toStdString();
    }
}

bool parseRSS(const feed::Infos& param, articles::ArticleList& result, QXmlStreamReader& xml)
{
    //xml.readNext();

    int article_id = 1;

    while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == param.main) && article_id <= config::articles_limit)
    {
        if(xml.isStartElement()) {
            if (xml.name() == param.item) {
                articles::Article article;
                article.id = param.id*512+article_id++;
                parseArticle(param, article, xml);
                result.emplace_back(article);
            }
        }

        xml.readNext();
    }
    if(article_id > config::articles_limit) {
        return true;
    }
    else
        return false;
}
}


void extractArticles(const feed::Infos& param, articles::ArticleList& result, QXmlStreamReader& XMLroot) {

    qDebug() << "expected main:"<<param.main;
    while (!XMLroot.atEnd() && !XMLroot.hasError()) {
        QXmlStreamReader::TokenType token = XMLroot.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement) {
            if (XMLroot.name() == param.main) {
                if(parseRSS(param, result, XMLroot))
                    break;
            }
            else qDebug() << "main:"<<XMLroot.name().toString();
        }
    }

    if (XMLroot.hasError()) {
        qDebug() << "xml error" << XMLroot.characterOffset();
        throw std::domain_error(XMLroot.errorString().toStdString());
    }
    qDebug() << "found articles :" << result.size();
}

} // XML

} // Client
