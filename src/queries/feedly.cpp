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
#include "feedly.h"

namespace sc = unity::scopes;

using namespace std;

/**
 * Define the layout for the results.
 *
 * The icon size is medium, and ask for the card layout
 * itself to be horizontal. I.e. the text will be placed
 * next to the image.
 */
const static string CATEGORY_TEMPLATE =
        R"(
{
        "schema-version": 1,
        "template": {
        "category-layout": "grid",
        "card-layout": "horizontal",
        "card-size": "medium"
        },
        "components": {
        "title": "title",
        "art" : {
        "field": "art"
        },
        "subtitle": "subtitle"
        }
        }
        )";

queries::Feedly::Feedly(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             config::Base::Ptr webconfig) :
    sc::SearchQueryBase(query, metadata), action_(webconfig) {
}

void queries::Feedly::cancelled() {
    action_.cancel();
}

void queries::Feedly::run(sc::SearchReplyProxy const& reply) {
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
        feed::SearchList results;
        if (query_string.empty()) {
            // If the string is empty, pick a default
            return;
        } else {
            // otherwise, use the search string
            results = action_.search(query_string);
        }

        // Register a category
        auto cat = reply->register_category("feedly_feedfound",
                                            _("1 feed found", "%d feeds found", results.size()), "",
                                            sc::CategoryRenderer(CATEGORY_TEMPLATE));

        for (const auto &result : results) {
            sc::CategorisedResult res(cat);
            res["mode"] = sc::Variant(QueryMode::FEED_SEARCH);
            // We must have a URI
            res.set_uri(result.uri);

            //list
            res.set_title(result.name);
            res.set_art(result.art);
            res["subtitle"] = result.url;

            // Other attr

            res["desc"] = result.desc;
            res["name"] = result.name;
            res["url"] = result.url;
            res["origin_url"] = result.origin_url;
            res["subscribers"] = result.subscribers;

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


