#include "web.h"

#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <core/net/http/header.h>
#include <unity/scopes/ScopeMetadata.h>

#include <iostream>
#include <QJsonDocument>
#include <QLocale>
#include <QTextCodec>
#include <QVariant>
#include <QDebug>

namespace http = core::net::http;
namespace net = core::net;

using namespace std;
namespace client {
Web::Web(config::Base::Ptr config) :
    config_(config), cancelled_(false) {
}

http::Request::Progress::Next Web::progress_report(
        const http::Request::Progress&) {

    return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
}

void Web::cancel() {
    cancelled_ = true;
}

config::Base::Ptr Web::config() {
    return config_;
}
void Web::get_readability(const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot_readability, path, parameters);
    configuration.uri = client->uri_to_string(uri);

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Web::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());

        // Open API error code can either be a string or int
        //QVariant cod = root.toVariant().toMap()["cod"];
        //if ((cod.canConvert<QString>() && cod.toString() != "200")
        //        || (cod.canConvert<unsigned int>() && cod.toUInt() != 200)) {
        //    throw domain_error(root.toVariant().toMap()["message"].toString().toStdString());
        //}
    } catch (net::Error &) {
    }
}
void Web::get_mercury(const std::string &key, const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot_mercury, path, parameters);
    configuration.uri = client->uri_to_string(uri);
    configuration.header.add("x-api-key", key);
    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Web::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());
        // Open API error code can either be a string or int
        //QVariant cod = root.toVariant().toMap()["cod"];
        //if ((cod.canConvert<QString>() && cod.toString() != "200")
        //        || (cod.canConvert<unsigned int>() && cod.toUInt() != 200)) {
        //    throw domain_error(root.toVariant().toMap()["message"].toString().toStdString());
        //}
    } catch (net::Error &) {

     }
}
void Web::get_feedly(const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, QJsonDocument &root) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot_feedly, path, parameters);
    configuration.uri = client->uri_to_string(uri);

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Web::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the JSON from the response
        root = QJsonDocument::fromJson(response.body.c_str());

        // Open API error code can either be a string or int
        //QVariant cod = root.toVariant().toMap()["cod"];
        //if ((cod.canConvert<QString>() && cod.toString() != "200")
        //        || (cod.canConvert<unsigned int>() && cod.toUInt() != 200)) {
        //    throw domain_error(root.toVariant().toMap()["message"].toString().toStdString());
        //}
    } catch (net::Error &) {
    }
}
void Web::get(const net::Uri::Path &path,
                 const net::Uri::QueryParameters &parameters, QXmlStreamReader &reader) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;

    // Build the URI from its components
    net::Uri uri = net::make_uri(config_->apiroot_feedly, path, parameters);
    configuration.uri = client->uri_to_string(uri);

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Web::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if (response.status != http::Status::ok) {
            throw domain_error(response.body);
        }
        // Parse the Xml from the response
        reader.addData(response.body.c_str());
    } catch (net::Error &) {
    }
}

QString Web::wget(const string uri) {
    // Create a new HTTP client
    auto client = http::make_client();

    // Start building the request configuration
    http::Request::Configuration configuration;


    configuration.uri = QString::fromStdString(uri).replace(" ", "%20").toStdString();

    // Give out a user agent string
    configuration.header.add("User-Agent", config_->user_agent);

    // Build a HTTP request object from our configuration
    auto request = client->head(configuration);

    try {
        // Synchronously make the HTTP request
        // We bind the cancellable callback to #progress_report
        auto response = request->execute(
                    bind(&Web::progress_report, this, placeholders::_1));

        // Check that we got a sensible HTTP status code
        if(response.status == http::Status::moved_permanently || response.status == http::Status::found || response.status == http::Status::see_other) { //http 301 302 303
            //return wget(response.header.get("location").c_str());
             cerr << "== HTTP == ERROR : " << response.status << endl;
             //enumerate(const std::function<void(const std::string&, const std::set<std::string>&)>& enumerator) const;
             std::string newurl = "";
             response.header.enumerate([&newurl](const std::string &header_key, const std::set<std::string>&header_values) {
		if(header_key == "Location") {
			std::set<std::string>::iterator it;
			for (it = header_values.begin(); it != header_values.end(); it++)
			{
				newurl += *it;
			}
                 }
		}
                                       );
             QRegExp rx2("^((https?://)?[^/]+)/.*$");
             rx2.setCaseSensitivity(Qt::CaseInsensitive);
             int pos = rx2.indexIn(QString::fromStdString(newurl));
             if (pos == -1) {
                 pos = rx2.indexIn(QString::fromStdString(uri));
                 newurl = rx2.cap(1).toStdString()+newurl;
             }
             cerr << "== HTTP == New URL : " << newurl <<"#"<< endl;

             //return wget(newurl);
             if(newurl != uri)
                throw client::RedirectException(newurl);
             else
                cerr << "== HTTP == error : redirection loop " << endl;
        }
        else if (response.status != http::Status::ok) {
            //throw domain_error(response.body);
            cerr << "== HTTP == error : " << response.status << endl;
            return "";
        }
        //return response.body.c_str();
        QString output = response.body.c_str();
        output.replace("\"//","\"http://");
        QRegExp encoding("<\\\?xml version=\"[^\"]+\" encoding=\"([^\"]+)\" ?\\\?>");
        encoding.setCaseSensitivity(Qt::CaseInsensitive);
        int pos = encoding.indexIn(output);
        if (pos != -1) {
            cerr << "Encoding found : " << encoding.cap(1).toUpper().toStdString();
            if(encoding.cap(1).toUpper() != "UTF-8")
                output = QTextCodec::codecForHtml(encoding.cap(1).toUpper().toLatin1())->toUnicode(response.body.c_str());
        }
        //output.replace(QRegExp(">([^<>]+)<!\\[CDATA\\["), "><![CDATA[\\1");
        return output;
        //return QTextCodec::codecForName("UTF-8")->toUnicode(response.body.c_str());
    } catch (net::Error &) {
        cerr << "== HTTP == network error" << endl;
    }
    return "";
}

QString Web::wget(const std::string uri, QXmlStreamReader &reader) {
        QString text = wget(uri);
        if(text != "")
            reader.addData(text);
        return text;
}


QString Web::get_favicon_url(std::string url, bool redirected_url) {
    try {
        QString baseurl = QString::fromStdString(url);
        QRegExp rx("^((https?://)?[^/]+/).*$");
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        int pos = rx.indexIn(QString::fromStdString(url));
        if (pos > -1) {
            baseurl = rx.cap(1);
            if(rx.cap(2).isEmpty())
                baseurl = baseurl.prepend("http://");
            QString data;
            if(redirected_url)
                data = Web::wget(url);
            else
                data = Web::wget(baseurl.toStdString());
            QRegExp rx2("<link [^>]*rel=\"shortcut icon\"[^>]*/?>");
            rx2.setCaseSensitivity(Qt::CaseInsensitive);
            int pos = rx2.indexIn(data);
            if(pos > -1) {
                QRegExp rx3("href=\"([^\"]*)\"");
                rx3.setCaseSensitivity(Qt::CaseInsensitive);
                int pos2 = rx3.indexIn(rx2.cap(0));
                if(pos2 > -1){
                    if(rx3.cap(1).left(4) != "http")
                        return baseurl+rx3.cap(1);
                    else
                        return rx3.cap(1);
                    }
            }

        }
        //unable to download from url search feedly
        QString queryurl = QString::fromStdString(url).split("?").first();
        cerr << "search icon using feedly @ " << queryurl.toStdString() << endl;
        QJsonDocument root;
        get_feedly( { "v3", "search", "feeds" },
        { { "query", queryurl.toStdString() }, { "locale", QLocale().uiLanguages().first().toStdString() }  }, root);
        // e.g. http://www.feedly.com/v3/search/feeds?query=
         QVariantMap variant = root.toVariant().toMap();
        for (const QVariant &i : variant["results"].toList()) {
            return i.toMap()["visualUrl"].toString();
        }
        return "";

    }
    catch (const client::RedirectException &e) {
                return get_favicon_url(e._newurl, true);
            }
    catch (const std::exception &e) {
        cerr << "Unable to reach favicon = Error === " << e.what();
        return QString();
    }
}

QString Web::get_source_url_from_feedly(std::string src_url, std::string redirected_url) {
    try {
        QString data;
        if(redirected_url.empty())
            data = Web::wget(src_url);
        else
            data = Web::wget(redirected_url);
        QRegExp rx("<link>([^<]+)</link>");
        rx.setCaseSensitivity(Qt::CaseInsensitive);
        int pos = rx.indexIn(data);
        if (pos > -1) {
            QRegExp rx2("^((https?://)?[^/]+/?)$");
            rx2.setCaseSensitivity(Qt::CaseInsensitive);
            int pos2 = rx2.indexIn(rx.cap(1));
            if(pos2 == -1) //not a basic feed
                return rx.cap(1);
            else
                return QString::fromStdString(src_url);
        }
        else
            return  QString::fromStdString(src_url);

    }
    catch (const client::RedirectException &e) {
                return get_source_url_from_feedly(src_url, e._newurl);
            }
    catch (const std::exception &e) {
        cerr << "Unable to reach source ... " << e.what();
        return QString::fromStdString(src_url);
    }
}

}
