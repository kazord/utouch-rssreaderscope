#ifndef QUERY_IMPORTOPML_H_
#define QUERY_IMPORTOPML_H_

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


class ImportOPML: public unity::scopes::SearchQueryBase {
public:
    ImportOPML(const unity::scopes::CannedQuery &query,
          const unity::scopes::SearchMetadata &metadata, config::Base::Ptr webconfig);

    ~ImportOPML() = default;

    void cancelled() override;

    void run(const unity::scopes::SearchReplyProxy &reply) override;

private:
    Action action_;
    bool s_spaced_template;
};

}

#endif // QUERY_IMPORTOPML_H_


