#include "feedconfig.h"

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <libintl.h>
#include <iostream>

namespace sc = unity::scopes;
namespace preview {
FeedConfig::FeedConfig(const sc::Result &result, const sc::ActionMetadata &metadata) :
    sc::PreviewQueryBase(result, metadata) {
    std::cerr << "new preview" << result.uri() << std::endl;
}
}


void preview::FeedConfig::cancelled() {
}
namespace  {
sc::PreviewWidget generator_actions(const std::string &name, sc::Result &res) {
    sc::PreviewWidget action("actions_"+name, "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
                          {"id", sc::Variant("")},
                          {"label", res["infos"].get_dict().at(name)}
                      });
    std::string parent = res["infos"].get_dict().at("item").get_string();
    for(const sc::Variant v : res["infos"].get_dict().at("nodes").get_array()) {
        if(name == "item" || v.get_string().find(parent+":") == 0) {
            builder.add_tuple({
                                  {"id", sc::Variant("set_"+v.get_string())},
                                  {"label", v}
                              });
        }
    }
    builder.add_tuple({
                          {"id", sc::Variant("set_")},
                          {"label", sc::Variant("-None-")}
                      });
    action.add_attribute_value("actions", builder.end());
    return action;
}
}
void preview::FeedConfig::run(sc::PreviewReplyProxy const& reply) {
    sc::Result result = PreviewQueryBase::result();

    sc::PreviewWidget toptext("top_explain_text", "text");
    toptext.add_attribute_value("text", sc::Variant(gettext("This section let you improve the feed result by modifying node names or node content formats to the correct value (autodetection isn't perfect). Orange background button, on the right, are current value")));
    sc::PreviewWidget colortext("color_explain_text", "text");
    colortext.add_attribute_value("text", sc::Variant(std::string("<b>")+gettext("Color")+" :</b> <font color='"+result["infos"].get_dict().at("color").get_string()+"'>"+gettext("current")+"</font> ; "+gettext("Apply to title, use color name or #xxxxxx")));
    sc::PreviewWidget itemtext("item_explain_text", "text");
    itemtext.add_attribute_value("text", sc::Variant(gettext("<b>Item :</b> Container for one article inside the feed")));
    sc::PreviewWidget linktext("link_explain_text", "text");
    linktext.add_attribute_value("text", sc::Variant(gettext("<b>Link :</b> Url of the article")));
    sc::PreviewWidget authortext("author_explain_text", "text");
    authortext.add_attribute_value("text", sc::Variant(gettext("<b>Author :</b> Should contains the source/writer")));
    sc::PreviewWidget categorytext("category_explain_text", "text");
    categorytext.add_attribute_value("text", sc::Variant(gettext("<b>Category :</b> For filtering")));
    sc::PreviewWidget titletext("title_explain_text", "text");
    titletext.add_attribute_value("text", sc::Variant(gettext("<b>Title</b>")));
    sc::PreviewWidget desctext("desc_explain_text", "text");
    desctext.add_attribute_value("text", sc::Variant(gettext("<b>Description :</b> Description of the article")));
    sc::PreviewWidget desctexthtml("desc_explain_html", "text");
    desctexthtml.add_attribute_value("text", sc::Variant(gettext("<b>Description - Content :</b> How should be handle description content, if html should be unescape (ex: from &amp;lt; to &lt;) or fully download")));
    sc::PreviewWidget datetext("date_explain_text", "text");
    datetext.add_attribute_value("text", sc::Variant(gettext("<b>Date :</b> Don't forget to select the right date format, it's needed ordering")));
    sc::PreviewWidget multimediatext("multimedia_explain_text", "text");
    multimediatext.add_attribute_value("text", sc::Variant(gettext("<b>Multimedia :</b> Image or music")));

    sc::PreviewWidget header("title", "header");
    sc::VariantArray names = result["infos"].get_dict().at("names").get_array();
    header.add_attribute_value("title", (names.size() > 0) ? names.at(0) : sc::Variant("Unnamed"));
    header.add_attribute_value("subtitle", result["infos"].get_dict().at("realurl"));
    header.add_attribute_mapping("mascot", "iconurl");

    sc::PreviewWidget save_action("conf_actions", "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
                          {"id", sc::Variant("feed_save")},
                          {"label", sc::Variant(gettext("Add to home"))}
                      });
    builder.add_tuple({
                          {"id", sc::Variant("feed_preview")},
                          {"label", sc::Variant(gettext("Preview"))}
                      });
    save_action.add_attribute_value("actions", builder.end());

    sc::PreviewWidget color_action("color_action", "comment-input");
    color_action.add_attribute_value("submit-label", sc::Variant("Change color"));

    sc::PreviewWidget item_action = generator_actions("item", result);
    sc::PreviewWidget link_action = generator_actions("link", result);
    sc::PreviewWidget author_action = generator_actions("author", result);
    sc::PreviewWidget category_action = generator_actions("category", result);
    sc::PreviewWidget title_action = generator_actions("title", result);
    sc::PreviewWidget desc_action = generator_actions("desc", result);
    sc::PreviewWidget date_action = generator_actions("date", result);
    sc::PreviewWidget multimedia_action = generator_actions("multimedia", result);

    sc::PreviewWidget date_format_action("actions_date_format", "actions");
    sc::VariantBuilder builder_date;
    int date_format_value = result["infos"].get_dict().at("date_format").get_int();
    for (int i = 0;i < 3 ; i++) {
        if(((i+date_format_value)%3) == 1) {
            builder_date.add_tuple({
                                       {"id", sc::Variant("set_1")},
                                       {"label", sc::Variant("RFC")}
                                   });
        }
        if(((i+date_format_value)%3) == 0) {
            builder_date.add_tuple({
                                       {"id", sc::Variant("set_0")},
                                       {"label", sc::Variant("ISO")}
                                   });
        }
        if(((i+date_format_value)%3) == 2) {
            builder_date.add_tuple({
                                       {"id", sc::Variant("set_2")},
                                       {"label", sc::Variant("TXT")}
                                   });
        }
    }
    date_format_action.add_attribute_value("actions", builder_date.end());




    sc::PreviewWidget desc_html_action("actions_desc_html", "actions");
        int desc_value = result["infos"].get_dict().at("desc_html").get_int();
    sc::VariantBuilder builder_html;
    int html_possibilities = 3*3; //number of combinaison
    for (int i = 0;i < html_possibilities ; i++) {
        if(((i+desc_value)%html_possibilities) == 0) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("set_0")},
                                       {"label", sc::Variant("Nothing")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 1) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("set_1")},
                                       {"label", sc::Variant("Unescape")}
                                   });
        }

        if(((i+desc_value)%html_possibilities) == 2) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("set_2")},
                                       {"label", sc::Variant("Remove")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 3) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("set_3")},
                                       {"label", sc::Variant("Unescape|Remove")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 4) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("set_4")},
                                       {"label", sc::Variant("Fetch w/ mercury")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 5) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("set_5")},
                                       {"label", sc::Variant("On open fech w/ mercury")}
                                   });
        }
    }
    desc_html_action.add_attribute_value("actions", builder_html.end());

    sc::PreviewWidget expandable("exp", "expandable");
    expandable.add_attribute_value("title", sc::Variant("Advanced configuration"));
    expandable.add_attribute_value("collapsed-widgets", sc::Variant(1));
    expandable.add_widget(toptext);
    expandable.add_widget(itemtext);
    expandable.add_widget(item_action);
    expandable.add_widget(linktext);
    expandable.add_widget(link_action);

   expandable.add_widget( authortext);
   expandable.add_widget( author_action);

   expandable.add_widget( categorytext);
   expandable.add_widget( category_action);

   expandable.add_widget( titletext);
   expandable.add_widget( title_action);

   expandable.add_widget( desctext);
  expandable.add_widget(  desc_action);
   expandable.add_widget(  desctexthtml);
  expandable.add_widget(  desc_html_action);





  expandable.add_widget(  datetext);
  expandable.add_widget(  date_action);
  expandable.add_widget(  date_format_action);

  expandable.add_widget(  multimediatext);
  expandable.add_widget(  multimedia_action);
    sc::ColumnLayout layout1col(1);

/*"top_explain_text",
    "item_explain_text",
    "item_actions",

    "link_explain_text",
    "link_actions",

    "author_explain_text",
    "author_actions",

    "category_explain_text",
    "category_actions",

    "title_explain_text",
    "title_actions",

    "desc_explain_text",
    "desc_actions",
    "desc_html_action",

    "date_explain_text",
    "date_actions",
    "date_format_action",

    "multimedia_explain_text",
    "multimedia_actions",*/
    // Single column layout
    layout1col.add_column( {
                             "title",
                             "conf_actions",

                             "color_explain_text",
                             "color_action",
                                "exp"

                           } );

    // Register the layouts we just created
    reply->register_layout( { layout1col } );
    // Push each of the sections
    reply->push( {

                     header,
                     save_action,

                     colortext,
                     color_action,
                expandable,
                     toptext,
                    itemtext,
                    item_action,
                    linktext,
                    link_action,

                    authortext,
                    author_action,

                    categorytext,
                    category_action,

                    titletext,
                    title_action,

                    desctext,
                    desc_action,
                    desc_html_action,

                    datetext,
                    date_action,
                    date_format_action,

                    multimediatext,
                    multimedia_action


                 } );
}
