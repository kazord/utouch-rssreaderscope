#include "home.h"

#include <localization.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/VariantBuilder.h>
#include <unity/scopes/SearchMetadata.h>

#include <QList>

#include <iomanip>
#include <iostream>
#include <sstream>

#include "types.h"
#include "client/db.h"
#include "client/xml.h"

#define SUMMUARY_SIZE 200

namespace sc = unity::scopes;

using namespace std;

/**
 * Define the layout for the results.
 *
 * The icon size is medium, and ask for the card layout
 * itself to be horizontal. I.e. the text will be placed
 * next to the image.
 */


queries::Home::Home(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             config::Base::Ptr webconfig) :
    sc::SearchQueryBase(query, metadata), config_(webconfig), action_(webconfig), aggregated_(metadata.is_aggregated()) {
}

void queries::Home::cancelled() {
    action_.cancel();
}

void queries::Home::run(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        config::articles_limit = settings().at("articles_limit").get_double();



        QQueue<feed::Infos> c = client::DB::instance()->feeds();
        if(c.size() > 0 || settings().at("feed_manage_display").get_int() == 2) {
            sc::Department::SPtr all_depts = sc::Department::create("", query, gettext("Home"));
            for (const feed::Infos &feed : c)
            {
                sc::Department::SPtr dept = sc::Department::create("dept_"+to_string(feed.id), query, gettext("Only ")+feed.name.toStdString());
                all_depts->add_subdepartment(dept);
                //reply->register_departments(dept);
            }
            if(settings().at("feed_manage_display").get_int() == 2) {
                sc::Department::SPtr dept = sc::Department::create("feed_manage", query, gettext("Feed management"));
                all_depts->add_subdepartment(dept);
            }

            reply->register_departments(all_depts);
        }

        articles::ArticleList results;
        if(QString::fromStdString(query.department_id()).left(5) == "dept_") {
            results = action_.fetchFeed(QString::fromStdString(query.department_id()).mid(5).toInt());
        }
        else if(query.department_id() != "feed_manage")
            results = action_.fetch();

        //display feed management
        if(!aggregated_ && ((settings().at("feed_manage_display").get_int() == 1 && query.department_id() == "")
                || (settings().at("feed_manage_display").get_int() == 2 && query.department_id() == "feed_manage"))) {
            auto feed_cat = reply->register_category("feed_manage",
                                                ((settings().at("feed_manage_display").get_int() == 2)?"":gettext("Feed management")), "",
                                                sc::CategoryRenderer(templates::FEED));
            sc::CategorisedResult res(feed_cat);

            res["mode"] = sc::Variant(QueryMode::FEED_MANAGE);
            // We must have a URI
            res.set_uri("feed_addnew");
            res.set_title("Add some feeds");
            res["url"] = sc::Variant(gettext("Use search tools to find new RSS feed or paste his URL"));
            res["art"] = sc::Variant("file:///home/phablet/.local/share/unity-scopes/" PACKAGE_FULLNAME "/fallbackicon.png");

            if (!reply->push(res)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
            for (const auto &feed : c) {
                sc::CategorisedResult res(feed_cat);
                res["mode"] = sc::Variant(QueryMode::FEED_MANAGE);
                // We must have a URI
                res.set_uri("feed_"+std::to_string(feed.id));
                res.set_title(feed.name.toStdString());
                res["id"] = sc::Variant((int)feed.id);
                res["url"] = sc::Variant(feed.url.toStdString());
                if(feed.favicon == "")
                    res["art"] = sc::Variant("file:///home/phablet/.local/share/unity-scopes/" PACKAGE_FULLNAME "/fallbackicon.png");
                else
                    res["art"] = sc::Variant(feed.favicon.toStdString());
                res["tabbed_info"] = feed::toVariant_tabbed(feed);
                if (!reply->push(res)) {
                    // If we fail to push, it means the query has been cancelled.
                    // So don't continue;
                    return;
                }
            }
        }
        if( query.department_id() != "feed_manage") {
            // Register a category
            auto cat = reply->register_category("home",
                                                ((settings().at("feed_manage_display").get_int() != 1)?"":gettext("Articles")), "",
                                                sc::CategoryRenderer(templates::ARTICLE));

            for (const articles::Article &result : results) {
                sc::CategorisedResult res(cat);
                res["mode"] = sc::Variant(QueryMode::ARTICLE_VIEW);
                if(aggregated_ && settings().at("readability").get_bool() == true) {
                    res["readability_open"] = sc::Variant(true);
                } else if(result.readability) {
                    res["readability_open"] = sc::Variant(true);
                }
                // We must have a URI
                res.set_uri("nb_"+std::to_string(result.id));

                //list
                res.set_title(result.title);
                QList<QString> notimageext;
                notimageext<<"MP3"<<"OGG"<<"WAV"<<"MP4"<<"AVI";
                if(notimageext.contains(QString::fromStdString(result.media).split(".").last().toUpper())) {
                    if(result.icon != "")
                        res.set_art(result.icon);
                    else
                        res.set_art("file:///home/phablet/.local/share/unity-scopes/" PACKAGE_FULLNAME "/fallbackicon.png");
                }
                else if(result.media != result.icon)
                    res.set_art(result.media);
                else {
                    if(result.icon != "")
                        res.set_art(result.icon);
                    else
                        res.set_art("file:///home/phablet/.local/share/unity-scopes/" PACKAGE_FULLNAME "/fallbackicon.png");
                }

                if(result.author != "")
                    res["subtitle"] = sc::Variant(result.date+gettext(" by ")+result.author);
                else
                    res["subtitle"] = sc::Variant(result.date);
                QString cleandescription = client::XML::cleanHTML(QString::fromStdString(result.description));
                if(cleandescription.length() < SUMMUARY_SIZE)
                 res["summary"] = sc::Variant(cleandescription.toStdString());
                else
                    res["summary"] = cleandescription.left(SUMMUARY_SIZE - 3).append("...").toStdString();
                res["emblem"] = sc::Variant(result.icon);


                sc::VariantBuilder builder;
                int link_count = 0;
                builder.add_tuple({
                                      {"id", sc::Variant("open")},
                                      {"label", sc::Variant("Open")},
                                      {"uri", sc::Variant(result.link)}
                                  });
                builder.add_tuple({
                                      {"id", sc::Variant("read_readability")},
                                      {"label", sc::Variant("Read")}
                                  });
                for (const auto &pair : result.innerlinks) {
                    builder.add_tuple({
                                          {"id", sc::Variant("open_"+std::to_string(link_count++))},
                                          {"label", sc::Variant(pair.first)},
                                          {"uri", sc::Variant(pair.second)}
                                      });

                }
                res["actions"] = builder.end();
                // Other attr
                res["link"] = sc::Variant(result.link);
                res["author"] = sc::Variant(result.author);
                res["category"] = sc::Variant(result.category);
                res["description"] = sc::Variant(result.description);
                res["date"] = sc::Variant(result.date);
                res["media"] = sc::Variant(result.media);
                res["icon"] = sc::Variant(result.icon);
                //

                // Push the result
                if (!reply->push(res)) {
                    // If we fail to push, it means the query has been cancelled.
                    // So don't continue;
                    return;
                }
            }
        }
    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}


