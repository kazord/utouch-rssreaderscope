#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QStringList>
#include <memory>
#include <string>
#include <deque>
#include <unity/scopes/Variant.h>

namespace config {
    /**
     * Client configuration
     */
    struct Base {
        typedef std::shared_ptr<Base> Ptr;

        // The root of all API request URLs
        std::string apiroot_feedly { "http://www.feedly.com" }; //http://www.feedly.com/v3/search/feeds?query=ubuntouch
        std::string apiroot_readability { "https://www.readability.com" };
        std::string apiroot_mercury { "https://mercury.postlight.com" };

        // The custom HTTP user agent string for this library
        //std::string user_agent { "Mozilla/5.0 (Android 5.1; Mobile; rv:41.0) Gecko/41.0 Firefox/41.0" };
        std::string user_agent { "Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:45.0) Gecko/20100101 Firefox/45.0" };

        std::string dir {""};
    };
    extern int articles_limit;
}

enum QueryMode { FEED_SEARCH = 0, FEED_CONFIG, ARTICLE_VIEW, FEED_MANAGE, IMPORT_OPML};

namespace feed {

    enum DateFormat { ISO = 0, RFC, TXT };
    enum HTML { DoNothing = 0, TagsDecode = 1, RemoveHTML = 2, Readability = 4, ReadabilityOnOpen = 5};

    struct Infos {
        int id;
        HTML html;
        DateFormat dateFormat;
        QString name;
        QString url;
        QString main;
        QString item;
        QString link;
        QString author;
        QString category;
        QString title;
        QString desc;
        QString date;
        QString multimedia;
        QString color;
        QString favicon;


    };

    struct Search {
        int subscribers;
        std::string uri;
        std::string name;
        std::string url;
        std::string origin_url;
        std::string desc;
        std::string art;
    };

    typedef std::deque<Search> SearchList;

    Infos fromVariant(const unity::scopes::Variant& variant);
    unity::scopes::Variant toVariant_tabbed(const Infos& infos);
}

namespace articles {
    /**
     * links inside articles
     **/
    typedef std::pair<std::string, std::string> InnerLinks;

    /**
     * Content of a article
     */
    struct Article {
        bool readability;
        unsigned int id;
        qint64 intdate;
        std::string source;
        std::string link;
        std::string author;
        std::string title;
        std::string category;
        std::string description;
        std::string date;
        std::string media;//art,music,movie
        std::string icon;
        //
        std::vector<InnerLinks> innerlinks;
    };

    typedef std::deque<Article> ArticleList;

}

namespace autodetect {
    class Node {
        public:
            QString parent;
            QString name;
            QStringList attr;
            bool operator==(const Node &b) {
                return (parent == b.parent) && (name == b.name);
            }
    };

    struct Infos {
        QList<QString> names;
        QString iconurl;
        QString realurl;
        QList<Node>  list;
        bool htmldecode = false;
        Node item;
        Node link;
        Node author;
        Node category;
        Node title;
        Node desc;
        Node date;
        Node multimedia;
        feed::DateFormat dateFormat;
    };

    feed::Infos Convert2Feed(const autodetect::Infos& import);

    unity::scopes::Variant toVariant(const Infos& infos);
}
namespace templates {
    const static std::string PREVIEW_ACTION =
            R"(
    {
            "schema-version": 1,
            "template": {
            "category-layout": "grid",
            "card-layout": "vertical",
            "card-size": "medium"
            },
            "components": {
            "title": "title",
            "subtitle": "subtitle"
            }
            }
            )";
    const static std::string ARTICLE =
            R"(
    {
            "schema-version": 1,
            "template": {
            "category-layout": "grid",
            "card-layout": "horizontal",
            "card-size": "medium",
            "collapsed-rows" : 30
            },
            "components": {
            "title": "title",
            "art" : {
            "field": "art",
            "fallback": "file:///home/phablet/.local/share/unity-scopes/rssreaderscope.kazord_rssreaderscope_0.7.1/fallbackicon.png"
            },
            "subtitle": "subtitle",
            "emblem" : "icon"
            }
            }
            )";
    const static std::string FEED =
            R"(
    {
            "schema-version": 1,
            "template": {
            "category-layout": "grid",
            "card-layout": "horizontal",
            "card-size": "medium",
            "collapsed-rows" : 1
            },
            "components": {
            "title": "title",
            "art" : {
            "field": "art",
            "fallback": "file:///home/phablet/.local/share/unity-scopes/rssreaderscope.kazord_rssreaderscope_0.7.1/fallbackicon.png"
            },
            "subtitle": "url"
            }
            }
            )";
}

#endif // TYPES_H

