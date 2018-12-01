#ifndef QUERY_FEEDLY_H_
#define QUERY_FEEDLY_H_

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


class Feedly: public unity::scopes::SearchQueryBase {
public:
    Feedly(const unity::scopes::CannedQuery &query,
          const unity::scopes::SearchMetadata &metadata, config::Base::Ptr webconfig);

    ~Feedly() = default;

    void cancelled() override;

    void run(const unity::scopes::SearchReplyProxy &reply) override;

private:
    Action action_;
    bool s_feedly;
    bool s_spaced_template;
};

}

#endif // QUERY_FEEDLY_H_


