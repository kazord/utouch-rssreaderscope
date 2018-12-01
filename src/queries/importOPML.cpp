#include <localization.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>

#include <iomanip>
#include <iostream>
#include <sstream>

#include "types.h"
#include "importOPML.h"

namespace sc = unity::scopes;

using namespace std;

queries::ImportOPML::ImportOPML(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             config::Base::Ptr webconfig) :
    sc::SearchQueryBase(query, metadata), action_(webconfig) {
}

void queries::ImportOPML::cancelled() {
    action_.cancel();
}

void queries::ImportOPML::run(sc::SearchReplyProxy const& reply) {
    try {
        // Start by getting information about the query
        const sc::CannedQuery &query(sc::SearchQueryBase::query());

        // Get the query string
        string query_string = query.query_string();
	if(query_string.substr(0,5) == "opml:")
		query_string = query_string.substr(5);
	cerr << "my query string is " << query_string << endl;
        config::articles_limit = settings().at("articles_limit").get_double();

        // the Client is the helper class that provides the results
        // without mixing APIs and scopes code.
        // Add your code to retreive xml, json, or any other kind of result
        // in the client.
        feed::SearchList results;
        if (query_string.empty()) {
            // If the string is empty, pick a default
            return;
        } else {
            // otherwise, use the search string
            results = action_.importOPML(query_string);
        }

        // Register a category
        auto cat = reply->register_category("OPML_feedfound",
                                            _("1 feed found", "%d feeds found", results.size()), "",
                                            sc::CategoryRenderer(templates::FEED));
        auto actions = reply->register_category("OPML_feedfound_actions",
                                            "Preview OPML", "",
                                            sc::CategoryRenderer(templates::PREVIEW_ACTION));

sc::CategorisedResult opml(actions);
            opml["mode"] = sc::Variant(QueryMode::IMPORT_OPML);
            // we must have a uri
            opml.set_uri("IMPORT_OPML");
            opml.set_intercept_activation();
            opml.set_title("Import");
            opml["subtitle"] = sc::Variant("Import all to my feeds please");
	string import_list = "";
        for (const auto &result : results) {
		import_list += result.url+" ";
	}
            opml["infos"] = sc::Variant(import_list);
            if (!reply->push(opml)) {return;}
 
 
        for (const auto &result : results) {
            sc::CategorisedResult res(cat);
            res["mode"] = sc::Variant(QueryMode::FEED_SEARCH);
            // We must have a URI
            res.set_uri(result.uri);

            //list
            res.set_title(result.name);
            res.set_art("");
            res["subtitle"] = result.url;
            // Other attr

            res["desc"] = result.url;
            res["name"] = result.name;
            res["url"] = result.url;
            res["origin_url"] = result.origin_url;
            res["subscribers"] = 0;

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


