#include "article.h"

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

#include <libintl.h>

#include <QList>
#include <QString>
#include <QFileInfo>
#include <QDebug>
#include "action.h"

namespace sc = unity::scopes;
namespace preview {
Article::Article(const sc::Result &result, const sc::ActionMetadata &metadata, config::Base::Ptr action_config) :
    sc::PreviewQueryBase(result, metadata) {
        _config = action_config;
}
}


void preview::Article::cancelled() {
}

void preview::Article::run(sc::PreviewReplyProxy const& reply) {
    sc::Result result = PreviewQueryBase::result();
    if(_config != 0) {
         Action action(_config);
         result = action.contentreading(result);
         qDebug() << QString::fromStdString(result["description"].get_string());
    }


    sc::ColumnLayout layout1col(1);

    sc::PreviewWidget header("header_widget", "header");
    // It has a "title" and a "subtitle" property
    header.add_attribute_mapping("title", "title");
    header.add_attribute_mapping("subtitle", "subtitle");
    // déplacé vers image header.add_attribute_mapping("mascot", "art");

    sc::PreviewWidget action("action_widget", "actions");

    action.add_attribute_value("actions", result["actions"]);

    // Define the summary section
    sc::PreviewWidget summary("summary_widget", "text");
    // It has a "text" property, mapped to the result's "description" property
    summary.add_attribute_value("text", result["description"]);
    if(result["art"].get_string() == result["emblem"].get_string() && result["media"].get_string() == result["art"].get_string()) { //no art to display
        // Single column layout
        layout1col.add_column( {  "header_widget", "action_widget", "summary_widget" } );
        // Register the layouts we just created
        reply->register_layout( { layout1col } );//, layout2col, layout3col

        // Push each of the sections
        reply->push( { header, action, summary } );
    }
    else {
        //multimedia file test
        QList<QString> music_ext_list;
        music_ext_list<<"MP3"<<"OGG"<<"WAV";
        QList<QString> video_ext_list;
        video_ext_list<<"MP4"<<"AVI";
        QFileInfo fi(QString::fromStdString(result["media"].get_string()).toUpper());
        QString ext = fi.suffix();
        if (video_ext_list.contains(ext)){
            qDebug() << QString::fromStdString(result["media"].get_string());
            sc::PreviewWidget video("video_widget", "video");
            // It has a single "source" property, mapped to the result's "art" property
            video.add_attribute_mapping("source", "media");
            video.add_attribute_mapping("screenshot", "art");

            // Single column layout
            layout1col.add_column( {   "header_widget", "action_widget", "summary_widget", "video_widget" } );
            // Register the layouts we just created
            reply->register_layout( { layout1col } );//, layout2col, layout3col

            // Push each of the sections
            reply->push( { header, action, summary, video } );
        }
        else if(music_ext_list.contains(ext)) {
            sc::PreviewWidget music("music_widget", "audio");
            sc::VariantBuilder builder;
            builder.add_tuple({
                {"title", result["title"]},
                {"source", result["media"]}
            });
            // It has a single "source" property, mapped to the result's "art" property
            music.add_attribute_value("tracks", builder.end());

            // Single column layout
            layout1col.add_column( {   "header_widget", "action_widget", "summary_widget", "music_widget" } );
            // Register the layouts we just created
            reply->register_layout( { layout1col } );//, layout2col, layout3col

            // Push each of the sections
            reply->push( { header, action, summary, music } );
        }
        else {
            sc::PreviewWidget image("image_widget", "image");
            // It has a single "source" property, mapped to the result's "art" property
            image.add_attribute_mapping("source", "art");

            // Single column layout
            layout1col.add_column( {  "image_widget", "header_widget", "action_widget", "summary_widget" } );
            // Register the layouts we just created
            reply->register_layout( { layout1col } );//, layout2col, layout3col

            // Push each of the sections
            reply->push( { image, header, action, summary } );
        }
    }
}
