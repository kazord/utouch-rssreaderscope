#include "message.h"

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <libintl.h>

#include <QList>
#include <QString>
#include <QFileInfo>


namespace sc = unity::scopes;
namespace preview {
Message::Message(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
}
Message::Message(const unity::scopes::Result &result,
     const unity::scopes::ActionMetadata &metadata,
     const std::string &message) :
    sc::PreviewQueryBase(result, metadata), _message(message) {
}
}

void preview::Message::cancelled() {
}

void preview::Message::run(sc::PreviewReplyProxy const& reply) {
    sc::Result result = PreviewQueryBase::result();

    sc::ColumnLayout layout1col(1);

    sc::PreviewWidget header("message", "header");
    // It has a "title" and a "subtitle" property
    header.add_attribute_value("title", sc::Variant(_message));

    sc::PreviewWidget action_w("action_w", "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
                          {"id", sc::Variant("message_ok")},
                          {"label", sc::Variant(gettext("Ok"))}
                      });
    action_w.add_attribute_value("actions", builder.end());
    if(result["uri"].get_string() != "feed_addnew")
    {
    // Single column layout
    layout1col.add_column( {   "message", "action_w" } );
    // Register the layouts we just created
    reply->register_layout( { layout1col } );//, layout2col, layout3col

    // Push each of the sections
    reply->push( { header , action_w} );
    }
    else
    {
        sc::PreviewWidget search_action("search_action", "comment-input");
        search_action.add_attribute_value("submit-label", sc::Variant("search"));

        // Single column layout
        layout1col.add_column( {   "message", "search_action", "action_w" } );
        // Register the layouts we just created
        reply->register_layout( { layout1col } );//, layout2col, layout3col

        // Push each of the sections
        reply->push( { header , search_action, action_w} );
    }
}
