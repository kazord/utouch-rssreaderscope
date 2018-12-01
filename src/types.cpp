#include "types.h"
#include <QString>
#include <unity/scopes/Variant.h>
#include <libintl.h>

 int config::articles_limit = 15;

feed::Infos autodetect::Convert2Feed(const autodetect::Infos& import) {
   feed::Infos ret;
    ret.id = -1;
    ret.html = (import.htmldecode)? feed::HTML::TagsDecode : feed::HTML::DoNothing;
    ret.dateFormat = import.dateFormat;
    if(import.names.size() > 0)
        ret.name = import.names.first();
    else
        ret.name = "Undef";
    ret.url = import.realurl;
    ret.main = import.item.parent;
    ret.item = import.item.name;
    ret.link = import.link.name;
    ret.author = import.author.name;
    ret.category = import.category.name;
    ret.title = import.title.name;
    ret.desc = import.desc.name;
    ret.date = import.date.name;
    ret.multimedia = import.multimedia.name;
    ret.color = "";
    ret.favicon = import.iconurl;
    return ret;
}

feed::Infos feed::fromVariant(const unity::scopes::Variant& variant) {
     feed::Infos ret;
     unity::scopes::VariantMap m = variant.get_dict();
     ret.id = -1;
     ret.html = (feed::HTML) m["desc_html"].get_int();
     ret.dateFormat = (feed::DateFormat) m["date_format"].get_int();
     if(m["names"].get_array().size() > 0)
         ret.name = QString::fromStdString(m["names"].get_array().at(0).get_string());
     else
         ret.name = "Undef";
     ret.url = QString::fromStdString(m["realurl"].get_string());
     ret.main = QString::fromStdString(m["main"].get_string());
     ret.item = QString::fromStdString(m["item"].get_string());
     ret.link = QString::fromStdString(m["link"].get_string());
     ret.author = QString::fromStdString(m["author"].get_string());
     ret.category = QString::fromStdString(m["category"].get_string());
     ret.title = QString::fromStdString(m["title"].get_string());
     ret.desc = QString::fromStdString(m["desc"].get_string());
     ret.date = QString::fromStdString(m["date"].get_string());
     ret.multimedia = QString::fromStdString(m["multimedia"].get_string());
     ret.color = QString::fromStdString(m["color"].get_string());;
     ret.favicon = QString::fromStdString(m["iconurl"].get_string());;

     return ret;
}
unity::scopes::Variant autodetect::toVariant(const autodetect::Infos& infos) {
    unity::scopes::VariantMap builder;

    builder["main"] =  unity::scopes::Variant(infos.item.parent.toStdString());
    builder["item"] = unity::scopes::Variant(infos.item.name.toStdString());
    builder["link"] = unity::scopes::Variant(infos.link.name.toStdString());
    builder["author"] = unity::scopes::Variant(infos.author.name.toStdString());
    builder["category"] = unity::scopes::Variant(infos.category.name.toStdString());
    builder["title"] = unity::scopes::Variant(infos.title.name.toStdString());
    builder["desc"] = unity::scopes::Variant(infos.desc.name.toStdString());
    builder["date"] = unity::scopes::Variant(infos.date.name.toStdString());
    builder["multimedia"] = unity::scopes::Variant(infos.multimedia.name.toStdString());
    builder["color"] = unity::scopes::Variant("");
    builder["date_format"] = unity::scopes::Variant((int)infos.dateFormat);
    builder["desc_html"] = unity::scopes::Variant((infos.htmldecode)?1:0);
    unity::scopes::VariantArray nodesBuilder;
    for (const autodetect::Node &node: infos.list) {
       nodesBuilder.push_back(unity::scopes::Variant(QString("%1:%2").arg(node.parent).arg(node.name).toStdString()));
    }
    builder["nodes"] = nodesBuilder;
    unity::scopes::VariantArray namesBuilder;
    for (const QString &str: infos.names) {
        namesBuilder.push_back(unity::scopes::Variant(str.toStdString()));
    }
    builder["names"] = namesBuilder;
    builder["iconurl"] = unity::scopes::Variant(infos.iconurl.toStdString());
    builder["realurl"] = unity::scopes::Variant(infos.realurl.toStdString());
    return unity::scopes::Variant(builder);
}
unity::scopes::Variant feed::toVariant_tabbed(const feed::Infos& infos) {
    unity::scopes::VariantArray values {
        unity::scopes::Variant{unity::scopes::VariantArray{unity::scopes::Variant{gettext("Feed id")}, unity::scopes::Variant(infos.id)}},
        unity::scopes::Variant{unity::scopes::VariantArray{unity::scopes::Variant{gettext("Feed url")}, unity::scopes::Variant(infos.url.toStdString())}},
        unity::scopes::Variant{unity::scopes::VariantArray{unity::scopes::Variant{gettext("Main tag")}, unity::scopes::Variant(infos.main.toStdString())}},
        unity::scopes::Variant{unity::scopes::VariantArray{unity::scopes::Variant{gettext("Item tag")}, unity::scopes::Variant(infos.item.toStdString())}}
    };
    return unity::scopes::Variant(values);
}
