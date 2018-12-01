#include <preview.h>
#include <scope.h>
#include <localization.h>

#include <iostream>
#include <sstream>
#include <fstream>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/CannedQuery.h>

#include "preview/feedsearch.h"
#include "preview/article.h"
#include "preview/feedconfig.h"
#include "preview/feedmanage.h"
#include "preview/message.h"
#include "queries/feedly.h"
#include "queries/feedpreview.h"
#include "queries/importOPML.h"
#include "queries/home.h"

#include "client/db.h"

#include "types.h"
#include <QMutex>
#include <QDebug>

namespace sc = unity::scopes;
using namespace std;

/**************** CUSTOM ACTION ********************/
namespace action {
    class DoNothing : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                return unity::scopes::ActivationResponse(unity::scopes::ActivationResponse::Status::NotHandled);
            }
            DoNothing(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata,
                 std::string const &widget_id,
                 std::string const &action_id) : unity::scopes::ActivationQueryBase(result, metadata, widget_id, action_id) {}
    };
    class Done : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                return unity::scopes::ActivationResponse(unity::scopes::ActivationResponse::Status::ShowDash);
            }
            Done(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata,
                 std::string const &widget_id,
                 std::string const &action_id) : unity::scopes::ActivationQueryBase(result, metadata, widget_id, action_id) {}
    };
    class DoUpdate : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                unity::scopes::ActivationResponse response(unity::scopes::ActivationResponse::ShowPreview);
                response.set_scope_data(_data);
                return response;
            }
            DoUpdate(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata,
                 std::string const &widget_id,
                 std::string const &action_id,
                 sc::Variant const &update_data = sc::Variant()) :  unity::scopes::ActivationQueryBase(result, metadata, widget_id, action_id), _data(update_data) {

            }
        sc::Variant _data;
    };
    class DoFctUpdate : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                unity::scopes::ActivationResponse response(unity::scopes::ActivationResponse::ShowPreview);
                response.set_scope_data(sc::Variant(_data()));
                return response;
            }
            DoFctUpdate(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata,
                 std::string const &widget_id,
                 std::string const &action_id,
                  std::function<std::string()> const &update_data) :  unity::scopes::ActivationQueryBase(result, metadata, widget_id, action_id), _data(update_data) {

            }
        std::function<std::string()> _data;
    };
    class DoQuery : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                unity::scopes::CannedQuery query(SCOPE_NAME, _url, _category);
                if(!_userdata.is_null()) {
                    query.set_user_data(_userdata);
                }
                return unity::scopes::ActivationResponse(query);
            }
            DoQuery(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata,
                 std::string const &widget_id,
                 std::string const &action_id,
                 std::string const &url,
                 std::string const &category = "",
                 unity::scopes::Variant const &userdata = unity::scopes::Variant())
                :  unity::scopes::ActivationQueryBase(result, metadata, widget_id, action_id), _url(url), _category(category), _userdata(userdata) {

            }
        std::string _url;
        std::string _category;
        unity::scopes::Variant _userdata;
    };
}

namespace res_action {
    class DoNothing : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                return unity::scopes::ActivationResponse(unity::scopes::ActivationResponse::Status::NotHandled);
            }
            DoNothing(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata) : unity::scopes::ActivationQueryBase(result, metadata) {}
    };
    class Done : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                return unity::scopes::ActivationResponse(unity::scopes::ActivationResponse::Status::HideDash);
            }
            Done(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata) : unity::scopes::ActivationQueryBase(result, metadata) {}
    };
    class DoUpdate : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                unity::scopes::ActivationResponse response(unity::scopes::ActivationResponse::Status::ShowPreview);
                response.set_scope_data(_data);
                return response;
            }
            DoUpdate(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata,
                 sc::Variant const &update_data = sc::Variant()) :  unity::scopes::ActivationQueryBase(result, metadata), _data(update_data) {

            }
        sc::Variant _data;
    };
    class DoQuery : public unity::scopes::ActivationQueryBase {
        public:
            virtual unity::scopes::ActivationResponse activate () {
                unity::scopes::CannedQuery query(SCOPE_NAME, _url, _category);
                if(!_userdata.is_null())
                    query.set_user_data(_userdata);
                return unity::scopes::ActivationResponse(query);
            }
            DoQuery(unity::scopes::Result const &result,
                 unity::scopes::ActionMetadata const &metadata,
                 std::string const &url,
                 std::string const &category = "",
                 unity::scopes::Variant const &userdata = unity::scopes::Variant())
                :  unity::scopes::ActivationQueryBase(result, metadata), _url(url), _category(category), _userdata(userdata) {

            }
        std::string _url;
        std::string _category;
        unity::scopes::Variant _userdata;
    };
}

namespace {
sc::Variant updateFeedInfosVariant(sc::VariantMap source_cpy, const std::string &key, sc::Variant value) {
    //value now a string in any case, should be convert for html & date_format
    if(key == "desc_html") {
        source_cpy["desc_html"] = sc::Variant(QString::fromStdString(value.get_string()).toInt());
    }
    else if(key == "date_format") {
        source_cpy["date_format"] = sc::Variant(QString::fromStdString(value.get_string()).toInt());
    }
    else if(value.get_string() == "") {
        source_cpy[key] = sc::Variant("");
    }
    else {//generic
        if(key == "item") {
            source_cpy["main"] = sc::Variant(QString::fromStdString(value.get_string()).split(":").first().toStdString());
        }
        source_cpy[key] = sc::Variant(QString::fromStdString(value.get_string()).split(":").last().toStdString());
    }
    return sc::Variant(source_cpy);
}
}
/********** CLASS STUFF *******************/

void Scope::start(string const&) {
    config_ = make_shared<config::Base>();

    setlocale(LC_ALL, "");
    string translation_directory = ScopeBase::scope_directory()
            + "/../share/locale/";
    bindtextdomain(GETTEXT_PACKAGE, translation_directory.c_str());

    config_->dir = scope_directory();

}

void Scope::stop() {
}

sc::SearchQueryBase::UPtr Scope::search(const sc::CannedQuery &query,
                                        const sc::SearchMetadata &metadata) {
    cerr << scope_directory() << endl;
    QMutex mutex;
    mutex.lock();
    feed_config_data_ = sc::Variant();
    mutex.unlock();
    if(query.query_string().empty())
         return sc::SearchQueryBase::UPtr(new queries::Home(query, metadata, config_));
    bool s_feedly = settings()["feedly"].get_bool();
    if(query.query_string().substr(0,5) == std::string("opml:"))
	    return sc::SearchQueryBase::UPtr(new queries::ImportOPML(query, metadata, config_));
    else if(!query.has_user_data() && s_feedly
	    && (query.query_string().find('.') == std::string::npos || query.query_string().find(' ') != std::string::npos)
            && query.query_string().substr(0,4) != std::string("http"))
		return sc::SearchQueryBase::UPtr(new queries::Feedly(query, metadata, config_));
    else {
        if(client::DB::instance()->idFromUrl(QString::fromStdString(query.query_string())) == -1)
            return sc::SearchQueryBase::UPtr(new queries::FeedPreview(query, metadata, config_));
        else
            return sc::SearchQueryBase::UPtr(new queries::Home(query, metadata, config_));
    }
}

sc::PreviewQueryBase::UPtr Scope::preview(sc::Result const& result,
                                          sc::ActionMetadata const& metadata) {
    // Boilerplate construction of Preview
    if(result["mode"].get_int() == QueryMode::FEED_SEARCH) {
        if(!metadata.scope_data().is_null()) {
            std::string message = metadata.scope_data().get_string();
            return sc::PreviewQueryBase::UPtr(new preview::Message(result, metadata, message));
        }
        else
            return sc::PreviewQueryBase::UPtr(new preview::FeedSearch(result, metadata));
    }
    else if(result["mode"].get_int() == QueryMode::ARTICLE_VIEW) {
        if(!metadata.scope_data().is_null() || result.contains("readability_open")) {
            return sc::PreviewQueryBase::UPtr(new preview::Article(result, metadata, config_));
        }
        else
            return sc::PreviewQueryBase::UPtr(new preview::Article(result, metadata));
    }
    else if(result["mode"].get_int() == QueryMode::FEED_CONFIG) {
        if(!metadata.scope_data().is_null()) {
            //message to draw
            std::string message = metadata.scope_data().get_string();
            return sc::PreviewQueryBase::UPtr(new preview::Message(result, metadata, message));
        }
        QMutex mutex;
        sc::Result n_result(result);
        mutex.lock();
        if(feed_config_data_.is_null())
            feed_config_data_ = result["infos"];
        else
            n_result["infos"] = feed_config_data_;
        mutex.unlock();
        return sc::PreviewQueryBase::UPtr(new preview::FeedConfig(n_result, metadata));

    }
    else if(result["mode"].get_int() == QueryMode::FEED_MANAGE) {
        if(result["uri"].get_string() == "feed_addnew") {
            return sc::PreviewQueryBase::UPtr(new preview::Message(result, metadata, result["url"].get_string()));
        }
        if(!metadata.scope_data().is_null()) {
          std::string message = metadata.scope_data().get_string();
          return sc::PreviewQueryBase::UPtr(new preview::Message(result, metadata, message));
          }
        else
            return sc::PreviewQueryBase::UPtr(new preview::FeedManage(result, metadata, config_));
    }
    else if(result["mode"].get_int() == QueryMode::IMPORT_OPML) {
	cerr << "importing " << result["infos"].get_string() << endl;
        std::string urls = result["infos"].get_string();
        config::Base::Ptr cfg_ptr = config_;
                Action action(cfg_ptr);
		action.importFeeds(urls);
          return sc::PreviewQueryBase::UPtr(new preview::Message(result, metadata, "Import finish"));
	}
    return sc::PreviewQueryBase::UPtr(new Preview(result, metadata));
}

unity::scopes::ActivationQueryBase::UPtr Scope::perform_action(unity::scopes::Result const &result,
                                                     unity::scopes::ActionMetadata const &metadata,
                                                     std::string const &widget_id,
                                                     std::string const &action_id) {
    if(action_id == "read_readability") {
        return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext(("READABILITY")))));
    }
    if(action_id == "commented" && widget_id == "color_action") {
        sc::Variant newcolor = metadata.scope_data().get_dict().at("comment");
        QMutex mutex;
        mutex.lock();
        feed_config_data_ = updateFeedInfosVariant(feed_config_data_.get_dict(),"color", newcolor);
        mutex.unlock();
        return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id));
    }
    else if(action_id == "commented" && widget_id == "color_update") {
        QString newcolor = QString::fromStdString(metadata.scope_data().get_dict().at("comment").get_string());
        QString key("color");
        if(client::DB::instance()->updateFeed(result["id"].get_int(), key, newcolor))
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext(("Update feed succeed")))));
        else
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext(("Update feed failed")))));
    }
    else if(action_id == "commented" && widget_id == "search_action") {
        return unity::scopes::ActivationQueryBase::UPtr(new action::DoQuery(result, metadata, widget_id, action_id, metadata.scope_data().get_dict().at("comment").get_string()));
    }
    if(QString::fromStdString(action_id).startsWith("feed_autoreconfig_")) {
        int id = QString(QString::fromStdString(action_id).split("_").last()).toInt();
        config::Base::Ptr cfg_ptr = config_;
        std::string url = result["url"].get_string();
        return unity::scopes::ActivationQueryBase::UPtr(new action::DoFctUpdate(result, metadata, widget_id, action_id, [url, cfg_ptr, id]() {
            feed::Infos info;
            autodetect::Infos autoinfos;
            Action action(cfg_ptr);
            action.autopreview(url, autoinfos);
            info = feed::fromVariant(autodetect::toVariant(autoinfos));
            if(client::DB::instance()->updateFeed(id, info))
                return std::string("Update feed succeed");
            else
                return std::string("Update feed failed");
        }));
    }
    if(QString::fromStdString(action_id).startsWith("update_")) {
        //update_ lentgth=7
        bool ret = false;
        QString key = QString::fromStdString(widget_id).mid(8);
        if(key == "html_decode" || key == "date_format") {
            int newvalue = QString::fromStdString(action_id).mid(7).toInt();
            qDebug() << key << newvalue << result["id"].get_int();
            ret = client::DB::instance()->updateFeed(result["id"].get_int(), key, newvalue);
        }
        else if(action_id.length() == 8) { //actions_
            QString v("");
            ret = client::DB::instance()->updateFeed(result["id"].get_int(), key, v);
        }
        else {//generic
            if(key == "item") {
                QString pkey("main");
                QString v =  QString::fromStdString(action_id).mid(7).split(":").first();
                client::DB::instance()->updateFeed(result["id"].get_int(), pkey, v);
            }
             QString v =  QString::fromStdString(action_id).mid(7).split(":").last();
            ret = client::DB::instance()->updateFeed(result["id"].get_int(), key, v);
        }
        if(ret)
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext(("Update feed succeed")))));
        else
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext(("Update feed failed")))));
    }
    if(QString::fromStdString(action_id).startsWith("set_")) {
        //set_ length=4
        //actions_ length=8
        std::string key =QString::fromStdString(widget_id).mid(8).toStdString();
        std::string newvalue = QString::fromStdString(action_id).mid(4).toStdString();
        QMutex mutex;
        mutex.lock();
        feed_config_data_ = updateFeedInfosVariant(feed_config_data_.get_dict(),key,sc::Variant(newvalue));
        mutex.unlock();
        cerr <<  "changing " << key << " to " << newvalue << endl;
        return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id));
    }
    if(QString::fromStdString(action_id).startsWith("feed_remove_")) {
        //set_ length=4
        //actions_ length
        QString id = QString::fromStdString(action_id).mid(12);
        cerr <<  "removing " << id.toStdString() << " from db " << endl;
        if(client::DB::instance()->removeFeed(id.toInt()))
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext("Removing feed succeed"))));
        else
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext("Removing feed failed"))));
    }
    if(action_id == "feed_save") {
        feed::Infos info;
        QMutex mutex;
        mutex.lock();
        if(feed_config_data_.is_null()) {
            //do autoconf
            mutex.unlock();
            config::Base::Ptr cfg_ptr = config_;
            std::string url = result["url"].get_string();
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoFctUpdate(result, metadata, widget_id, action_id, [cfg_ptr, url]() {
                feed::Infos info;
                autodetect::Infos autoinfos;
                Action action(cfg_ptr);
                action.autopreview(url, autoinfos);
                info = feed::fromVariant(autodetect::toVariant(autoinfos));
                if(client::DB::instance()->addFeed(info))
                    return std::string("Add feed succeed");
                else
                    return std::string("Add feed failed");

            }));
        }
        else
        {
            mutex.unlock();
            info = feed::fromVariant(feed_config_data_);
        }
        if(client::DB::instance()->addFeed(info))
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext("Add feed succeed"))));
        else
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoUpdate(result, metadata, widget_id, action_id, sc::Variant(gettext("Add feed failed"))));
    }
    else if(action_id == "feed_preview") {
        if(widget_id == "conf_actions") {
            QMutex mutex;
            mutex.lock();
            sc::Variant cp = feed_config_data_;
            mutex.unlock();
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoQuery(result, metadata, widget_id, action_id,
                                                                                result["infos"].get_dict().at("realurl").get_string(), "", cp));
        }
        else
            return unity::scopes::ActivationQueryBase::UPtr(new action::DoQuery(result, metadata, widget_id, action_id, result["url"].get_string()));
    }
    else if(action_id == "message_ok") {
        return unity::scopes::ActivationQueryBase::UPtr(new action::DoQuery(result, metadata, widget_id, action_id, ""));
    }
    else
        return unity::scopes::ActivationQueryBase::UPtr(new action::DoNothing(result, metadata, widget_id, action_id));
}
unity::scopes::ActivationQueryBase::UPtr Scope::activate(unity::scopes::Result const &result,
                                                     unity::scopes::ActionMetadata const &metadata) {
     cerr << result.uri() << endl;
    if(result.uri() == "DIRECT_ADD") {
        bool ok = false;
        feed::Infos info;
        QMutex mutex;
        mutex.lock();
        if(feed_config_data_.is_null())
            info = feed::fromVariant(result["infos"]);
        else
            info = feed::fromVariant(feed_config_data_);
        mutex.unlock();
        ok = client::DB::instance()->addFeed(info);

    }

    if(result.uri() == "TRY_OPML") {
	cerr << "trying opml on " << result["infos"].get_string() << endl;
    	return unity::scopes::ActivationQueryBase::UPtr(new res_action::DoQuery(result, metadata, "opml:"+result["infos"].get_string()));
	}
    if(result.uri() == "IMPORT_OPML") {
	cerr << "importing " << result["infos"].get_string() << endl;
        std::string urls = result["infos"].get_string();
        config::Base::Ptr cfg_ptr = config_;
                Action action(cfg_ptr);
		action.importFeeds(urls);
    return unity::scopes::ActivationQueryBase::UPtr(new res_action::Done(result, metadata));
	}
    return unity::scopes::ActivationQueryBase::UPtr(new res_action::DoQuery(result, metadata, ""));
}



#define EXPORT __attribute__ ((visibility ("default")))

// These functions define the entry points for the scope plugin
extern "C" {

EXPORT
unity::scopes::ScopeBase*
// cppcheck-suppress unusedFunction
UNITY_SCOPE_CREATE_FUNCTION() {
    return new Scope();
}

EXPORT
void
// cppcheck-suppress unusedFunction
UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base) {
    delete scope_base;
}

}

