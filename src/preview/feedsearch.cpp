#include "feedsearch.h"

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <libintl.h>

namespace sc = unity::scopes;
namespace preview {
FeedSearch::FeedSearch(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}
}


void preview::FeedSearch::cancelled() {
}

void preview::FeedSearch::run(sc::PreviewReplyProxy const& reply) {
    sc::Result result = PreviewQueryBase::result();

    sc::ColumnLayout layout1col(1);
    layout1col.add_column( { "header_w", "detail_w", "description_w", "actions_w"});
    reply->register_layout( { layout1col });

    //header section
    sc::PreviewWidget header("header_w", "header");
    header.add_attribute_mapping("title", "name");
    header.add_attribute_mapping("subtitle", "origin_url");
    header.add_attribute_mapping("mascot", "art");

    //detail section
    sc::PreviewWidget detail("detail_w", "table");
    //detail.add_attribute_value("title", sc::Variant(gettext("")));
    sc::VariantArray values {
        sc::Variant{sc::VariantArray{sc::Variant{gettext("Subscribers")}, result["subscribers"]}},
        sc::Variant{sc::VariantArray{sc::Variant{gettext("Feed url")}, result["url"]}}
    };
    detail.add_attribute_value("values", sc::Variant(values));

    //description
    sc::PreviewWidget description("description_w", "text");
    description.add_attribute_mapping("text", "desc");

    sc::PreviewWidget actions("actions_w", "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
                          {"id", sc::Variant("feed_preview")},
                          {"label", sc::Variant(gettext("Preview"))}
                      });
    builder.add_tuple({
                          {"id", sc::Variant("feed_save")},
                          {"label", sc::Variant(gettext("Autoconfigure+Add"))}
                      });
    builder.add_tuple({
                          {"id", sc::Variant("open")},
                          {"label", sc::Variant(gettext("Browse source"))},
                          {"uri", result["origin_url"]}
                      });
    actions.add_attribute_value("actions", builder.end());

    reply->push( {  header, detail, description, actions } );
}
