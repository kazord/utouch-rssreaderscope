#include <localization.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/VariantBuilder.h>

#include <iomanip>
#include <iostream>
#include <sstream>

#include "types.h"
#include "feedpreview.h"

namespace sc = unity::scopes;

using namespace std;

/**
 * Define the layout for the results.
 *
 * The icon size is medium, and ask for the card layout
 * itself to be horizontal. I.e. the text will be placed
 * next to the image.
 */


queries::FeedPreview::FeedPreview(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             config::Base::Ptr webconfig) :
    sc::SearchQueryBase(query, metadata), action_(webconfig) {
}

void queries::FeedPreview::cancelled() {
    action_.cancel();
}

void queries::FeedPreview::run(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Get the query string
        string query_string = query.query_string();

        config::articles_limit = settings().at("articles_limit").get_double();

        // the Client is the helper class that provides the results
        // without mixing APIs and scopes code.
        // Add your code to retreive xml, json, or any other kind of result
        // in the client.
        //feed::SearchList results;
        articles::ArticleList results;
        if (query_string.empty()) {
            // If the string is empty, pick a default
            return;
        }
            // otherwise, use the search string
            //results = action_.preview(query_string);
        sc::Variant feed_conf_data;
        std::string feed_name;
        if(query.has_user_data()) {
            feed_conf_data = query.user_data();
            feed::Infos infos = feed::fromVariant(feed_conf_data);
            results = action_.view(query_string,infos);
            feed_name = infos.name.toStdString();
        }
        else {
            autodetect::Infos autoinfos;
            results = action_.autopreview(query_string, autoinfos);
            feed_conf_data = autodetect::toVariant(autoinfos);
            if(autoinfos.names.size() > 0)
                feed_name = autoinfos.names.first().toStdString();
            else
                feed_name = "Unnamed";
        }

        // Register a category
        auto cat = reply->register_category("feed_preview",
                                            "Preview "+feed_name, "",
                                            sc::CategoryRenderer(templates::ARTICLE));
        auto actions = reply->register_category("feed_preview_actions",
                                            "Preview "+feed_name, "",
                                            sc::CategoryRenderer(templates::PREVIEW_ACTION));

        //nothing found
        if(results.size() == 0) {
            sc::CategorisedResult res(cat);
            res["mode"] = sc::Variant(QueryMode::ARTICLE_VIEW);
            res.set_uri("nb_-1");
            res.set_title(std::string("<b><font color='red'>")+gettext("No articles found")+"</font></b>");
            res["subtitle"] = sc::Variant(query_string);
            res["art"] = sc::Variant("file:///home/phablet/.local/share/unity-scopes/" PACKAGE_FULLNAME "/notfoundicon.png");
            if (!reply->push(res)) {
                // If we fail to push, it means the query has been cancelled.
                // So don't continue;
                return;
            }
            sc::CategorisedResult opml(actions);
            opml["mode"] = sc::Variant(QueryMode::IMPORT_OPML);
            // we must have a uri
            opml.set_uri("TRY_OPML");
            opml.set_intercept_activation();
            opml.set_title("It's a OPML file ?");
            opml["subtitle"] = sc::Variant("Try to read it");
            opml["infos"] = sc::Variant(query_string);
            if (!reply->push(opml)) {return;}
 
        }
        else { // feed found
            //header, configure/edit & add
            sc::CategorisedResult conf(actions);
            conf["mode"] = sc::Variant(QueryMode::FEED_CONFIG);
            // We must have a URI
            conf.set_uri("GOTO_Configure");
            conf.set_title("Change");
            conf["subtitle"] = sc::Variant("this feed configuration");
            conf["infos"] = sc::Variant(feed_conf_data);
            if (!reply->push(conf)) {return;}

            sc::CategorisedResult add(actions);
            add["mode"] = sc::Variant(QueryMode::FEED_CONFIG);
            // We must have a URI
            add.set_uri("DIRECT_ADD");
            add.set_intercept_activation();
            add.set_title("Add");
            add["subtitle"] = sc::Variant("this feed to home");
            add["infos"] = sc::Variant(feed_conf_data);
            if (!reply->push(add)) {return;}
        }
        for (const auto &result : results) {
            sc::CategorisedResult res(cat);
            res["mode"] = sc::Variant(QueryMode::ARTICLE_VIEW);
            if(result.readability) {
                res["readability_open"] = sc::Variant(true);
            }
            // We must have a URI
            res.set_uri("nb_"+std::to_string(result.id));

            //list
            res.set_title(result.title);
            res.set_art(result.media);
            if(result.author != "")
                res["subtitle"] = sc::Variant(result.date+gettext(" by ")+result.author);
            else
                res["subtitle"] = sc::Variant(result.date);
            if(result.description.length() < 200)
             res["summary"] = sc::Variant(result.description);
            else
                res["summary"] = QString::fromStdString(result.description).left(197).append("...").toStdString();

            res["emblem"] = sc::Variant(result.icon);


            sc::VariantBuilder builder;
            int link_count = 0;
            builder.add_tuple({
                                  {"id", sc::Variant("open")},
                                  {"label", sc::Variant("Open")},
                                  {"uri", sc::Variant(result.link)}
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
    } catch (domain_error &e) {
        // Handle exceptions being thrown by the client API
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}


