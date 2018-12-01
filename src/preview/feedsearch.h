#ifndef PREVIEW_FEED_H
#define PREVIEW_FEED_H

#include <unity/scopes/PreviewQueryBase.h>

namespace unity {
namespace scopes {
class Result;
}
}

namespace preview {

    class FeedSearch : public unity::scopes::PreviewQueryBase
    {
    public:
        FeedSearch(const unity::scopes::Result &result,
             const unity::scopes::ActionMetadata &metadata);

        ~FeedSearch() = default;

        void cancelled() override;

        /**
         * Populates the reply object with preview information.
         */
        void run(unity::scopes::PreviewReplyProxy const& reply) override;
    };

}


#endif // PREVIEW_FEED_H
