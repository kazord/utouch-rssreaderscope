#include "feedmanage.h"

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <action.h>

#include <libintl.h>
#include <QDebug>

namespace sc = unity::scopes;
namespace preview {
FeedManage::FeedManage(const sc::Result &result, const sc::ActionMetadata &metadata, config::Base::Ptr action_config) :
    sc::PreviewQueryBase(result, metadata) {
    _config = action_config;
}
}


void preview::FeedManage::cancelled() {
     qDebug() << "sfsdfdfsfsdf";
}

sc::PreviewWidget preview::FeedManage::generator_actions(const std::string &name, std::string label) {
    sc::PreviewWidget action("actions_"+name, "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
                          {"id", sc::Variant("")},
                          {"label", sc::Variant(label)}
                      });
    std::string parent = feed_conf_data_.get_dict().at("item").get_string();
    for(const sc::Variant v : feed_conf_data_.get_dict().at("nodes").get_array()) {
        if(name == "item" || v.get_string().find(parent+":") == 0) {
            builder.add_tuple({
                                  {"id", sc::Variant("update_"+v.get_string())},
                                  {"label", v}
                              });
        }
    }
    builder.add_tuple({
                          {"id", sc::Variant("update_")},
                          {"label", sc::Variant("-None-")}
                      });
    action.add_attribute_value("actions", builder.end());
    return action;
}

void preview::FeedManage::run(sc::PreviewReplyProxy const& reply) {
    sc::Result result = PreviewQueryBase::result();
    autodetect::Infos autoinfos;
    Action action(_config);
    action.autopreview(result["url"].get_string(), autoinfos);
    _curr_info = action.feedInfo(result["id"].get_int());
    feed_conf_data_ = autodetect::toVariant(autoinfos);
    //detail section
    //sc::PreviewWidget detail("detail_w", "table");
    //detail.add_attribute_value("title", sc::Variant(gettext("")));
    //detail.add_attribute_mapping("values", "tabbed_info");


    //actions.add_attribute_value("actions", builder.end());

    //reply->push( {  header, detail, actions } );
    sc::PreviewWidget toptext("top_explain_text", "text");
    toptext.add_attribute_value("text", sc::Variant(gettext("This section let you improve the feed result by modifying node names or node content formats to the correct value (autodetection isn't perfect). Orange background button, on the right, are current value")));
    sc::PreviewWidget colortext("color_explain_text", "text");
    colortext.add_attribute_value("text", sc::Variant(std::string("<b>")+gettext("Color")+" :</b> <font color='"+_curr_info.color.toStdString()+"'>"+gettext("current")+"</font> ; "+gettext("Apply to title, use color name or #xxxxxx")));
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
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "url");
    header.add_attribute_mapping("mascot", "art");

    sc::PreviewWidget save_action("conf_actions", "actions");
    sc::VariantBuilder builder;
    builder.add_tuple({
                          {"id", sc::Variant("feed_remove_"+std::to_string(result["id"].get_int()))},
                          {"label", sc::Variant(gettext("Remove"))}
                      });
    builder.add_tuple({
                          {"id", sc::Variant("feed_autoreconfig_"+std::to_string(result["id"].get_int()))},
                          {"label", sc::Variant(gettext("Autoreconfigure"))}
                      });
    save_action.add_attribute_value("actions", builder.end());

    sc::PreviewWidget color_action("color_update", "comment-input");
    color_action.add_attribute_value("submit-label", sc::Variant("Change color"));

    sc::PreviewWidget item_action = generator_actions("item", _curr_info.item.toStdString());
    sc::PreviewWidget link_action = generator_actions("link", _curr_info.link.toStdString());
    sc::PreviewWidget author_action = generator_actions("author", _curr_info.author.toStdString());
    sc::PreviewWidget category_action = generator_actions("category", _curr_info.category.toStdString());
    sc::PreviewWidget title_action = generator_actions("title", _curr_info.title.toStdString());
    sc::PreviewWidget desc_action = generator_actions("desc", _curr_info.desc.toStdString());
    sc::PreviewWidget date_action = generator_actions("date", _curr_info.date.toStdString());
    sc::PreviewWidget multimedia_action = generator_actions("multimedia", _curr_info.multimedia.toStdString());

    sc::PreviewWidget date_format_action("actions_date_format", "actions");
    sc::VariantBuilder builder_date;
    //int date_format_value = feed_conf_data_.get_dict().at("date_format").get_int();
    int date_format_value = (int) _curr_info.dateFormat;
    for (int i = 0;i < 3 ; i++) {
        if(((i+date_format_value)%3) == 1) {
            builder_date.add_tuple({
                                       {"id", sc::Variant("update_1")},
                                       {"label", sc::Variant("RFC")}
                                   });
        }
        if(((i+date_format_value)%3) == 0) {
            builder_date.add_tuple({
                                       {"id", sc::Variant("update_0")},
                                       {"label", sc::Variant("ISO")}
                                   });
        }
        if(((i+date_format_value)%3) == 2) {
            builder_date.add_tuple({
                                       {"id", sc::Variant("update_2")},
                                       {"label", sc::Variant("TXT")}
                                   });
        }
    }
    date_format_action.add_attribute_value("actions", builder_date.end());




    sc::PreviewWidget desc_html_action("actions_html_decode", "actions");
    //int desc_value = feed_conf_data_.get_dict().at("desc_html").get_int();
    int desc_value = (int) _curr_info.html;
    sc::VariantBuilder builder_html;
    int html_possibilities = 3*3; //number of combinaison
    for (int i = 0;i < html_possibilities ; i++) {
        if(((i+desc_value)%html_possibilities) == 0) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("update_0")},
                                       {"label", sc::Variant("Nothing")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 1) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("update_1")},
                                       {"label", sc::Variant("Unescape")}
                                   });
        }

        if(((i+desc_value)%html_possibilities) == 2) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("update_2")},
                                       {"label", sc::Variant("Remove")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 3) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("update_3")},
                                       {"label", sc::Variant("Unescape|Remove")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 4) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("update_4")},
                                       {"label", sc::Variant("Fetch w/ mercury")}
                                   });
        }
        if(((i+desc_value)%html_possibilities) == 5) {
            builder_html.add_tuple({
                                       {"id", sc::Variant("update_5")},
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
                             "color_update",
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
