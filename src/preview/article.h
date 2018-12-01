#ifndef PREVIEW_ARTICLE_H
#define PREVIEW_ARTICLE_H

#include <unity/scopes/PreviewQueryBase.h>
#include "types.h"

namespace unity {
namespace scopes {
class Result;
}
}

namespace preview {

    class Article : public unity::scopes::PreviewQueryBase
    {
    public:
        Article(const unity::scopes::Result &result,
             const unity::scopes::ActionMetadata &metadata, config::Base::Ptr action_config = 0);

        ~Article() = default;

        void cancelled() override;

        /**
         * Populates the reply object with preview information.
         */
        void run(unity::scopes::PreviewReplyProxy const& reply) override;
    private:
        config::Base::Ptr _config;
    };

}


#endif // PREVIEW_ARTICLE_H
