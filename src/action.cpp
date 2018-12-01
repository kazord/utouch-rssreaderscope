#include "action.h"

#include "types.h"
#include <QJsonDocument>
#include <QLocale>
#include <QRegExp>
#include <iostream>
#include <QDebug>

#include "client/db.h"
#include "client/xml.h"
#include <unity/scopes/VariantBuilder.h>

#include "apikey.h"
//from apikey.h
//#define READABILITY_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
//#define POSTLIGHT_MERCURY_KEY "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"


using namespace std;

Action::Action(config::Base::Ptr webconf) : _web(webconf)
{

}

void Action::cancel() {
    _web.cancel();
}

QString Action::readability(const std::string &url) {
    QString content;
    try {
            QJsonDocument root;
            std::string newurl = url;
            /*if(newurl.find('#') != std::string::npos)
                newurl = newurl.substr(0, url.find('#'));
            if(newurl.find('?')!= std::string::npos)
                newurl = url.substr(0, newurl.find('?'));*/
            //cerr << url << endl << newurl << endl;
           _web.get_readability( { "api", "content", "v1", "parser" },
           { { "url", newurl }, { "token", READABILITY_KEY }  }, root);
           // e.g. https://www.readability.com/api/content/v1/parser?url=URL&token=KEY
           QVariantMap variant = root.toVariant().toMap();
           QRegExp exp("<figure class=\\\"illustration_haut.+</figure>(.+)</div>");
           exp.setMinimal(true);
            int pos = exp.indexIn(variant["content"].toString());
            if(pos > -1)
             content = exp.cap(1);
            else
            content = variant["content"].toString();
            content.replace("\\\"", "\"");
    }catch (const std::exception &) {
    }
    return content;
}
QString Action::mercury(const std::string &url) {
    QString content;
    try {
            QJsonDocument root;
            std::string newurl = url;
            //cerr << url << endl << newurl << endl;
           _web.get_mercury(POSTLIGHT_MERCURY_KEY, { "parser" },
           { { "url", newurl }  }, root);
           //GET https://mercury.postlight.com/parser?url=URL
//           Content-Type: application/json
//           x-api-key: KEY
           QVariantMap variant = root.toVariant().toMap();
           /*QRegExp exp("<figure class=\\\"illustration_haut.+</figure>(.+)</div>");
           exp.setMinimal(true);
            int pos = exp.indexIn(variant["content"].toString());
            if(pos > -1)
             content = exp.cap(1);
            else*/
            content = variant["content"].toString();
            content.replace("\\\"", "\"");
    }catch (const std::exception &) {
    }
    return content;
}

feed::SearchList Action::search(const string &query) {
    feed::SearchList results;

    try {
         QJsonDocument root;
        _web.get_feedly( { "v3", "search", "feeds" },
        { { "query", query }, { "locale", QLocale().uiLanguages().first().toStdString() }  }, root);
        // e.g. http://www.feedly.com/v3/search/feeds?query=

        //cerr << "get worked"<< endl;
        //feedid
        //title
        //website
        QVariantMap variant = root.toVariant().toMap();
unsigned int id = 0;
        // Iterate through the data
        for (const QVariant &i : variant["results"].toList()) {
            QVariantMap item = i.toMap();

            // Extract the first weather item
            QString feed_url = item["feedId"].toString().mid(5); //removing feed/ @t beginning
            if(feed_url.contains("feedburner.com"))
                feed_url = feed_url.append("?format=xml");
            QString origin = item["website"].toString();
            QString title = item["title"].toString();
            //cerr << "adding "<< feed_url.toStdString() << origin.toStdString() << title.toStdString() << endl;
            feed::Search s;
            s.uri = id++;
            s.name = title.toStdString();
            s.url = feed_url.toStdString();
            s.art =  item["visualUrl"].toString().toStdString();
            s.origin_url = origin.toStdString();
            s.desc = item["description"].toString().toStdString();
            s.subscribers = item["subscribers"].toInt();

            results.emplace_back(s);
        }

    }catch (const std::exception &) {
    }
    return results;
}
feed::SearchList Action::importOPML(const string &url) {
    feed::SearchList results;

    try {
        QXmlStreamReader reader;
	cerr << "url" << url << endl;
        QString xmldata = _web.wget(url, reader);
	cerr << "XML Lu:" << xmldata.size() << endl;
	QStringList urls = client::XML::fromOPML(reader);
	qDebug() << urls;
	unsigned int id=0;
        // Iterate through the data
	for (int i=0;i< urls.count();i+=2)
	{
	    feed::Search s;
            s.uri = std::to_string(id++);
            s.name = urls[i+1].toStdString();
            s.url = urls[i].toStdString();
            s.art =  "";
            s.origin_url = "";
            s.desc = "";
            s.subscribers = 0;

            results.emplace_back(s);
	}

    }catch (const std::exception &) {
		cerr <<"=== OPML import FAILED ! === " << endl;
    }
    return results;
}
void Action::importFeeds(const string &url_list) {
        QStringList urls = QString::fromStdString(url_list).split(" ");
	QXmlStreamReader reader;
	autodetect::Infos autoinfos;
	int goodcount=0;
	int badcount=0;
	for(QString qurl : urls) {
		bool ok = false;
		string url = qurl.toStdString();
	    reader.clear();
		try {
			int redirection = 1;
			QString xmldata = "";
			 while(redirection > 0 && redirection < 30) {
			try {
				xmldata =  _web.wget(url, reader);
				redirection = 0;
			}
			catch (const client::RedirectException &e) {
				url = e._newurl;
				redirection++;
			    }
			}
			if(xmldata != "") {
			    client::XML::autodetect(reader, autoinfos);
			    reader.clear();
			    reader.addData(xmldata);
			    autoinfos.names = client::XML::foundChannel(reader, autoinfos.item.parent, autoinfos.item.name);
			    autoinfos.iconurl = _web.get_favicon_url(url);
			    autoinfos.realurl = QString::fromStdString(url);
			    feed::Infos infos = autodetect::Convert2Feed(autoinfos);

        		    ok = client::DB::instance()->addFeed(infos);
			 }
		}catch (const std::exception &e) {
			cerr << "== OPML import == something get wrong "<< e.what() << endl;
		    }
		//cerr << url << " imported" << ok << " " << std::to_string(okcount) << endl;
		if(ok) goodcount++; else badcount++;
	}
	cerr << "import finished " << std::to_string(goodcount) << std::to_string(badcount) << endl;
}
articles::ArticleList Action::view(const std::string &url, const feed::Infos &infos) {
    articles::ArticleList results;
    try {
    cerr << "VIEW === ";
        QXmlStreamReader reader;
        _web.wget(url, reader);
        client::XML::extractArticles(infos, results, reader);
        if(infos.html == feed::HTML::Readability) {
            for (articles::Article &article : results) {
                QString desc = mercury(article.link);
                article.description = client::XML::fastHTMLDecode(desc).toStdString();
                article.innerlinks.clear();
                client::XML::extractInnerLinks(article, desc);
             }
        }
    }
    catch (const client::RedirectException &e) {
                results = view(e._newurl, infos);
            }
    catch (const std::exception &e) {
        cerr << "PROCESSING Error === " << e.what();
    }
    return results;
}

articles::ArticleList Action::autopreview(const std::string &url, autodetect::Infos &autoinfos) {
    articles::ArticleList results;
    try {
        QXmlStreamReader reader;
        QString xmldata = _web.wget(url, reader);
	cerr << "XML Lu:" << xmldata.size() << endl;
        if(xmldata != "") {
            bool do_preview = client::XML::autodetect(reader, autoinfos);
            reader.clear();
            reader.addData(xmldata);
            autoinfos.names = client::XML::foundChannel(reader, autoinfos.item.parent, autoinfos.item.name);
            autoinfos.iconurl = _web.get_favicon_url(url);
            autoinfos.realurl = QString::fromStdString(url);
             if(do_preview) {
                reader.clear();
                reader.addData(xmldata);
                feed::Infos infos = autodetect::Convert2Feed(autoinfos);
                client::XML::extractArticles(infos, results, reader);
            }
        }
    }
    catch (const client::RedirectException &e) {
                results = autopreview(e._newurl, autoinfos);
            }
    catch (const std::exception &e) {
        cerr << "PROCESSING Error === " << e.what();
    }
    return results;
}

articles::ArticleList Action::fetch() {
    articles::ArticleList results;
    for (const auto feed : client::DB::instance()->feeds()) {
        try {
        QXmlStreamReader reader;
        _web.wget( feed.url.toStdString(), reader);
        // e.g. http://lxer.com/module/newswire/headlines.rss
        int index = results.size();
        client::XML::extractArticles(feed, results, reader);
        if(feed.html == feed::HTML::Readability) {
            for (unsigned int i = index ; i < results.size();i++) {
                QString desc = mercury(results[i].link);
                results[i].description = client::XML::fastHTMLDecode(desc).toStdString();
                results[i].innerlinks.clear();
                client::XML::extractInnerLinks(results[i], desc);
             }
        } else if (feed.html == feed::HTML::ReadabilityOnOpen) {
            for (unsigned int i = index ; i < results.size();i++) {
                results[i].readability = true;
             }
        }
        }
        catch(const std::exception &e) {
            cerr << "PROCESSING Error === On feed " << feed.url.toStdString() <<" === " << e.what();
        }
    }
    //sort result by intdate
    std::sort( results.begin() , results.end() ,
               [](const articles::Article &a, const articles::Article &b) {
        return a.intdate>b.intdate;} );
    return results;
}

articles::ArticleList Action::fetchFeed(const int id) {
    articles::ArticleList ret;
    if(id == -1)
        return ret;
    feed::Infos infos = client::DB::instance()->feed(id);
    return view(infos.url.toStdString(), infos);
}
feed::Infos Action::feedInfo(const int id) {
    return client::DB::instance()->feed(id);
}
unity::scopes::Result Action::contentreading(unity::scopes::Result &src) {
        unity::scopes::Result ret(src);
        QString desc = client::XML::fastHTMLDecode(mercury(src["link"].get_string()));
        ret["description"] = unity::scopes::Variant(desc.toStdString());
        articles::Article article;
        client::XML::extractInnerLinks(article, desc);
        unity::scopes::VariantBuilder builder;
        int link_count = 0;
        builder.add_tuple({
                              {"id", unity::scopes::Variant("open")},
                              {"label", unity::scopes::Variant("Open")},
                              {"uri", src["link"]}
                          });
        for (const auto &pair : article.innerlinks) {
            builder.add_tuple({
                                  {"id", unity::scopes::Variant("open_"+std::to_string(link_count++))},
                                  {"label", unity::scopes::Variant(pair.first)},
                                  {"uri", unity::scopes::Variant(pair.second)}
                              });

        }
        ret["actions"] = builder.end();

        //cerr << ret["description"].get_string() <<endl;
        return ret;

}
