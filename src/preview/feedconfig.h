#ifndef PREVIEW_FEEDCONFIG_H
#define PREVIEW_FEEDCONFIG_H

#include <unity/scopes/PreviewQueryBase.h>

namespace unity {
namespace scopes {
class Result;
}
}

namespace preview {

    class FeedConfig : public unity::scopes::PreviewQueryBase
    {
    public:
        FeedConfig(const unity::scopes::Result &result,
             const unity::scopes::ActionMetadata &metadata);

        ~FeedConfig() = default;

        void cancelled() override;

        /**
         * Populates the reply object with preview information.
         */
        void run(unity::scopes::PreviewReplyProxy const& reply) override;
    };

}


#endif // PREVIEW_FEEDCONFIG_H
