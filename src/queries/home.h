#ifndef QUERY_HOME_H_
#define QUERY_HOME_H_

#include <client/web.h>
#include <action.h>

#include <unity/scopes/SearchQueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>

/**
 * Represents an individual query.
 *
 * A new Query object will be constructed for each query. It is
 * given query information, metadata about the search, and
 * some scope-specific configuration.
 */
namespace queries {


class Home: public unity::scopes::SearchQueryBase {
public:
    Home(const unity::scopes::CannedQuery &query,
          const unity::scopes::SearchMetadata &metadata, config::Base::Ptr webconfig);

    ~Home() = default;

    void cancelled() override;

    void run(const unity::scopes::SearchReplyProxy &reply) override;

private:
    config::Base::Ptr config_;
    Action action_;
    bool aggregated_;
};

}

#endif // QUERY_FEEDLY_H_


