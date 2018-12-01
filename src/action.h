#ifndef ACTION_H
#define ACTION_H

#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include "client/web.h"

#include "types.h"
#include <unity/scopes/Result.h>

class Action
{
public:
    Action(config::Base::Ptr webconf);
    virtual QString readability(const std::string &url);
    virtual QString mercury(const std::string &url);
    virtual feed::SearchList search(const std::string &query);
    virtual feed::SearchList importOPML(const std::string &url);
    virtual void importFeeds(const std::string &url_list);
    virtual articles::ArticleList autopreview(const std::string &url, autodetect::Infos &infos);
    virtual articles::ArticleList view(const std::string &url, const feed::Infos &infos);
    virtual articles::ArticleList fetch();
    virtual articles::ArticleList fetchFeed(const int id);
    virtual feed::Infos feedInfo(const int id);
    virtual unity::scopes::Result contentreading(unity::scopes::Result& res);
    virtual void cancel();

protected:

private:
    client::Web _web;
};

#endif // ACTION_H
